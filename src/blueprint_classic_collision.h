// Additional collision consumer of the ORIGINAL capture reader, not the dev11
// component-discovery scanner. It does not place anything or write a blueprint.
static bool bp_classic_scan_requested(){return g_bpSnapshotTrace.classicReader;}
static bool bp_classic_snapshot_request(const props::Bounds& region){
    if(!bp_snapshot_request(region,true))return false;
    LOG(.str("classic composition SCAN_REQUEST method=original_blueprint_reader"));return true;
}
static bool bp_classic_verify_visit(const BpFound&,void*){return g_bpSnapshotState==2;}
static bool bp_classic_collision_visit(const BpFound& found,void*){
    if(g_bpSnapshotState!=2)return false;
    auto& d=g_bpSnapshotTrace;d.item=found.item;d.entity=found.entity;d.visited=g_bpScanExamined;++d.props;d.objectData=false;
    for(int a=0;a<3;++a)d.position[a]=found.pos[a]/core::kFix;
    memcpy(d.rotation,found.rot,16);memcpy(d.scale,found.scale,12);
    auto item=g_bpItemById(found.item,g_bpItemReg);
    if(!item||!readable(item,4)||*(uint32_t*)item!=found.item||!bp_item_bounds_raw(item,d.low,d.high))return bp_snapshot_fail(nativescan::ItemBoundsMissing,found.item,found.entity);
    if(g_bpSnapshotState!=2)return false;d.objectData=true;
    props::Bounds box;nativescan::Error error;bool flat=false;
    const auto result=nativescan::proxy(d.position,d.rotation,d.scale,d.low,d.high,g_bpSnapshotRegion,&box,&error,&flat);
    if(result==nativescan::Corrupt||result==nativescan::Unresolved)return bp_snapshot_fail(error,found.item,found.entity);
    if(result==nativescan::Solid){if(d.relevant==props::MaxWorldObjects)return bp_snapshot_fail(nativescan::Capacity,found.item,found.entity);
        g_bpSnapshotWorking[d.relevant++]={found.item,box,found.entity};}
    else if(flat)++d.excludedDegenerate;return true;
}
static void bp_classic_snapshot_step(uint8_t* sys,uint8_t* frame){
    if(g_bpSnapshotState!=1)return;g_bpSnapshotState=2;g_bpSnapshotAttemptStart=GetTickCount();g_bpSnapshotReads.reset();
    auto& d=g_bpSnapshotTrace;d={};d.classicReader=true;d.attempts=1;g_bpSnapshotCount=0;
    if(!bp_snapshot_header(sys,&d.before))return;d.slots=d.before.slots;
    BpScanAudit captured;captured.started=g_bpSnapshotAttemptStart;
    bool complete=bp_scan_entities(sys,frame,bp_classic_collision_visit,nullptr,&captured);d.visited=g_bpScanExamined;d.transformIndex=captured.transform;
    int verified=0,passes=1;
    if(complete&&g_bpSnapshotState==2){
        if(!bp_snapshot_header(sys,&d.after))return;d.change=nativescan::difference(d.before,d.after);
        if(d.change!=nativescan::Unchanged){bp_snapshot_fail(nativescan::TableChanged);return;}
        if((uint32_t)d.visited!=d.before.count){bp_snapshot_fail(nativescan::TableInvalid);return;}
        BpScanAudit checked;checked.started=g_bpSnapshotAttemptStart;passes=2;
        complete=bp_scan_entities(sys,frame,bp_classic_verify_visit,nullptr,&checked);verified=g_bpScanExamined;
        if(complete&&g_bpSnapshotState==2){const char* field=nullptr;uint64_t expected=0,observed=0;
            if(captured.transform!=checked.transform){d.change=nativescan::Transform;field="transform_component";expected=captured.transform;observed=checked.transform;}
            else if(captured.population!=checked.population){d.change=nativescan::Entry;field="classic_population_fingerprint";expected=captured.population;observed=checked.population;}
            else if(captured.items!=checked.items){d.change=nativescan::Item;field="classic_item_fingerprint";expected=captured.items;observed=checked.items;}
            else if(captured.transforms!=checked.transforms){d.change=nativescan::Transform;field="classic_transform_fingerprint";expected=captured.transforms;observed=checked.transforms;}
            if(field){d.field=field;d.expected=expected;d.observed=observed;
                LOG(.str("classic composition COHERENCE method=original_blueprint_reader passes=2 change=").str(nativescan::change_name(d.change)).str(" field=").str(field).str(" expected=").x(expected).str(" observed=").x(observed).str(" visited=").i(d.visited).str(" verified=").i(verified));
                bp_snapshot_fail(nativescan::TableChanged);return;}}
    }
    d.durationMs=GetTickCount()-g_bpSnapshotAttemptStart;
    if(g_bpSnapshotState!=2)return;
    if(!complete){if(g_bpSnapshotState==-1)return;
        LOG(.str("classic composition SCAN_REFUSED method=original_blueprint_reader code=").str(g_bpScanError?g_bpScanError:"E_CLASSIC_SCAN_INCOMPLETE"));
        const auto code=g_bpScanError&&project::equal(g_bpScanError,"E_CLASSIC_ITEM_UNRESOLVED")?nativescan::ItemUnresolved:
            g_bpScanError&&project::equal(g_bpScanError,"E_CLASSIC_SCAN_BUDGET")?nativescan::ClassicBudget:
            g_bpScanError&&project::equal(g_bpScanError,"E_CLASSIC_ITEM_COMPONENT")?nativescan::ItemComponentUnknown:
            g_bpScanError&&project::equal(g_bpScanError,"E_CLASSIC_ENTRY")?nativescan::TableInvalid:nativescan::ComponentUnreadable;
        bp_snapshot_fail(code,g_bpScanItem,g_bpScanEntity);return;}
    if(!bp_snapshot_header(sys,&d.after))return;d.change=nativescan::difference(d.before,d.after);
    if(d.change!=nativescan::Unchanged){bp_snapshot_fail(nativescan::TableChanged);return;}
    if((uint32_t)d.visited!=d.before.count){bp_snapshot_fail(nativescan::TableInvalid);return;}
    if((uint32_t)verified!=d.before.count){bp_snapshot_fail(nativescan::TableInvalid);return;}
    if(d.durationMs>5000){bp_snapshot_fail(nativescan::ClassicBudget);return;}
    memcpy(g_bpSnapshot,g_bpSnapshotWorking,d.relevant*sizeof(*g_bpSnapshot));g_bpSnapshotCount=d.relevant;g_bpSnapshotCompleted=GetTickCount();g_bpSnapshotState=3;d.error=nativescan::Ok;
    LOG(.str("classic composition SCAN_COMPLETE method=original_blueprint_reader published=1 passes=").i(passes).str(" visited=").i(d.visited).str(" verified=").i(verified).str(" relevant=").i(d.relevant).str(" excluded_degenerate=").i(d.excludedDegenerate).str(" elapsed_ms=").u(d.durationMs));
}
