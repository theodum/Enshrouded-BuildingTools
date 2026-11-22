// Compile the actual native exporter against file-only Win32 adapters. Exercises
// publication, read-back, bounds, native-piece IDs and non-overwrite semantics.
#include "project_export_host.h"
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
    // Export only furniture from a draft which still contains structure/pieces.
    ++tick;failFlush=-1;g_projectManifest.spaceCount=0;
    CHECK(project_export(filename,sizeof filename,true));bytes=read(folder/filename);
    CHECK(project::decode((char*)bytes.data(),(int)bytes.size(),&parsed,&error));
    CHECK(parsed.count==1&&parsed.pieceCount==0&&parsed.spaceCount==0&&parsed.anchor[0]==8);
    auto props=read(folder/parsed.modules[0].file);core::BpHeader h{};uint8_t empty;
    CHECK(core::bp_decode(props.data(),props.size(),&h,&empty,1));CHECK(h.nm==0&&h.objects==1&&empty==core::BP_EMPTY);
    CHECK(g_projectPlan.count==3&&g_projectManifest.pieceCount==1);
    std::printf("export: %d failures\n",failures);return failures!=0;
}
