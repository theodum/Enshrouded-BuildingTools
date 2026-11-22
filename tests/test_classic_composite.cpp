// Real user composition, production decoders/queue/PlaceProp bridge/export/import.
// Native APIs are spies: this is not an Enshrouded placement/persistence test.
#include "project_export_host.h"
#include "../src/native_item_policy.h"
#include "../src/strings.h"
#include <array>
#include <cmath>
static int checks=0,failures=0;
#define CHECK(c) do{++checks;if(!(c)){++failures;printf("FAIL %d: %s\n",__LINE__,#c);}}while(0)
static std::vector<uint8_t> read_bytes(const fs::path& p){std::ifstream f(p,std::ios::binary);return {(std::istreambuf_iterator<char>(f)),{}};}
static const uint32_t ids[]={0x58bbb7d6,0xaf3025e4,0x301dfbe1,0xc672cf0d};
alignas(16) static uint8_t definitions[4][0x830]{};
static bool g_nativeObjectAbi=true;static int g_projectBlueprintLogic=0;
static uint8_t* lookup(uint32_t id,void*){for(int i=0;i<4;++i)if(ids[i]==id)return definitions[i];return nullptr;}
static auto g_bpItemById=lookup;static void* g_bpItemReg=definitions;
static bool flagsReadable=true;
static bool readable(const void* p,size_t){return p&&(!(!flagsReadable&&p==definitions[1]+0x5d8));}
static bool prop_item(const uint8_t* p){return p!=nullptr;}
// Bowl/coaster/table bounds were observed in the native export. Candle bounds
// below are a SYNTHETIC test fixture: dev11's old export did not include them.
static float low[4][3]={{-.2f,0,-.2f},{-.08f,0,-.08f},{-.125033f,.0005f,-.125033f},{-.656332f,0,-.465419f}};
static float high[4][3]={{.2f,.2f,.2f},{.08f,.25f,.08f},{.125033f,.0295f,.125033f},{.656332f,.75f,.465419f}};
static bool bp_item_bounds(uint8_t* p,float* lo,float* hi){for(int i=0;i<4;++i)if(p==definitions[i]){memcpy(lo,low[i],12);memcpy(hi,high[i],12);return true;}return false;}
#include "../src/native_item_resolution.h"
static bool bp_logic_item(const uint8_t* p){return (*(uint32_t*)(p+0x5d8)&0x80000)!=0;}
static bool bp_logic_object(void*,const uint8_t*){return false;}
struct BpEventBuf{uint8_t* data;uint64_t cap;uint32_t used,pad;};
alignas(16) static uint8_t g_bpEvData[512]{},g_bpEvWriter[0x200]{};
static BpEventBuf g_bpEvBuf{};static int g_bpObjRecord=0;
static nativecatalog::Outcomes g_nativeItemOutcomes;
static std::vector<std::array<uint8_t,0x50>> payloads;static std::vector<uint32_t> placed;
static int privateWriterCalls=0;static bool nativeSuccess=true;
static uint8_t native_place(void* ctx,uint8_t* info,uint32_t id){
    CHECK(g_bpObjRecord==1);std::array<uint8_t,0x50> copy{};memcpy(copy.data(),info,0x50);payloads.push_back(copy);placed.push_back(id);
    if(*(void**)((uint8_t*)ctx+0xe0)==g_bpEvWriter)++privateWriterCalls;return nativeSuccess;
}
static auto g_placePropOrig=native_place;
#include "../src/blueprint_object_place.h"
struct CostEntry{uint8_t b[24];};struct CostList{CostEntry* data;uint64_t count,cap;};
static constexpr int kMaxPending=512;static struct{int n=0;}g_pend;
static uint32_t g_pendCost[513];static uint8_t g_pendMat[512];static uint16_t g_pendObj[512];static int64_t g_pendPos[512][3];
static CostEntry* list_push(CostList* list){return list->count<list->cap?&list->data[list->count++]:nullptr;}
#include "../src/blueprint_object_queue.h"
static struct{bool ok=false;core::BpHeader h{};wchar_t path[MAX_PATH]{};}g_bp;
static bool g_projectList=false;static int g_bpPage=0,g_bpPlaced=0,g_bpWin=0;static constexpr int BPW_PLACE=2;
static bool project_load(const wchar_t* leaf){auto b=read_bytes(folder/narrow(leaf));project::Manifest parsed{};project::Error e;
    return project::decode((char*)b.data(),b.size(),&parsed,&e)&&parsed.modules[0].objectsOnly&&parsed.modules[0].classicObjects;}
