#pragma once
// Actual architecture transaction + three-storey adapter with bounded host APIs.
// Native bounds are supplied fixtures; no price, placement or world is simulated.
#include "../src/voxel_stairs.h"
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
#define LOG(expr) do{}while(0)
struct ProjectStateGuard {};
static bool g_projectRunning=false,g_projectLoaded=false,g_projectList=false,g_projectProblem=false;
static bool g_projectModuleVisible[project::MaxModules]{};
static int g_projectGeneration=0,g_bpPage=0,g_bpMirror=0,g_projectNeedN=0,g_bpBitsOk=0,g_bpRot=0;
static int g_projectUseObjects=0;
static const char* g_projectLastProblem=nullptr;
static project::Plan g_projectPlan{};static project::Manifest g_projectManifest{};
static extended::Box g_projectModuleBoxes[project::MaxModules],g_projectPieceBoxes[project::MaxPieces];
static extended::Object* g_projectObjects=nullptr;static int g_projectObjectCount=0;
static struct {core::BpHeader h{};bool ok=false;int filled=0;} g_bp;
static uint32_t g_catalogMaterial=100,g_catalogPiece=200;
static bool g_nativePartsAvailable=true,g_bounds=true,failBounds=false;
alignas(8) static uint8_t block[0x4b8],stair[0x4b8];
static void* g_bpItemReg=block;
static uint32_t hostMaterial=100;
static uint8_t* lookup(uint32_t id,void*){return id==hostMaterial?block:id==200?stair:nullptr;}
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
static bool project_validate_spaces(){return spaces::validate(g_projectPlan,g_projectManifest)==spaces::Issue::None;}
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
static int lstrlenW(const wchar_t* s){return std::wcslen(s);}static void lstrcpyW(wchar_t* a,const wchar_t* b){std::wcscpy(a,b);}static void lstrcatW(wchar_t* a,const wchar_t* b){std::wcscat(a,b);}
static char* read_file(const wchar_t* path,DWORD* size){std::ifstream file(narrow(path),std::ios::binary);if(!file)return nullptr;
    std::string data((std::istreambuf_iterator<char>(file)),{});*size=data.size();char* p=(char*)std::malloc(data.size()+1);memcpy(p,data.c_str(),data.size()+1);return p;}
static uint64_t g_projectClassicObjects=0,g_projectClassicFiles[project::MaxModules]{};
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
    Buf& x(uint64_t v){return str("0x").u(v,16);}template<class... A> Buf& fmt(const char* value,A...){return str(value);}
};
#include "../src/project_export.h"
