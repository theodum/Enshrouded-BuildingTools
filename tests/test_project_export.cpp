// Compile the actual native exporter against file-only Win32 adapters. Exercises
// publication, read-back, bounds, native-piece IDs and non-overwrite semantics.
#include "../src/extended_core.h"
#include <filesystem>
#include <fstream>
#include <cstdio>
#include <cstring>
#include <chrono>
#include <vector>
#include <string>
namespace fs = std::filesystem;
using DWORD = unsigned long; using BOOL = bool; using HANDLE = FILE*;
#define MAX_PATH 1024
#define CP_UTF8 65001
#define MB_ERR_INVALID_CHARS 8
#define GENERIC_WRITE 1
#define CREATE_NEW 1
#define FILE_ATTRIBUTE_NORMAL 0
#define MOVEFILE_WRITE_THROUGH 1
#define INVALID_HANDLE_VALUE nullptr
#define LOG(expr) do {} while (0)
struct SYSTEMTIME { int wYear=2026,wMonth=11,wDay=17,wHour=12,wMinute=30,wSecond=0; };
static void GetLocalTime(SYSTEMTIME* t) { *t = SYSTEMTIME{}; }
static int tick=1000, failWrite=-1, failFlush=-1;
static DWORD GetTickCount() { return tick; }
static std::string narrow(const wchar_t* s) { std::string out; while (*s) out += (char)*s++; return out; }
static int MultiByteToWideChar(int,int,const char* s,int,wchar_t* out,int n) { int i=0; do { if(i==n) return 0; out[i]=(unsigned char)s[i]; } while(s[i++]); return i; }
static BOOL CreateDirectoryW(const wchar_t* p,void*) { return fs::create_directory(narrow(p)); }
static HANDLE CreateFileW(const wchar_t* p,int,int,void*,int,int,void*) { auto path=narrow(p); if(fs::exists(path)) return nullptr; return std::fopen(path.c_str(),"wb"); }
static BOOL WriteFile(HANDLE f,const void* p,DWORD n,DWORD* written,void*) { if(failWrite==0){*written=0;return false;} if(failWrite>0)--failWrite; *written=std::fwrite(p,1,n,f); return *written==n; }
static BOOL FlushFileBuffers(HANDLE f) { if(failFlush==0)return false;if(failFlush>0)--failFlush;return std::fflush(f)==0; }
static void CloseHandle(HANDLE f) { std::fclose(f); }
static BOOL MoveFileExW(const wchar_t* a,const wchar_t* b,int) { if(fs::exists(narrow(b))) return false; fs::rename(narrow(a),narrow(b)); return true; }
struct Buf {
    char s[1024]{}; int n=0;
    Buf& c(char v){if(n<1023)s[n++]=v;return *this;}
    Buf& str(const char* v){while(*v)c(*v++);return *this;}
    Buf& u(uint64_t v,int base=10,int width=0){char b[64];int k=0;do{int digit=(int)(v%base);b[k++]=(char)(digit<10?'0'+digit:'a'+digit-10);v/=base;}while(v);while(k<width)b[k++]='0';while(k)c(b[--k]);return *this;}
    Buf& i(int64_t v){if(v<0){c('-');v=-v;}return u((uint64_t)v);}
    Buf& f(double v,int digits){char b[80];std::snprintf(b,sizeof b,"%.*f",digits,v);return str(b);}
    const char* z(){s[n]=0;return s;}
};
static void copy_text(char* out,const char* in,int size){std::snprintf(out,size,"%s",in);}
struct ProjectStateGuard { ProjectStateGuard(){} ~ProjectStateGuard(){} };
static fs::path folder;
static bool project_path(wchar_t* out,const wchar_t* leaf){auto path=(folder/narrow(leaf)).wstring();if(path.size()>=MAX_PATH)return false;std::wcscpy(out,path.c_str());return true;}
static bool g_projectLoaded=true, g_projectRunning=false;
static project::Plan g_projectPlan{};
static project::Manifest g_projectManifest{};
static extended::Object g_projectObjects[2]; static int g_projectObjectCount=0;
#include "../src/project_export.h"
static int failures=0;
#define CHECK(c) do { if(!(c)){std::printf("FAIL line %d: %s\n",__LINE__,#c);++failures;} }while(0)
static std::vector<uint8_t> read(const fs::path& p){std::ifstream f(p,std::ios::binary);return std::vector<uint8_t>((std::istreambuf_iterator<char>(f)),{});}
int main(){
    // A fresh, owned build directory; tests do not delete or touch any game path.
    fs::path parent=fs::current_path()/"build";fs::create_directories(parent);
    folder=parent/("export-test-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::vector<project::CellEntry> cells(3); cells[0]={{-8,0,0},0,0,extended::Wall};cells[1]={{256,8,0},0,0,extended::Wall};cells[2]={{264,8,0},0,1,extended::Wall};
    g_projectPlan.cells=cells.data();g_projectPlan.count=3;g_projectPlan.capacity=3;g_projectPlan.materialCount=2;
    g_projectPlan.materials[0]={0x25a43c2f,0x9d,2,0};g_projectPlan.materials[1]={1234,0x81,1,0};
    std::strcpy(g_projectManifest.name,"Hall \"quoted\" \\ export");g_projectManifest.grid=8;g_projectManifest.count=1;g_projectManifest.anchor[0]=8;
    g_projectManifest.pieceCount=1;g_projectManifest.pieces[0]={0xffffffffu,{16,32,-16},1,0x25a43c2f,32};
    g_projectObjectCount=1;g_projectObjects[0]={55,{2.25,1.5,-1.25},{0,0,0,1},0};
    char filename[128];CHECK(project_export(filename,sizeof filename));
    auto bytes=read(folder/filename); project::Manifest parsed{};project::Error error;
    CHECK(project::decode((char*)bytes.data(),(int)bytes.size(),&parsed,&error));CHECK(parsed.count==3);CHECK(parsed.pieceCount==1);CHECK(parsed.pieces[0].item==0xffffffffu);
    CHECK(std::strcmp(parsed.name,g_projectManifest.name)==0);CHECK(parsed.anchor[0]==8); CHECK(parsed.pieces[0].clearance==32);
    std::vector<project::CellEntry> back(4096);project::Plan plan{back.data(),(int)back.size()};int objects=0;
    for(int i=0;i<parsed.count;++i){
        auto source=read(folder/parsed.modules[i].file);core::BpHeader h{};std::vector<uint8_t> decoded(65536);
        CHECK(core::bp_decode(source.data(),(int)source.size(),&h,decoded.data(),65536));
        CHECK(project::add(&plan,parsed.modules[i],i,h,decoded.data(),&error));
        objects+=h.objects;if(h.objects){extended::Object o;CHECK(extended::transform_object(parsed.modules[i],i,h,h.obj[0],&o));CHECK(o.p[0]==2.25&&o.p[2]==-1.25);}
    }
    CHECK(project::finish(&plan,&error));CHECK(plan.count==3&&objects==1);
    CHECK(plan.cells[0].p[0]==-8&&plan.cells[1].p[0]==256&&plan.cells[2].p[0]==264);
    auto original=read(folder/filename);CHECK(!project_export(filename,sizeof filename));CHECK(original==read(folder/filename));
    ++tick;failWrite=0;CHECK(!project_export(filename,sizeof filename));
    int manifests=0;for(auto& p:fs::directory_iterator(folder))if(p.path().extension()==".btproject")++manifests;CHECK(manifests==1);
    ++tick;failWrite=4;CHECK(!project_export(filename,sizeof filename));
    manifests=0;for(auto& p:fs::directory_iterator(folder))if(p.path().extension()==".btproject")++manifests;CHECK(manifests==1);
    failWrite=-1;++tick;failFlush=0;CHECK(!project_export(filename,sizeof filename));
    manifests=0;for(auto& p:fs::directory_iterator(folder))if(p.path().extension()==".btproject")++manifests;CHECK(manifests==1);
    ++tick;failFlush=3;CHECK(!project_export(filename,sizeof filename));
    manifests=0;for(auto& p:fs::directory_iterator(folder))if(p.path().extension()==".btproject")++manifests;CHECK(manifests==1);
    CHECK(original==read(folder/filename));
    std::printf("export: %d failures\n",failures);return failures!=0;
}
