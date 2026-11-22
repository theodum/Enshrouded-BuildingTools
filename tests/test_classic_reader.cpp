// Historical capture/components and its production collision visitor.
// Bounded synthetic ECS, actual user BTBP. Does not run the game.
#include "../src/native_snapshot.h"
#include "../src/blueprint_object_data.h"
#include <cstdio>
#include <cstring>
#include <fstream>
#include <vector>
#include <functional>
using DWORD=uint32_t;using LONG=int32_t;
#define LOG(expr) do{}while(0)
static DWORD tick=100,increment=0;static DWORD GetTickCount(){tick+=increment;return tick;}
struct Range{const uint8_t* p;size_t size;};static std::vector<Range> ranges;
static bool readable(const void* p,size_t size){auto address=(uintptr_t)p;for(auto r:ranges){auto start=(uintptr_t)r.p;if(address>=start&&address-start<=r.size&&size<=r.size-(address-start))return true;}return false;}
static bool bp_snapshot_memory_region(const void* p,uintptr_t* begin,uintptr_t* end){auto address=(uintptr_t)p;for(auto r:ranges){auto start=(uintptr_t)r.p;if(address>=start&&address-start<r.size){*begin=start;*end=start+r.size;return true;}}return false;}
alignas(16) static uint8_t definitions[4][0x830]{};
static const uint32_t ids[]={0x58bbb7d6,0xaf3025e4,0x301dfbe1,0xc672cf0d};
static uint8_t* lookup(uint32_t id,void*){for(int i=0;i<4;++i)if(id==ids[i])return definitions[i];return nullptr;}
static auto g_bpItemById=lookup;static void* g_bpItemReg=definitions;
static bool prop_item(const uint8_t* item){return item!=nullptr;}
static uint32_t g_bpItemComp=0xffffffff;
#include "../src/blueprint_object_components.h"
static BpFound g_bpFound[core::BP_MAX_OBJS];static int g_bpFoundN=0;static bool g_bpFoundOverflow=false;
static float g_bpProbeBox[8]{};
#include "../src/blueprint_object_scan.h"
static int g_bpSnapshotState=0,g_bpSnapshotCount=0,g_bpProbeHits=1,g_bpProbeReq=0,g_bpScanning=0,g_bpProbeLive=0;
static bool g_bpWorldAbi=true;static void* g_bpQResults=(void*)1;
static DWORD g_bpProbeSeen=100,g_bpSnapshotTick=0,g_bpSnapshotCompleted=0;
static uint32_t g_bpSnapshotBadItem=0;static props::Bounds g_bpSnapshotRegion;static nativescan::Trace g_bpSnapshotTrace;
static props::WorldObject g_bpSnapshot[512];static bool bp_snapshot_fail(nativescan::Error,uint32_t=0,uint32_t=0);
static uint32_t zeroItem=0;static bool noBounds=false;static std::function<void()> onBounds;
static bool bp_item_bounds_raw(uint8_t* item,float* low,float* high){
    if(onBounds)onBounds();for(int a=0;a<3;++a){low[a]=-.2f;high[a]=.2f;}low[1]=0;
    if(*(uint32_t*)item==zeroItem)for(int a=0;a<3;++a)low[a]=high[a]=0;
    return !noBounds;
}
#include "../src/native_snapshot_runtime.h"
#include "../src/blueprint_classic_collision.h"
alignas(16) static uint8_t systemData[16]{},frame[0x68]{},world[0x838]{},base[0x1a0]{},spatialArch[0xf84]{},tagArch[0xf84]{};
struct alignas(16) Entity{uint8_t e[0x38]{};alignas(8) uint8_t data[64]{};};
static Entity entities[7];static uint64_t bits[16];static uint32_t keys[1024];static uint8_t* values[1024];static uint16_t transformIndex=133;
static core::BpHeader source;static int checks=0,failures=0;
#define CHECK(c) do{++checks;if(!(c)){++failures;printf("FAIL %d: %s\n",__LINE__,#c);}}while(0)
static void setup(){
    memset(base,0,sizeof base);memset(spatialArch,0,sizeof spatialArch);memset(tagArch,0,sizeof tagArch);memset(entities,0,sizeof entities);memset(bits,0,sizeof bits);memset(keys,0,sizeof keys);memset(values,0,sizeof values);
    ranges={{systemData,sizeof systemData},{frame,sizeof frame},{world,sizeof world},{base,sizeof base},{spatialArch,sizeof spatialArch},{tagArch,sizeof tagArch},
        {(uint8_t*)entities,sizeof entities},{(uint8_t*)bits,sizeof bits},{(uint8_t*)keys,sizeof keys},{(uint8_t*)values,sizeof values},
        {(uint8_t*)&transformIndex,2},{(uint8_t*)definitions,sizeof definitions}};
    *(uint8_t**)systemData=world;*(uint8_t**)(world+0x30)=base;*(uint16_t**)(frame+0x60)=&transformIndex;
    *(uint64_t**)(base+0x150)=bits;*(uint64_t*)(base+0x158)=1024;*(uint32_t**)(base+0x170)=keys;*(uint8_t***)(base+0x188)=values;*(uint32_t*)(base+0x19c)=6;
    // A zero-sized tag precedes UsedItem. Original discovery correctly skips it.
    *(uint64_t*)(spatialArch+(17>>6)*8)|=1ull<<(17&63);*(uint64_t*)(tagArch+(17>>6)*8)|=1ull<<(17&63);
    *(uint64_t*)(spatialArch+(133>>6)*8)|=1ull<<(133&63);*(uint16_t*)(spatialArch+0xa84+133*2)=56;
    *(uint64_t*)(spatialArch+(610>>6)*8)|=1ull<<(610&63);*(uint16_t*)(spatialArch+0xa84+610*2)=4;*(uint16_t*)(spatialArch+0x84+610*2)=56;
    for(int i=0;i<6;++i){auto& entity=entities[i];values[i]=entity.e;keys[i]=0x100+i;bits[0]|=1ull<<i;
        *(uint32_t*)(entity.e+0x10)=keys[i];*(uint8_t**)(entity.e+0x18)=i?spatialArch:tagArch;*(uint8_t**)(entity.e+0x20)=entity.data;
        if(i){const auto& object=source.obj[i-1];const double offset[]={100,50,200};
            for(int a=0;a<3;++a)*(int64_t*)(entity.data+a*8)=(int64_t)((offset[a]+object.pos[a])*core::kFix);
            memcpy(entity.data+24,object.rot,16);for(int a=0;a<3;++a)*(float*)(entity.data+40+a*4)=1;
            *(uint32_t*)(entity.data+56)=object.item;*(uint8_t**)(entity.e+0x28)=lookup(object.item,nullptr)+0x3c0;
        }
    }
    for(int i=0;i<4;++i){*(uint32_t*)definitions[i]=ids[i];memset(definitions[i]+0x3c0,i+1,16);}
    const float low[]={100,50,200},high[]={104,54,204};for(int a=0;a<3;++a){g_bpProbeBox[a]=low[a];g_bpProbeBox[a+4]=high[a];}
    tick=100;increment=0;g_bpItemComp=0xffffffff;g_bpFoundN=0;g_bpFoundOverflow=false;g_bpSnapshotState=0;g_bpSnapshotCount=0;g_bpProbeSeen=100;g_bpWorldAbi=true;zeroItem=0;noBounds=false;onBounds={};
}
static props::Bounds region{{100,50,200},{104,54,204}};
static void collect(const props::Bounds& box=region){CHECK(bp_classic_snapshot_request(box));bp_classic_snapshot_step(systemData,frame);}
int main(){
    std::ifstream f("examples/dev12/Dev12_Table_Classique_Source.btbp",std::ios::binary);std::vector<uint8_t> raw((std::istreambuf_iterator<char>(f)),{}),cells(65536);
    CHECK(core::bp_decode(raw.data(),raw.size(),&source,cells.data(),cells.size()));
    setup();CHECK(bp_scan_entities(systemData,frame));CHECK(g_bpFoundN==5&&!g_bpFoundOverflow&&g_bpItemComp==610);
    const double origin[]={100,50,200};
    for(int i=0;i<5;++i){core::BpObject captured{};CHECK(props::capture(&captured,g_bpFound[i].item,g_bpFound[i].pos,g_bpFound[i].rot,origin));CHECK(!memcmp(&captured,&source.obj[i],sizeof captured));}
    collect();CHECK(g_bpSnapshotState==3&&g_bpSnapshotCount==5&&g_bpSnapshotTrace.classicReader);CHECK(g_bpSnapshotTrace.visited==6);CHECK(g_bpFoundN==5);
    setup();collect(props::Bounds{{0,0,0},{5,5,5}});CHECK(g_bpSnapshotState==3&&g_bpSnapshotCount==0);
    setup();zeroItem=ids[0];collect(props::Bounds{{0,0,0},{5,5,5}});CHECK(g_bpSnapshotState==3&&g_bpSnapshotCount==0&&g_bpSnapshotTrace.excludedDegenerate==1);
    setup();zeroItem=ids[0];collect();CHECK(g_bpSnapshotState==-1&&g_bpSnapshotTrace.error==nativescan::DegenerateNear&&g_bpSnapshotCount==0);
    setup();noBounds=true;collect();CHECK(g_bpSnapshotState==-1&&g_bpSnapshotTrace.error==nativescan::ItemBoundsMissing);
    setup();g_bpItemComp=610;*(uint32_t*)(entities[1].data+56)=0xdeadbeef;collect();CHECK(g_bpSnapshotState==-1&&g_bpSnapshotTrace.error==nativescan::ItemUnresolved&&g_bpSnapshotCount==0);
    setup();g_bpItemComp=610;*(uint64_t*)(spatialArch+(133>>6)*8)&=~(1ull<<(133&63));collect();CHECK(g_bpSnapshotState==-1&&g_bpSnapshotTrace.error==nativescan::ComponentUnreadable&&g_bpSnapshotCount==0);
    setup();g_bpItemComp=610;*(uint64_t*)(spatialArch+(133>>6)*8)&=~(1ull<<(133&63));*(uint32_t*)(entities[1].data+56)=0xdeadbeef;collect();CHECK(g_bpSnapshotState==-1&&g_bpSnapshotTrace.error==nativescan::ItemUnresolved&&g_bpSnapshotCount==0);
    setup();*(uint32_t*)(entities[1].e+0x30)=1000000;collect();CHECK(g_bpSnapshotState==-1&&g_bpSnapshotTrace.error==nativescan::ComponentUnreadable&&g_bpSnapshotCount==0);
    setup();onBounds=[](){*(uint32_t*)(base+0x19c)=7;};collect();CHECK(g_bpSnapshotState==-1&&g_bpSnapshotTrace.error==nativescan::TableChanged&&g_bpSnapshotCount==0);
    // Same population count is not sufficient: a prop can enter the target
    // after it was first read, or an item/entity can change without resizing.
    setup();onBounds=[](){for(int a=0;a<3;++a)*(int64_t*)(entities[1].data+a*8)=int64_t(core::kFix);};
    collect(props::Bounds{{0,0,0},{5,5,5}});CHECK(g_bpSnapshotState==-1&&g_bpSnapshotTrace.error==nativescan::TableChanged&&g_bpSnapshotTrace.change==nativescan::Transform&&g_bpSnapshotCount==0);
    setup();onBounds=[done=false]()mutable{if(!done){done=true;std::swap(values[0],values[1]);std::swap(keys[0],keys[1]);}};
    collect();CHECK(g_bpSnapshotState==-1&&g_bpSnapshotTrace.error==nativescan::TableChanged&&g_bpSnapshotTrace.change==nativescan::Entry&&g_bpSnapshotCount==0);
    setup();onBounds=[](){*(uint32_t*)(entities[1].data+56)=ids[2];};collect();
    CHECK(g_bpSnapshotState==-1&&g_bpSnapshotTrace.error==nativescan::TableChanged&&g_bpSnapshotTrace.change==nativescan::Item&&g_bpSnapshotCount==0);
    setup();keys[0]=0xdeadbeef;collect();CHECK(g_bpSnapshotState==-1&&g_bpSnapshotTrace.error==nativescan::TableInvalid&&g_bpSnapshotCount==0);
    setup();onBounds=[](){bp_snapshot_cancel("escape");};collect();CHECK(g_bpSnapshotState==0&&g_bpSnapshotCount==0&&g_bpSnapshotTrace.error==nativescan::Cancelled);
    setup();CHECK(bp_classic_snapshot_request(region));increment=6000;bp_classic_snapshot_step(systemData,frame);CHECK(g_bpSnapshotState==-1&&g_bpSnapshotTrace.error==nativescan::ClassicBudget&&g_bpSnapshotCount==0);
    {
        // The real historical reader, both passes, with a large sparse table.
        // Only relevant props consume the 512-result budget; no game timing claim.
        setup();constexpr size_t count=60000,slots=1u<<20;
        std::vector<Entity> many(count);std::vector<uint64_t> mask(slots/64);
        std::vector<uint32_t> manyKeys(slots);std::vector<uint8_t*> manyValues(slots);
        ranges.push_back({(uint8_t*)many.data(),many.size()*sizeof(Entity)});
        ranges.push_back({(uint8_t*)mask.data(),mask.size()*8});ranges.push_back({(uint8_t*)manyKeys.data(),manyKeys.size()*4});
        ranges.push_back({(uint8_t*)manyValues.data(),manyValues.size()*sizeof(uint8_t*)});
        for(size_t i=0;i<count;++i){auto& entity=many[i];memcpy(entity.e,entities[5].e,sizeof entity.e);memcpy(entity.data,entities[5].data,sizeof entity.data);
            *(uint8_t**)(entity.e+0x20)=entity.data;*(uint32_t*)(entity.e+0x10)=uint32_t(0x10000+i);
            const size_t slot=i*16;manyValues[slot]=entity.e;manyKeys[slot]=uint32_t(0x10000+i);mask[slot/64]|=1ull<<(slot&63);
            for(int a=0;a<3;++a)*(int64_t*)(entity.data+a*8)=int64_t((i?1000.+i:1.)*core::kFix);
        }
        *(uint64_t**)(base+0x150)=mask.data();*(uint64_t*)(base+0x158)=slots;*(uint32_t**)(base+0x170)=manyKeys.data();
        *(uint8_t***)(base+0x188)=manyValues.data();*(uint32_t*)(base+0x19c)=count;
        collect(props::Bounds{{0,0,0},{5,5,5}});CHECK(g_bpSnapshotState==3&&g_bpSnapshotCount==1&&g_bpSnapshotTrace.visited==count&&g_bpFoundN==0);
        for(auto& entity:many)for(int a=0;a<3;++a)*(int64_t*)(entity.data+a*8)=int64_t(core::kFix);
        collect(props::Bounds{{0,0,0},{5,5,5}});CHECK(g_bpSnapshotState==-1&&g_bpSnapshotTrace.error==nativescan::Capacity&&g_bpSnapshotCount==0);
    }
    printf("classic reader: %d checks, %d failures; production capture/components/collision visitor, synthetic ECS\n",checks,failures);return failures!=0;
}
