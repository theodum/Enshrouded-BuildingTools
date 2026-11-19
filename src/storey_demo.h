// Parametric three-storey test project. Integer sixteenths of a metre.
// Geometry proves reserved volumes and nominal connections, never walkability
// of an unknown native model. The player supplies its calibrated rise/direction.
#pragma once
#include "extended_core.h"
namespace storeys {
enum class Issue { None, Dimensions, Rise, Budget, Clearance };
struct Layout {
    int width=0, height=0, run=0, rise=0, flightsPerLevel=0, levelHeight=0;
    int landing=0, end=0, count=0;
    extended::Box floors[3]{}, bodies[32]{}, clearance[32]{};
};
inline Issue prepare(int width,int height,int run,int rise,Layout* out) {
    if (!out || width<16 || width>256 || height<8 || height>256 || run<8 || run>512 || width%8 || height%8 || run%8) return Issue::Dimensions;
    if (rise<8 || rise>256 || rise%8) return Issue::Rise;
    Layout l; l.width=width;l.height=height;l.run=run;l.rise=rise;
    l.flightsPerLevel=(64+rise-1)/rise;l.levelHeight=l.flightsPerLevel*rise;
    l.count=2*l.flightsPerLevel;l.landing=width>32?width:32;
    l.end=2*l.flightsPerLevel*run+l.landing;
    if (l.count>32 || l.end+128>2048 || width+112>2048 || 3*l.levelHeight+40>2048) return Issue::Budget;
    // Same 4096-voxel native part budget and 2 m clearance as the runtime.
    if ((int64_t)(width/8)*(height/8+4)*(run/8)>4096) return Issue::Budget;
    if (height-rise+32>l.levelHeight) return Issue::Clearance;
    for (int level=0;level<3;++level) l.floors[level]={{-48,level*l.levelHeight-8,-48},{width+48,level*l.levelHeight,l.end+64}};
    for (int level=0;level<2;++level) for (int f=0;f<l.flightsPerLevel;++f) {
        const int i=level*l.flightsPerLevel+f, y=level*l.levelHeight+f*rise;
        const int z=level*(l.flightsPerLevel*run+l.landing)+f*run;
        l.bodies[i]={{0,y,z},{width,y+height,z+run}};
        l.clearance[i]=l.bodies[i];l.clearance[i].hi[1]+=32;
    }
    *out=l;return Issue::None;
}
inline bool reserved(const Layout& l,int x,int y,int z) {
    extended::Box cell={{x,y,z},{x+8,y+8,z+8}};
    for (int i=0;i<l.count;++i) if(extended::intersects(cell,l.clearance[i]))return true;
    return false;
}
// Sink.module(index,category,box) and Sink.cell(x,y,z,index,category) return false
// on capacity/failure. The native adapter owns the surrounding draft transaction.
template<class Sink> bool emit(const Layout& l,Sink& sink) {
    int module=0;
    for (int level=0;level<3;++level,++module) {
        const auto& b=l.floors[level];
        if(!sink.module(module,extended::Floor,b))return false;
        for(int z=b.lo[2];z<b.hi[2];z+=8)for(int x=b.lo[0];x<b.hi[0];x+=8)
            if(!reserved(l,x,b.lo[1],z) && !sink.cell(x,b.lo[1],z,module,extended::Floor))return false;
    }
    // Four wall modules per storey. Doors are openings, not logic-linked props.
    const auto& base=l.floors[0];
    for (int level=0;level<3;++level) for(int side=0;side<4;++side,++module) {
        const bool front=side<2;
        extended::Box b={{base.lo[0],level*l.levelHeight,base.lo[2]},{base.hi[0],(level+1)*l.levelHeight,base.hi[2]}};
        if(front) { if(side==0)b.hi[2]=b.lo[2]+8;else b.lo[2]=b.hi[2]-8; }
        else { if(side==2)b.hi[0]=b.lo[0]+8;else b.lo[0]=b.hi[0]-8; }
        if(!sink.module(module,extended::Wall,b))return false;
        const int middle=l.width/2/8*8;
        for(int y=b.lo[1];y<b.hi[1];y+=8)for(int z=b.lo[2];z<b.hi[2];z+=8)for(int x=b.lo[0];x<b.hi[0];x+=8) {
            const int localY=y-level*l.levelHeight;
            bool opening=front && x>=middle-16 && x<middle+16 && localY<40; // 2 m x 2.5 m door bays
            if(!front && localY>=16 && localY<40 && ((z+48)/64)%2==0 && z>base.lo[2]+8 && z<base.hi[2]-16)opening=true;
            if(!opening && !sink.cell(x,y,z,module,extended::Wall))return false;
        }
    }
    extended::Component roof;roof.kind=extended::Roof;roof.size[0]=(base.hi[0]-base.lo[0]+16)/8;
    roof.size[1]=4;roof.size[2]=(base.hi[2]-base.lo[2]+16)/8;roof.thickness=1;
    const int origin[]={base.lo[0]-8,3*l.levelHeight,base.lo[2]-8};
    extended::Box bounds{};for(int a=0;a<3;++a){bounds.lo[a]=origin[a];bounds.hi[a]=origin[a]+roof.size[a]*8;}
    if(!extended::valid(roof) || !sink.module(module,extended::Roof,bounds))return false;
    for(int y=0;y<roof.size[1];++y)for(int z=0;z<roof.size[2];++z)for(int x=0;x<roof.size[0];++x)
        if(extended::component_cell(roof,x,y,z) && !sink.cell(origin[0]+x*8,origin[1]+y*8,origin[2]+z*8,module,extended::Roof))return false;
    return true;
}
} // namespace storeys
