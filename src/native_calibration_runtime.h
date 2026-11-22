// Calibration is a file/draft operation. Native placement still uses the
// original BP/PlaceProp bridge and full composite preflight.
static calibration::Definition g_nativeCalibration;static bool g_nativeCalibrationValid=false;
static wchar_t g_nativeCalibrationFiles[64][192];static int g_nativeCalibrationCount=0;
static char g_nativeCalibrationError[256]={};
static bool calibration_error(const char* code){copy_text(g_nativeCalibrationError,code,sizeof g_nativeCalibrationError);Buf b;b.fmt(T(S_EXT_CALIBRATION_ERROR),code);flash(b.z());LOG(.str("calibration REFUSED code=").str(code));return false;}
static void calibration_build(uint32_t* time,uint32_t* size){const auto* pe=(const IMAGE_NT_HEADERS64*)(g_base+((const IMAGE_DOS_HEADER*)g_base)->e_lfanew);*time=pe->FileHeader.TimeDateStamp;*size=pe->OptionalHeader.SizeOfImage;}
static bool calibration_current(const calibration::Definition& d){
    if(!g_bpWorldAbi||!g_nativeObjectAbi)return calibration_error("E_CALIBRATION_ABI");
    for(float s:d.scale)if(!(s>=.99999f&&s<=1.00001f))return calibration_error("E_CALIBRATION_SCALE_UNSUPPORTED");
    if(!calibration::valid(d))return calibration_error("E_CALIBRATION_INVALID_DATA_OR_SCALE");
    uint32_t time,size;calibration_build(&time,&size);if(d.buildTime!=time||d.imageSize!=size)return calibration_error("E_CALIBRATION_GAME_CHANGED_RECAPTURE");
    const auto status=native_item_resolution(d.item);if(status!=nativecatalog::Ready)return calibration_error(nativecatalog::name(status));
    uint8_t* it=g_bpItemById(d.item,g_bpItemReg);if(!readable(it+0x3c0,16)||memcmp(it+0x3c0,d.templateId,16))return calibration_error("E_CALIBRATION_TEMPLATE_CHANGED");
    float low[3],high[3];if(!bp_item_bounds(it,low,high))return calibration_error("E_CALIBRATION_BOUNDS_MISSING");
    for(int a=0;a<3;++a)if(props::abs(low[a]-d.low[a])>.00001||props::abs(high[a]-d.high[a])>.00001)return calibration_error("E_CALIBRATION_BOUNDS_CHANGED");
    return true;
}
static bool bp_calibration_capture(const BpFound& found,const core::BpObject& source){
    calibration::Definition d;d.item=found.item;d.entity=found.entity;calibration_build(&d.buildTime,&d.imageSize);
    memcpy(d.templateId,found.templateId,16);memcpy(d.rotation,found.rot,16);memcpy(d.scale,found.scale,12);
    for(int a=0;a<3;++a){d.position[a]=found.pos[a]/core::kFix;d.local[a]=source.pos[a];}
    const auto status=native_item_resolution(d.item);if(status!=nativecatalog::Ready)return calibration_error(nativecatalog::name(status));
    uint8_t* item=g_bpItemById(d.item,g_bpItemReg);if(!bp_item_bounds(item,d.low,d.high))return calibration_error("E_CALIBRATION_BOUNDS_MISSING");
    const char* name=bp_item_name_cached(d.item);if(!name){const auto* i=itemindex::find(d.item);if(i)name=i->name;}
    copy_text(d.name,name?name:"Native captured furniture",sizeof d.name);if(!calibration_current(d))return false;
    Buf data;data.str("{\"format\":\"BuildingToolsCalibration\",\"version\":1,\"item\":").u(d.item).str(",\"entity\":").u(d.entity).str(",\"template\":\"");
    for(auto b:d.templateId)data.u(b,16,2);data.str("\",\"buildTime\":").u(d.buildTime).str(",\"imageSize\":").u(d.imageSize).str(",\"name\":\"");
    for(const char* s=d.name;*s;++s){if(*s=='"'||*s=='\\')data.c('\\');if((uint8_t)*s>=32)data.c(*s);}data.c('"');
    auto vec=[&](const char* name,const auto* values,int count){data.str(",\"").str(name).str("\":[");for(int a=0;a<count;++a){if(a)data.c(',');data.f(values[a],9);}data.c(']');};
    vec("position",d.position,3);vec("local",d.local,3);vec("rotation",d.rotation,4);vec("scale",d.scale,3);vec("low",d.low,3);vec("high",d.high,3);data.str("}\n");
    if(data.n>=int(sizeof data.s)-1)return calibration_error("E_CALIBRATION_JSON_BUDGET");
    Buf leaf;leaf.str("Calibration_").x(d.item).c('_').u(GetTickCount()).str(".btcalibration");wchar_t wide[192],path[MAX_PATH],folder[MAX_PATH];
    if(!MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,leaf.z(),-1,wide,192)||!project_path(path,wide)||!project_path(folder,L""))return calibration_error("E_CALIBRATION_PATH");
    wchar_t partial[MAX_PATH];if(lstrlenW(path)+6>=MAX_PATH)return calibration_error("E_CALIBRATION_PATH");lstrcpyW(partial,path);lstrcatW(partial,L".part");
    CreateDirectoryW(folder,nullptr);HANDLE file=CreateFileW(partial,GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr);if(file==INVALID_HANDLE_VALUE)return calibration_error("E_CALIBRATION_CREATE");
    DWORD written=0;bool ok=WriteFile(file,data.z(),data.n,&written,nullptr)&&written==(DWORD)data.n&&FlushFileBuffers(file);CloseHandle(file);
    if(!ok||!MoveFileExW(partial,path,MOVEFILE_WRITE_THROUGH))return calibration_error("E_CALIBRATION_WRITE");g_nativeCalibration=d;g_nativeCalibrationValid=true;g_nativeCalibrationError[0]=0;
    Buf message;message.fmt(T(S_EXT_CALIBRATION_READY),leaf.z());flash(message.z());LOG(.str("calibration SAVED code=OK source=classic_native_capture item=").x(d.item).str(" entity=").x(d.entity).str(" file=").str(leaf.z()));return true;
}
static bool calibration_prepare(const calibration::Definition& d){
    ProjectStateGuard guard;if(g_projectRunning)return calibration_error("E_CALIBRATION_BUSY");if(!calibration_current(d))return false;
    extended::Object object;if(!calibration::instance(d,&object))return calibration_error("E_CALIBRATION_GEOMETRY");
    ArchitectureEdit edit;if(!edit.before)return calibration_error("E_CALIBRATION_ALLOCATION");auto old=g_projectObjects;
    g_projectObjects=(extended::Object*)VirtualAlloc(nullptr,sizeof(extended::Object)*kProjectMaxObjects,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
    if(!g_projectObjects||!project_generated_begin(d.name))return calibration_error("E_CALIBRATION_ALLOCATION");
    g_projectManifest.modules[0].category=extended::Furniture;g_projectObjects[0]=object;g_projectObjectCount=g_projectPlan.objects=1;g_projectUseObjects=1;
    props::Bounds b;if(!props::bounds(object.p,object.rotation,d.low,d.high,&b))return calibration_error("E_CALIBRATION_GEOMETRY");
    for(int a=0;a<3;++a){g_projectPlan.lo[a]=prop_floor(b.lo[a]*2)*8;g_projectPlan.hi[a]=prop_ceil(b.hi[a]*2)*8;
        g_projectModuleBoxes[0].lo[a]=g_projectPlan.lo[a];g_projectModuleBoxes[0].hi[a]=g_projectPlan.hi[a];}
    project_generated_finish();edit.committed=true;if(old)VirtualFree(old,0,MEM_RELEASE);g_nativeCalibration=d;g_nativeCalibrationValid=true;g_nativeCalibrationError[0]=0;
    LOG(.str("calibration DRAFT code=OK objects=1 cells=0 pieces=0 source=native_calibration item=").x(d.item));return true;
}
static bool calibration_single_table(){
    const uint32_t item=0xd109e828;const auto status=native_item_resolution(item);if(status!=nativecatalog::Ready)return calibration_error(nativecatalog::name(status));
    calibration::Definition d;d.item=item;d.entity=1; // authoring sentinel; this is NOT a world capture
    calibration_build(&d.buildTime,&d.imageSize);uint8_t* native=g_bpItemById(item,g_bpItemReg);memcpy(d.templateId,native+0x3c0,16);
    if(!bp_item_bounds(native,d.low,d.high))return calibration_error("E_CALIBRATION_BOUNDS_MISSING");copy_text(d.name,"Dev10 - table native unique",sizeof d.name);
    LOG(.str("native single-object demo source=live_registry NOT_CALIBRATED_CAPTURE item=").x(item));return calibration_prepare(d);
}
static void calibration_list(){
    g_nativeCalibrationCount=0;wchar_t pattern[MAX_PATH];if(!project_path(pattern,L"*.btcalibration"))return;
    WIN32_FIND_DATAW entry;HANDLE find=FindFirstFileW(pattern,&entry);if(find==INVALID_HANDLE_VALUE)return;
    do{if(entry.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)continue;if(g_nativeCalibrationCount==64){calibration_error("E_CALIBRATION_LIST_LIMIT");break;}
        lstrcpynW(g_nativeCalibrationFiles[g_nativeCalibrationCount++],entry.cFileName,192);
    }while(FindNextFileW(find,&entry));FindClose(find);
}
static bool calibration_load(int index){
    if(index<0||index>=g_nativeCalibrationCount)return calibration_error("E_CALIBRATION_SELECTION");wchar_t path[MAX_PATH];
    if(!project_path(path,g_nativeCalibrationFiles[index]))return calibration_error("E_CALIBRATION_PATH");DWORD size=0;char* data=read_file(path,&size);
    if(!data)return calibration_error("E_CALIBRATION_READ");calibration::Definition d;project::Error error;
    const bool ok=calibration::decode(data,(int)size,&d,&error);HeapFree(GetProcessHeap(),0,data);if(!ok)return calibration_error(error.message?error.message:"E_CALIBRATION_PARSE");
    return calibration_prepare(d);
}
