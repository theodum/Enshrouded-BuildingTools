static bool project_validate_spaces(){
    if(!g_projectManifest.spaceCount)return true;
    int bad=-1;auto issue=spaces::validate(g_projectPlan,g_projectManifest,&bad);
    Str reason=issue==spaces::Issue::NoSurface?S_EXT_VOXEL_SURFACE:issue==spaces::Issue::Budget?S_EXT_DEMO_BUDGET:S_EXT_VOXEL_OBSTRUCTED;
    if(issue==spaces::Issue::None)for(int i=0;i<g_projectManifest.spaceCount && issue==spaces::Issue::None;++i){
        const auto b=spaces::box(g_projectManifest.spaces[i]);bad=i;
        for(int p=0;p<g_projectManifest.pieceCount;++p)if(extended::intersects(b,g_projectPieceBoxes[p])){issue=spaces::Issue::Obstructed;break;}
        if(g_projectUseObjects)for(int p=0;p<g_projectObjectCount && issue==spaces::Issue::None;++p){
            const auto& o=g_projectObjects[p];float low[3],high[3];extended::Box box{};
            uint8_t* item=g_bpItemById && g_bpItemReg?g_bpItemById(o.item,g_bpItemReg):nullptr;
            if(!bp_item_bounds(item,low,high)||!extended::object_box(o.p,o.rotation,low,high,&box)){issue=spaces::Issue::Invalid;reason=S_PROJECT_UNAVAILABLE;break;}
            if(extended::intersects(b,box))issue=spaces::Issue::Obstructed;
        }
    }
    if(issue==spaces::Issue::None)return true;
    Buf text,index;index.i(bad);text.str(T(reason)).c(' ').fmt(T(S_EXT_VOXEL_SPACE_DETAIL),index.z());copy_text(g_projectSpaceError,text.z(),sizeof g_projectSpaceError);
    LOG(.str("project space REFUSED index=").i(bad).str(" issue=").i((int)issue).str(" reason=").str(text.z()));return false;
}
