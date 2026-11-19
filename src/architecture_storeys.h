// Included after ArchitectureEdit. No engine placement or file writes here.
static Str architecture_storeys_prepare(int rotation,int calibratedRise) {
    if(g_projectRunning)return S_EXT_ACTION_BUSY;
    if(!g_nativePartsAvailable || !g_bounds)return S_EXT_DEMO_ABI;
    project::NativePiece selected{};selected.item=g_catalogPiece;
    uint8_t* item=project_piece_item(selected);
    uint8_t* material=g_bpItemById && g_bpItemReg ? g_bpItemById(g_catalogMaterial,g_bpItemReg) : nullptr;
    if(!item || !bp_block_item(material))return S_EXT_DEMO_SELECT;
    if(calibratedRise<8 || calibratedRise%8)return S_EXT_DEMO_RISE;
    CursorXf pose{};pose.scale=1;const float identity[]={0,0,0,1};core::bp_turn_rot(g_stairModelTurn,identity,pose.rot);
    Aabb bounds{};shape_bounds(item,&pose,&bounds);
    int minimum[3],size[3];
    for(int a=0;a<3;++a) {
        if(!extended::finite(bounds.mn[a]) || !extended::finite(bounds.mx[a]) || bounds.mx[a]<=bounds.mn[a])return S_EXT_DEMO_BOUNDS;
        minimum[a]=extended::snap(bounds.mn[a]*16,1);size[a]=extended::snap((bounds.mx[a]-bounds.mn[a])*16,1);
        const double error=bounds.mn[a]*16-minimum[a],extent=(bounds.mx[a]-bounds.mn[a])*16-size[a];
        if(error < -0.01 || error > 0.01 || extent < -0.01 || extent > 0.01)return S_EXT_DEMO_BOUNDS;
    }
    storeys::Layout layout;
    const auto issue=storeys::prepare(size[0],size[1],size[2],calibratedRise,&layout);
    if(issue==storeys::Issue::Rise)return S_EXT_DEMO_RISE;
    if(issue==storeys::Issue::Dimensions)return S_EXT_DEMO_BOUNDS;
    if(issue==storeys::Issue::Clearance)return S_EXT_DEMO_CLEARANCE;
    if(issue!=storeys::Issue::None)return S_EXT_DEMO_BUDGET;
    ArchitectureEdit edit;if(!edit.before)return S_EXT_DEMO_BUDGET;
    if(!project_generated_begin("Dev6 - trois etages et escaliers natifs"))return S_EXT_DEMO_BUDGET;
    g_projectManifest.count=0;
    g_projectPlan.materialCount=1;g_projectPlan.materials[0]={g_catalogMaterial,material[0x454],0,0};
    struct Sink {
        bool module(int index,int category,const extended::Box& bounds) {
            if(index>=project::MaxModules)return false;
            auto& m=g_projectManifest.modules[index];memset(&m,0,sizeof m);m.category=category;
            g_projectModuleBoxes[index]=bounds;g_projectManifest.count=index+1;return true;
        }
        bool cell(int x,int y,int z,int module,int category) {
            if(g_projectPlan.count>=g_projectPlan.capacity)return false;
            g_projectPlan.cells[g_projectPlan.count++]={{x,y,z},(uint16_t)module,0,(uint8_t)category};return true;
        }
    } sink;
    if(!storeys::emit(layout,sink))return S_EXT_DEMO_BUDGET;
    project::Error error;
    if(!project::finish(&g_projectPlan,&error))return S_EXT_DEMO_BUDGET;
    for(int i=0;i<layout.count;++i) {
        auto& p=g_projectManifest.pieces[g_projectManifest.pieceCount++];p={};p.item=g_catalogPiece;p.material=g_catalogMaterial;p.clearance=32;p.rotation=g_stairModelTurn&3;
        for(int a=0;a<3;++a)p.offset[a]=layout.bodies[i].lo[a]-minimum[a];
    }
    if(!project_piece_bounds())return S_EXT_DEMO_BOUNDS;
    for(int i=0;i<g_projectManifest.pieceCount;++i) {
        auto clear=g_projectPieceBoxes[i];clear.hi[1]+=32;
        if(extended::plan_intersects(g_projectPlan,clear))return S_EXT_DEMO_CLEARANCE;
        for(int j=0;j<i;++j)if(extended::intersects(g_projectPieceBoxes[i],g_projectPieceBoxes[j],32)
            || extended::intersects(g_projectPieceBoxes[j],g_projectPieceBoxes[i],32))return S_EXT_DEMO_CLEARANCE;
    }
    g_bpRot=rotation&3;project_generated_finish();edit.committed=true;
    LOG(.str("storeys demo PREPARED modules=").i(g_projectManifest.count).str(" cells=").i(g_projectPlan.count).str(" native_pieces=").i(layout.count)
        .str(" item=").x(g_catalogPiece).str(" material=").x(g_catalogMaterial).str(" model_turn=").i(g_stairModelTurn).str(" global_turn=").i(g_bpRot)
        .str(" rise16=").i(calibratedRise).str(" level_height16=").i(layout.levelHeight).str(" walkability=requires_game_test"));
    return S_COUNT;
}
static bool architecture_storeys(int rotation,int calibratedRise) {
    ProjectStateGuard guard;
    const Str result=architecture_storeys_prepare(rotation,calibratedRise);
    if(result==S_COUNT){flash(T(S_EXT_DEMO_READY));return true;}
    // The transaction has restored the old draft before publishing this reason.
    g_projectLastProblem=T(result);flash(T(result));
    LOG(.str("storeys demo REFUSED item=").x(g_catalogPiece).str(" material=").x(g_catalogMaterial).str(" rise16=").i(calibratedRise).str(" reason=").str(T(result)));
    return false;
}
