// Actual composite prop preflight against an independent, read-only voxel stub.
// No simulated query is evidence of native physics, capture or interaction.
#include "project_export_host.h"
#include "../src/native_furniture.h"
#include "../src/native_item_policy.h"
#include "../src/native_snapshot.h"
#include "../src/strings.h"
#include <set>
#include <tuple>
#include <cmath>
using core::I3;using core::CursorXf;
static int failures=0,checks=0;
#define CHECK(c) do{++checks;if(!(c)){std::printf("FAIL %d: %s\n",__LINE__,#c);++failures;}}while(0)
static const char* T(Str key){static const char* values[]={
#define TEST_TEXT(id,key,en) en,
    BT_STRINGS(TEST_TEXT)
#undef TEST_TEXT
};return values[key];}
static struct {char s[24];} number;
static auto num(int n){std::snprintf(number.s,sizeof number.s,"%d",n);return number;}
static int g_projectUseObjects=1,g_projectObjectsInJob=11,g_projectRotation=0;
static struct{bool bpObjects=true;}g_cfg;
static I3 g_projectOrigin{};static props::Counts g_projectPropCounts;
static char g_projectPropsError[512];static const char* g_projectLastProblem=nullptr;
static extended::Box g_projectPieceBoxes[project::MaxPieces];
static int g_bpSnapshotState=0,g_bpSnapshotCount=0;static DWORD g_bpSnapshotTick=0,g_bpSnapshotCompleted=0;
static uint32_t g_bpSnapshotBadItem=0;static bool g_bpSavePending=false;
static props::WorldObject g_bpSnapshot[512];static project::Job g_projectJob;
static nativescan::Trace g_bpSnapshotTrace;
static project::Plan g_projectCapturedSupport{};
static bool project_classic_mode(){return composition::classic(g_projectManifest);}
static bool bp_snapshot_fail(nativescan::Error code,uint32_t=0,uint32_t=0){g_bpSnapshotTrace.error=code;g_bpSnapshotState=-1;return false;}
static void bp_snapshot_cancel(const char*){g_bpSnapshotState=g_bpSnapshotCount=0;g_bpSnapshotCompleted=0;}
static const char* lastTraceCode="";static int lastTraceIndex=-1;
static void project_trace_object(int index,const char*,const char* code){lastTraceIndex=index;lastTraceCode=code;}
static bool creative=false;static int g_creativeProps=0,costCalls=0;
static bool creative_on(){return creative;}
static void flash(const char*){}
static void project_stop(const char* why){g_projectLastProblem=why;g_projectRunning=false;g_bpSnapshotState=0;}
static bool bp_snapshot_request(const props::Bounds&){g_bpSnapshotState=1;g_bpSnapshotTick=GetTickCount();return true;}
static bool bp_classic_snapshot_request(const props::Bounds& b){return bp_snapshot_request(b);}
static props::Definition defs[8];alignas(8) static uint8_t items[8][0x830];
static uint8_t* lookup(uint32_t id,void*){for(int i=0;i<8;++i)if(defs[i].item==id)return items[i];return nullptr;}
static auto g_bpItemById=lookup;static void* g_bpItemReg=items;
static bool project_prop_item(uint8_t* p){return p!=nullptr;}
static nativecatalog::Status native_item_resolution(uint32_t id){return lookup(id,nullptr)?nativecatalog::Ready:nativecatalog::Unknown;}
static nativecatalog::Status project_object_resolution(uint32_t id){return native_item_resolution(id);}
static bool bp_item_bounds(uint8_t* p,float* lo,float* hi){for(int i=0;i<8;++i)if(p==items[i]){memcpy(lo,defs[i].low,12);memcpy(hi,defs[i].high,12);return true;}return false;}
struct CostEntry{uint8_t b[32];};struct CostList{CostEntry* data;int count,cap;void* extra;};
static float g_remVols[256];static int g_refundN=0;static void* g_refundList=nullptr;
static std::set<std::tuple<int,int,int>> world;
static void price_removal(CostList*,void*,void*,const CursorXf* xf){
    int x=(int)std::floor(xf->pos[0]/core::kFix*2),y=(int)std::floor(xf->pos[1]/core::kFix*2),z=(int)std::floor(xf->pos[2]/core::kFix*2);
    if(world.count({x,y,z}))g_remVols[157]=1;
}
static bool covered=true;static constexpr int64_t kProjectFixUnit=1ll<<28;
static bool project_covered(const I3&){return covered;}
static bool project_spatial_stop(Str reason,const I3&,int=-1,int=-1){project_stop(T(reason));return false;}
struct ProjectMaterialContext{int module;uint32_t saved,native;};
static bool project_cost(uint32_t,int,const ProjectMaterialContext&,bool,bool){++costCalls;return true;}
#include "../src/project_props_runtime.h"
static std::vector<project::CellEntry> cells(10000);
static void reset(){
    g_projectManifest={};g_projectPlan={};g_projectPlan.cells=cells.data();g_projectPlan.capacity=cells.size();g_projectPlan.materialCount=1;
    g_projectPlan.materials[0]={0xf1000000,157,0,0};g_projectManifest.count=2;g_projectManifest.grid=8;
    props::demo_room([](int x,int y,int z){cells[g_projectPlan.count++]={{x,y,z},0,0,extended::Wall};return true;});
    project::Error error;CHECK(project::finish(&g_projectPlan,&error));
    props::Demo demo;CHECK(props::furniture_demo(defs,&demo));memcpy(g_projectObjects,demo.objects,sizeof demo.objects);g_projectObjectCount=demo.count;
    g_projectOrigin={};g_projectRotation=0;g_projectRunning=false;covered=true;creative=false;world.clear();g_bpSnapshotCount=0;g_bpSavePending=false;g_projectLastProblem=nullptr;costCalls=0;lastTraceCode="";lastTraceIndex=-1;
}
static bool check_all(){
    for(int first=0;first<g_projectObjectsInJob;first+=4){g_projectJob.next=g_projectPlan.count+first;g_projectJob.batch=first+4<=g_projectObjectsInJob?4:g_projectObjectsInJob-first;g_bpSnapshotState=3;g_bpSnapshotCompleted=GetTickCount();
        const int64_t offset[]={0,0,0};CursorXf target{};if(!project_check_props(nullptr,nullptr,target,offset,true))return false;}
    return true;
}
int main(){
    const float sizes[8][3]={{2,1,3},{2,1,1},{.6f,1,.6f},{.3f,1,.3f},{2,1.2f,.1f},{1,.8f,1},{.2f,.3f,.2f},{.5f,1,.5f}};
    for(int i=0;i<8;++i){defs[i].item=0xf0000000+i;for(int a=0;a<3;++a){defs[i].low[a]=a==1?0:-sizes[i][a]/2;defs[i].high[a]=defs[i].low[a]+sizes[i][a];}}
    reset();CHECK(project_prepare_objects());CHECK(check_all());CHECK(costCalls==11);
    g_cfg.bpObjects=false;CHECK(!project_prepare_objects());g_cfg.bpObjects=true;
    reset();g_projectObjects[0].p[1]=-.5;CHECK(!project_prepare_objects());CHECK(g_projectLastProblem!=nullptr);
    reset();g_projectObjects[2]=g_projectObjects[0];CHECK(!project_prepare_objects());
    reset();g_projectObjectCount=2049;CHECK(!project_prepare_objects());
    reset();g_projectObjects[0].item=0xdeadbeef;CHECK(!project_prepare_objects());
    reset();g_projectManifest.pieceCount=1;g_projectPieceBoxes[0]={{16,0,16},{64,32,80}};CHECK(!project_prepare_objects());
    reset();CHECK(project_prepare_objects());g_bpSnapshot[0]={defs[0].item,g_projectPropWorld[0]};g_bpSnapshotCount=1;CHECK(!check_all());
    CHECK(lastTraceIndex>=0&&!strcmp(lastTraceCode,"E_EXISTING_PROP_DUPLICATE_RISK"));
    reset();CHECK(project_prepare_objects());covered=false;CHECK(!check_all());CHECK(g_projectLastProblem==T(S_PROJECT_COVERAGE));
    CHECK(lastTraceIndex>=0&&!strcmp(lastTraceCode,"E_ALTAR_COVERAGE"));
    reset();CHECK(project_prepare_objects());world.insert({4,0,4});CHECK(!check_all());CHECK(g_projectLastProblem==T(S_PROJECT_COLLISION));
    CHECK(lastTraceIndex>=0&&!strcmp(lastTraceCode,"E_VOXEL_COLLISION"));
    reset();g_projectPlan.count=0;CHECK(project_prepare_objects());CHECK(!check_all());CHECK(g_projectLastProblem!=nullptr);
    CHECK(lastTraceIndex>=0&&!strcmp(lastTraceCode,"E_PROP_SUPPORT"));
    // An objects-only overlay may be supported by existing floor/wall voxels.
    reset();g_projectPlan.count=0;CHECK(project_prepare_objects());
    for(int x=0;x<24;++x)for(int z=0;z<20;++z)world.insert({x,-1,z});
    for(int y=0;y<9;++y){for(int z=0;z<20;++z)world.insert({0,y,z});for(int x=0;x<24;++x)world.insert({x,y,19});}
    CHECK(check_all());CHECK(costCalls==11);
    reset();CHECK(project_prepare_objects());creative=true;CHECK(!check_all());CHECK(g_projectLastProblem==T(S_EXT_OBJECT_CREATIVE));
    reset();CHECK(project_prepare_objects());CHECK(!project_prop_scan_ready());CHECK(g_bpSnapshotState==1);
    g_bpSnapshotState=3;g_bpSnapshotCompleted=GetTickCount();CHECK(project_prop_scan_ready());g_bpSnapshotState=-1;CHECK(!project_prop_scan_ready());
    g_bpSnapshotState=1;g_bpSnapshotTick=GetTickCount()-31000;CHECK(!project_prop_scan_ready());
    g_bpSavePending=true;CHECK(!project_prop_scan_ready());
    g_bpSavePending=false;g_bpSnapshotState=3;g_bpSnapshotCompleted=GetTickCount()-500;
    CHECK(!project_prop_scan_ready());CHECK(g_bpSnapshotTrace.error==nativescan::SnapshotStale);
    g_bpSnapshotState=3;g_bpSnapshotCompleted=GetTickCount()-500;const int64_t noOffset[]={0,0,0};CursorXf staleTarget{};
    CHECK(!project_check_props(nullptr,nullptr,staleTarget,noOffset,false));CHECK(g_bpSnapshotTrace.error==nativescan::SnapshotStale);
    // Shipped examples, native metadata observed in the user's report. World
    // remains a voxel stub; this proves preflight, not real-game attachment.
    const uint32_t ids[]={0x566c8dfe,0xd109e828,0xdd5f8bb9,0x66f057f6};
    const float lows[][3]={{-1.514455f,0,-.757414f},{-1.240024f,0,-.650846f},{-.239282f,.0171f,-.313819f},{-.129753f,-.358858f,-.110265f}};
    const float highs[][3]={{1.514455f,1.007178f,.757414f},{1.247342f,.9f,.649154f},{.239282f,1.079451f,.234222f},{.129753f,.358858f,.405391f}};
    for(int i=0;i<4;++i){defs[i].item=ids[i];memcpy(defs[i].low,lows[i],12);memcpy(defs[i].high,highs[i],12);}
    for(const char* name:{"Dev10_A_Table_Unique","Dev10_D_Cinq_Meubles"}){
        g_projectManifest={};g_projectManifest.count=1;g_projectManifest.grid=8;g_projectPlan.count=0;g_projectManifest.pieceCount=0;
        g_bpSavePending=false;g_projectRunning=false;g_bpSnapshotCount=0;creative=false;covered=true;g_projectOrigin={};g_projectRotation=0;world.clear();costCalls=0;
        std::ifstream file(fs::path("examples/dev10")/(std::string(name)+".btbp"),std::ios::binary);
        std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(file)),{});core::BpHeader header{};uint8_t empty;
        CHECK(core::bp_decode(bytes.data(),bytes.size(),&header,&empty,1));CHECK(header.nm==0&&empty==core::BP_EMPTY);
        g_projectObjectCount=g_projectObjectsInJob=header.objects;
        for(int i=0;i<header.objects;++i)CHECK(extended::transform_object(g_projectManifest.modules[0],0,header,header.obj[i],&g_projectObjects[i]));
        CHECK(project_prepare_objects());
        for(int x=-10;x<10;++x)for(int z=-6;z<6;++z)world.insert({x,-1,z});
        for(int x=-1;x<1;++x)for(int y=0;y<6;++y)world.insert({x,y,4});
        CHECK(check_all());CHECK(costCalls==header.objects);
        g_bpSnapshot[0]={g_projectObjects[0].item,g_projectPropWorld[0]};g_bpSnapshotCount=1;
        CHECK(!check_all());CHECK(g_projectLastProblem!=nullptr);g_bpSnapshotCount=0;
        world.clear();CHECK(!check_all()); // missing existing support still refuses
    }
    // Exact captured composition: decorations precede their table, which is
    // 0.050049 m inside the captured floor. Preserve this source contact only.
    // Candle bounds are SYNTHETIC here; its old native export filtered them out.
    const uint32_t classicIds[]={0x58bbb7d6,0xaf3025e4,0x301dfbe1,0xc672cf0d};
    const float classicLow[][3]={{-.2f,0,-.2f},{-.08f,0,-.08f},{-.125033f,.0005f,-.125033f},{-.656332f,0,-.465419f}};
    const float classicHigh[][3]={{.2f,.2f,.2f},{.08f,.25f,.08f},{.125033f,.0295f,.125033f},{.656332f,.75f,.465419f}};
    for(int i=0;i<4;++i){defs[i].item=classicIds[i];memcpy(defs[i].low,classicLow[i],12);memcpy(defs[i].high,classicHigh[i],12);}
    std::ifstream sourceFile("examples/dev12/Dev12_Table_Classique_Source.btbp",std::ios::binary),manifestFile("examples/dev12/Dev12_Table_Classique.btproject",std::ios::binary);
    std::vector<uint8_t> raw((std::istreambuf_iterator<char>(sourceFile)),{});std::string manifest((std::istreambuf_iterator<char>(manifestFile)),{});
    core::BpHeader classicHeader{};std::vector<uint8_t> classicCells(65536);project::Error classicError;
    CHECK(core::bp_decode(raw.data(),raw.size(),&classicHeader,classicCells.data(),classicCells.size()));
    CHECK(project::decode(manifest.data(),manifest.size(),&g_projectManifest,&classicError));
    std::vector<project::CellEntry> supportCells(1024);g_projectCapturedSupport={supportCells.data(),(int)supportCells.size()};auto sourceModule=g_projectManifest.modules[0];sourceModule.objectsOnly=false;
    CHECK(project::add(&g_projectCapturedSupport,sourceModule,0,classicHeader,classicCells.data(),&classicError));CHECK(g_projectCapturedSupport.count==64);
    g_projectPlan.count=0;g_projectManifest.pieceCount=0;g_projectObjectCount=g_projectObjectsInJob=classicHeader.objects;
    for(int i=0;i<classicHeader.objects;++i)CHECK(extended::transform_object(g_projectManifest.modules[0],0,classicHeader,classicHeader.obj[i],&g_projectObjects[i]));
    g_projectClassicObjects=composition::fingerprint(g_projectObjects,g_projectObjectCount);
    auto check_group=[&](){g_projectJob.next=0;g_projectJob.batch=5;g_bpSnapshotState=3;g_bpSnapshotCompleted=GetTickCount();const int64_t offset[]={0,0,0};CursorXf target{};return project_check_props(nullptr,nullptr,target,offset,false);};
    for(int turn=0;turn<4;++turn){
        world.clear();g_bpSnapshotCount=0;g_projectOrigin={int64_t(100*core::kFix),int64_t(50*core::kFix),int64_t(200*core::kFix)};g_projectRotation=turn;costCalls=0;
        CHECK(project_prepare_objects());CHECK(g_projectObjects[0].item==classicIds[0]&&g_projectObjects[4].item==classicIds[3]);
        for(int i=0;i<g_projectCapturedSupport.count;++i){const auto centre=project::position(g_projectCapturedSupport.cells[i],g_projectManifest.anchor,turn,g_projectOrigin);
            world.insert({(int)std::floor(centre.x/core::kFix*2),(int)std::floor(centre.y/core::kFix*2),(int)std::floor(centre.z/core::kFix*2)});}
        CHECK(check_group());CHECK(costCalls==5&&g_projectPlan.count==0);
        g_bpSnapshot[0]={classicIds[0],g_projectPropWorld[0],0x123};g_bpSnapshotCount=1;CHECK(!check_group());CHECK(!strcmp(lastTraceCode,"E_EXISTING_PROP_DUPLICATE_RISK"));g_bpSnapshotCount=0;
        const auto& bowl=g_projectPropWorld[0];auto obstacle=std::make_tuple((int)std::floor((bowl.lo[0]+bowl.hi[0])),(int)std::floor((bowl.lo[1]+bowl.hi[1])),(int)std::floor((bowl.lo[2]+bowl.hi[2])));
        world.insert(obstacle);CHECK(!check_group());CHECK(!strcmp(lastTraceCode,"E_VOXEL_COLLISION"));world.erase(obstacle);
        world.clear();CHECK(!check_group());CHECK(!strcmp(lastTraceCode,"E_PROP_SUPPORT"));
    }
    g_projectObjects[0].p[0]+=.25;CHECK(!project_prepare_objects());CHECK(g_projectLastProblem!=nullptr);
    std::printf("native prop preflight: %d checks, %d failures (stub world)\n",checks,failures);return failures!=0;
}
