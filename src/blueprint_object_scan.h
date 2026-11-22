// Historical blueprint reader, with a visitor for collision consumers. Ordinary
// capture retains its pivot selection, object order and fast component reader.
using BpObjectVisitor=bool(*)(const BpFound&,void*);
// Optional coherence fingerprints for collision consumers. They do not change
// ordinary capture, component discovery, filtering or object order.
struct BpScanAudit {DWORD started=0;uint32_t transform=0xffff;
    uint64_t population=14695981039346656037ull,items=14695981039346656037ull,transforms=14695981039346656037ull;};
static void bp_scan_hash(uint64_t& hash,const void* bytes,size_t size){const auto* p=(const uint8_t*)bytes;while(size--){hash^=*p++;hash*=1099511628211ull;}}
static const char* g_bpScanError=nullptr;static uint32_t g_bpScanItem=0,g_bpScanEntity=0;
static uint32_t g_bpScanExamined=0;
static bool bp_scan_entities(uint8_t* sys,uint8_t* frame,BpObjectVisitor visitor=nullptr,void* user=nullptr,BpScanAudit* audit=nullptr){
    g_bpScanError=nullptr;g_bpScanItem=g_bpScanEntity=g_bpScanExamined=0;
    auto fail=[&](const char* code,uint32_t item=0,uint32_t entity=0){g_bpScanError=code;g_bpScanItem=item;g_bpScanEntity=entity;return false;};
    uint16_t* trH=frame&&readable(frame+0x60,8)?*(uint16_t**)(frame+0x60):nullptr;
    const uint32_t trIdx=trH&&readable(trH,2)?*trH:0xffff;
    uint8_t* w=*(uint8_t**)sys;uint8_t* base=readable(w+0x30,8)?*(uint8_t**)(w+0x30):nullptr;
    if(!base||trIdx>=640||!readable(base+0x148,0x48)){LOG(.str("blueprint objects: entity table not readable"));return fail("E_CLASSIC_TABLE");}
    uint8_t* map=base+0x148;auto bits=*(uint64_t**)(map+8);const uint64_t slots=*(uint64_t*)(map+0x10);auto vals=*(uint8_t***)(base+0x188);
    if(!bits||!vals||!slots||slots>(1u<<24)||!readable(bits,(size_t)((slots+63)/64)*8)||!readable(vals,(size_t)slots*8)){
        LOG(.str("blueprint objects: entity table looks wrong (").u(slots).str(" slots)"));return fail("E_CLASSIC_TABLE");}
    auto keys=*(uint32_t**)(base+0x170);
    if(audit&&(!keys||!readable(keys,(size_t)slots*4)))return fail("E_CLASSIC_ENTRY");
    const DWORD t0=audit?audit->started:GetTickCount();if(audit)audit->transform=trIdx;
    g_bpArchN=0;uint32_t seen=0,inBox=0,spatial=0;
    auto item_of=[&](uint8_t* e){const auto item=bp_entity_item(e);
        if(visitor&&g_bpItemComp<640){uint8_t* arch=*(uint8_t**)(e+0x18);if(!arch||!readable(arch,0xa84+640*2)){fail("E_CLASSIC_COMPONENT");return uint32_t(0);}
            const bool present=(*(uint64_t*)(arch+(g_bpItemComp>>6)*8)>>(g_bpItemComp&63))&1;
            uint32_t n=0;uint8_t* value=bp_comp(e,g_bpItemComp,&n);
            if(present&&(!value||n!=4)){fail("E_CLASSIC_COMPONENT");return uint32_t(0);}
            if(value&&n==4){const auto raw=*(uint32_t*)value;auto definition=raw?g_bpItemById(raw,g_bpItemReg):nullptr;
                if(raw&&(!definition||!readable(definition,4)||*(uint32_t*)definition!=raw)){fail("E_CLASSIC_ITEM_UNRESOLVED",raw,*(uint32_t*)(e+0x10));return uint32_t(0);}}}
        return item;};
    for(uint64_t s=0;s<slots;++s){
        if(audit&&!(s&63))bp_scan_hash(audit->population,&bits[s>>6],8);
        if(!((bits[s>>6]>>(s&63))&1))continue;uint8_t* e=vals[s];
        if(!e){if(visitor)return fail("E_CLASSIC_ENTITY");continue;}
        ++seen;g_bpScanExamined=seen;
        if(visitor&&(seen&127)==0&&GetTickCount()-t0>5000)return fail("E_CLASSIC_SCAN_BUDGET");
        if(visitor&&(!bp_ptr(e)||!readable(e,0x38)))return fail("E_CLASSIC_ENTITY");
        if(audit){const auto id=*(uint32_t*)(e+0x10);if(!id||keys[s]!=id)return fail("E_CLASSIC_ENTRY",0,id);
            bp_scan_hash(audit->population,&s,8);bp_scan_hash(audit->population,&e,sizeof e);
            bp_scan_hash(audit->population,e+0x10,4);bp_scan_hash(audit->population,e+0x18,28);}
        uint32_t size=0;uint8_t* tr=bp_comp_fast(e,trIdx,&size);
        if(!tr||size<56){
            if(visitor){uint8_t* arch=*(uint8_t**)(e+0x18);if(!arch||!readable(arch,0xa84+640*2))return fail("E_CLASSIC_COMPONENT");
                // Tags/absent transforms are normal; a real prop without its
                // transform is unresolved, never evidence of an empty zone.
                const auto item=item_of(e);if(g_bpScanError)return false;if(item)return fail("E_CLASSIC_COMPONENT",item,*(uint32_t*)(e+0x10));}
            continue;
        }
        if(visitor&&!readable(tr,56))return fail("E_CLASSIC_COMPONENT",0,*(uint32_t*)(e+0x10));
        ++spatial;
        const int64_t* position=(const int64_t*)tr;bool inside=true;
        for(int a=0;a<3&&inside;++a){const float v=(float)(position[a]/core::kFix);if(v<g_bpProbeBox[a]-.01f||v>g_bpProbeBox[4+a]+.01f)inside=false;}
        if(!visitor&&!inside)continue;if(inside)++inBox;
        if(!readable(e,0x38)){if(visitor)return fail("E_CLASSIC_ENTITY");continue;}
        const uint32_t item=item_of(e);if(g_bpScanError)return false;if(!item)continue;
        BpFound found{};found.entity=*(uint32_t*)(e+0x10);found.item=item;
        memcpy(found.pos,position,24);memcpy(found.rot,tr+24,16);memcpy(found.scale,tr+40,12);
        uint8_t* definition=*(uint8_t**)(e+0x28);if(definition&&readable(definition,16))memcpy(found.templateId,definition,16);
        if(audit){bp_scan_hash(audit->items,&s,8);bp_scan_hash(audit->items,&found.item,4);
            bp_scan_hash(audit->transforms,&s,8);bp_scan_hash(audit->transforms,found.pos,24);
            bp_scan_hash(audit->transforms,found.rot,16);bp_scan_hash(audit->transforms,found.scale,12);}
        if(visitor){if(!visitor(found,user))return false;}
        else {
            if(g_bpFoundN==core::BP_MAX_OBJS){g_bpFoundOverflow=true;break;}
            g_bpFound[g_bpFoundN++]=found;
            LOG(.str("  object ").x(item).str(" at (").fix(found.pos[0]).c(',').fix(found.pos[1]).c(',').fix(found.pos[2]).str(") rot (")
                .f(found.rot[0]).c(',').f(found.rot[1]).c(',').f(found.rot[2]).c(',').f(found.rot[3]).c(')'));
        }
        if(visitor&&GetTickCount()-t0>5000)return fail("E_CLASSIC_SCAN_BUDGET");
    }
    if(visitor&&spatial&&g_bpItemComp>=640)return fail("E_CLASSIC_ITEM_COMPONENT");
    LOG(.str("blueprint objects: ").u(seen).str(" entities scanned in ").u(GetTickCount()-t0).str(" ms, ").u(inBox).str(" in the box, ").i(visitor?0:g_bpFoundN).str(" objects method=classic_blueprint_reader visitor=").i(visitor!=nullptr));
    return true;
}
