static Str voxel_issue(voxel::Issue issue){
    switch(issue){case voxel::Issue::Rise:return S_EXT_VOXEL_RISE;case voxel::Issue::Landing:return S_EXT_VOXEL_BAD_LANDING;
    case voxel::Issue::Budget:return S_EXT_DEMO_BUDGET;case voxel::Issue::Obstructed:return S_EXT_VOXEL_OBSTRUCTED;default:return S_EXT_VOXEL_DIM;}
}
static Str architecture_voxels_prepare(const voxel::Spec& spec,const int* offset,int rotation,bool append,bool demo){
    if(g_projectRunning)return S_EXT_ACTION_BUSY;
    if(!offset || rotation<0 || rotation>3)return S_EXT_VOXEL_DIM;
    for(int a=0;a<3;++a)if(offset[a]%8 || offset[a]<-project::MaxCoord || offset[a]>project::MaxCoord-8)return S_EXT_VOXEL_DIM;
    uint8_t* material=g_bpItemById && g_bpItemReg ? g_bpItemById(g_catalogMaterial,g_bpItemReg):nullptr;
    if(!bp_block_item(material))return S_EXT_VOXEL_MATERIAL;
    voxel::Layout stairs;voxel::Demo building;auto issue=demo?voxel::prepare_demo(spec,&building):voxel::prepare(spec,&stairs);
    if(issue!=voxel::Issue::None)return voxel_issue(issue);
    if(demo)stairs=building.stairs;
    ArchitectureEdit edit;if(!edit.before)return S_EXT_DEMO_BUDGET;
    const bool add=append && g_projectLoaded && !demo;
    if(!add){if(!project_generated_begin(demo?"Dev7 - trois niveaux et escaliers voxels":"Voxel stairs"))return S_EXT_DEMO_BUDGET;g_projectManifest.count=0;}
    int mat=0;while(mat<g_projectPlan.materialCount && g_projectPlan.materials[mat].item!=g_catalogMaterial)++mat;
    if(mat==core::BP_MAX_MATS)return S_EXT_DEMO_BUDGET;
    if(mat==g_projectPlan.materialCount)g_projectPlan.materials[g_projectPlan.materialCount++]={g_catalogMaterial,material[0x454],0,0};
    const int firstModule=g_projectManifest.count;
    struct Sink {int base,material,rotation;const int* offset;
        bool module(int index,int kind,const extended::Box& box){
            if(base+index>=project::MaxModules)return false;auto b=extended::turn_box(box,rotation);
            auto& m=g_projectManifest.modules[base+index];memset(&m,0,sizeof m);m.category=kind;
            for(int a=0;a<3;++a){b.lo[a]+=offset[a];b.hi[a]+=offset[a];}g_projectModuleBoxes[base+index]=b;g_projectManifest.count=base+index+1;g_projectModuleVisible[base+index]=true;return true;
        }
        bool cell(int x,int y,int z,int index,int kind){
            if(g_projectPlan.count>=g_projectPlan.capacity)return false;project::CellEntry c{{x,y,z},(uint16_t)(base+index),(uint8_t)material,(uint8_t)kind};
            int zero[]={0,0,0},p[3];project::cell_low(c,zero,rotation,p);
            for(int a=0;a<3;++a){c.p[a]=p[a]+offset[a];if(c.p[a]<-project::MaxCoord || c.p[a]>project::MaxCoord-8)return false;}
            g_projectPlan.cells[g_projectPlan.count++]=c;return true;
        }
    } sink{firstModule,mat,rotation,offset};
    if(demo){if(!voxel::emit_demo(building,sink))return S_EXT_DEMO_BUDGET;}
    else {if(!sink.module(0,extended::Stairs,stairs.bounds) || !voxel::emit(stairs,[&](int x,int y,int z){return sink.cell(x,y,z,0,extended::Stairs);}))return S_EXT_DEMO_BUDGET;}
    for(int level=0;level<(demo?2:1);++level)for(int i=0;i<stairs.count;++i){
        if(g_projectManifest.spaceCount==project::MaxSpaces)return S_EXT_DEMO_BUDGET;
        auto air=voxel::air(stairs.surfaces[i],stairs.spec.headroom);extended::Box b=spaces::box(air);
        if(demo){b.lo[1]+=level*stairs.spec.height;b.hi[1]+=level*stairs.spec.height;const int dz=level?building.upperZ:stairs.spec.landing;b.lo[2]+=dz;b.hi[2]+=dz;}
        b=extended::turn_box(b,rotation);auto& s=g_projectManifest.spaces[g_projectManifest.spaceCount++];
        for(int a=0;a<3;++a){s.lo[a]=b.lo[a]+offset[a];s.hi[a]=b.hi[a]+offset[a];}
    }
    project::Error error;if(!project::finish(&g_projectPlan,&error))return S_EXT_VOXEL_OBSTRUCTED;
    if(!project_piece_bounds() || !project_validate_spaces())return S_EXT_VOXEL_OBSTRUCTED;
    project_generated_finish();if(!add)g_bpRot=0;edit.committed=true;return S_COUNT;
}
static bool architecture_voxels(const voxel::Spec& spec,const int* offset,int rotation,bool append,bool demo=false){
    ProjectStateGuard guard;const Str result=architecture_voxels_prepare(spec,offset,rotation,append,demo);
    LOG(.str("voxel stairs ").str(result==S_COUNT?"PREPARED":"REFUSED").str(" kind=").i(demo?voxel::Straight:spec.kind).str(" height16=").i(spec.height).str(" steps=").i(spec.steps)
        .str(" width16=").i(spec.width).str(" tread16=").i(spec.tread).str(" rotation=").i(rotation).str(" demo=").i(demo).str(" reason=").str(result==S_COUNT?"none":T(result))
        .str(" support=").i(spec.support).str(" landing16=").i(spec.landing).str(" headroom16=").i(spec.headroom)
        .str(" cells=").i(g_projectPlan.count).str(" native_pieces=").i(g_projectManifest.pieceCount).str(" spaces=").i(g_projectManifest.spaceCount).str(" walkability=requires_game_test"));
    if(result!=S_COUNT){g_projectLastProblem=T(result);flash(T(result));return false;}
    flash(T(S_EXT_VOXEL_READY));return true;
}
