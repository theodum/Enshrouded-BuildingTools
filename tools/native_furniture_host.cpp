// File-only adapter around the production layout, BTBP codec and exporter.
// stdin: material ID/byte, then 8 (ID, low xyz, high xyz) definitions.
// No game access, placement or embedded furniture ID.
#include "../tests/project_export_host.h"
#include "../src/native_furniture.h"
#include <iostream>
int main(int argc,char** argv){
    const bool objectsOnly=argc==3&&!strcmp(argv[1],"--objects");
    if(argc!=2&&!objectsOnly){std::fprintf(stderr,"Usage: native_furniture_host [--objects] NEW_OUTPUT_DIRECTORY < definitions.txt\n");return 2;}
    folder=fs::absolute(argv[objectsOnly?2:1]);if(fs::exists(folder)){std::fprintf(stderr,"Output directory already exists; refusing overwrite\n");return 2;}
    if(objectsOnly){
        if(!(std::cin>>g_projectObjectCount)||g_projectObjectCount<1||g_projectObjectCount>512)return 2;
        for(int& a:g_projectManifest.anchor)if(!(std::cin>>a)||a%8||a< -project::MaxCoord||a>project::MaxCoord)return 2;
        for(int i=0;i<g_projectObjectCount;++i){auto& o=g_projectObjects[i];if(!(std::cin>>o.item)||!o.item)return 2;
            for(double& p:o.p)if(!(std::cin>>p)||!extended::finite(p))return 2;
            double norm=0;for(float& q:o.rotation){if(!(std::cin>>q)||!(q>=-1.001&&q<=1.001))return 2;norm+=q*q;}if(norm<.99||norm>1.01)return 2;}
        strcpy(g_projectManifest.name,"Independent native furnishing - verify in game");g_projectManifest.grid=8;g_projectManifest.count=1;
        char file[128];if(!project_export(file,sizeof file,true))return 1;std::puts(file);return 0;
    }
    uint32_t material=0;int materialByte=0;if(!(std::cin>>material>>materialByte)||!material||materialByte<1||materialByte>255)return 2;
    props::Definition definitions[props::RoleCount];
    for(auto& d:definitions){if(!(std::cin>>d.item))return 2;for(float& v:d.low)if(!(std::cin>>v))return 2;for(float& v:d.high)if(!(std::cin>>v))return 2;}
    props::Demo demo;if(!props::furniture_demo(definitions,&demo)){std::fprintf(stderr,"Selected model does not fit: role index %d\n",demo.badRole);return 1;}
    std::vector<project::CellEntry> cells(10000);g_projectPlan.cells=cells.data();g_projectPlan.capacity=cells.size();
    g_projectPlan.materialCount=1;g_projectPlan.materials[0]={material,(uint8_t)materialByte,0,0};
    if(!props::demo_room([&](int x,int y,int z){cells[g_projectPlan.count++]={{x,y,z},0,0,extended::Wall};return true;}))return 1;
    project::Error error;if(!project::finish(&g_projectPlan,&error))return 1;
    strcpy(g_projectManifest.name,"Dev8 - native furniture test room (11 objects)");g_projectManifest.grid=8;g_projectManifest.count=2;
    g_projectManifest.spaceCount=1;g_projectManifest.spaces[0]={{80,0,8},{112,48,136}};
    g_projectObjectCount=demo.count;memcpy(g_projectObjects,demo.objects,sizeof demo.objects);
    // A new output folder is the only filesystem mutation.
    if(!fs::exists(folder.parent_path())){std::fprintf(stderr,"Output parent missing\n");return 2;}
    char filename[128];if(!project_export(filename,sizeof filename)){std::fprintf(stderr,"Export failed; no complete manifest published\n");return 1;}
    std::printf("%s\n",filename);return 0;
}
