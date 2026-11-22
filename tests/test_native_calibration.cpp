// Actual capture-file/calibrated-draft code, fake native registry + file APIs.
#include "architecture_host.h"
#include "../src/native_calibration.h"
#include "../src/native_item_policy.h"
#include "../src/native_item_index.h"
#include <limits>
struct IMAGE_DOS_HEADER {int e_lfanew;};
struct IMAGE_NT_HEADERS64 {struct{uint32_t TimeDateStamp;}FileHeader;struct{uint32_t SizeOfImage;}OptionalHeader;};
alignas(16) static uint8_t fakeImage[128]{},native[0x828]{};
static uint8_t* g_base=fakeImage;static bool g_bpWorldAbi=true,g_nativeObjectAbi=true;
static constexpr int kProjectMaxObjects=1024;
struct BpFound {uint32_t entity,item;int64_t pos[3];float rot[4];float scale[3]{1,1,1};uint8_t templateId[16]{};};
static uint8_t* calibration_lookup(uint32_t id,void*){return id==0xf0001234||id==0xd109e828?native:nullptr;}
static nativecatalog::Status status=nativecatalog::Ready;
static nativecatalog::Status native_item_resolution(uint32_t id){return calibration_lookup(id,nullptr)?status:nativecatalog::Unknown;}
static float low[3]={-1.240024f,0,-.650846f},high[3]={1.247342f,.9f,.649154f};
static bool bp_item_bounds(uint8_t*,float* lo,float* hi){memcpy(lo,low,12);memcpy(hi,high,12);return true;}
static const char* bp_item_name_cached(uint32_t){return "SYNTHETIC \"meuble\"";}
static bool readable(const void* p,size_t){return p!=nullptr;}
static int prop_floor(double v){return (int)std::floor(v);}static int prop_ceil(double v){return (int)std::ceil(v);}
static void lstrcpynW(wchar_t* a,const wchar_t* b,int n){std::wcsncpy(a,b,n-1);a[n-1]=0;}
constexpr int FILE_ATTRIBUTE_DIRECTORY=16;struct WIN32_FIND_DATAW{int dwFileAttributes=0;wchar_t cFileName[192]{};};
static std::vector<fs::path> matches;static int matchIndex=0;
static HANDLE FindFirstFileW(const wchar_t* pattern,WIN32_FIND_DATAW* out){matches.clear();matchIndex=0;auto parent=fs::path(narrow(pattern)).parent_path();
    if(fs::exists(parent))for(auto p:fs::directory_iterator(parent))if(p.path().extension()==".btcalibration")matches.push_back(p.path());
    if(matches.empty())return nullptr;std::wcscpy(out->cFileName,matches[0].filename().wstring().c_str());return (FILE*)1;}
static bool FindNextFileW(HANDLE,WIN32_FIND_DATAW* out){if(++matchIndex==(int)matches.size())return false;std::wcscpy(out->cFileName,matches[matchIndex].filename().wstring().c_str());return true;}
static void FindClose(HANDLE){}
#include "../src/native_calibration_runtime.h"
static int checks=0,failures=0;
#define CHECK(c) do{++checks;if(!(c)){++failures;printf("FAIL %d: %s\n",__LINE__,#c);}}while(0)
int main(){
    ((IMAGE_DOS_HEADER*)fakeImage)->e_lfanew=16;auto* pe=(IMAGE_NT_HEADERS64*)(fakeImage+16);pe->FileHeader.TimeDateStamp=1793791052;pe->OptionalHeader.SizeOfImage=44298240;
    g_bpItemById=calibration_lookup;memset(native+0x3c0,0xab,16);
    BpFound found{};found.item=0xf0001234;found.entity=42;found.pos[0]=int64_t(5195.25*core::kFix);found.pos[1]=int64_t(1083*core::kFix);found.pos[2]=int64_t(2873.5*core::kFix);found.rot[3]=1;memcpy(found.templateId,native+0x3c0,16);
    core::BpObject source{};source.item=found.item;source.pos[0]=2.125f;source.rot[3]=1;
    exportFolder=fs::current_path()/"build"/("calibration-test-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    CHECK(bp_calibration_capture(found,source));CHECK(g_nativeCalibrationValid);
    calibration_list();CHECK(g_nativeCalibrationCount==1);auto before=g_nativeCalibration;
    CHECK(calibration_load(0));CHECK(g_projectObjectCount==1&&g_projectPlan.count==0&&g_projectManifest.pieceCount==0);
    CHECK(std::abs(g_projectObjects[0].p[0]+.003659)<1e-5);CHECK(g_projectObjects[0].p[1]==0);
    CHECK(g_projectObjects[0].rotation[3]==1);CHECK(g_nativeCalibration.entity==42&&g_nativeCalibration.local[0]==2.125);
    auto* keep=g_projectObjects;auto keepItem=g_projectObjects[0].item;
    auto changed=before;changed.templateId[0]^=1;CHECK(!calibration_prepare(changed));CHECK(!strcmp(g_nativeCalibrationError,"E_CALIBRATION_TEMPLATE_CHANGED"));CHECK(g_projectObjects==keep&&g_projectObjects[0].item==keepItem);
    changed=before;changed.buildTime^=1;CHECK(!calibration_prepare(changed));CHECK(!strcmp(g_nativeCalibrationError,"E_CALIBRATION_GAME_CHANGED_RECAPTURE"));
    changed=before;changed.high[0]+=1;CHECK(!calibration_prepare(changed));CHECK(!strcmp(g_nativeCalibrationError,"E_CALIBRATION_BOUNDS_CHANGED"));
    changed=before;changed.scale[0]=2;CHECK(!calibration_prepare(changed));CHECK(!strcmp(g_nativeCalibrationError,"E_CALIBRATION_SCALE_UNSUPPORTED"));
    changed=before;changed.scale[0]=std::numeric_limits<float>::quiet_NaN();CHECK(!calibration::valid(changed));
    changed=before;changed.rotation[3]=0;CHECK(!calibration::valid(changed));
    allocFail=2;CHECK(!calibration_prepare(before));allocFail=-1;CHECK(g_projectObjects==keep&&g_projectObjects[0].item==keepItem);
    CHECK(!bp_calibration_capture(found,source)); // same filename must not overwrite
    calibration_list();CHECK(g_nativeCalibrationCount==1);
    status=nativecatalog::Unknown;CHECK(!calibration_prepare(before));status=nativecatalog::Ready;
    const char* malformed="{\"format\":\"BuildingToolsCalibration\",\"format\":\"BuildingToolsCalibration\"}";project::Error error;calibration::Definition decoded;
    CHECK(!calibration::decode(malformed,strlen(malformed),&decoded,&error));
    CHECK(!calibration::decode("{}",2,&decoded,&error));
    VirtualFree(g_projectPlan.cells,0,MEM_RELEASE);VirtualFree(g_projectObjects,0,MEM_RELEASE);
    printf("native calibration: %d checks, %d failures; synthetic registry, actual file/draft code\n",checks,failures);return failures!=0;
}
