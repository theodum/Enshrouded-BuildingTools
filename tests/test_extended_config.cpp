// Compile the actual Windows profile adapter against an isolated in-memory
// profile store. No user's INI, game folder or save is read/written.
#include "../src/extended_core.h"
#include <map>
#include <string>
#include <cwchar>
#include <cstdio>
#include <climits>
#define MAX_PATH 260
static wchar_t g_dir[MAX_PATH]=L"test-only/";
static std::map<std::wstring,int> profile;
static std::map<std::wstring,int> otherSection;
static bool writable=true;
static wchar_t* lstrcpynW(wchar_t* out,const wchar_t* in,int n) { std::wcsncpy(out,in,n-1);out[n-1]=0;return out; }
static wchar_t* lstrcatW(wchar_t* out,const wchar_t* in) { return std::wcscat(out,in); }
static unsigned GetPrivateProfileIntW(const wchar_t* section,const wchar_t* key,int fallback,const wchar_t*) {
    const auto& store=std::wstring(section)==L"Extended" ? profile : otherSection;
    auto it=store.find(key);return (unsigned)(it==store.end()?fallback:it->second);
}
static bool WritePrivateProfileStringW(const wchar_t* section,const wchar_t* key,const wchar_t* value,const wchar_t*) {
    if (!writable) return false;
    auto& store=std::wstring(section)==L"Extended" ? profile : otherSection;store[key]=(int)std::wcstol(value,nullptr,10);return true;
}
struct Buf { char s[80]{};int n=0;Buf& i(int value){n=std::snprintf(s,sizeof s,"%d",value);return *this;}const char* z(){return s;} };
#include "../src/extended_config.h"
static int checks=0,failures=0;
#define CHECK(c) do{++checks;if(!(c)){std::printf("FAIL config line %d: %s\n",__LINE__,#c);++failures;}}while(0)
int main(){
    extended_load();CHECK(!g_grid.enabled && g_grid.height==1920 && g_grid.spacing==8);
    CHECK(!g_experimentalNative && !g_creativeProps && g_projectBatchLimit==128 && g_projectUseObjects==1);
    CHECK(g_gridLevels[6]==1920 && g_projectSliceHeight==64);
    profile={{L"Height16",INT_MAX},{L"OriginX16",INT_MIN},{L"OriginZ16",17},{L"GridStep16",3},{L"GridRadius",999},{L"ProjectBatchCells",0},{L"Level0",7},{L"Level1",320},{L"SelectedLevel",99},{L"SliceY16",INT_MIN},{L"SliceHeight16",0}};
    extended_load();CHECK(g_grid.height==1920 && g_grid.x==0 && g_grid.z==16 && g_grid.spacing==8 && g_grid.extent==32);
    CHECK(g_projectBatchLimit==128 && g_gridLevel==0 && g_projectSliceY==0 && g_projectSliceHeight==64);
    CHECK(g_gridLevels[0]==0 && g_gridLevels[1]==320);
    profile[L"ProjectBatchCells"]=999;extended_load();CHECK(g_projectBatchLimit==256);
    profile[L"ForeignExtensionKey"]=52;otherSection[L"OriginalKeyBinding"]=123;
    g_grid.enabled=1;g_grid.height=-64;g_grid.x=-128;g_grid.z=256;g_grid.spacing=16;g_gridLevel=3;g_gridLevels[3]=2048;
    g_experimentalNative=1;g_creativeProps=1;g_nativeObjectTrace=1;g_showAltars=0;g_showObjects=0;g_showModules=0;
    g_projectBatchLimit=32;g_projectSlice=1;g_projectSliceY=-32;g_projectSliceHeight=128;
    CHECK(extended_save());g_grid={};g_gridLevels[3]=0;g_projectSliceHeight=8;extended_load();
    CHECK(g_grid.enabled && g_grid.height==-64 && g_grid.x==-128 && g_grid.z==256 && g_grid.spacing==16);
    CHECK(g_gridLevel==3 && g_gridLevels[3]==2048 && g_projectBatchLimit==32);
    CHECK(g_experimentalNative && g_creativeProps && g_nativeObjectTrace);
    CHECK(!g_showAltars && !g_showObjects && !g_showModules);
    CHECK(g_projectSlice && g_projectSliceY==-32 && g_projectSliceHeight==128);
    CHECK(profile[L"ForeignExtensionKey"]==52 && otherSection[L"OriginalKeyBinding"]==123);
    const auto before=profile; writable=false;g_grid.height=32;CHECK(!extended_save());CHECK(profile==before);writable=true;
    std::printf("config: %d checks, %d failures\n",checks,failures);return failures!=0;
}
