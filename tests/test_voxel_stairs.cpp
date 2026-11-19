#include "architecture_host.h"
#include "../src/project_actions.h"
#include <set>
#include <tuple>
static int checks=0,failures=0;
#define CHECK(c) do{++checks;if(!(c)){std::printf("FAIL voxel stairs line %d: %s\n",__LINE__,#c);++failures;}}while(0)
static const int zero[]={0,0,0};
static bool adjacent(const voxel::Surface& a,const voxel::Surface& b,int rise){
    const auto& x=a.body;const auto& y=b.body;if(std::abs(x.hi[1]-y.hi[1])>rise)return false;
    const bool overlapX=x.lo[0]<y.hi[0]&&x.hi[0]>y.lo[0],overlapZ=x.lo[2]<y.hi[2]&&x.hi[2]>y.lo[2];
    return (overlapX&&(x.hi[2]==y.lo[2]||y.hi[2]==x.lo[2])) || (overlapZ&&(x.hi[0]==y.lo[0]||y.hi[0]==x.lo[0]));
}
static void layout_tests(){
    *(uint32_t*)block=100;block[0x454]=0x9d;g_nativePartsAvailable=g_bounds=false;g_catalogPiece=999;
    for(int kind=0;kind<4;++kind)for(int support=0;support<2;++support){voxel::Spec s;s.kind=kind;s.support=support;voxel::Layout l;
        CHECK(voxel::prepare(s,&l)==voxel::Issue::None);CHECK(l.rise==8);
        // Geometric adjacency graph only: it does not model the player's step height.
        std::vector<bool> reached(l.count);reached[0]=true;
        for(int n=0;n<l.count;++n)for(int a=0;a<l.count;++a)if(reached[a])for(int b=0;b<l.count;++b)if(adjacent(l.surfaces[a],l.surfaces[b],l.rise))reached[b]=true;
        for(bool yes:reached)CHECK(yes);
        for(int r=0;r<4;++r){const int offset[]={32,-16,48};CHECK(architecture_voxels(s,offset,r,false));CHECK(g_projectManifest.pieceCount==0&&g_projectObjectCount==0);
            CHECK(g_projectManifest.spaceCount==l.count);CHECK(spaces::validate(g_projectPlan,g_projectManifest)==spaces::Issue::None);
            for(int i=0;i<g_projectPlan.count;++i){const auto& c=g_projectPlan.cells[i];CHECK(c.material==0&&c.category==extended::Stairs);for(int a=0;a<3;++a)CHECK(c.p[a]%8==0);}
            std::set<std::tuple<int64_t,int64_t,int64_t>> samples;int calls=0;
            for(int i=0;i<g_projectPlan.count;++i)CHECK(spaces::visit(g_projectPlan.cells[i],g_projectManifest,1,core::I3{},[&](const core::I3& p){++calls;samples.emplace(p.x,p.y,p.z);return true;}));
            CHECK(calls>0 && calls==(int)samples.size());
        }
    }
    voxel::Spec s;voxel::Layout l;s.steps=16;CHECK(voxel::prepare(s,&l)==voxel::Issue::Rise); // 0.25 m cannot use this backend
    s={};s.height=60;CHECK(voxel::prepare(s,&l)==voxel::Issue::Rise);
    s={};s.width=24;s.landing=24;CHECK(voxel::prepare(s,&l)==voxel::Issue::None); // odd number of width voxels
    s.landing=16;CHECK(voxel::prepare(s,&l)==voxel::Issue::Landing);
    s={};s.kind=voxel::U;s.steps=1;CHECK(voxel::prepare(s,&l)==voxel::Issue::Landing);
    s={};s.width=30;CHECK(voxel::prepare(s,&l)==voxel::Issue::Dimensions);
    s={};s.tread=128;s.steps=64;s.height=512;CHECK(voxel::prepare(s,&l)==voxel::Issue::Budget);
    for(int kind=0;kind<4;++kind)for(int n=2;n<=13;++n)for(int width:{16,24,32})for(int tread:{8,16})for(int rise:{8,16}){
        s={};s.kind=kind;s.steps=n;s.height=n*rise;s.width=width;s.landing=width;s.tread=tread;
        CHECK(voxel::prepare(s,&l)==voxel::Issue::None);CHECK(l.rise==rise);
        std::vector<bool> reachable(l.count);reachable[0]=true;
        for(int pass=0;pass<l.count;++pass)for(int a=0;a<l.count;++a)if(reachable[a])for(int b=0;b<l.count;++b)if(adjacent(l.surfaces[a],l.surfaces[b],rise))reachable[b]=true;
        for(bool reached:reachable)CHECK(reached);
    }
}
static void collision_tests(){
    voxel::Spec s;CHECK(architecture_voxels(s,zero,0,false));
    int seen=0;bool stopped=false;
    for(int i=0;i<g_projectPlan.count && !stopped;++i){const bool ok=spaces::visit(g_projectPlan.cells[i],g_projectManifest,0,core::I3{},[&](const core::I3&){++seen;return false;});if(!ok)stopped=true;}
    CHECK(stopped&&seen==1); // first failed coverage/collision probe terminates the visitor
    const int count=g_projectPlan.count;auto pointer=g_projectPlan.cells;std::vector<project::CellEntry> old(pointer,pointer+count);
    extended::Component wall;wall.kind=extended::Wall;wall.size[0]=wall.size[1]=wall.size[2]=1;
    const int obstruct[]={0,8,0};CHECK(!architecture_component(wall,obstruct,0,true,100));
    CHECK(g_projectPlan.cells==pointer&&g_projectPlan.count==count&&!std::memcmp(old.data(),pointer,count*sizeof(*pointer)));
    s.steps=16;CHECK(!architecture_voxels(s,zero,0,true));CHECK(g_projectPlan.cells==pointer&&g_projectPlan.count==count);
    s={};allocFail=1;CHECK(!architecture_voxels(s,zero,0,false));allocFail=-1;CHECK(g_projectPlan.cells==pointer&&g_projectPlan.count==count);
    auto plan=g_projectPlan;auto air=spaces::box(g_projectManifest.spaces[1]);
    auto corrupt=old;corrupt.push_back({{air.lo[0],air.lo[1],air.lo[2]},0,0,extended::Wall});
    plan.cells=corrupt.data();plan.count=corrupt.size();plan.capacity=corrupt.size();project::Error error;CHECK(project::finish(&plan,&error));CHECK(spaces::validate(plan,g_projectManifest)==spaces::Issue::Obstructed);
    plan=g_projectPlan;corrupt=old;const auto& first=g_projectManifest.spaces[0];
    for(auto it=corrupt.begin();it!=corrupt.end();++it)if(it->p[0]==first.lo[0]&&it->p[1]==first.lo[1]-8&&it->p[2]==first.lo[2]){corrupt.erase(it);break;}
    plan.cells=corrupt.data();plan.count=corrupt.size();CHECK(spaces::validate(plan,g_projectManifest)==spaces::Issue::NoSurface);
}
static void parser_tests(){
    const std::string prefix=R"({"format":"BuildingToolsExtendedProject","version":1,"name":"space","gridSize":0.5,"anchor":[0,0,0],"modules":[{"file":"a.btbp","offset":[0,0,0]}])";
    const std::string box=R"({"low":[0,0.5,0],"high":[1,2.5,1]})";
    project::Manifest m{};project::Error e;
    auto parse=[&](const std::string& list){const auto json=prefix+",\"spaces\":"+list+"}";return project::decode(json.data(),json.size(),&m,&e);};
    CHECK(parse("["+box+"]")&&m.spaceCount==1);
    std::string old=prefix+"}";CHECK(project::decode(old.data(),old.size(),&m,&e)&&m.spaceCount==0);
    CHECK(!parse(R"([{"low":[0,0,0],"high":[1,1,1]}])"));
    CHECK(!parse(R"([{"low":[0,0.25,0],"high":[1,2.5,1]}])"));
    CHECK(!parse(R"([{"low":[0,0,0],"low":[0,0,0],"high":[1,2,1]}])"));
    CHECK(!parse(R"([{"low":[0,0,0],"high":[1,2,1],"unknown":1}])"));
    std::string many="[";for(int i=0;i<17;++i){if(i)many+=',';many+=R"({"low":[0,0,0],"high":[32,2,32]})";}many+=']';CHECK(!parse(many));
    many="[";for(int i=0;i<project::MaxSpaces+1;++i){if(i)many+=',';many+=box;}many+=']';CHECK(!parse(many));
}
static void demo_tests(){
    voxel::Spec s;
    for(int support=0;support<2;++support)for(int rotation=0;rotation<4;++rotation){s.support=support;s.kind=voxel::Monumental;
        CHECK(architecture_voxels(s,zero,rotation,false,true));CHECK(g_projectManifest.count==18&&g_projectManifest.pieceCount==0&&g_projectManifest.spaceCount==22);
        CHECK(spaces::validate(g_projectPlan,g_projectManifest)==spaces::Issue::None);
    }
    s={};CHECK(architecture_voxels(s,zero,0,false,true));CHECK(g_projectManifest.count==18&&g_projectManifest.pieceCount==0&&g_projectManifest.spaceCount==22);
    CHECK(spaces::validate(g_projectPlan,g_projectManifest)==spaces::Issue::None);
    project::ActionContext c;c.window=c.loaded=c.local=c.enabled=c.mode=c.systems=c.bases=true;c.step[0]=c.step[1]=c.step[2]=0.5f;c.cells=g_projectPlan.count;c.native=false;
    CHECK(project::action_issue(c)==project::ActionIssue::None); // no native experimental opt-in required
    fs::path parent=fs::current_path()/"build";fs::create_directories(parent);exportFolder=parent/("voxel-export-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    char filename[128];CHECK(project_export(filename,sizeof filename));
    auto read=[](const fs::path& p){std::ifstream f(p,std::ios::binary);return std::vector<uint8_t>((std::istreambuf_iterator<char>(f)),{});};
    auto bytes=read(exportFolder/filename);project::Manifest decoded{};project::Error e;CHECK(project::decode((const char*)bytes.data(),bytes.size(),&decoded,&e));
    CHECK(decoded.spaceCount==22&&decoded.pieceCount==0);for(int i=0;i<decoded.spaceCount;++i)for(int a=0;a<3;++a)CHECK(decoded.spaces[i].lo[a]==g_projectManifest.spaces[i].lo[a]&&decoded.spaces[i].hi[a]==g_projectManifest.spaces[i].hi[a]);
    std::vector<project::CellEntry> cells(project::MaxCells);project::Plan p{cells.data(),(int)cells.size()};
    for(int i=0;i<decoded.count;++i){auto data=read(exportFolder/decoded.modules[i].file);core::BpHeader h{};std::vector<uint8_t> source(65536);CHECK(core::bp_decode(data.data(),data.size(),&h,source.data(),source.size()));CHECK(project::add(&p,decoded.modules[i],i,h,source.data(),&e));}
    CHECK(project::finish(&p,&e));CHECK(p.count==g_projectPlan.count);CHECK(spaces::validate(p,decoded)==spaces::Issue::None);
    for(int i=0;i<p.count;++i){for(int a=0;a<3;++a)CHECK(p.cells[i].p[a]==g_projectPlan.cells[i].p[a]);CHECK(p.cells[i].category==g_projectPlan.cells[i].category);}
    CHECK(!project_export(filename,sizeof filename));
    std::printf("voxel demo: cells=%d source modules=%d exported modules=%d spaces=%d native pieces=0\n",p.count,g_projectManifest.count,decoded.count,decoded.spaceCount);
}
int main(){layout_tests();collision_tests();parser_tests();demo_tests();std::free(g_projectPlan.cells);std::printf("voxel stairs: %d checks, %d failures\n",checks,failures);return failures?1:0;}
