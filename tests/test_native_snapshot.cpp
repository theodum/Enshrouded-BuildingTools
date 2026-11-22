// Production scanner against bounded synthetic ECS memory. Native metadata and
// mutation timings are fixtures, never a claim of actual placement in Enshrouded.
#include "../src/native_snapshot.h"
#include <cstdio>
#include <cstring>
#include <vector>
#include <limits>
#include <functional>
#include <chrono>
using DWORD=uint32_t;using LONG=int32_t;
#define LOG(expr) do{}while(0)
static DWORD clockTick=100,tickIncrement=0;static DWORD GetTickCount(){clockTick+=tickIncrement;return clockTick;}
struct Range{const uint8_t* p;size_t n;};static std::vector<Range> ranges;
static bool readable(const void* p,size_t n){auto v=(uintptr_t)p;for(auto r:ranges){auto start=(uintptr_t)r.p;if(v>=start&&v-start<=r.n&&n<=r.n-(v-start))return true;}return false;}
static bool bp_snapshot_memory_region(const void* p,uintptr_t* begin,uintptr_t* end){const auto v=(uintptr_t)p;
    for(auto r:ranges){const auto start=(uintptr_t)r.p;if(v>=start&&v-start<r.n){*begin=start;*end=start+r.n;return true;}}return false;}
