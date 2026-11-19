#include "architecture_host.h"
static int checks=0,failures=0;
#define CHECK(c) do{++checks;if(!(c)){std::printf("FAIL storeys line %d: %s\n",__LINE__,#c);++failures;}}while(0)
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
