// Complete, bounded read in ONE native server callback, as in classic capture.
// No native command/query/placement is issued. Only validated results are published.
struct BpSnapshotRecord {
    uint32_t slot=0,id=0,item=0,row=0;
    uint8_t *entity=nullptr,*arch=nullptr,*data=nullptr,*definition=nullptr;
    bool hasItem=false,isProp=false;
    uint8_t transform[56]{};
};
static uint64_t g_bpSnapshotMask[nativescan::MaxSlots/64];
static BpSnapshotRecord g_bpSnapshotRecords[nativescan::MaxEntities];
static int g_bpSnapshotRecordCount=0;
static DWORD g_bpSnapshotAttemptStart=0;
static props::WorldObject g_bpSnapshotWorking[props::MaxWorldObjects];
static nativescan::ReadCache g_bpSnapshotReads;
static bool bp_snapshot_readable(const void* p,size_t n){return g_bpSnapshotReads.check(p,n,bp_snapshot_memory_region);}
static const char* bp_snapshot_method(){return g_bpSnapshotTrace.classicReader?"original_blueprint_reader":"classic_ecs_bitset_snapshot";}

static void bp_snapshot_header_log(const char* label,const nativescan::Header& h){
    LOG(.str("native scan COHERENCE method=").str(bp_snapshot_method()).str(" phase=").str(label)
        .str(" world=").x(h.world).str(" table=").x(h.table).str(" bits=").x(h.bits).str(" keys=").x(h.keys).str(" values=").x(h.values).str(" slots=").u(h.slots).str(" count=").u(h.count));
}
static bool bp_snapshot_fail(nativescan::Error error,uint32_t item,uint32_t entity){
    auto& d=g_bpSnapshotTrace;d.error=error;d.item=item;d.entity=entity;
    if(g_bpSnapshotState==2)d.durationMs=GetTickCount()-g_bpSnapshotAttemptStart;
    g_bpSnapshotBadItem=item;g_bpSnapshotState=-1;g_bpSnapshotCount=0;g_bpSnapshotCompleted=0;
    const auto* named=itemindex::find(item);
    LOG(.str("native scan REFUSED method=").str(bp_snapshot_method()).str(" code=").str(nativescan::code(error)).str(" change=").str(nativescan::change_name(d.change))
        .str(" scope=world_scan slot=").u(d.slot).c('/').u(d.slots).str(" entity=").x(entity).str(" item=").x(item)
        .str(" name=").str(named?named->name:"(unresolved)").str(" visited=").i(d.visited).str(" props=").i(d.props).str(" relevant=").i(d.relevant)
        .str(" published=0 words=").i(d.words).str(" restarts=").i(d.restarts).str(" attempts=").i(d.attempts).str(" elapsed_ms=").u(d.durationMs).str(" memory_queries=").u(g_bpSnapshotReads.queries).str(" transform_component=").u(d.transformIndex));
    bp_snapshot_header_log("before",d.before);bp_snapshot_header_log("after",d.after);
    if(d.objectData)LOG(.str("native scan DATA code=").str(nativescan::code(error)).str(" native_low=(").f(d.low[0],6).c(',').f(d.low[1],6).c(',').f(d.low[2],6)
        .str(") native_high=(").f(d.high[0],6).c(',').f(d.high[1],6).c(',').f(d.high[2],6).str(") position=(").f(d.position[0]).c(',').f(d.position[1]).c(',').f(d.position[2])
        .str(") rotation=(").f(d.rotation[0],6).c(',').f(d.rotation[1],6).c(',').f(d.rotation[2],6).c(',').f(d.rotation[3],6).str(") scale=(").f(d.scale[0]).c(',').f(d.scale[1]).c(',').f(d.scale[2]).c(')'));
    return false;
}
static void bp_snapshot_cancel(const char* reason){
    if(g_bpSnapshotState==1||g_bpSnapshotState==2||g_bpSnapshotState==3){
        LOG(.str("native scan CANCELLED method=").str(bp_snapshot_method()).str(" code=E_WORLD_READ_CANCELLED reason=").str(reason).str(" published_discarded=").i(g_bpSnapshotCount));
        g_bpSnapshotTrace.error=nativescan::Cancelled;
    }
    g_bpSnapshotState=0;g_bpSnapshotCount=0;g_bpSnapshotRecordCount=0;g_bpSnapshotCompleted=0;
}
static bool bp_snapshot_component(uint8_t* e,uint32_t index,uint8_t** value,uint32_t* size){
    *value=nullptr;*size=0;if(index>=640||!e||!bp_snapshot_readable(e,0x38))return false;
    uint8_t* arch=*(uint8_t**)(e+0x18);if(!arch||!bp_snapshot_readable(arch,0xa84+640*2))return false;
    if(!((*(uint64_t*)(arch+(index>>6)*8)>>(index&63))&1))return true;
    const uint32_t stride=*(uint16_t*)(arch+0xa84+index*2),offset=*(uint16_t*)(arch+0x84+index*2),row=*(uint32_t*)(e+0x30);
    uint8_t* data=*(uint8_t**)(e+0x20);if(!data||!stride||row>1000000||stride>4096)return false;
    const uintptr_t address=(uintptr_t)data+offset+(uint64_t)row*stride;
    if(address<(uintptr_t)data||!bp_snapshot_readable((void*)address,stride))return false;
    *size=stride;*value=(uint8_t*)address;return true;
}
static bool bp_snapshot_header(uint8_t* sys,nativescan::Header* h){
    *h={};if(!sys||!bp_snapshot_readable(sys,8))return bp_snapshot_fail(nativescan::SystemUnreadable);
    h->world=(uint64_t)*(uint8_t**)sys;
    if(!h->world||!bp_snapshot_readable((void*)(h->world+0x30),8))return bp_snapshot_fail(nativescan::SystemUnreadable);
    h->table=*(uint64_t*)(h->world+0x30);
    if(!h->table||!bp_snapshot_readable((void*)h->table,0x1a0))return bp_snapshot_fail(nativescan::TableUnreadable);
    const uint8_t* map=(uint8_t*)h->table+0x148;
    h->bits=*(uint64_t*)(map+8);h->slots=*(uint64_t*)(map+0x10);h->keys=*(uint64_t*)(map+0x28);
    h->values=*(uint64_t*)(map+0x40);h->count=*(uint32_t*)(map+0x54);
    if(h->slots>nativescan::MaxSlots||(h->slots&&(h->slots&(h->slots-1)))||h->count>h->slots)return bp_snapshot_fail(nativescan::TableBudget);
    if(h->count>nativescan::MaxEntities)return bp_snapshot_fail(nativescan::EntityBudget);
    if(h->slots&&(!h->bits||!h->keys||!h->values||!bp_snapshot_readable((void*)h->bits,(size_t)((h->slots+63)/64)*8)
        ||!bp_snapshot_readable((void*)h->keys,(size_t)h->slots*4)||!bp_snapshot_readable((void*)h->values,(size_t)h->slots*8)))return bp_snapshot_fail(nativescan::TableUnreadable);
    return true;
}
static bool bp_snapshot_changed(nativescan::Change change){
    auto& d=g_bpSnapshotTrace;d.change=change;d.durationMs=GetTickCount()-g_bpSnapshotAttemptStart;g_bpSnapshotCount=0;g_bpSnapshotCompleted=0;
    bp_snapshot_header_log("before",d.before);bp_snapshot_header_log("after",d.after);
    if(d.restarts==2)return bp_snapshot_fail(nativescan::TableChanged,d.item,d.entity);
    ++d.restarts;g_bpSnapshotState=1;
    LOG(.str("native scan RESTART method=classic_ecs_bitset_snapshot code=W_TABLE_CHANGED change=").str(nativescan::change_name(change))
        .str(" attempt=").i(d.restarts).str(" visited=").i(d.visited).str(" relevant=").i(d.relevant).str(" published=0"));
    return false;
}
static bool bp_snapshot_content_changed(nativescan::Change change,const char* field,uint64_t expected,uint64_t observed,int element=-1){
    auto& d=g_bpSnapshotTrace;d.field=field;d.expected=expected;d.observed=observed;d.element=element;
    LOG(.str("native scan CONTENT method=classic_ecs_bitset_snapshot change=").str(nativescan::change_name(change)).str(" field=").str(field)
        .str(" slot=").u(d.slot).str(" entity=").x(d.entity).str(" element=").i(element).str(" expected=").x(expected).str(" observed=").x(observed));
    return bp_snapshot_changed(change);
}
static bool bp_snapshot_mask_unchanged(const nativescan::Header& h){
    for(uint64_t w=0;w<(h.slots+63)/64;++w){const auto current=((uint64_t*)h.bits)[w];
        if(g_bpSnapshotMask[w]!=current){g_bpSnapshotTrace.slot=w*64;return bp_snapshot_content_changed(nativescan::Occupancy,"occupancy_word",g_bpSnapshotMask[w],current);}}
    return true;
}
static bool bp_snapshot_current(uint8_t* sys,const nativescan::Header& h,uint64_t word){
    auto& d=g_bpSnapshotTrace;
    if(!bp_snapshot_header(sys,&d.after))return false;
    if(const auto changed=nativescan::difference(h,d.after))return bp_snapshot_changed(changed);
    const auto current=((uint64_t*)h.bits)[word];
    if(current!=g_bpSnapshotMask[word]){d.slot=word*64;return bp_snapshot_content_changed(nativescan::Occupancy,"occupancy_word",g_bpSnapshotMask[word],current);}
    return true;
}
static bool bp_snapshot_budget(DWORD start){
    if(g_bpSnapshotState!=2)return false;
    auto& d=g_bpSnapshotTrace;d.durationMs=GetTickCount()-start;
    return d.durationMs<=nativescan::CallbackBudgetMs||bp_snapshot_fail(nativescan::SnapshotBudget,d.item,d.entity);
}
static bool bp_snapshot_entry(const nativescan::Header& h,uint32_t slot,BpSnapshotRecord* record){
    auto& d=g_bpSnapshotTrace;d.slot=slot;d.objectData=false;d.item=d.entity=0;
    auto values=(uint8_t**)h.values;auto keys=(uint32_t*)h.keys;
    uint8_t* e=values[slot];if(!e||!bp_snapshot_readable(e,0x38))return bp_snapshot_fail(nativescan::EntityUnreadable);
    const uint32_t id=*(uint32_t*)(e+0x10);
    if(!id||keys[slot]!=id){d.entity=id;return bp_snapshot_content_changed(nativescan::Entry,"entity_id_vs_key",keys[slot],id);}
    *record={};record->slot=slot;record->id=id;record->entity=e;record->arch=*(uint8_t**)(e+0x18);
    record->data=*(uint8_t**)(e+0x20);record->definition=*(uint8_t**)(e+0x28);record->row=*(uint32_t*)(e+0x30);return true;
}
// Same component-discovery rule as classic blueprint capture, with checked
// component reads. Once found, restart enumeration from the copied occupancy.
static bool bp_snapshot_discover(uint8_t* sys,const nativescan::Header& h,DWORD start){
    if(g_bpItemComp<640)return true;
    for(uint64_t w=0;w<(h.slots+63)/64;++w){uint64_t word=g_bpSnapshotMask[w];
        while(word){const auto slot=(uint32_t)(w*64+__builtin_ctzll(word));word&=word-1;
            if(!bp_snapshot_current(sys,h,w))return false;
            BpSnapshotRecord record;if(!bp_snapshot_entry(h,slot,&record))return false;
            ++g_bpSnapshotTrace.discovery;
            for(uint32_t k=0;k<640;++k){uint8_t* value;uint32_t size;
                if(!bp_snapshot_component(record.entity,k,&value,&size))return bp_snapshot_fail(nativescan::ComponentUnreadable,0,record.id);
                if(!value||size!=4)continue;const uint32_t id=*(uint32_t*)value;if(!id)continue;
                uint8_t* definition=g_bpItemById(id,g_bpItemReg);
                if(definition&&bp_snapshot_readable(definition,4)&&*(uint32_t*)definition==id&&prop_item(definition)){
                    g_bpItemComp=k;LOG(.str("native scan COMPONENT source=classic_capture_discovery item_component=").u(k));return true;}
            }
            if(!bp_snapshot_budget(start))return false;
        }
    }
    return h.count==0||bp_snapshot_fail(nativescan::ItemComponentUnknown);
}
static void bp_snapshot_step(uint8_t* sys,uint8_t* frame){
    if(g_bpSnapshotState!=1)return;
    const DWORD start=GetTickCount();g_bpSnapshotAttemptStart=start;g_bpSnapshotReads.reset();auto& d=g_bpSnapshotTrace;
    const int restarts=d.restarts,attempts=d.attempts+1;d={};d.restarts=restarts;d.attempts=attempts;
    g_bpSnapshotCount=0;g_bpSnapshotRecordCount=0;g_bpSnapshotState=2;
    if(!g_bpWorldAbi){bp_snapshot_fail(nativescan::AbiUnsupported);return;}
    uint16_t* handle=frame&&bp_snapshot_readable(frame+0x60,8)?*(uint16_t**)(frame+0x60):nullptr;
    const uint32_t transform=handle&&bp_snapshot_readable(handle,2)?*handle:0xffff;d.transformIndex=transform;
    if(transform>=640){bp_snapshot_fail(nativescan::TransformHandle);return;}
    if(!bp_snapshot_header(sys,&d.before))return;d.after=d.before;const auto& h=d.before;d.slots=h.slots;
    bp_snapshot_header_log("before",h);
    const auto maskBytes=(size_t)((h.slots+63)/64)*8;
    if(maskBytes)memcpy(g_bpSnapshotMask,(void*)h.bits,maskBytes);
    if(!bp_snapshot_discover(sys,h,start)||g_bpSnapshotState!=2)return;
    for(uint64_t w=0;w<(h.slots+63)/64;++w){++d.words;uint64_t word=g_bpSnapshotMask[w];
        if(w%128==0&&!bp_snapshot_budget(start))return;
        while(word){const auto slot=(uint32_t)(w*64+__builtin_ctzll(word));word&=word-1;
            if(!bp_snapshot_current(sys,h,w))return;
            if(slot>=h.slots){bp_snapshot_fail(nativescan::TableInvalid);return;}
            if(g_bpSnapshotRecordCount==nativescan::MaxEntities){bp_snapshot_fail(nativescan::EntityBudget);return;}
            auto& r=g_bpSnapshotRecords[g_bpSnapshotRecordCount++];if(!bp_snapshot_entry(h,slot,&r))return;++d.visited;
            uint8_t* value;uint32_t size;
            if(!bp_snapshot_component(r.entity,g_bpItemComp,&value,&size)){bp_snapshot_fail(nativescan::ComponentUnreadable,0,r.id);return;}
            if(value&&size!=4){bp_snapshot_fail(nativescan::ComponentUnreadable,0,r.id);return;}
            r.hasItem=value!=nullptr;r.item=value?*(uint32_t*)value:0;
            if(r.item){uint8_t* definition=g_bpItemById(r.item,g_bpItemReg);
                if(!definition||!bp_snapshot_readable(definition,4)||*(uint32_t*)definition!=r.item){bp_snapshot_fail(nativescan::ItemUnresolved,r.item,r.id);return;}
                r.isProp=prop_item(definition);
                if(r.isProp){++d.props;d.item=r.item;d.entity=r.id;
                    uint8_t* tr;if(!bp_snapshot_component(r.entity,transform,&tr,&size)||!tr||size!=56){bp_snapshot_fail(nativescan::ComponentUnreadable,r.item,r.id);return;}
                    memcpy(r.transform,tr,56);const int64_t* fixed=(const int64_t*)tr;
                    for(int a=0;a<3;++a)d.position[a]=fixed[a]/core::kFix;
                    memcpy(d.rotation,tr+24,16);memcpy(d.scale,tr+40,12);
                    if(!bp_item_bounds_raw(definition,d.low,d.high)){bp_snapshot_fail(nativescan::ItemBoundsMissing,r.item,r.id);return;}
                    if(!bp_snapshot_budget(start))return;d.objectData=true;
                    props::Bounds box;nativescan::Error error;bool flat=false;
                    const auto result=nativescan::proxy(d.position,d.rotation,d.scale,d.low,d.high,g_bpSnapshotRegion,&box,&error,&flat);
                    if(result==nativescan::Unresolved||result==nativescan::Corrupt){bp_snapshot_fail(error,r.item,r.id);return;}
                    if(result==nativescan::Solid){if(d.relevant==props::MaxWorldObjects){bp_snapshot_fail(nativescan::Capacity,r.item,r.id);return;}
                        g_bpSnapshotWorking[d.relevant++]={r.item,box,r.id};}
                    else if(flat)++d.excludedDegenerate;
                }
            }
            if((d.visited&63)==0&&!bp_snapshot_budget(start))return;
        }
    }
    if(g_bpSnapshotState!=2||!bp_snapshot_header(sys,&d.after))return;
    if(const auto changed=nativescan::difference(h,d.after)){bp_snapshot_changed(changed);return;}
    if((uint32_t)g_bpSnapshotRecordCount!=h.count){d.change=nativescan::Population;bp_snapshot_fail(nativescan::TableInvalid);return;}
    if(!bp_snapshot_mask_unchanged(h))return;
    // A stable count does not prove a stable collection: compare every occupied
    // key/value and entity row, and the item/transform used for the candidates.
    for(int i=0;i<g_bpSnapshotRecordCount;++i){const auto& r=g_bpSnapshotRecords[i];uint8_t* e=r.entity;
        if(!bp_snapshot_current(sys,h,r.slot/64))return;
        d.slot=r.slot;d.entity=r.id;d.item=r.item;d.objectData=false;
        const auto current=((uint8_t**)h.values)[r.slot];
        if(current!=e){bp_snapshot_content_changed(nativescan::Entry,"entity_pointer",(uint64_t)e,(uint64_t)current);return;}
        const uint32_t key=((uint32_t*)h.keys)[r.slot];if(key!=r.id){bp_snapshot_content_changed(nativescan::Entry,"entity_key",r.id,key);return;}
        if(!bp_snapshot_readable(e,0x38)){bp_snapshot_fail(nativescan::EntityUnreadable,r.item,r.id);return;}
        if(*(uint32_t*)(e+0x10)!=r.id){bp_snapshot_content_changed(nativescan::Entry,"entity_id",r.id,*(uint32_t*)(e+0x10));return;}
        if(*(uint8_t**)(e+0x18)!=r.arch){bp_snapshot_content_changed(nativescan::Entry,"archetype_pointer",(uint64_t)r.arch,*(uint64_t*)(e+0x18));return;}
        if(*(uint8_t**)(e+0x20)!=r.data){bp_snapshot_content_changed(nativescan::Entry,"component_data_pointer",(uint64_t)r.data,*(uint64_t*)(e+0x20));return;}
        if(*(uint8_t**)(e+0x28)!=r.definition){bp_snapshot_content_changed(nativescan::Entry,"entity_template_pointer",(uint64_t)r.definition,*(uint64_t*)(e+0x28));return;}
        if(*(uint32_t*)(e+0x30)!=r.row){bp_snapshot_content_changed(nativescan::Entry,"component_row",r.row,*(uint32_t*)(e+0x30));return;}
        uint8_t* value;uint32_t size;
        if(!bp_snapshot_component(e,g_bpItemComp,&value,&size)){bp_snapshot_fail(nativescan::ComponentUnreadable,r.item,r.id);return;}
        if((value!=nullptr)!=r.hasItem){bp_snapshot_content_changed(nativescan::Item,"used_item_presence",r.hasItem,value!=nullptr);return;}
        if(value&&(size!=4||*(uint32_t*)value!=r.item)){bp_snapshot_content_changed(nativescan::Item,"used_item_id",r.item,size==4?*(uint32_t*)value:0);return;}
        if(r.isProp){uint8_t* tr;if(!bp_snapshot_component(e,transform,&tr,&size)||!tr||size!=56){bp_snapshot_fail(nativescan::ComponentUnreadable,r.item,r.id);return;}
            if(memcmp(tr,r.transform,56)){for(int b=0;b<56;++b)if(tr[b]!=r.transform[b]){
                bp_snapshot_content_changed(nativescan::Transform,"transform_byte",r.transform[b],tr[b],b);return;}}}
        if((i&63)==0&&!bp_snapshot_budget(start))return;
    }
    // Verify the header and mask once more after the content consistency pass.
    if(!bp_snapshot_header(sys,&d.after))return;
    if(const auto changed=nativescan::difference(h,d.after)){bp_snapshot_changed(changed);return;}
    if(!bp_snapshot_mask_unchanged(h))return;
    if(!bp_snapshot_readable(handle,2)||*handle!=transform){bp_snapshot_fail(nativescan::TransformHandle);return;}
    if(!bp_snapshot_budget(start))return;
    memcpy(g_bpSnapshot,g_bpSnapshotWorking,d.relevant*sizeof(*g_bpSnapshot));g_bpSnapshotCount=d.relevant;
    g_bpSnapshotCompleted=GetTickCount();g_bpSnapshotState=3;d.error=nativescan::Ok;d.change=nativescan::Unchanged;
    bp_snapshot_header_log("after",d.after);
    LOG(.str("native scan COMPLETE method=classic_ecs_bitset_snapshot code=OK visited=").i(d.visited).str(" props=").i(d.props)
        .str(" relevant=").i(d.relevant).str(" kept=").i(g_bpSnapshotCount).str(" published=1 words=").i(d.words).str(" excluded_degenerate=").i(d.excludedDegenerate)
        .str(" restarts=").i(d.restarts).str(" attempts=").i(d.attempts).str(" elapsed_ms=").u(d.durationMs).str(" memory_queries=").u(g_bpSnapshotReads.queries)
        .str(" memory_cache_hits=").u(g_bpSnapshotReads.hits).str(" completeness=all_occupied_entries collision=NOT_CHECKED"));
}
static bool bp_snapshot_request(const props::Bounds& region,bool classicReader=false){
    if(g_bpSnapshotState==1||g_bpSnapshotState==2)return bp_snapshot_fail(nativescan::ReaderBusy);
    g_bpSnapshotTrace={};g_bpSnapshotReads.reset();g_bpSnapshotBadItem=0;g_bpSnapshotCount=0;g_bpSnapshotCompleted=0;g_bpSnapshotRecordCount=0;
    g_bpSnapshotTrace.classicReader=classicReader;
    if(!g_bpWorldAbi)return bp_snapshot_fail(nativescan::AbiUnsupported);
    if(!g_bpQResults)return bp_snapshot_fail(nativescan::HookMissing);
    if(!g_bpItemById||!g_bpItemReg)return bp_snapshot_fail(nativescan::RegistryMissing);
    if(!g_bpProbeHits||GetTickCount()-g_bpProbeSeen>3000)return bp_snapshot_fail(nativescan::ComfortInactive);
    if(g_bpProbeReq||g_bpScanning||g_bpProbeLive)return bp_snapshot_fail(nativescan::ReaderBusy);
    for(int a=0;a<3;++a)if(!extended::finite(region.lo[a])||!extended::finite(region.hi[a])||region.hi[a]<=region.lo[a])return bp_snapshot_fail(nativescan::BoundsCorrupt);
    g_bpSnapshotRegion=region;g_bpSnapshotTick=GetTickCount();g_bpSnapshotState=1;
    LOG(.str("native scan REQUEST method=").str(bp_snapshot_method()).str(" region_low=(").f(region.lo[0]).c(',').f(region.lo[1]).c(',').f(region.lo[2])
        .str(") region_high=(").f(region.hi[0]).c(',').f(region.hi[1]).c(',').f(region.hi[2]).str(") callback_budget_ms=").i(classicReader?5000:nativescan::CallbackBudgetMs));return true;
}
