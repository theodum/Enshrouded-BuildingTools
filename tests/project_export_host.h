// File-only Win32 adapters shared by exporter and native prop integration tests.
#pragma once
#include "../src/extended_core.h"
#include <filesystem>
#include <fstream>
#include <cstdio>
#include <cstdlib>
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
static int lstrlenW(const wchar_t* s){return std::wcslen(s);}
static void lstrcpyW(wchar_t* out,const wchar_t* s){std::wcscpy(out,s);}
static void lstrcatW(wchar_t* out,const wchar_t* s){std::wcscat(out,s);}
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
    Buf& x(uint64_t v){return str("0x").u(v,16);}
    // Admission tests inspect refusal keys; localized formatting is covered by core.
    template<class... A> Buf& fmt(const char* v,A...){return str(v);}
    Buf& f(double v,int digits){char b[80];std::snprintf(b,sizeof b,"%.*f",digits,v);return str(b);}
    const char* z(){s[n]=0;return s;}
};
static void copy_text(char* out,const char* in,int size){if(size<=0)return;int i=0;while(i+1<size&&in[i]){out[i]=in[i];++i;}out[i]=0;}
struct ProjectStateGuard { ProjectStateGuard(){} ~ProjectStateGuard(){} };
static fs::path folder;
static bool project_path(wchar_t* out,const wchar_t* leaf){auto path=(folder/narrow(leaf)).wstring();if(path.size()>=MAX_PATH)return false;std::wcscpy(out,path.c_str());return true;}
static bool g_projectLoaded=true, g_projectRunning=false;
static project::Plan g_projectPlan{};
static project::Manifest g_projectManifest{};
static extended::Object g_projectObjects[512]; static int g_projectObjectCount=0;
static uint64_t g_projectClassicObjects=0,g_projectClassicFiles[project::MaxModules]{};
static void* GetProcessHeap(){return nullptr;}
static bool HeapFree(void*,int,void* p){std::free(p);return true;}
static char* read_file(const wchar_t* path,DWORD* size){std::ifstream file(narrow(path),std::ios::binary);if(!file)return nullptr;
    std::string data((std::istreambuf_iterator<char>(file)),{});*size=data.size();char* p=(char*)std::malloc(data.size()+1);memcpy(p,data.c_str(),data.size()+1);return p;}
#include "../src/project_export.h"