static int g_bpSnapshotState=0,g_bpSnapshotCount=0,g_bpProbeHits=1,g_bpProbeReq=0,g_bpScanning=0,g_bpProbeLive=0;
static bool g_bpWorldAbi=true;static void* g_bpQResults=(void*)1;static void* g_bpItemReg=(void*)1;
static DWORD g_bpProbeSeen=100,g_bpSnapshotTick=0,g_bpSnapshotCompleted=0;
static uint32_t g_bpSnapshotBadItem=0,g_bpItemComp=610;static props::Bounds g_bpSnapshotRegion;static nativescan::Trace g_bpSnapshotTrace;
static props::WorldObject g_bpSnapshot[512];static bool bp_snapshot_fail(nativescan::Error,uint32_t=0,uint32_t=0);
static constexpr uint32_t PropId=0xd109e828,NonPropId=0xf0000001;
alignas(16) static uint8_t itemDef[2][32]{};static bool nativeBoundsAvailable=true;
static uint8_t* lookup(uint32_t id,void*){return id==PropId?itemDef[0]:id==NonPropId?itemDef[1]:nullptr;}static auto g_bpItemById=lookup;
static bool prop_item(uint8_t* p){return p==itemDef[0];}
static float nativeLow[3]{},nativeHigh[3]{};static int boundsCalls=0;
static std::function<void()> onBounds;
static bool bp_item_bounds_raw(uint8_t*,float* lo,float* hi){++boundsCalls;if(onBounds)onBounds();memcpy(lo,nativeLow,12);memcpy(hi,nativeHigh,12);return nativeBoundsAvailable;}
#include "../src/native_snapshot_runtime.h"
alignas(16) static uint8_t systemData[16],frame[0x68],world[0x838],otherWorld[0x838],base[0x1a0],arch[0xf84];
struct alignas(16) Entity{alignas(8) uint8_t e[0x38]{};alignas(8) uint8_t data[64]{};};
static std::vector<Entity> entities;static std::vector<uint64_t> occupancy;static std::vector<uint32_t> keys;static std::vector<uint8_t*> values;
static uint16_t transformIndex=125;static uint32_t capacity;
static int failures=0,checks=0;
#define CHECK(c) do{++checks;if(!(c)){++failures;printf("FAIL %d: %s\n",__LINE__,#c);}}while(0)
static uint32_t slot_of(int i){return (uint32_t)(i*17)&(capacity-1);}
static void add(int i){auto s=slot_of(i);occupancy[s>>6]|=1ull<<(s&63);values[s]=entities[i].e;keys[s]=0x101+i;}
static void setup(int count=1,uint32_t slots=1024){
    capacity=slots;entities.assign(count+1,{});occupancy.assign((slots+63)/64,0);keys.assign(slots,0);values.assign(slots,nullptr);
    memset(base,0,sizeof base);memset(arch,0,sizeof arch);onBounds={};
    ranges={{systemData,sizeof systemData},{frame,sizeof frame},{world,sizeof world},{otherWorld,sizeof otherWorld},{base,sizeof base},{arch,sizeof arch},
        {(uint8_t*)entities.data(),entities.size()*sizeof(Entity)},{(uint8_t*)occupancy.data(),occupancy.size()*8},
        {(uint8_t*)keys.data(),keys.size()*4},{(uint8_t*)values.data(),values.size()*8},{(uint8_t*)&transformIndex,2},{(uint8_t*)itemDef,sizeof itemDef}};
    *(uint8_t**)systemData=world;*(uint8_t**)(world+0x30)=base;*(uint8_t**)(otherWorld+0x30)=base;*(uint16_t**)(frame+0x60)=&transformIndex;
    *(uint64_t**)(base+0x150)=occupancy.data();*(uint64_t*)(base+0x158)=slots;*(uint32_t**)(base+0x170)=keys.data();
    *(uint8_t***)(base+0x188)=values.data();*(uint32_t*)(base+0x19c)=count;
    *(uint64_t*)(arch+(125>>6)*8)|=1ull<<(125&63);*(uint64_t*)(arch+(610>>6)*8)|=1ull<<(610&63);
    *(uint16_t*)(arch+0xa84+125*2)=56;*(uint16_t*)(arch+0xa84+610*2)=4;*(uint16_t*)(arch+0x84+610*2)=56;
    for(int i=0;i<=count;++i){auto& r=entities[i];*(uint32_t*)(r.e+0x10)=0x101+i;*(uint8_t**)(r.e+0x18)=arch;*(uint8_t**)(r.e+0x20)=r.data;
        *(float*)(r.data+36)=1;for(int a=0;a<3;++a)*(float*)(r.data+40+a*4)=1;*(uint32_t*)(r.data+56)=PropId;
        if(i<count)add(i);}
    *(uint32_t*)itemDef[0]=PropId;*(uint32_t*)itemDef[1]=NonPropId;
    for(int a=0;a<3;++a){nativeLow[a]=0;nativeHigh[a]=1;}
    clockTick=100;tickIncrement=0;g_bpProbeSeen=100;g_bpSnapshotState=0;g_bpSnapshotCount=0;g_bpWorldAbi=true;nativeBoundsAvailable=true;g_bpItemComp=610;boundsCalls=0;
}
static props::Bounds region{{-2,-1,-2},{2,2,2}};
static void start(){CHECK(bp_snapshot_request(region));}
static void step(){bp_snapshot_step(systemData,frame);}
static void far(int i){*(int64_t*)entities[i].data=1000ll*core::kFix;}
int main(){
    setup();nativescan::ReadCache permissions;
    CHECK(permissions.check(entities[0].data,56,bp_snapshot_memory_region));const auto queries=permissions.queries;
    CHECK(permissions.check(entities[0].data+24,16,bp_snapshot_memory_region)&&permissions.queries==queries);
    CHECK(!permissions.check(nullptr,1,bp_snapshot_memory_region));
    CHECK(!permissions.check((void*)(uintptr_t(-2)),8,bp_snapshot_memory_region));
    CHECK(!permissions.check(entities.data(),entities.size()*sizeof(Entity)+1,bp_snapshot_memory_region));
    auto savedRanges=ranges;ranges.clear();permissions.reset();CHECK(!permissions.check(entities[0].data,56,bp_snapshot_memory_region));ranges=savedRanges;
    // Complete empty collection, and empty placement zone with distant furniture.
    setup(0);start();step();CHECK(g_bpSnapshotState==3&&g_bpSnapshotCount==0);CHECK(g_bpSnapshotTrace.visited==0);
    setup(12);for(int i=0;i<12;++i)far(i);start();step();CHECK(g_bpSnapshotState==3&&g_bpSnapshotCount==0);CHECK(g_bpSnapshotTrace.visited==12);
    setup();start();step();CHECK(g_bpSnapshotState==3&&g_bpSnapshotCount==1);CHECK(g_bpSnapshot[0].entity==0x101);
    CHECK(props::intersects(g_bpSnapshot[0].box,region)); // confirmed proxy collision, separately from reading
    setup();*(int64_t*)entities[0].data=core::kFix*3/2;start();step();
    const props::Bounds target{{-.5,0,-.5},{.5,1,.5}};
    CHECK(g_bpSnapshotState==3&&g_bpSnapshotCount==1&&!props::intersects(target,g_bpSnapshot[0].box)); // near, but no collision
    setup();nativeLow[0]=-5;nativeHigh[0]=5;*(int64_t*)entities[0].data=4ll*core::kFix;start();step();CHECK(g_bpSnapshotCount==1); // pivot outside, box inside
    setup();*(float*)(entities[0].data+40)=3;start();step();CHECK(g_bpSnapshot[0].box.hi[0]==3);
    setup();nativeHigh[0]=nativeHigh[1]=nativeHigh[2]=0;far(0);start();step();CHECK(g_bpSnapshotState==3&&g_bpSnapshotCount==0&&g_bpSnapshotTrace.excludedDegenerate==1);
    setup();nativeHigh[0]=nativeHigh[1]=nativeHigh[2]=0;start();step();CHECK(g_bpSnapshotState==-1&&g_bpSnapshotTrace.error==nativescan::DegenerateNear);CHECK(g_bpSnapshotCount==0);
    setup();nativeHigh[0]=std::numeric_limits<float>::quiet_NaN();far(0);start();step();CHECK(g_bpSnapshotTrace.error==nativescan::BoundsCorrupt&&g_bpSnapshotCount==0);
    setup();nativeBoundsAvailable=false;start();step();CHECK(g_bpSnapshotTrace.error==nativescan::ItemBoundsMissing);
    setup();*(float*)(entities[0].data+40)=std::numeric_limits<float>::quiet_NaN();start();step();CHECK(g_bpSnapshotTrace.error==nativescan::ScaleCorrupt);
    setup();*(float*)(entities[0].data+36)=0;start();step();CHECK(g_bpSnapshotTrace.error==nativescan::TransformCorrupt);
    setup();*(uint32_t*)(entities[0].e+0x30)=0xffffffff;start();step();CHECK(g_bpSnapshotTrace.error==nativescan::ComponentUnreadable);
    setup();g_bpWorldAbi=false;CHECK(!bp_snapshot_request(region));CHECK(g_bpSnapshotTrace.error==nativescan::AbiUnsupported);
    setup();g_bpItemComp=0xffff;start();step();CHECK(g_bpSnapshotState==3&&g_bpItemComp==610&&g_bpSnapshotTrace.discovery==1);
    setup();*(uint32_t*)(entities[0].data+56)=0xdeadbeef;start();step();CHECK(g_bpSnapshotTrace.error==nativescan::ItemUnresolved&&g_bpSnapshotCount==0);
    // Mutations WITHIN one callback discard the entire working result.
    setup();onBounds=[](){*(uint32_t*)(base+0x19c)=2;add(1);};start();step();
    CHECK(g_bpSnapshotState==1&&g_bpSnapshotCount==0&&g_bpSnapshotTrace.change==nativescan::Count);
    CHECK(g_bpSnapshotTrace.before.count==1&&g_bpSnapshotTrace.after.count==2&&g_bpSnapshotTrace.restarts==1);
    onBounds={};step();CHECK(g_bpSnapshotState==3&&g_bpSnapshotCount==2&&g_bpSnapshotTrace.attempts==2);
    setup();onBounds=[](){occupancy[0]=0;occupancy[1]=1;};start();step();CHECK(g_bpSnapshotState==1&&g_bpSnapshotTrace.change==nativescan::Occupancy&&g_bpSnapshotCount==0);
    setup();onBounds=[](){values[0]=entities[1].e;keys[0]=0x102;};start();step();CHECK(g_bpSnapshotState==1&&g_bpSnapshotTrace.change==nativescan::Entry&&g_bpSnapshotCount==0);
    CHECK(!strcmp(g_bpSnapshotTrace.field,"entity_pointer")&&g_bpSnapshotTrace.expected==(uint64_t)entities[0].e&&g_bpSnapshotTrace.observed==(uint64_t)entities[1].e);
    setup();onBounds=[](){*(int64_t*)entities[0].data+=core::kFix;};start();step();CHECK(g_bpSnapshotState==1&&g_bpSnapshotTrace.change==nativescan::Transform&&g_bpSnapshotCount==0);
    step();step();CHECK(g_bpSnapshotState==-1&&g_bpSnapshotTrace.error==nativescan::TableChanged&&g_bpSnapshotTrace.restarts==2);CHECK(g_bpSnapshotCount==0);
    setup();onBounds=[](){*(uint32_t*)(entities[0].data+56)=NonPropId;};start();step();CHECK(g_bpSnapshotState==1&&g_bpSnapshotTrace.change==nativescan::Item);
    setup();onBounds=[](){*(uint8_t**)systemData=otherWorld;};start();step();CHECK(g_bpSnapshotState==1&&g_bpSnapshotTrace.change==nativescan::World);
    // A reallocation must be noticed before touching a pointer from the old
    // values buffer. ASan checks this case with the old buffer really freed.
    {setup(2);std::vector<uint8_t*> replacement;
        onBounds=[&](){if(!replacement.empty())return;replacement=values;*(uint8_t***)(base+0x188)=replacement.data();
            ranges.push_back({(uint8_t*)replacement.data(),replacement.size()*sizeof(uint8_t*)});values.clear();values.shrink_to_fit();};
        start();step();CHECK(g_bpSnapshotState==1&&g_bpSnapshotTrace.change==nativescan::Values&&g_bpSnapshotCount==0);
    }
    setup();*(uint32_t*)(base+0x19c)=2;start();step();CHECK(g_bpSnapshotTrace.error==nativescan::TableInvalid&&g_bpSnapshotCount==0);
    // Changes BETWEEN completed reads are normal. No previous table is retained.
    setup();start();step();CHECK(g_bpSnapshotState==3);g_bpSnapshotState=0;add(1);*(uint32_t*)(base+0x19c)=2;
    clockTick+=1000;g_bpProbeSeen=clockTick;start();step();CHECK(g_bpSnapshotState==3&&g_bpSnapshotCount==2&&g_bpSnapshotTrace.restarts==0);
    // Cancellation pending, between attempts and after success never publishes partial data.
    setup();start();bp_snapshot_cancel("escape");step();CHECK(g_bpSnapshotState==0&&g_bpSnapshotCount==0&&boundsCalls==0);
    setup();onBounds=[](){*(int64_t*)entities[0].data+=core::kFix;};start();step();bp_snapshot_cancel("escape");step();CHECK(g_bpSnapshotState==0&&g_bpSnapshotCount==0);
    setup();start();step();bp_snapshot_cancel("world_exit");CHECK(g_bpSnapshotState==0&&g_bpSnapshotCount==0);
    setup();onBounds=[](){bp_snapshot_cancel("escape_during_read");};start();step();CHECK(g_bpSnapshotState==0&&g_bpSnapshotCount==0&&g_bpSnapshotTrace.error==nativescan::Cancelled);
    setup();start();tickIncrement=201;step();CHECK(g_bpSnapshotState==-1&&g_bpSnapshotTrace.error==nativescan::SnapshotBudget&&g_bpSnapshotCount==0);
    // Sparse million-slot table: one callback visits occupied entries only.
    setup(60000,1u<<20);for(int i=0;i<60000;++i)far(i);*(int64_t*)entities[0].data=0;
    start();auto startTime=std::chrono::steady_clock::now();step();auto elapsed=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()-startTime).count();
    CHECK(g_bpSnapshotState==3&&g_bpSnapshotCount==1);CHECK(g_bpSnapshotTrace.visited==60000&&g_bpSnapshotTrace.words==16384&&g_bpSnapshotTrace.attempts==1);
    CHECK(g_bpSnapshotReads.queries<32&&g_bpSnapshotReads.hits>60000);
    printf("sparse fixture: 1048576 capacity, %d occupied, %d relevant, one callback, host elapsed=%lld ms (not game timing)\n",g_bpSnapshotTrace.visited,g_bpSnapshotCount,(long long)elapsed);
    setup(513);start();step();CHECK(g_bpSnapshotTrace.error==nativescan::Capacity&&g_bpSnapshotCount==0);
    setup();*(uint32_t*)(base+0x19c)=nativescan::MaxEntities+1;*(uint64_t*)(base+0x158)=1u<<20;start();step();CHECK(g_bpSnapshotTrace.error==nativescan::EntityBudget);
    printf("native snapshot: %d checks, %d failures; synthetic ECS, actual production reader\n",checks,failures);return failures!=0;
}
