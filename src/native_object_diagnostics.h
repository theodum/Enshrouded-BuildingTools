static void project_trace_object(int index,const char* stage,const char* code){
    if(index<0||index>=g_projectObjectCount)return;const auto& o=g_projectObjects[index];
    const char* name=bp_item_name_cached(o.item);const auto* source=itemindex::find(o.item);if(!name&&source)name=source->name;
    uint8_t* it=g_bpItemById&&g_bpItemReg?g_bpItemById(o.item,g_bpItemReg):nullptr;
    Buf guid;if(it&&readable(it+0x3c0,16))for(int a=0;a<16;++a)guid.u(it[0x3c0+a],16,2);else guid.str("unresolved");
    double position[3];float rotation[4];extended::object_world(o,g_projectManifest.anchor,g_projectRotation,g_projectOrigin,position,rotation);
    const bool loading=project::equal(stage,"load_resolved")||project::equal(stage,"load_refused");
    if(loading){memcpy(position,o.p,sizeof position);memcpy(rotation,o.rotation,sizeof rotation);}
    LOG(.str("native object TRACE stage=").str(stage).str(" code=").str(code).str(" index=").i(index).str(" name=").str(name?name:"(uncached)")
        .str(" inventory_id=").x(o.item).str(" template_guid=").str(guid.z()).str(" source=native_registry transform=BTBP/module world_entity=not_observed")
        .str(" position_frame=").str(loading?"project_local":"world_candidate").str(" position=(").f(position[0],6).c(',').f(position[1],6).c(',').f(position[2],6).str(") quaternion=(").f(rotation[0],6).c(',').f(rotation[1],6).c(',').f(rotation[2],6).c(',').f(rotation[3],6)
        .str(") scan_state=").i(g_bpSnapshotState).str(" scan_code=").str(nativescan::code(g_bpSnapshotTrace.error)));
    float low[3],high[3];const auto status=project_object_resolution(o.item);
    if((nativecatalog::admitted(status)||status==nativecatalog::InvalidBounds)&&it&&prop_item(it)&&bp_item_bounds_raw(it,low,high))
        LOG(.str("native object BOUNDS index=").i(index).str(" source=native_placement_aabb low=(").f(low[0],6).c(',').f(low[1],6).c(',').f(low[2],6)
            .str(") high=(").f(high[0],6).c(',').f(high[1],6).c(',').f(high[2],6).str(") size=(").f(high[0]-low[0],6).c(',').f(high[1]-low[1],6).c(',').f(high[2]-low[2],6).c(')'));
    else LOG(.str("native object BOUNDS index=").i(index).str(" source=native_registry unavailable resolution=").str(nativecatalog::name(status)));
}