#include "../src/blueprint_classic_import.h"

int main(){
    for(int i=0;i<4;++i){*(uint32_t*)definitions[i]=ids[i];for(int a=0;a<16;++a){definitions[i][4+a]=(uint8_t)(i*17+a);definitions[i][0x3c0+a]=(uint8_t)(i+1+a);}}
    *(uint32_t*)(definitions[1]+0x5d8)=0x80001;
    auto raw=read_bytes("examples/dev12/Dev12_Table_Classique_Source.btbp");core::BpHeader h{};std::vector<uint8_t> cells(65536);
    CHECK(core::bp_decode(raw.data(),raw.size(),&h,cells.data(),cells.size()));CHECK(h.objects==5&&h.nm==1);
    auto json=read_bytes("examples/dev12/Dev12_Table_Classique.btproject");project::Error error;
    CHECK(project::decode((char*)json.data(),json.size(),&g_projectManifest,&error));
    const std::string originalManifest((char*)json.data(),json.size());
    auto rejects=[&](const char* from,const char* to){auto changed=originalManifest;auto at=changed.find(from);CHECK(at!=std::string::npos);changed.replace(at,strlen(from),to);project::Manifest parsed{};return !project::decode(changed.data(),changed.size(),&parsed,&error);};
    CHECK(rejects("\"content\": \"objects\"","\"content\": \"all\""));
    CHECK(rejects("\"objectPlacement\": \"blueprint\"","\"objectPlacement\": \"invented\""));
    CHECK(rejects("\"content\": \"objects\"","\"content\": \"objects\", \"content\": \"objects\""));
    std::vector<project::CellEntry> buildCells(1024),supportCells(1024);
    g_projectPlan={buildCells.data(),(int)buildCells.size()};
    CHECK(project::add(&g_projectPlan,g_projectManifest.modules[0],0,h,cells.data(),&error));
    CHECK(g_projectPlan.count==0&&g_projectPlan.materialCount==0&&g_projectPlan.objects==5);
    project::Plan captured{supportCells.data(),(int)supportCells.size()};auto sourceModule=g_projectManifest.modules[0];sourceModule.objectsOnly=false;
    CHECK(project::add(&captured,sourceModule,0,h,cells.data(),&error));CHECK(captured.count==64);
    const int originalSupport[]={8,0,40},air[]={8,8,40};
    CHECK(composition::support_cell(captured,0,originalSupport));CHECK(!composition::support_cell(captured,0,air));
    CHECK(!composition::support_cell(captured,1,originalSupport));
    for(int i=0;i<h.objects;++i)CHECK(extended::transform_object(g_projectManifest.modules[0],0,h,h.obj[i],&g_projectObjects[i]));
    g_projectObjectCount=h.objects;g_projectClassicObjects=composition::fingerprint(g_projectObjects,g_projectObjectCount);
    CHECK(native_item_resolution(ids[1])==nativecatalog::Special);CHECK(native_item_resolution(ids[1],true)==nativecatalog::ClassicCandidate);
    CHECK(native_item_resolution(0xdeadbeef,true)==nativecatalog::Unknown);
    flagsReadable=false;CHECK(native_item_resolution(ids[1],true)==nativecatalog::Unreadable);flagsReadable=true;
    *(uint32_t*)(definitions[1]+0x5d8)=1u<<30;CHECK(native_item_resolution(ids[1],true)==nativecatalog::Special);*(uint32_t*)(definitions[1]+0x5d8)=0x80001;
    definitions[1][0x427]=1;CHECK(native_item_resolution(ids[1],true)==nativecatalog::Altar);definitions[1][0x427]=0;
    const auto oldHigh=high[1][1];high[1][1]=0;CHECK(native_item_resolution(ids[1],true)==nativecatalog::InvalidBounds);high[1][1]=oldHigh;
    // Same queue writer, references, counts and source order for both callers.
    CostEntry originalCosts[5]{},compositeCosts[5]{};CostList original{originalCosts,0,5},composite{compositeCosts,0,5};
    g_pend.n=0;for(int i=0;i<h.objects;++i)CHECK(bp_queue_native_object(&original,lookup(h.obj[i].item,nullptr),h.obj[i].item,i+1));
    g_pend.n=0;for(int i=0;i<g_projectObjectCount;++i)CHECK(bp_queue_native_object(&composite,lookup(g_projectObjects[i].item,nullptr),g_projectObjects[i].item,i+1));
    CHECK(original.count==5&&composite.count==5&&!memcmp(originalCosts,compositeCosts,sizeof originalCosts));
    for(int i=0;i<5;++i){CHECK(g_pendObj[i]==i+1&&g_pendMat[i]==255);CHECK(*(uint32_t*)(compositeCosts[i].b+0x14)==1);}
    CostList shortList{compositeCosts,0,4};g_pend.n=0;for(int i=0;i<4;++i)CHECK(bp_queue_native_object(&shortList,definitions[0],ids[0],i+1));
    CHECK(!bp_queue_native_object(&shortList,definitions[0],ids[0],5));CHECK(shortList.count==4&&g_pend.n==4);
    // Original bounded blueprint rotation vs composite rotation around its anchor.
    // Match the user's observed rebuild origin and all four whole-project turns.
    alignas(16) uint8_t ctx[0x100]{};const core::I3 origin{int64_t(5440*core::kFix),int64_t(1126.5*core::kFix),int64_t(2468.5*core::kFix)};
    const double shifts[4][3]={{0,0,0},{4,0,0},{4,0,4},{0,0,4}};
    for(int turn=0;turn<4;++turn){
        std::vector<std::array<uint8_t,0x50>> classicPayload;
        for(int i=0;i<5;++i){float point[3],q[4];core::bp_turn_point(turn,h.obj[i].pos,4,4,point);core::bp_turn_rot(turn,h.obj[i].rot,q);
            double p[]={origin.x/core::kFix-shifts[turn][0]+point[0],origin.y/core::kFix-.5+point[1],origin.z/core::kFix-shifts[turn][2]+point[2]};
            CHECK(bp_place_object(ctx,h.obj[i].item,p,q));classicPayload.push_back(payloads.back());}
        for(int i=0;i<5;++i){double p[3];float q[4];extended::object_world(g_projectObjects[i],g_projectManifest.anchor,turn,origin,p,q);
            CHECK(bp_place_object(ctx,g_projectObjects[i].item,p,q));CHECK(payloads.back()==classicPayload[i]);CHECK(*(void**)(ctx+0xe0)==nullptr);}
    }
    CHECK(privateWriterCalls==8);CHECK(g_nativeItemOutcomes.find(ids[1])->accepted==8);
    // Actual file exporter preserves references/support bytes and rejects edits.
    folder=fs::current_path()/"build"/("classic-composition-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));fs::create_directories(folder);
    const auto sourcePath=folder/g_projectManifest.modules[0].file;{std::ofstream f(sourcePath,std::ios::binary);f.write((char*)raw.data(),raw.size());}
    g_projectClassicFiles[0]=composition::bytes(14695981039346656037ull,raw.data(),raw.size());char filename[128];
    CHECK(project_export(filename,sizeof filename,true));auto output=read_bytes(folder/filename);project::Manifest exported{};
    CHECK(project::decode((char*)output.data(),output.size(),&exported,&error));CHECK(exported.modules[0].classicObjects&&exported.modules[0].objectsOnly);
    CHECK(!strcmp(exported.modules[0].file,g_projectManifest.modules[0].file));CHECK(read_bytes(sourcePath)==raw);
    CHECK(!project_export(filename,sizeof filename,true));++tick;
    g_projectObjects[0].p[0]+=.25;CHECK(!project_export(filename,sizeof filename,true));g_projectObjects[0].p[0]-=.25;
    {auto changed=raw;changed.back()^=1;std::ofstream f(sourcePath,std::ios::binary);f.write((char*)changed.data(),changed.size());}
    CHECK(!project_export(filename,sizeof filename,true));{std::ofstream f(sourcePath,std::ios::binary);f.write((char*)raw.data(),raw.size());}
    // UI import's production file path: byte-preserving copy then load manifest.
    g_bp.ok=true;g_bp.h=h;std::wcscpy(g_bp.path,sourcePath.wstring().c_str());
    CHECK(project_import_classic_blueprint());CHECK(g_projectList&&g_bpPage==1&&g_bpWin==BPW_PLACE);
    int imports=0;for(auto p:fs::directory_iterator(folder))if(p.path().filename().string().find("Classic_2026")==0&&p.path().extension()==".btbp"){++imports;CHECK(read_bytes(p.path())==raw);}
    CHECK(imports==1);CHECK(!project_import_classic_blueprint());CHECK(read_bytes(sourcePath)==raw);
    printf("classic composition: %d checks, %d failures; real BTBP, native calls simulated; candle bounds synthetic\n",checks,failures);
    return failures!=0;
}
