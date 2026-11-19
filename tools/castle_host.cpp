// Offline bridge to the production stair layout, BTBP reader and composite planner.
// This program has no game/world adapters and performs no native placement.
#include "../src/voxel_stairs.h"
#include <cstdio>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>
#include <filesystem>

static void require(bool ok, const std::string& message) {
    if (!ok) throw std::runtime_error(message);
}
static void profiles(bool spacious=false) {
    std::printf("{\"source\":\"src/voxel_stairs.h\",\"profiles\":{");
    const char* names[]={"monumental","L","U"};
    const int kinds[]={voxel::Monumental,voxel::L,voxel::U};
    for(int k=0;k<3;++k) {
        voxel::Spec s;s.kind=kinds[k];s.height=spacious?80:72;s.width=spacious?40:24;s.tread=8;
        s.steps=spacious?10:9;s.landing=spacious?48:24;s.headroom=spacious?56:40;s.support=voxel::Solid;
        voxel::Layout l;require(voxel::prepare(s,&l)==voxel::Issue::None,"Stair profile refused");
        if(k)std::printf(",");
        std::printf("\"%s\":{\"rise\":1,\"height\":%d,\"headroom\":%d,\"width\":%d,\"landing\":%d,\"surfaces\":[",names[k],s.height/8,s.headroom/8,s.width/8,s.landing/8);
        for(int i=0;i<l.count;++i) {
            const auto& b=l.surfaces[i].body;
            if(i)std::printf(",");
            std::printf("{\"low\":[%d,%d,%d],\"high\":[%d,%d,%d],\"landing\":%s}",
                b.lo[0]/8,b.lo[1]/8,b.lo[2]/8,b.hi[0]/8,b.hi[1]/8,b.hi[2]/8,
                l.surfaces[i].landing?"true":"false");
        }
        std::printf("],\"cells\":[");bool first=true;
        require(voxel::emit(l,[&](int x,int y,int z){
            std::printf("%s[%d,%d,%d]",first?"":",",x/8,y/8,z/8);first=false;return true;
        }),"Stair emission failed");
        std::printf("]}");
    }
    std::printf("}}\n");
}
static std::vector<uint8_t> read(const std::filesystem::path& p) {
    std::ifstream f(p,std::ios::binary);require(bool(f),"Missing file: "+p.string());
    return {(std::istreambuf_iterator<char>(f)),{}};
}
static void validate(const std::filesystem::path& path) {
    const auto bytes=read(path);project::Manifest m{};project::Error e;
    require(project::decode((const char*)bytes.data(),bytes.size(),&m,&e),
        std::string("Manifest: ")+(e.message?e.message:"invalid"));
    std::vector<project::CellEntry> cells(project::MaxCells);
    project::Plan p{cells.data(),(int)cells.size()};
    for(int i=0;i<m.count;++i) {
        auto data=read(path.parent_path()/m.modules[i].file);
        core::BpHeader h{};std::vector<uint8_t> decoded(65536);
        require(core::bp_decode(data.data(),data.size(),&h,decoded.data(),decoded.size()),
            std::string("Invalid BTBP: ")+m.modules[i].file);
        require(h.objects==0,"Unexpected objects");
        require(project::add(&p,m.modules[i],i,h,decoded.data(),&e),
            std::string("Plan: ")+(e.message?e.message:"invalid"));
    }
    require(project::finish(&p,&e),std::string("Conflicting plan: ")+(e.message?e.message:"invalid"));
    int bad=-1;
    auto issue=spaces::validate(p,m,&bad);
    require(issue==spaces::Issue::None,"Reserved space refused at index "+std::to_string(bad)+" issue "+std::to_string((int)issue));
    require(m.pieceCount==0 && p.objects==0,"Experimental native dependency");
    require(p.duplicates==0,"Unexpected duplicated source cells");
    // Exact global volume rotation, including negative offsets and the anchor.
    for(int r=0;r<4;++r)for(int i=0;i<p.count;++i) {
        const auto& c=p.cells[i];int q[3];project::cell_low(c,m.anchor,r,q);
        int x=c.p[0]-m.anchor[0],z=c.p[2]-m.anchor[2];
        for(int n=0;n<r;++n){int next=-z-8;z=x;x=next;}
        require(q[0]==x && q[1]==c.p[1]-m.anchor[1] && q[2]==z,"Rotation mismatch");
    }
    // Both check-only work and stopped construction use the actual Job state machine.
    int batches=0;
    for(int limit:{1,32,128,256}) {
        project::Job job;job.start(p.count);
        while(job.phase==project::Phase::Checking){int n=job.prepare(limit);require(job.checked(n),"Preflight failed");}
        require(job.completed==0,"Preflight advanced construction");
        while(job.phase==project::Phase::Building){int n=job.prepare(limit);require(job.committed(n),"Ack failed");if(limit==128)++batches;}
        require(job.completed==p.count && job.phase==project::Phase::Complete,"Incomplete job");
        for(int stop:{0,limit,p.count/2,p.count-1}) {
            job.start(p.count);
            while(job.phase==project::Phase::Checking){int n=job.prepare(limit);require(job.checked(n),"Check failed");}
            while(job.completed<stop && job.phase==project::Phase::Building){int n=job.prepare(limit);require(job.committed(n),"Ack failed");}
            int acknowledged=job.completed;job.phase=project::Phase::Stopped;
            require(job.prepare(limit)==0 && !job.committed(1) && job.completed==acknowledged,"Stop advanced job");
        }
        job.start(p.count);int n=job.prepare(limit);
        require(!job.checked(n-1) && job.phase==project::Phase::Stopped && job.completed==0,"Partial check accepted");
    }
    std::printf("{\"result\":\"PASS\",\"cells\":%d,\"modules\":%d,\"spaces\":%d,\"materials\":%d,\"duplicates\":%d,\"nativePieces\":%d,\"objects\":%d,\"batches128\":%d,\"boundsMeters\":[[%g,%g,%g],[%g,%g,%g]],\"rotations\":4,\"batchSizes\":[1,32,128,256],\"interruptionStateMachine\":\"PASS\",\"gameplayTested\":false}\n",
        p.count,m.count,m.spaceCount,p.materialCount,p.duplicates,m.pieceCount,p.objects,batches,
        p.lo[0]/16.,p.lo[1]/16.,p.lo[2]/16.,p.hi[0]/16.,p.hi[1]/16.,p.hi[2]/16.);
}
int main(int argc,char** argv) {
    try {
        if(argc==2 && std::string(argv[1])=="--profiles")profiles();
        else if(argc==2 && std::string(argv[1])=="--profiles-spacious")profiles(true);
        else if(argc==3 && std::string(argv[1])=="--validate")validate(argv[2]);
        else throw std::runtime_error("Usage: castle_host --profiles | --validate PROJECT.btproject");
        return 0;
    } catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
