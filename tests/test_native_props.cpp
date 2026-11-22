// Synthetic IDs are test doubles, never a catalogue of real game items.
#include "project_export_host.h"
#include "../src/native_furniture.h"
#include "../src/native_item_policy.h"
#include "../src/native_item_index.h"
#include <cmath>
#include <limits>
#define MOD_VERSION "SYNTHETIC-test"
static int failures=0, assertions=0;
#define CHECK(c) do { ++assertions; if(!(c)){std::printf("FAIL line %d: %s\n",__LINE__,#c);++failures;} }while(0)
static std::vector<uint8_t> read(const fs::path& p){std::ifstream f(p,std::ios::binary);return std::vector<uint8_t>((std::istreambuf_iterator<char>(f)),{});}
static props::Definition definitions[props::RoleCount];
alignas(16) static uint8_t nativeItems[props::RoleCount][0x830]{};
alignas(16) static uint8_t nativeMaterial[0x830]{};
static uint8_t* itemById(uint32_t id,void*){if(id==0xf1000000)return nativeMaterial;for(int i=0;i<props::RoleCount;++i)if(id==definitions[i].item)return nativeItems[i];return nullptr;}
static uint8_t* (*g_bpItemById)(uint32_t,void*)=itemById;
static void* g_bpItemReg=nativeItems;
static bool prop_item(const uint8_t* it){return it!=nullptr&&it!=nativeMaterial;}
static bool bp_block_item(const uint8_t* it){return it==nativeMaterial;}
static bool project_prop_item(const uint8_t* it){return prop_item(it);}
static const char* bp_item_name(uint32_t){return "SYNTHETIC \"quoted\" object";}
static struct {uint32_t item;int kind;} g_catalog[8];static int g_catalogCount=8;
static bool g_catalogTruncated=false,g_nativeObjectAbi=true;
static int g_projectBlueprintLogic=0;static uint32_t g_catalogMaterial=0xf1000000,g_furnitureSelected[props::RoleCount]{};
static const char* g_furnitureRoles[]={"bed","table","chair","wall_torch","painting","chest","small_decoration","floor_decoration"};
static bool bp_logic_item(const uint8_t* it){return (*(const uint32_t*)(it+0x5d8)&0x80000)!=0;}
static bool bp_logic_object(void*,const uint8_t*){return false;}
static int boundsCalls=0;
static bool bp_item_bounds(uint8_t* it,float* lo,float* hi){++boundsCalls;for(int i=0;i<props::RoleCount;++i)if(it==nativeItems[i]){memcpy(lo,definitions[i].low,12);memcpy(hi,definitions[i].high,12);return true;}return false;}
static bool bp_item_bounds_raw(uint8_t* it,float* lo,float* hi){return bp_item_bounds(it,lo,hi);}
static bool readable(const void* p,size_t){return p!=nullptr;}
static nativecatalog::Outcomes g_nativeItemOutcomes;
#include "../src/native_item_resolution.h"
static const char* native_catalog_image(){return "{\"synthetic\":true}";}
#include "../src/native_catalog_export.h"
struct BpEventBuf{uint8_t* data;uint64_t cap;uint32_t used,pad;};
alignas(16) static uint8_t g_bpEvData[512]{},g_bpEvWriter[0x200]{};
static BpEventBuf g_bpEvBuf{};static int g_bpObjRecord=0;
static uint32_t dispatchedItem=0;static uint8_t dispatchedInfo[0x50];
static bool nativeSuccess=true,sawPrivateWriter=false;
static uint8_t nativePlace(void* ctx,uint8_t* info,uint32_t item){
    CHECK(g_bpObjRecord==1);dispatchedItem=item;memcpy(dispatchedInfo,info,0x50);
    sawPrivateWriter=*(void**)((uint8_t*)ctx+0xe0)==g_bpEvWriter;
    return nativeSuccess;
}
static uint8_t (*g_placePropOrig)(void*,uint8_t*,uint32_t)=nativePlace;
// Compile the original bridge, not a second implementation of dispatch.
#include "../src/blueprint_object_place.h"
static void init(){
    const float sizes[8][3]={{2,1,3},{2,1,1},{.6f,1,.6f},{.3f,1,.3f},{2,1.2f,.1f},{1,.8f,1},{.2f,.3f,.2f},{.5f,1,.5f}};
    for(int i=0;i<8;++i){auto& d=definitions[i];d.item=0xf0000000u+i;
        // Intentionally off-centre native pivots: the arrangement uses bounds.
        for(int a=0;a<3;++a){d.low[a]=a==1?-.125f:-sizes[i][a]*.5f+.125f;d.high[a]=d.low[a]+sizes[i][a];}
        memset(nativeItems[i]+0x3c0,0x40+i,16);
        *(uint32_t*)nativeItems[i]=d.item;
        g_catalog[i]={d.item,1};g_furnitureSelected[i]=d.item;
    }
    nativeMaterial[0x454]=0x9d;
    *(uint32_t*)nativeMaterial=0xf1000000;
}
static void geometry(){
    props::Demo d;CHECK(props::furniture_demo(definitions,&d));CHECK(d.count==11&&d.badRole==-1);
    int order[11];CHECK(props::order(d.boxes,11,order));
    CHECK(props::parent(d.boxes,11,8)==1&&props::parent(d.boxes,11,9)==1);
    for(int i=0;i<11;++i){CHECK(order[i]<11);for(int j=0;j<i;++j)CHECK(!props::intersects(d.boxes[i],d.boxes[j]));}
    double p[3];for(int i=0;i<5;++i){props::face_probe(d.boxes[0],2,i,p);CHECK(std::abs(p[1]+props::Contact)<1e-6);}
    CHECK(std::abs(d.boxes[4].lo[0]-.5)<1e-5);CHECK(std::abs(d.boxes[6].hi[2]-9.5)<1e-5);
    CHECK(props::supported(d.boxes[0],d.boxes,11,0,nullptr,0,[](const double* p){return p[1]<0;}));
    CHECK(!props::supported(d.boxes[0],d.boxes,11,0,nullptr,0,[](const double*){return false;}));
    CHECK(props::supported(d.boxes[4],d.boxes,11,4,nullptr,0,[](const double* p){return p[0]<.5;}));
    CHECK(props::supported(d.boxes[8],d.boxes,11,8,nullptr,0,[](const double*){return false;}));
    props::Bounds reversed[]={d.boxes[8],d.boxes[1]};CHECK(!props::supported(reversed[0],reversed,2,0,nullptr,0,[](const double*){return false;}));
    props::WorldObject table{definitions[1].item,d.boxes[1]};CHECK(props::supported(d.boxes[8],d.boxes+8,1,0,&table,1,[](const double*){return false;}));
    CHECK(!props::supported(d.boxes[0],d.boxes,11,0,nullptr,0,[](const double* p){return p[1]<0&&p[0]>2;})); // partial floor
    auto duplicate=d.boxes[0];CHECK(props::intersects(duplicate,d.boxes[0]));
    props::Bounds touch{{0,0,0},{1,1,1}},next{{1,0,0},{2,1,1}};CHECK(!props::intersects(touch,next));
    touch.hi[0]=1.1;CHECK(props::intersects(touch,next));
    auto bad=definitions[0];definitions[0].item=0;CHECK(!props::furniture_demo(definitions,&d)&&d.badRole==0);definitions[0]=bad;
    definitions[0].high[0]=50;CHECK(!props::furniture_demo(definitions,&d)&&d.badRole==0);definitions[0]=bad;
    definitions[0].low[0]=std::numeric_limits<float>::quiet_NaN();CHECK(!props::furniture_demo(definitions,&d));definitions[0]=bad;
    props::Bounds b;const double zero[]={0,0,0};const float invalid[]={0,0,0,0};CHECK(!props::bounds(zero,invalid,bad.low,bad.high,&b));
    CHECK(!extended::object_flags_allowed(0x80000));CHECK(extended::object_flags_allowed(0x80000,true));CHECK(!extended::object_flags_allowed(0x80001,true));
}
static void roundtrip_and_dispatch(){
    props::Demo demo;CHECK(props::furniture_demo(definitions,&demo));
    std::vector<project::CellEntry> source(10000);g_projectPlan={};g_projectPlan.cells=source.data();g_projectPlan.capacity=source.size();
    g_projectPlan.materialCount=1;g_projectPlan.materials[0]={0xf1000000,0x9d,0,0}; // test material, too
    CHECK(props::demo_room([&](int x,int y,int z){source[g_projectPlan.count++]={{x,y,z},0,0,extended::Wall};return true;}));
    project::Error error{};CHECK(project::finish(&g_projectPlan,&error));int cells=g_projectPlan.count;CHECK(cells==1692);
    g_projectManifest={};strcpy(g_projectManifest.name,"SYNTHETIC native prop integration test");g_projectManifest.grid=8;g_projectManifest.count=2;
    g_projectManifest.spaceCount=1;g_projectManifest.spaces[0]={{80,0,8},{112,48,136}};
    g_projectObjectCount=demo.count;memcpy(g_projectObjects,demo.objects,sizeof demo.objects);
    fs::path parent=fs::current_path()/"build";fs::create_directories(parent);
    folder=parent/("native-props-test-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    char filename[128];CHECK(project_export(filename,sizeof filename));
    auto encoded=read(folder/filename);project::Manifest parsed{};CHECK(project::decode((char*)encoded.data(),encoded.size(),&parsed,&error));
    std::vector<project::CellEntry> restored(10000);project::Plan plan{restored.data(),(int)restored.size()};
    std::vector<extended::Object> objects;int objectFiles=0;
    for(int m=0;m<parsed.count;++m){auto bp=read(folder/parsed.modules[m].file);core::BpHeader h{};std::vector<uint8_t> decoded(65536);
        CHECK(core::bp_decode(bp.data(),bp.size(),&h,decoded.data(),decoded.size()));CHECK(project::add(&plan,parsed.modules[m],m,h,decoded.data(),&error));
        if(h.objects){++objectFiles;CHECK(h.nm==0&&h.objects==11&&decoded[0]==core::BP_EMPTY);}
        for(int i=0;i<h.objects;++i){extended::Object o;CHECK(extended::transform_object(parsed.modules[m],m,h,h.obj[i],&o));objects.push_back(o);}
    }
    CHECK(project::finish(&plan,&error));CHECK(plan.count==cells&&objects.size()==11&&objectFiles==1);
    alignas(16) uint8_t context[0x200]{};props::Counts count;
    for(int i=0;i<11;++i){const auto& o=objects[i];CHECK(o.item==demo.objects[i].item);
        for(int a=0;a<3;++a)CHECK(std::abs(o.p[a]-demo.objects[i].p[a])<1e-6);
        for(int a=0;a<4;++a)CHECK(o.rotation[a]==demo.objects[i].rotation[a]);
        count.result(bp_place_object(context,o.item,o.p,o.rotation));CHECK(dispatchedItem==o.item);
        auto it=itemById(o.item,nullptr);CHECK(!memcmp(dispatchedInfo,it+0x3c0,16));CHECK(!memcmp(dispatchedInfo+0x1c,o.rotation,16));
    }
    CHECK(count.placed==11&&count.failed==0&&count.attempted==11&&g_bpObjRecord==0);
    nativeSuccess=false;count.result(bp_place_object(context,objects[0].item,objects[0].p,objects[0].rotation));nativeSuccess=true;
    CHECK(count.placed==11&&count.failed==1&&count.attempted==12);
    auto savedHigh=definitions[0].high[0];definitions[0].high[0]=definitions[0].low[0];auto attempts=g_nativeItemOutcomes.find(definitions[0].item)->accepted;
    CHECK(!bp_place_object(context,objects[0].item,objects[0].p,objects[0].rotation));CHECK(g_nativeItemOutcomes.find(definitions[0].item)->accepted==attempts);definitions[0].high[0]=savedHigh;
    auto before=dispatchedItem;CHECK(!bp_place_object(context,0xdeadbeef,objects[0].p,objects[0].rotation));CHECK(dispatchedItem==before);
    *(uint32_t*)(nativeItems[0]+0x5d8)=0x80000;
    CHECK(bp_place_object(context,objects[0].item,objects[0].p,objects[0].rotation));CHECK(sawPrivateWriter);CHECK(*(void**)(context+0xe0)==nullptr);
    void* existing=nativeItems;*(void**)(context+0xe0)=existing;
    CHECK(bp_place_object(context,objects[0].item,objects[0].p,objects[0].rotation));CHECK(!sawPrivateWriter&&*(void**)(context+0xe0)==existing);
    *(uint32_t*)(nativeItems[0]+0x5d8)=0;
    // Objects-only decoration uses exactly the same exporter/format, no voxels.
    ++tick;g_projectPlan.count=0;g_projectPlan.materialCount=0;g_projectManifest.spaceCount=0;g_projectManifest.anchor[0]=160;
    CHECK(project_export(filename,sizeof filename));encoded=read(folder/filename);
    CHECK(project::decode((char*)encoded.data(),encoded.size(),&parsed,&error));CHECK(parsed.count==1&&parsed.anchor[0]==160&&parsed.pieceCount==0);
}
static void original_capture_format(){
    // Same capture helper as bp_save_write, encoded by the original BTBP v1 codec.
    const double origin[]={-45.5,127,200};const int64_t fixed[]={int64_t(-43.25*core::kFix),int64_t(128.5*core::kFix),int64_t(205.125*core::kFix)};
    const float tilted[]={.70710677f,0,0,.70710677f};core::BpHeader h{};h.n[0]=h.n[1]=h.n[2]=1;h.step[0]=h.step[1]=h.step[2]=.5;h.objects=1;
    CHECK(props::capture(&h.obj[0],definitions[0].item,fixed,tilted,origin));CHECK(h.obj[0].pos[0]==2.25f&&h.obj[0].pos[2]==5.125f);
    uint8_t encoded[2048],empty=core::BP_EMPTY,decoded;int size=core::bp_encode(h,&empty,encoded,sizeof encoded);CHECK(size>0);core::BpHeader back{};
    CHECK(core::bp_decode(encoded,size,&back,&decoded,1));CHECK(!memcmp(&h.obj[0],&back.obj[0],sizeof(core::BpObject)));
    project::Module m{};extended::Object o;CHECK(extended::transform_object(m,0,back,back.obj[0],&o));
    for(int turn=0;turn<4;++turn){double world[3];float q[4];const int pivot[]={0,0,0};core::I3 anchor{int64_t(origin[0]*core::kFix),int64_t(origin[1]*core::kFix),int64_t(origin[2]*core::kFix)};
        extended::object_world(o,pivot,turn,anchor,world,q);alignas(16) uint8_t ctx[0x200]{};CHECK(bp_place_object(ctx,o.item,world,q));CHECK(!memcmp(q,dispatchedInfo+0x1c,16));
    }
    float bad[]={0,0,0,0};CHECK(!props::capture(&h.obj[0],definitions[0].item,fixed,bad,origin));CHECK(!props::capture(&h.obj[0],0,fixed,tilted,origin));
}
static void catalogue(){
    ++tick;char name[128];CHECK(native_catalog_export(name,sizeof name));auto json=read(folder/name);std::string s(json.begin(),json.end());
    CHECK(s.find("\\\"quoted\\\"")!=std::string::npos);CHECK(s.find("\"complete\":true")!=std::string::npos);
    CHECK(s.find("\"structureMaterial\":{\"item\":4043309056,\"byte\":157}")!=std::string::npos);
    CHECK(s.find("\"bed\":4026531840")!=std::string::npos);
    auto original=json;CHECK(!native_catalog_export(name,sizeof name));CHECK(read(folder/name)==original);
    ++tick;g_catalogTruncated=true;CHECK(!native_catalog_export(name,sizeof name));g_catalogTruncated=false;
    failWrite=0;CHECK(!native_catalog_export(name,sizeof name));failWrite=-1;
    int count=0;for(auto& p:fs::directory_iterator(folder))if(p.path().filename().string().find("NativeCatalog_")==0&&p.path().extension()==".json")++count;
    CHECK(count==1);++tick;failFlush=0;CHECK(!native_catalog_export(name,sizeof name));failFlush=-1;
    g_nativeObjectAbi=false;CHECK(!native_catalog_export(name,sizeof name));g_nativeObjectAbi=true;
}
static void resolution_tests(){
    CHECK(native_item_resolution(definitions[0].item)==nativecatalog::Ready);
    CHECK(native_item_resolution(0xdeadbeef)==nativecatalog::Unknown);
    *(uint32_t*)nativeItems[0]=999;CHECK(native_item_resolution(definitions[0].item)==nativecatalog::Unknown);*(uint32_t*)nativeItems[0]=definitions[0].item;
    nativeItems[0][0x427]=1;CHECK(native_item_resolution(definitions[0].item)==nativecatalog::Altar);nativeItems[0][0x427]=0;
    int before=boundsCalls;
    *(uint32_t*)(nativeItems[0]+0x5d8)=1;CHECK(native_item_resolution(definitions[0].item)==nativecatalog::Special);CHECK(boundsCalls==before);
    *(uint32_t*)(nativeItems[0]+0x5d8)=0x80000;CHECK(native_item_resolution(definitions[0].item)==nativecatalog::LogicDisabled);CHECK(boundsCalls==before);
    g_projectBlueprintLogic=1;CHECK(native_item_resolution(definitions[0].item)==nativecatalog::Ready);g_projectBlueprintLogic=0;*(uint32_t*)(nativeItems[0]+0x5d8)=0;
    auto old=definitions[0].high[0];definitions[0].high[0]=definitions[0].low[0];CHECK(native_item_resolution(definitions[0].item)==nativecatalog::InvalidBounds);definitions[0].high[0]=old;
    g_nativeObjectAbi=false;CHECK(native_item_resolution(definitions[0].item)==nativecatalog::AbiUnavailable);g_nativeObjectAbi=true;
    auto registry=g_bpItemReg;g_bpItemReg=nullptr;CHECK(native_item_resolution(definitions[0].item)==nativecatalog::Unobserved);g_bpItemReg=registry;
    CHECK(native_item_resolution(0xf1000000)==nativecatalog::NotProp);
    CHECK(g_nativeItemOutcomes.find(definitions[0].item)->accepted>0);CHECK(g_nativeItemOutcomes.find(definitions[0].item)->failed==1);
    CHECK(itemindex::Count==3526);CHECK(itemindex::find(0x566c8dfe)->item==0x566c8dfe);CHECK(itemindex::contains("Crude Wooden Bed","wooden BED"));CHECK(!itemindex::contains(nullptr,"bed"));
    for(int i=1;i<itemindex::Count;++i)CHECK(itemindex::Entries[i-1].item<itemindex::Entries[i].item);
}
int main(){init();geometry();roundtrip_and_dispatch();original_capture_format();catalogue();resolution_tests();std::printf("native props: %d assertions, %d failures (synthetic registry; no live game claim)\n",assertions,failures);return failures!=0;}
