// Offline asset generator. Reuses the production authoring/export code with the
// same file-only host adapters as the regression tests. Never opens a game/world.
#include "../tests/architecture_host.h"
#include <iostream>
static std::vector<uint8_t> read_bytes(const fs::path& p){std::ifstream f(p,std::ios::binary);if(!f)throw std::runtime_error("Cannot read "+p.string());return {(std::istreambuf_iterator<char>(f)),{}};}
int main(int argc,char** argv){try{
    if(argc!=5 || std::string(argv[1])!="--material-from" || (std::string(argv[3])!="--output"&&std::string(argv[3])!="--verify"))throw std::runtime_error("Usage: make_voxel_example --material-from source.btbp --output NEW-DIRECTORY (or --verify FIXTURE-DIRECTORY)");
    const bool verify=std::string(argv[3])=="--verify";fs::path expected=argv[4];
    exportFolder=verify?fs::current_path()/"build"/("voxel-fixture-check-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count())):fs::path(argv[4]);
    if(fs::exists(exportFolder))throw std::runtime_error("Refusing to overwrite existing output directory");
    auto bytes=read_bytes(argv[2]);core::BpHeader header{};std::vector<uint8_t> cells(65536);
    if(!core::bp_decode(bytes.data(),bytes.size(),&header,cells.data(),cells.size()) || header.nm<1 || !header.m[0].item || header.m[0].mat<0x80)throw std::runtime_error("Invalid source material");
    hostMaterial=g_catalogMaterial=header.m[0].item;*(uint32_t*)block=hostMaterial;block[0x454]=header.m[0].mat;
    g_nativePartsAvailable=g_bounds=false;g_catalogPiece=0;voxel::Spec spec;const int zero[]={0,0,0};
    if(!architecture_voxels(spec,zero,0,false,true))throw std::runtime_error("Demo generation failed");
    fs::create_directories(exportFolder.parent_path());char filename[128];if(!project_export(filename,sizeof filename))throw std::runtime_error("Export failed");
    bytes=read_bytes(exportFolder/filename);std::string text(bytes.begin(),bytes.end());project::Manifest manifest{};project::Error error;
    if(!project::decode(text.data(),text.size(),&manifest,&error))throw std::runtime_error("Exported manifest invalid");
    for(int i=0;i<manifest.count;++i){char name[80];std::snprintf(name,sizeof name,"Dev7_Voxel_M%02d.btbp",i);const std::string old=manifest.modules[i].file;
        if(fs::exists(exportFolder/name))throw std::runtime_error("Output filename collision");fs::rename(exportFolder/old,exportFolder/name);
        auto at=text.find('"'+old+'"');if(at==std::string::npos)throw std::runtime_error("Missing module filename");text.replace(at+1,old.size(),name);
    }
    const char* name="Dev7_Batiment_trois_niveaux_voxels.btproject";if(fs::exists(exportFolder/name))throw std::runtime_error("Manifest collision");
    fs::rename(exportFolder/filename,exportFolder/name);{std::ofstream f(exportFolder/name,std::ios::binary|std::ios::trunc);f.write(text.data(),text.size());if(!f)throw std::runtime_error("Manifest write failed");}
    int count=0;for(const auto& f:fs::directory_iterator(exportFolder)){++count;if(verify && read_bytes(f.path())!=read_bytes(expected/f.path().filename()))throw std::runtime_error("Fixture differs: "+f.path().filename().string());}
    if(verify){int known=0;for(const auto& f:fs::directory_iterator(expected))if(f.path().extension()==".btbp"||f.path().extension()==".btproject")++known;if(known!=count)throw std::runtime_error("Fixture file count differs");}
    std::printf("voxel example %s: %d files, %d cells, %d modules, %d spaces, native pieces=%d\n",verify?"verified":"created",count,g_projectPlan.count,manifest.count,manifest.spaceCount,manifest.pieceCount);
    std::free(g_projectPlan.cells);return 0;
}catch(const std::exception& e){std::fprintf(stderr,"%s\n",e.what());return 1;}}
