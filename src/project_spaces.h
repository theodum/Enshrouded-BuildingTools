#pragma once
#include "extended_core.h"
namespace spaces {
enum class Issue { None, Invalid, Budget, Obstructed, NoSurface };
inline extended::Box box(const project::Space& s){return {{s.lo[0],s.lo[1],s.lo[2]},{s.hi[0],s.hi[1],s.hi[2]}};}
inline bool surface(const project::Plan& p,int x,int y,int z) {
    project::CellEntry key{{x,y,z},0,0,0};int lo=0,hi=p.count;
    while(lo<hi){int mid=lo+(hi-lo)/2;if(project::compare(p.cells[mid],key)<0)lo=mid+1;else hi=mid;}
    return lo<p.count && project::compare(p.cells[lo],key)==0;
}
inline Issue validate(const project::Plan& p,const project::Manifest& m,int* bad=nullptr) {
    if(m.spaceCount<0 || m.spaceCount>project::MaxSpaces)return Issue::Budget;
    int64_t columns=0;
    for(int i=0;i<m.spaceCount;++i){if(bad)*bad=i;const auto& s=m.spaces[i];
        for(int a=0;a<3;++a)if(s.lo[a]%8 || s.hi[a]%8 || s.lo[a]<-project::MaxCoord || s.hi[a]>project::MaxCoord || s.hi[a]<=s.lo[a])return Issue::Invalid;
        const int64_t area=(int64_t)(s.hi[0]-s.lo[0])*(s.hi[2]-s.lo[2])/64;
        if(s.hi[1]-s.lo[1]<32 || s.hi[1]-s.lo[1]>128)return Issue::Invalid;
        columns+=area;if(area>4096 || columns>project::MaxSpaceColumns)return Issue::Budget;
        if(extended::plan_intersects(p,box(s)))return Issue::Obstructed;
        for(int z=s.lo[2];z<s.hi[2];z+=8)for(int x=s.lo[0];x<s.hi[0];x+=8)
            if(!surface(p,x,s.lo[1]-8,z))return Issue::NoSurface;
    }return Issue::None;
}
// Every air column is owned by the voxel immediately below it, independently
// of module/category (deduplication may assign a landing to a floor module).
template<class Visit> bool visit(const project::CellEntry& c,const project::Manifest& m,int rotation,const core::I3& origin,Visit visitor) {
    int end=c.p[1]+8;
    for(int i=0;i<m.spaceCount;++i){const auto& s=m.spaces[i];if(s.lo[1]==c.p[1]+8 && c.p[0]>=s.lo[0]&&c.p[0]<s.hi[0]&&c.p[2]>=s.lo[2]&&c.p[2]<s.hi[2]&&s.hi[1]>end)end=s.hi[1];}
    auto air=c;for(int y=c.p[1]+8;y<end;y+=8){air.p[1]=y;if(!visitor(project::position(air,m.anchor,rotation,origin)))return false;}return true;
}
} // namespace spaces
