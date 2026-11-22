// Native prop validation around the EXISTING blueprint placement mechanism.
static int prop_floor(double v){int n=(int)v;return n>v?n-1:n;}
static int prop_ceil(double v){int n=(int)v;return n<v?n+1:n;}
static props::Bounds g_projectPropBoxes[props::MaxObjects];
static props::Bounds g_projectPropWorld[props::MaxObjects];
static void project_trace_object(int,const char*,const char*);
static const char* project_prop_code(Str reason){
    switch(reason){
        case S_EXT_PROP_LIMIT:return "E_PROP_COUNT_LIMIT";
        case S_EXT_OBJECT_UNSAFE:return "E_NATIVE_ITEM_UNSAFE";
        case S_EXT_PROP_INVALID:return "E_PROP_BOUNDS_OR_TRANSFORM";
        case S_EXT_PIECE_OVERLAP:return "E_STRUCTURE_OVERLAP";
        case S_EXT_PROP_OVERLAP:return "E_PROP_OVERLAP";
        case S_EXT_ALREADY_PRESENT:return "E_EXISTING_PROP_DUPLICATE_RISK";
        case S_EXT_PROP_SUPPORT:return "E_PROP_SUPPORT";
        case S_EXT_OBJECT_BUDGET:return "E_PROP_QUERY_BUDGET";
        case S_EXT_CLASSIC_EDITED:return "E_CAPTURED_COMPOSITION_EDITED";
        case S_EXT_BP_OBJECTS_OFF:return "E_BLUEPRINT_OBJECTS_DISABLED";
        default:return "E_PROP_REFUSED";
    }
}
static bool project_prop_scan_problem(){
    Buf reason;reason.fmt(T(S_EXT_SCAN_EXACT),nativescan::code(g_bpSnapshotTrace.error));
    copy_text(g_projectPropsError,reason.z(),sizeof g_projectPropsError);g_projectLastProblem=g_projectPropsError;
    LOG(.str("project object REFUSED scope=world_scan index=-1 code=").str(nativescan::code(g_bpSnapshotTrace.error)).str(" world_entity=").x(g_bpSnapshotTrace.entity).str(" world_item=").x(g_bpSnapshotTrace.item));
    if(g_projectRunning)project_stop(g_projectPropsError);return false;
}
static bool project_prop_problem(Str reason,int index,uint32_t item){
    Buf b,id;id.x(item);b.str(T(reason)).c(' ').fmt(T(S_EXT_PROP_DETAIL),num(index).s,id.z());
    copy_text(g_projectPropsError,b.z(),sizeof g_projectPropsError);g_projectLastProblem=g_projectPropsError;
    LOG(.str("project object REFUSED scope=planned_object index=").i(index).str(" item=").x(item).str(" code=").str(project_prop_code(reason)).str(" reason=").str(T(reason)));
    if(index>=0)project_trace_object(index,"refused",project_prop_code(reason));
    if(g_projectRunning)project_stop(g_projectPropsError);return false;
}
static bool project_prepare_objects(){
    g_projectPropsError[0]=0;g_projectPropCounts={};bp_snapshot_cancel("new_operation");g_bpSnapshotTrace={};
    if(!g_projectUseObjects||!g_projectObjectCount)return true;
    if(!g_cfg.bpObjects)return project_prop_problem(S_EXT_BP_OBJECTS_OFF,-1,0);
    if(g_projectObjectCount<0||g_projectObjectCount>props::MaxObjects)return project_prop_problem(S_EXT_PROP_LIMIT,-1,0);
    if(project_classic_mode()&&composition::fingerprint(g_projectObjects,g_projectObjectCount)!=g_projectClassicObjects)return project_prop_problem(S_EXT_CLASSIC_EDITED,-1,0);
    for(int i=0;i<g_projectObjectCount;++i){const auto& o=g_projectObjects[i];
        uint8_t* it=g_bpItemById&&g_bpItemReg?g_bpItemById(o.item,g_bpItemReg):nullptr;float low[3],high[3];
        const auto resolution=project_object_resolution(o.item);project_trace_object(i,"resolve",nativecatalog::name(resolution));
        if(!nativecatalog::admitted(resolution))return project_prop_problem(S_EXT_OBJECT_UNSAFE,i,o.item);
        if(!bp_item_bounds(it,low,high)||!props::bounds(o.p,o.rotation,low,high,&g_projectPropBoxes[i]))return project_prop_problem(S_EXT_PROP_INVALID,i,o.item);
        const auto& b=g_projectPropBoxes[i];extended::Box box{};
        for(int a=0;a<3;++a){box.lo[a]=prop_floor((b.lo[a]+.001)*16);box.hi[a]=prop_ceil((b.hi[a]-.001)*16);}
        if(extended::plan_intersects(g_projectPlan,box))return project_prop_problem(S_EXT_PIECE_OVERLAP,i,o.item);
        for(int p=0;p<g_projectManifest.pieceCount;++p){props::Bounds piece;
            for(int a=0;a<3;++a){piece.lo[a]=g_projectPieceBoxes[p].lo[a]/16.;piece.hi[a]=g_projectPieceBoxes[p].hi[a]/16.;}
            if(props::intersects(b,piece))return project_prop_problem(S_EXT_PIECE_OVERLAP,i,o.item);}
    }
    static int order[props::MaxObjects];int a=-1,b=-1;
    if(project_classic_mode()){
        for(int i=0;i<g_projectObjectCount;++i){order[i]=i;
            for(int j=0;j<i;++j)if(g_projectObjects[i].module!=g_projectObjects[j].module&&props::intersects(g_projectPropBoxes[i],g_projectPropBoxes[j]))return project_prop_problem(S_EXT_PROP_OVERLAP,i,g_projectObjects[i].item);}
        LOG(.str("classic composition PREPARE order=source_BTBP internal_contacts=preserved external_collisions=checked captured_support_cells=").i(g_projectCapturedSupport.count));
    }else if(!props::order(g_projectPropBoxes,g_projectObjectCount,order,&a,&b))return project_prop_problem(S_EXT_PROP_OVERLAP,b,b>=0?g_projectObjects[b].item:0);
    static extended::Object sorted[props::MaxObjects];static props::Bounds boxes[props::MaxObjects];
    for(int i=0;i<g_projectObjectCount;++i){sorted[i]=g_projectObjects[order[i]];boxes[i]=g_projectPropBoxes[order[i]];}
    memcpy(g_projectObjects,sorted,g_projectObjectCount*sizeof(*sorted));memcpy(g_projectPropBoxes,boxes,g_projectObjectCount*sizeof(*boxes));
    for(int i=0;i<g_projectObjectCount;++i){const auto& o=g_projectObjects[i];float low[3],high[3],rotation[4];double position[3];
        uint8_t* it=g_bpItemById(o.item,g_bpItemReg);
        if(!bp_item_bounds(it,low,high))return project_prop_problem(S_EXT_PROP_INVALID,i,o.item);
        extended::object_world(o,g_projectManifest.anchor,g_projectRotation,g_projectOrigin,position,rotation);
        if(!props::bounds(position,rotation,low,high,&g_projectPropWorld[i]))return project_prop_problem(S_EXT_PROP_INVALID,i,o.item);
    }return true;
}
static bool project_prop_scan_ready(){
    if(g_bpSavePending){bp_snapshot_fail(nativescan::SavePending);return project_prop_scan_problem();}
    if(g_bpSnapshotState==0){
        auto region=g_projectPropWorld[0];for(int i=1;i<g_projectObjectsInJob;++i)for(int a=0;a<3;++a){
            if(g_projectPropWorld[i].lo[a]<region.lo[a])region.lo[a]=g_projectPropWorld[i].lo[a];
            if(g_projectPropWorld[i].hi[a]>region.hi[a])region.hi[a]=g_projectPropWorld[i].hi[a];}
        for(int a=0;a<3;++a){region.lo[a]-=.5;region.hi[a]+=.5;}
        if(!(project_classic_mode()?bp_classic_snapshot_request(region):bp_snapshot_request(region)))return project_prop_scan_problem();
        flash(T(S_EXT_PROP_SCANNING));return false;
    }
    if(g_bpSnapshotState==-1)return project_prop_scan_problem();
    if(g_bpSnapshotState==1||g_bpSnapshotState==2){
        if(GetTickCount()-g_bpSnapshotTick>30000){bp_snapshot_fail(nativescan::Timeout);return project_prop_scan_problem();}return false;}
    if(g_bpSnapshotState==3&&GetTickCount()-g_bpSnapshotCompleted>nativescan::SnapshotMaxAgeMs){
        bp_snapshot_fail(nativescan::SnapshotStale);return project_prop_scan_problem();}
    return g_bpSnapshotState==3;
}
static bool project_prop_planned_support(const double* world){
    const double x=world[0]-g_projectOrigin.x/core::kFix,z=world[2]-g_projectOrigin.z/core::kFix;
    double local[3]={x,world[1]-g_projectOrigin.y/core::kFix,z};
    switch(g_projectRotation&3){case 1:local[0]=z;local[2]=-x;break;case 2:local[0]=-x;local[2]=-z;break;case 3:local[0]=-z;local[2]=x;break;}
    int point[3];for(int a=0;a<3;++a)point[a]=prop_floor((local[a]+g_projectManifest.anchor[a]/16.)*2)*8;
    return spaces::surface(g_projectPlan,point[0],point[1],point[2]);
}
static bool project_prop_captured_support(int index,const double* world){
    if(!project_classic_mode())return false;
    const double x=world[0]-g_projectOrigin.x/core::kFix,z=world[2]-g_projectOrigin.z/core::kFix;
    double local[3]={x,world[1]-g_projectOrigin.y/core::kFix,z};
    switch(g_projectRotation&3){case 1:local[0]=z;local[2]=-x;break;case 2:local[0]=-x;local[2]=-z;break;case 3:local[0]=-z;local[2]=x;break;}
    int point[3];for(int a=0;a<3;++a)point[a]=prop_floor((local[a]+g_projectManifest.anchor[a]/16.)*2)*8;
    return composition::support_cell(g_projectCapturedSupport,g_projectObjects[index].module,point);
}
static bool project_prop_solid(const double* point,void* ctx,void* shape,const CursorXf& target,const int64_t* offset){
    if(project_prop_planned_support(point))return true;
    CursorXf probe=target;for(int a=0;a<3;++a)probe.pos[a]=(int64_t)((prop_floor(point[a]*2)*.5+.25)*core::kFix)+offset[a];
    static CostEntry entries[256];CostList cost{entries,0,256,nullptr};memset(g_remVols,0,sizeof g_remVols);
    price_removal(&cost,ctx,shape,&probe);g_refundN=0;g_refundList=nullptr;
    for(int a=1;a<256;++a)if(g_remVols[a]>0)return true;return false;
}
static bool project_check_props(void* ctx,void* shape,const CursorXf& target,const int64_t* offset,bool costs){
    if(creative_on()&&!g_creativeProps){project_trace_object(0,"refused","E_CREATIVE_PROPS_DISABLED");project_stop(T(S_EXT_OBJECT_CREATIVE));return false;}
    const int firstObject=g_projectJob.next-g_projectPlan.count-g_projectManifest.pieceCount;
    if(g_bpSnapshotState!=3)return project_prop_scan_problem();
    if(GetTickCount()-g_bpSnapshotCompleted>nativescan::SnapshotMaxAgeMs){bp_snapshot_fail(nativescan::SnapshotStale);return project_prop_scan_problem();}
    for(int n=0;n<g_projectJob.batch;++n){const int index=firstObject+n;const auto& o=g_projectObjects[index];const auto& b=g_projectPropWorld[index];
        if(!nativecatalog::admitted(project_object_resolution(o.item)))return project_prop_problem(S_EXT_OBJECT_UNSAFE,index,o.item);
        for(int w=0;w<g_bpSnapshotCount;++w)if(props::intersects(b,g_bpSnapshot[w].box)){
            LOG(.str("native collision CHECK method=native_placement_aabb result=COLLISION code=E_PROP_OVERLAP index=").i(index).str(" item=").x(o.item)
                .str(" existing_entity=").x(g_bpSnapshot[w].entity).str(" existing_item=").x(g_bpSnapshot[w].item));
            return project_prop_problem(g_bpSnapshot[w].item==o.item?S_EXT_ALREADY_PRESENT:S_EXT_PROP_OVERLAP,index,o.item);}
        bool support=(project_classic_mode()&&composition::captured_parent(g_projectPropBoxes,g_projectObjects,g_projectObjectsInJob,index))||props::supported(b,g_projectPropBoxes,g_projectObjectsInJob,index,g_bpSnapshot,g_bpSnapshotCount,
            [&](const double* point){return project_prop_solid(point,ctx,shape,target,offset);});
        if(!support)return project_prop_problem(S_EXT_PROP_SUPPORT,index,o.item);
        int low[3],high[3];int64_t count=1;for(int a=0;a<3;++a){low[a]=prop_floor((b.lo[a]+.001)*2);high[a]=prop_ceil((b.hi[a]-.001)*2);count*=high[a]-low[a];}
        if(count<1||count>512)return project_prop_problem(S_EXT_OBJECT_BUDGET,index,o.item);
        for(int y=low[1];y<high[1];++y)for(int z=low[2];z<high[2];++z)for(int x=low[0];x<high[0];++x){
            I3 centre{(int64_t)(x*8+4)*kProjectFixUnit,(int64_t)(y*8+4)*kProjectFixUnit,(int64_t)(z*8+4)*kProjectFixUnit};
            if(!project_covered(centre)){project_trace_object(index,"refused","E_ALTAR_COVERAGE");return project_spatial_stop(S_PROJECT_COVERAGE,centre,-1,o.module);}
            CursorXf probe=target;probe.pos[0]=centre.x+offset[0];probe.pos[1]=centre.y+offset[1];probe.pos[2]=centre.z+offset[2];
            static CostEntry entries[256];CostList cost{entries,0,256,nullptr};memset(g_remVols,0,sizeof g_remVols);price_removal(&cost,ctx,shape,&probe);g_refundN=0;g_refundList=nullptr;
            for(int m=1;m<256;++m)if(g_remVols[m]>0){const double point[]={centre.x/core::kFix,centre.y/core::kFix,centre.z/core::kFix};
                if(project_prop_captured_support(index,point)){LOG(.str("classic composition CONTACT source=captured_voxel item=").x(o.item).str(" index=").i(index));break;}
                project_trace_object(index,"refused","E_VOXEL_COLLISION");return project_spatial_stop(S_PROJECT_COLLISION,centre,-1,o.module);}
        }
        ProjectMaterialContext context;context.module=o.module;context.saved=context.native=o.item;
        if(!project_cost(o.item,1,context,costs,true)){project_trace_object(index,"refused","E_NATIVE_COST_REFUSED");return false;}
    }
    if(GetTickCount()-g_bpSnapshotCompleted>nativescan::SnapshotMaxAgeMs){bp_snapshot_fail(nativescan::SnapshotStale);return project_prop_scan_problem();}
    LOG(.str("native collision CHECK method=native_placement_aabb result=CLEAR checked=").i(g_projectJob.batch).str(" relevant=").i(g_bpSnapshotCount).str(" snapshot_age_ms=").u(GetTickCount()-g_bpSnapshotCompleted));
    g_bpSnapshotState=0;return true;
}
