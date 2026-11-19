// Executes the DLL's actual read-only scanner, native adapter and shared policy
// against bounded ItemInfo allocations. Native functions are recorded adapters;
// this does not establish the behaviour of Enshrouded's pricing or world writes.
#include "../src/core.h"
#include "../src/materials.h"
#include <cstdio>
#include <cstring>
#include <vector>
#include <algorithm>
static int checks = 0, failures = 0;
#define CHECK(c) do { ++checks; if (!(c)) { std::printf("FAIL materials line %d: %s\n", __LINE__, #c); ++failures; } } while (0)
struct LogBuf { LogBuf& str(const char*) { return *this; } LogBuf& x(uint64_t) { return *this; } LogBuf& i(int64_t) { return *this; } LogBuf& u(uint64_t) { return *this; } };
#define LOG(expr) do { LogBuf b; b expr; } while (0)
static bool creative=false;
static bool creative_on() { return creative; }
using DWORD = uint32_t;
constexpr DWORD PAGE_READONLY=1,PAGE_READWRITE=2,PAGE_WRITECOPY=4,PAGE_EXECUTE_READ=8,PAGE_EXECUTE_READWRITE=16,PAGE_GUARD=32,PAGE_NOACCESS=64,MEM_COMMIT=1;
struct MEMORY_BASIC_INFORMATION { void* AllocationBase; void* BaseAddress; size_t RegionSize; DWORD State, Protect; };
alignas(8) static uint8_t assets[0x3000], shape[0x1000];
static int scans = 0;
static bool inside(const void* p, const uint8_t* base, size_t size, size_t len=1) {
    const uintptr_t v=(uintptr_t)p, b=(uintptr_t)base; return v>=b && v-b<=size && len<=size-(v-b);
}
static bool readable(const void* p, size_t n) { return inside(p,assets,sizeof assets,n) || inside(p,shape,sizeof shape,n); }
static size_t VirtualQuery(const void* p, MEMORY_BASIC_INFORMATION* m, size_t) {
    ++scans;
    for (auto base : {assets,shape}) {
        size_t size=base==assets?sizeof assets:sizeof shape;
        if (inside(p,base,size)) { *m={base,base,size,MEM_COMMIT,PAGE_READWRITE}; return sizeof *m; }
    }
    return 0;
}
static DWORD GetTickCount() { return 100; }
static std::vector<uint8_t*> registry;
static uint8_t* lookup(uint32_t id, void*) { for(auto p:registry) if(*(uint32_t*)p==id) return p; return nullptr; }
static uint8_t* (*g_bpItemById)(uint32_t,void*)=lookup;
static void* g_bpItemReg=assets;
static bool bp_block_item(const uint8_t* p) {
    return p && readable(p,0x4b8) && p[0x440] && p[0x454]>=0x80 && (*(uint64_t*)(p+0x458) || *(uint64_t*)(p+0x460))
        && !*(uint64_t*)(p+0x468) && !*(uint64_t*)(p+0x470);
}
struct CostEntry { uint8_t b[24]; };
static bool refundKnown = true;
static uint32_t refundId = 0x7ad0559c;
static void* lastContext = nullptr;
static bool bp_entry(void* ctx,int,CostEntry* e) { lastContext=ctx; if(!refundKnown)return false; memset(e,0,sizeof *e); *(uint32_t*)(e->b+0x10)=refundId; return true; }
#include "../src/blueprint_material_scan.h"
#include "../src/blueprint_material.h"
struct CostList { void* data; uint64_t count, cap; void* grow; };
struct ProjectMaterialContext { int cell=-1, piece=-1; };
static materials::Need g_projectNeed[128],g_projectBuildNeed[128];
static int g_projectNeedN=0,g_projectBuildNeedN=0;
static materials::CostResult lastCost;
static bool project_cost_result(const materials::CostResult& r,const ProjectMaterialContext&) { lastCost=r;return r.issue==materials::CostIssue::None; }
#include "../src/project_material_cost.h"
static uint8_t* make_block(int offset,uint32_t id,uint8_t byte=0x9d) {
    uint8_t* p=assets+offset; memset(p,0,0x4b8); *(uint32_t*)p=id; p[0x454]=byte; p[0x440]=1; *(uint64_t*)(p+0x458)=1; registry.push_back(p); return p;
}
static void reset() { memset(assets,0,sizeof assets); memset(shape,0,sizeof shape); registry.clear();g_bpMatItemN=0;scans=0;refundKnown=true;refundId=0x7ad0559c;g_bpItemById=lookup;g_bpItemReg=assets; }
static void resolution_tests() {
    reset(); auto real=make_block(0x800,0x7ad0559c); core::BpMat m{};m.item=0x7ad0559c;m.mat=0x9d;
    auto r=bp_resolve_material(shape,assets,m); CHECK(r.item==real && r.route==materials::Route::Registry && scans==0);
    m.mat=0x81; r=bp_resolve_material(shape,assets,m); CHECK(r.item==real && r.nativeByte==0x9d && scans==0); // stable ID, renumbered hint
    m.item=0x25a43c2f;m.mat=0x9d;
    // Actual dev4 evidence: a verified block exists but the file saved another ID.
    r=bp_resolve_material(shape,assets,m);
    CHECK(!r.item && r.issue==materials::Issue::RefundMismatch && r.candidate==0x7ad0559c && r.refund==0x7ad0559c);
    CHECK(r.refundKnown && lastContext==shape && g_bpMatItemN==1);
    g_bpItemById=+[](uint32_t,void*) -> uint8_t* { return assets+0x800; };
    r=bp_resolve_material(shape,assets,m);CHECK(!r.item && r.issue==materials::Issue::RefundMismatch); // wrong lookup identity cannot bypass the guard
    g_bpItemById=lookup;
    refundId=m.item;r=bp_resolve_material(shape,assets,m);CHECK(r.item==real && r.route==materials::Route::LegacyRefund);
    refundKnown=false;r=bp_resolve_material(shape,assets,m);CHECK(r.item==real && r.route==materials::Route::LegacyNoRefund);
    // A plain refund item in the registry is not passed as the building material.
    uint8_t* plain=assets+0x1800;*(uint32_t*)plain=m.item;registry.push_back(plain);
    refundKnown=true;refundId=m.item;r=bp_resolve_material(shape,assets,m);CHECK(r.item==real && r.item!=plain);
    m.mat=0x7f;r=bp_resolve_material(shape,assets,m);CHECK(!r.item && r.issue==materials::Issue::Unknown);
    reset();real=make_block(0x800,0x7ad0559c);m.mat=0x9d;m.item=0x25a43c2f;refundId=m.item;
    // The wrong shape allocation must not permanently cache a negative result.
    r=bp_resolve_material(shape,shape,m);CHECK(!r.item && g_bpMatItemN==0);
    r=bp_resolve_material(shape,assets,m);CHECK(r.item==real && g_bpMatItemN==1);
    // Cached pointers must still be the definition returned by the current registry.
    registry.clear();auto moved=make_block(0x1000,0x7ad0559c);
    r=bp_resolve_material(shape,assets,m);CHECK(r.item==moved && g_bpMatItemN==1);
    registry.clear();r=bp_resolve_material(shape,assets,m);CHECK(!r.item && g_bpMatItemN==0); // stray matching bytes refused
    real=make_block(0x800,0x7ad0559c);real[0x440]=0;
    r=bp_resolve_material(shape,assets,m);CHECK(!r.item); // large/other shape refused
    real[0x440]=1;*(uint64_t*)(real+0x468)=1;
    r=bp_resolve_material(shape,assets,m);CHECK(!r.item); // terrain refused
    g_bpItemReg=nullptr;r=bp_resolve_material(shape,assets,m);CHECK(!r.item);
}
static void cost_tests() {
    using namespace materials;
    Need needs[MaxNeeds]{};int n=0;
    auto r=add(needs,n,7,0);CHECK(r.issue==CostIssue::None && n==0);
    r=add(needs,n,0,0);CHECK(r.issue==CostIssue::UnknownItem && n==0);
    r=add(needs,n,7,0xffffffff);CHECK(r.issue==CostIssue::Excessive && r.attempted==0xffffffff && n==0);
    CHECK(add(needs,n,7,(uint32_t)MaxCost,true).issue==CostIssue::None && n==1);
    r=add(needs,n,7,1);CHECK(r.issue==CostIssue::Excessive && r.previous==MaxCost && r.attempted==MaxCost+1 && needs[0].count==MaxCost && needs[0].prop);
    for (uint32_t id=8;id<8+MaxNeeds-1;++id) CHECK(add(needs,n,id,1).issue==CostIssue::None);
    CHECK(n==MaxNeeds);r=add(needs,n,1000,1);CHECK(r.issue==CostIssue::TooManyItems && r.attempted==129 && r.limit==128 && n==MaxNeeds);
    CHECK(add(needs,n,1000,0).issue==CostIssue::None && n==MaxNeeds);
    n=0;add(needs,n,7,1,true);add(needs,n,7,2,false);add(needs,n,7,3,true);CHECK(n==1 && needs[0].count==6 && !needs[0].prop);
    CHECK(valid_list(needs,0,256,needs,256));CHECK(valid_list(needs,256,256,needs,256));
    CHECK(!valid_list(needs,257,256,needs,256));CHECK(!valid_list(needs,2,1,needs,256));
    CHECK(!valid_list(needs,0,257,needs,256));CHECK(!valid_list(nullptr,1,256,needs,256));
}
static void native_cost_tests() {
    reset(); uint8_t* plain=assets;*(uint32_t*)plain=7;registry.push_back(plain);
    ProjectMaterialContext context; CostEntry entries[256]{};CostList price{entries,1,256,nullptr};
    auto entry=[&](uint32_t id,uint32_t count){*(uint32_t*)(entries[0].b+0x10)=id;*(uint32_t*)(entries[0].b+0x14)=count;};
    g_projectNeedN=g_projectBuildNeedN=0;
    // The actual native adapter reads ID/quantity at +0x10/+0x14, not GUID or saved BTBP estimates.
    memset(entries[0].b,0xa5,16);entry(7,3);
    CHECK(project_native_cost(price,entries,context,true));CHECK(g_projectNeedN==1 && g_projectNeed[0].item==7 && g_projectNeed[0].count==3);
    CHECK(project_native_cost(price,entries,context,true));CHECK(g_projectNeed[0].count==6);
    CHECK(project_native_cost(price,entries,context,false));CHECK(g_projectNeed[0].count==6 && g_projectBuildNeed[0].count==3);
    entry(7,0);CHECK(project_native_cost(price,entries,context,true));CHECK(g_projectNeed[0].count==6);
    entry(0xdeadbeef,9);CHECK(!project_native_cost(price,entries,context,true));
    CHECK(lastCost.issue==materials::CostIssue::UnknownItem && lastCost.item==0xdeadbeef && lastCost.requested==9 && g_projectNeed[0].count==6);
    entry(0,0);CHECK(!project_native_cost(price,entries,context,true));CHECK(lastCost.issue==materials::CostIssue::UnknownItem);
    entry(7,0x7fffffff);CHECK(!project_native_cost(price,entries,context,true));CHECK(lastCost.issue==materials::CostIssue::Excessive && lastCost.attempted==0x80000005 && g_projectNeed[0].count==6);
    // Creative mode affects logs but must never bypass any pricing safety gate.
    context.cell=0;
    for (bool free : {false,true}) for (bool checking : {false,true}) {
        creative=free;
        price.count=257;CHECK(!project_native_cost(price,entries,context,checking));CHECK(lastCost.issue==materials::CostIssue::InvalidList);
        price.count=1;price.data=(void*)1;CHECK(!project_native_cost(price,entries,context,checking));CHECK(lastCost.issue==materials::CostIssue::InvalidList);
        price.data=entries;price.cap=0;CHECK(!project_native_cost(price,entries,context,checking));price.cap=256;
        entry(7,0xffffffff);CHECK(!project_native_cost(price,entries,context,checking));CHECK(lastCost.issue==materials::CostIssue::Excessive);
        entry(0xdeadbeef,9);CHECK(!project_native_cost(price,entries,context,checking));CHECK(lastCost.issue==materials::CostIssue::UnknownItem);
    }
    entry(7,1);price.count=0;CHECK(project_native_cost(price,entries,context,true));
}
int main() { resolution_tests();cost_tests();native_cost_tests();std::printf("materials: %d checks, %d failures\n",checks,failures);return failures?1:0; }
