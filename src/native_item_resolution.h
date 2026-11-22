// Fresh native registry lookup used at project load and before placement.
// Fresh registry resolution: imported names/IDs never authorize placement.
static nativecatalog::Status native_item_resolution(uint32_t id,bool classic=false){
    nativecatalog::Facts f;f.registry=g_bpItemById&&g_bpItemReg;f.abi=g_nativeObjectAbi;
    uint8_t* it=f.registry?g_bpItemById(id,g_bpItemReg):nullptr;f.exists=it&&readable(it,4)&&*(uint32_t*)it==id;
    if(f.exists){f.altar=readable(it+0x427,1)&&it[0x427];f.prop=prop_item(it);
        if(classic&&f.prop&&!readable(it+0x5d8,4))return nativecatalog::Unreadable;
        if(readable(it+0x5d8,4))f.flags=*(uint32_t*)(it+0x5d8);else f.flags=0xffffffffu;
        if(f.prop&&f.abi&&!f.altar&&(classic||extended::object_flags_allowed(f.flags,g_projectBlueprintLogic!=0))){float lo[3],hi[3];props::Bounds box;const double zero[]={0,0,0};const float q[]={0,0,0,1};
            f.bounds=bp_item_bounds(it,lo,hi)&&props::bounds(zero,q,lo,hi,&box);}}
    return nativecatalog::resolve(f,g_projectBlueprintLogic!=0,classic);
}
