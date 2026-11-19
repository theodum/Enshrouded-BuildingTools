// Actual architecture transaction + three-storey adapter with bounded host APIs.
// Native bounds are supplied fixtures; no price, placement or world is simulated.
#include "../src/storey_demo.h"
#include "../src/project_diagnostics.h"
#include "../src/strings.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <vector>
#include <filesystem>
#include <fstream>
#include <string>
#include <chrono>
using core::CursorXf;using core::Aabb;
static int checks=0,failures=0;
#define CHECK(c) do{++checks;if(!(c)){std::printf("FAIL storeys line %d: %s\n",__LINE__,#c);++failures;}}while(0)
#define LOG(expr) do{}while(0)
struct ProjectStateGuard {};
static bool g_projectRunning=false,g_projectLoaded=false,g_projectList=false,g_projectProblem=false;
static bool g_projectModuleVisible[project::MaxModules]{};
static int g_projectGeneration=0,g_bpPage=0,g_bpMirror=0,g_projectNeedN=0,g_bpBitsOk=0,g_bpRot=0;
static const char* g_projectLastProblem=nullptr;
static project::Plan g_projectPlan{};static project::Manifest g_projectManifest{};
static extended::Box g_projectModuleBoxes[project::MaxModules],g_projectPieceBoxes[project::MaxPieces];
static extended::Object* g_projectObjects=nullptr;static int g_projectObjectCount=0;
static struct {core::BpHeader h{};bool ok=false;int filled=0;} g_bp;
static uint32_t g_catalogMaterial=100,g_catalogPiece=200;
static bool g_nativePartsAvailable=true,g_bounds=true,failBounds=false;
alignas(8) static uint8_t block[0x4b8],stair[0x4b8];
static void* g_bpItemReg=block;
static uint8_t* lookup(uint32_t id,void*){return id==100?block:id==200?stair:nullptr;}
static uint8_t* (*g_bpItemById)(uint32_t,void*)=lookup;
static bool bp_block_item(const uint8_t* p){return p==block;}
static uint8_t* project_piece_item(const project::NativePiece& p){return g_nativePartsAvailable&&p.item==200?stair:nullptr;}
static const char* T(Str key){static const char* values[]={
#define DEMO_TEXT(id,key,en) en,
    BT_STRINGS(DEMO_TEXT)
#undef DEMO_TEXT
};return values[key];}
static void flash(const char*){}
static void copy_text(char* out,const char* text,int n){std::snprintf(out,n,"%s",text);}
constexpr int MEM_RESERVE=1,MEM_COMMIT=2,MEM_RELEASE=4,PAGE_READWRITE=1;
static int allocFail=-1;
static void* GetProcessHeap(){return nullptr;}
static void* HeapAlloc(void*,int,size_t n){if(allocFail==0)return nullptr;if(allocFail>0)--allocFail;return std::malloc(n);}
static bool HeapFree(void*,int,void* p){std::free(p);return true;}
static void* VirtualAlloc(void*,size_t n,int,int){if(allocFail==0)return nullptr;if(allocFail>0)--allocFail;return std::calloc(1,n);}
static bool VirtualFree(void* p,int,int){std::free(p);return true;}
static Aabb model{{-1,0,-1.5f},0,{1,2,1.5f},0};
static void shape_bounds(void*,const CursorXf* xf,Aabb* out){
    // Independent quaternion matrix, not the layout's box rotation helper.
    for(int a=0;a<3;++a){out->mn[a]=1e9f;out->mx[a]=-1e9f;}
    const double y=xf->rot[1],w=xf->rot[3];
    for(int c=0;c<8;++c){double p[3];for(int a=0;a<3;++a)p[a]=((c>>a)&1)?model.mx[a]:model.mn[a];
        double v[]={(1-2*y*y)*p[0]+2*y*w*p[2],p[1],-2*y*w*p[0]+(1-2*y*y)*p[2]};
        for(int a=0;a<3;++a){float t=(float)(v[a]+(double)xf->pos[a]/core::kFix);if(t<out->mn[a])out->mn[a]=t;if(t>out->mx[a])out->mx[a]=t;}
    }
}
static bool project_piece_bounds(){
    if(failBounds)return false;
    for(int i=0;i<g_projectManifest.pieceCount;++i){const auto& p=g_projectManifest.pieces[i];CursorXf xf{};xf.scale=1;const float q[]={0,0,0,1};core::bp_turn_rot(p.rotation,q,xf.rot);
        for(int a=0;a<3;++a)xf.pos[a]=(int64_t)p.offset[a]*(1ll<<28);
        Aabb b{};shape_bounds(stair,&xf,&b);
        for(int a=0;a<3;++a){double lo=extended::native_bound16(b.mn[a]),hi=extended::native_bound16(b.mx[a]);
            g_projectPieceBoxes[i].lo[a]=(int)std::floor(lo);g_projectPieceBoxes[i].hi[a]=(int)std::ceil(hi);
            if(g_projectPieceBoxes[i].lo[a]<g_projectPlan.lo[a])g_projectPlan.lo[a]=g_projectPieceBoxes[i].lo[a];
            if(g_projectPieceBoxes[i].hi[a]>g_projectPlan.hi[a])g_projectPlan.hi[a]=g_projectPieceBoxes[i].hi[a];}
    }return true;
}
static bool project_generated_begin(const char* name){
    if(!g_projectPlan.cells)g_projectPlan.cells=(project::CellEntry*)std::calloc(project::MaxCells,sizeof(project::CellEntry));
    if(!g_projectPlan.cells)return false;auto cells=g_projectPlan.cells;g_projectPlan={};g_projectPlan.cells=cells;g_projectPlan.capacity=project::MaxCells;
    g_projectManifest={};g_projectManifest.grid=8;copy_text(g_projectManifest.name,name,sizeof g_projectManifest.name);g_projectObjectCount=0;g_projectLoaded=false;return true;
}
static void project_generated_finish(){g_projectLoaded=g_projectList=g_bp.ok=true;++g_projectGeneration;g_bpPage=1;g_projectNeedN=0;g_projectProblem=false;g_projectLastProblem=nullptr;}
#include "../src/extended_architecture.h"
// File-only adapters also exercise exporting/reloading this complete draft.
namespace fs=std::filesystem;
using DWORD=unsigned long;using BOOL=bool;using HANDLE=FILE*;
#define MAX_PATH 1024
#define CP_UTF8 65001
#define MB_ERR_INVALID_CHARS 8
#define GENERIC_WRITE 1
#define CREATE_NEW 1
#define FILE_ATTRIBUTE_NORMAL 0
#define MOVEFILE_WRITE_THROUGH 1
#define INVALID_HANDLE_VALUE nullptr
struct SYSTEMTIME {int wYear=2026,wMonth=11,wDay=19,wHour=12,wMinute=0,wSecond=0;};
static void GetLocalTime(SYSTEMTIME* t){*t=SYSTEMTIME{};}
static DWORD GetTickCount(){return 2000;}
static fs::path exportFolder;
static std::string narrow(const wchar_t* s){std::string out;while(*s)out+=(char)*s++;return out;}
static int MultiByteToWideChar(int,int,const char* s,int,wchar_t* out,int n){int i=0;do{if(i==n)return 0;out[i]=(unsigned char)s[i];}while(s[i++]);return i;}
static BOOL CreateDirectoryW(const wchar_t* p,void*){return fs::create_directory(narrow(p));}
static HANDLE CreateFileW(const wchar_t* p,int,int,void*,int,int,void*){if(fs::exists(narrow(p)))return nullptr;return std::fopen(narrow(p).c_str(),"wb");}
static BOOL WriteFile(HANDLE f,const void* p,DWORD n,DWORD* written,void*){*written=std::fwrite(p,1,n,f);return *written==n;}
static BOOL FlushFileBuffers(HANDLE f){return std::fflush(f)==0;}
static void CloseHandle(HANDLE f){std::fclose(f);}
static BOOL MoveFileExW(const wchar_t* a,const wchar_t* b,int){if(fs::exists(narrow(b)))return false;fs::rename(narrow(a),narrow(b));return true;}
static bool project_path(wchar_t* out,const wchar_t* leaf){auto path=(exportFolder/narrow(leaf)).wstring();if(path.size()>=MAX_PATH)return false;std::wcscpy(out,path.c_str());return true;}
struct Buf {
    char s[1024]{};int n=0;Buf& c(char v){if(n<1023)s[n++]=v;return *this;}Buf& str(const char* v){while(*v)c(*v++);return *this;}
    Buf& u(uint64_t v,int base=10,int width=0){char b[64];int k=0;do{int d=v%base;b[k++]=d<10?'0'+d:'a'+d-10;v/=base;}while(v);while(k<width)b[k++]='0';while(k)c(b[--k]);return *this;}
    Buf& i(int64_t v){if(v<0){c('-');v=-v;}return u((uint64_t)v);}Buf& f(double v,int digits){char b[80];std::snprintf(b,sizeof b,"%.*f",digits,v);return str(b);}const char* z(){s[n]=0;return s;}
};
#include "../src/project_export.h"
static bool has(int x,int y,int z){for(int i=0;i<g_projectPlan.count;++i){const auto& c=g_projectPlan.cells[i];if(c.p[0]==x&&c.p[1]==y&&c.p[2]==z)return true;}return false;}
static void geometry_tests(){
    *(uint32_t*)block=100;block[0x454]=0x9d;*(uint32_t*)stair=200;
    storeys::Layout l;CHECK(storeys::prepare(32,32,48,32,&l)==storeys::Issue::None);CHECK(l.count==4&&l.levelHeight==64);
    for(int turn=0;turn<4;++turn){g_stairModelTurn=turn;
        CHECK(architecture_storeys(3-turn,32));CHECK(g_projectManifest.count==16&&g_projectManifest.pieceCount==4);CHECK(g_bpRot==3-turn);
        for(int i=0;i<4;++i){auto clear=g_projectPieceBoxes[i];clear.hi[1]+=32;CHECK(!extended::plan_intersects(g_projectPlan,clear));
            CHECK(g_projectManifest.pieces[i].rotation==turn && g_projectManifest.pieces[i].material==100);
            for(int j=0;j<i;++j)CHECK(!extended::intersects(clear,g_projectPieceBoxes[j]));}
    }
    g_stairModelTurn=0;CHECK(architecture_storeys(0,32));
    CHECK(has(-40,-8,-40)&&has(-40,56,-40)&&has(-40,120,-40)); // three separate floor surfaces
    CHECK(!has(0,56,0)&&!has(0,56,88));CHECK(!has(0,120,128)&&!has(0,120,216)); // aligned shaft openings
    CHECK(has(0,56,96)&&has(24,56,120));CHECK(has(0,120,224)&&has(24,120,248)); // flat middle/top landings
    CHECK(!has(0,0,-48)&&!has(24,32,-48));CHECK(has(-16,0,-48)); // front door bay and adjacent wall
    int roofs=0;for(int i=0;i<g_projectPlan.count;++i)if(g_projectPlan.cells[i].category==extended::Roof)++roofs;CHECK(roofs>0);
    for(int i=0;i<g_projectPlan.count;++i)CHECK(g_projectPlan.cells[i].material==0);
    // Nominal route: each flight exits at its calibrated height; no voxel intersects body/headroom.
    for(int i=0;i<l.count;++i){const int top=l.bodies[i].lo[1]+l.rise,end=l.bodies[i].hi[2];
        if(i+1==l.flightsPerLevel || i+1==l.count)CHECK(has(0,top-8,end));
        else CHECK(l.bodies[i+1].lo[1]==top&&l.bodies[i+1].lo[2]==end);
    }
    std::printf("storeys fixture: modules=%d cells=%d pieces=%d dimensions16=%d,%d,%d\n",g_projectManifest.count,g_projectPlan.count,g_projectManifest.pieceCount,
        g_projectPlan.hi[0]-g_projectPlan.lo[0],g_projectPlan.hi[1]-g_projectPlan.lo[1],g_projectPlan.hi[2]-g_projectPlan.lo[2]);
}
static void rollback_tests(){
    auto previous=g_projectPlan.cells;const int count=g_projectPlan.count,pieces=g_projectManifest.pieceCount;std::vector<project::CellEntry> copy(previous,previous+count);
    auto preserved=[&](){CHECK(g_projectPlan.cells==previous&&g_projectPlan.count==count&&g_projectManifest.pieceCount==pieces);CHECK(!std::memcmp(copy.data(),g_projectPlan.cells,count*sizeof(project::CellEntry)));};
    CHECK(!architecture_storeys(0,0));preserved();CHECK(g_projectLastProblem==T(S_EXT_DEMO_RISE));
    failBounds=true;CHECK(!architecture_storeys(0,32));failBounds=false;preserved(); // failure AFTER building cells/pieces must restore the actual draft
    for(int fail=0;fail<2;++fail){allocFail=fail;CHECK(!architecture_storeys(0,32));allocFail=-1;preserved();}
    g_nativePartsAvailable=false;CHECK(!architecture_storeys(0,32));g_nativePartsAvailable=true;preserved();
    g_catalogPiece=999;CHECK(!architecture_storeys(0,32));g_catalogPiece=200;preserved();
    g_projectRunning=true;CHECK(!architecture_storeys(0,32));g_projectRunning=false;preserved();
    storeys::Layout l;CHECK(storeys::prepare(32,32,48,0,&l)==storeys::Issue::Rise);
    CHECK(storeys::prepare(31,32,48,32,&l)==storeys::Issue::Dimensions);
    CHECK(storeys::prepare(256,256,512,8,&l)==storeys::Issue::Budget);
    CHECK(storeys::prepare(32,80,48,8,&l)==storeys::Issue::Clearance);
}
static void export_tests(){
    fs::path parent=fs::current_path()/"build";fs::create_directories(parent);
    exportFolder=parent/("storeys-export-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    char filename[128];CHECK(project_export(filename,sizeof filename));
    auto read=[](const fs::path& p){std::ifstream f(p,std::ios::binary);return std::vector<uint8_t>((std::istreambuf_iterator<char>(f)),{});};
    auto manifest=read(exportFolder/filename);project::Manifest decoded{};project::Error error;
    CHECK(project::decode((const char*)manifest.data(),(int)manifest.size(),&decoded,&error));CHECK(decoded.pieceCount==4);
    std::vector<project::CellEntry> cells(project::MaxCells);project::Plan plan{cells.data(),(int)cells.size()};
    for(int i=0;i<decoded.count;++i){auto bytes=read(exportFolder/decoded.modules[i].file);core::BpHeader h{};std::vector<uint8_t> data(65536);
        CHECK(core::bp_decode(bytes.data(),(int)bytes.size(),&h,data.data(),(int)data.size()));CHECK(project::add(&plan,decoded.modules[i],i,h,data.data(),&error));}
    CHECK(project::finish(&plan,&error));CHECK(plan.count==g_projectPlan.count&&plan.materialCount==1&&plan.materials[0].item==100);
    for(int i=0;i<plan.count;++i){for(int a=0;a<3;++a)CHECK(plan.cells[i].p[a]==g_projectPlan.cells[i].p[a]);CHECK(plan.cells[i].material==0);CHECK(plan.cells[i].category==g_projectPlan.cells[i].category);}
    for(int i=0;i<4;++i){const auto& p=decoded.pieces[i];const auto& old=g_projectManifest.pieces[i];CHECK(p.item==old.item&&p.material==old.material&&p.rotation==old.rotation&&p.clearance==32);for(int a=0;a<3;++a)CHECK(p.offset[a]==old.offset[a]);}
    CHECK(!project_export(filename,sizeof filename));CHECK(read(exportFolder/filename)==manifest);
}
static void diagnostic_tests(){
    CHECK(extended::native_bound16(-0.000001f)==0);CHECK(extended::native_bound16(1.000001f)==16);
    CHECK(extended::native_bound16(-0.00001f)<0);CHECK(extended::native_bound16(1.00001f)>16);
    CHECK(extended::native_bound16(1.0625f)==17);CHECK(extended::native_bound16(5357.0625f)==85713);
    diagnostics::Context c{10,0,{1,2,3},false,true};diagnostics::Validation v;
    CHECK(!v.current(c));v.begin(c,5);CHECK(v.current(c)&&!v.fullQuote);CHECK(!v.pass());CHECK(v.progress(3));CHECK(!v.progress(2)&&!v.progress(6));
    CHECK(v.progress(5)&&v.pass()&&v.fullQuote);v.fail();CHECK(v.state==diagnostics::State::Failed&&v.fullQuote);
    auto different=c;++different.generation;CHECK(!v.current(different));different=c;++different.origin.x;CHECK(!v.current(different));
    different=c;++different.rotation;CHECK(!v.current(different));different=c;different.creative=true;CHECK(!v.current(different));different=c;different.objects=false;CHECK(!v.current(different));
    v.begin(c,5);CHECK(!v.fullQuote&&v.checked==0);v.fail();CHECK(v.state==diagnostics::State::Failed&&!v.fullQuote);
}
int main(){geometry_tests();rollback_tests();export_tests();diagnostic_tests();std::free(g_projectPlan.cells);std::printf("storeys: %d checks, %d failures\n",checks,failures);return failures?1:0;}
