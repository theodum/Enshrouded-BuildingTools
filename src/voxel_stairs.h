#pragma once
#include "project_spaces.h"
#include "storey_demo.h"
namespace voxel {
enum Kind { Straight, L, U, Monumental };
enum Support { Solid, Walls };
enum class Issue { None, Dimensions, Rise, Landing, Budget, Obstructed };
struct Spec {int kind=Straight,height=64,width=32,tread=16,steps=8,landing=32,support=Solid,headroom=32;};
struct Surface {extended::Box body;int direction=0;bool landing=false;};
struct Layout {Spec spec;int count=0,rise=0;Surface surfaces[132]{};extended::Box bounds{};};
// Capability of the existing composite cube backend. No sub-cube inference.
constexpr int Cube=8;
inline Issue prepare(const Spec& s,Layout* out) {
    if(!out || s.kind<Straight || s.kind>Monumental || s.support<Solid || s.support>Walls || s.height<8 || s.height>512 || s.width<16 || s.width>256 || s.tread<8 || s.tread>128 || s.steps<1 || s.steps>64 || s.headroom<32 || s.headroom>128)return Issue::Dimensions;
    if(s.width%Cube || s.tread%Cube)return Issue::Dimensions;
    if(s.height%s.steps || (s.height/s.steps)%Cube)return Issue::Rise;
    if(s.landing<s.width || s.landing%Cube || s.landing>512 || (s.kind!=Straight && s.steps<2))return Issue::Landing;
    Layout l;l.spec=s;l.rise=s.height/s.steps;
    auto put=[&](int x,int z,int w,int d,int top,int dir,bool landing){
        auto& p=l.surfaces[l.count++];p={{{x,top?0:-8,z},{x+w,top,z+d}},dir,landing};
    };
    const int w=s.width,d=s.tread,a=s.steps/2,b=s.steps-a,len=s.landing,h=l.rise;
    put(0,-len,s.kind==Monumental?2*w:w,len,0,0,true);
    for(int i=0;i<a;++i)put(0,i*d,s.kind==Monumental?2*w:w,d,(i+1)*h,0,false);
    const int z=a*d;
    if(s.kind==Straight){
        if(a)put(0,z,w,len,a*h,0,true);
        for(int i=0;i<b;++i)put(0,z+(a?len:0)+i*d,w,d,(a+i+1)*h,0,false);
        put(0,s.steps*d+(a?len:0),w,len,s.height,0,true);
    } else if(s.kind==L){
        put(0,z,len,w,a*h,3,true);
        for(int i=0;i<b;++i)put(len+i*d,z,d,w,(a+i+1)*h,3,false);
        put(len+b*d,z,len,w,s.height,3,true);
    } else if(s.kind==U){
        put(0,z,2*w+8,len,a*h,0,true);
        for(int i=0;i<b;++i)put(w+8,z-(i+1)*d,w,d,(a+i+1)*h,2,false);
        put(w+8,z-b*d-len,w,len,s.height,2,true);
    } else {
        put(-w,z,4*w,len,a*h,0,true);
        for(int i=0;i<b;++i){put(-w-(i+1)*d,z,d,w,(a+i+1)*h,1,false);put(3*w+i*d,z,d,w,(a+i+1)*h,3,false);}
        put(-w-b*d-len,z,len,w,s.height,1,true);put(3*w+b*d,z,len,w,s.height,3,true);
    }
    int64_t volume=0,columns=0;
    for(int i=0;i<l.count;++i){const auto& p=l.surfaces[i].body;
        const int64_t area=(int64_t)(p.hi[0]-p.lo[0])*(p.hi[2]-p.lo[2])/64;
        volume+=area*(p.hi[1]-p.lo[1])/8;columns+=area;
        if(area>4096 || columns>project::MaxSpaceColumns || volume>project::MaxCells)return Issue::Budget;
        for(int k=0;k<3;++k){if(!i || p.lo[k]<l.bounds.lo[k])l.bounds.lo[k]=p.lo[k];if(!i || p.hi[k]>l.bounds.hi[k])l.bounds.hi[k]=p.hi[k];}
    }
    for(int k=0;k<3;++k)if(l.bounds.hi[k]-l.bounds.lo[k]>2048)return Issue::Budget;
    for(int i=0;i<l.count;++i){auto air=l.surfaces[i].body;air.lo[1]=air.hi[1];air.hi[1]+=s.headroom;
        for(int j=0;j<l.count;++j)if(extended::intersects(air,l.surfaces[j].body))return Issue::Obstructed;
    }
    *out=l;return Issue::None;
}
inline project::Space air(const Surface& p,int headroom){return {{p.body.lo[0],p.body.hi[1],p.body.lo[2]},{p.body.hi[0],p.body.hi[1]+headroom,p.body.hi[2]}};}
inline bool occupied(const Surface& s,int support,int x,int y,int z){
    const auto& b=s.body;if(support==Solid || y>=b.hi[1]-8)return true;
    if(s.landing)return x==b.lo[0] || x==b.hi[0]-8 || z==b.lo[2] || z==b.hi[2]-8;
    return s.direction&1 ? z==b.lo[2] || z==b.hi[2]-8 : x==b.lo[0] || x==b.hi[0]-8;
}
template<class Cell> bool emit(const Layout& l,Cell cell){
    for(int i=0;i<l.count;++i){const auto& p=l.surfaces[i];const auto& b=p.body;
        for(int y=b.lo[1];y<b.hi[1];y+=8)for(int z=b.lo[2];z<b.hi[2];z+=8)for(int x=b.lo[0];x<b.hi[0];x+=8)
            if(occupied(p,l.spec.support,x,y,z) && !cell(x,y,z))return false;
    }return true;
}
struct Demo {Layout stairs;storeys::Layout shell;int upperZ=0;};
inline Issue prepare_demo(Spec s,Demo* out){
    if(!out)return Issue::Dimensions;s.kind=Straight;Demo d;auto issue=prepare(s,&d.stairs);if(issue!=Issue::None)return issue;
    if(s.height<64)return Issue::Obstructed;
    d.upperZ=s.landing+d.stairs.bounds.hi[2];
    d.shell.width=s.width;d.shell.levelHeight=s.height;d.shell.end=d.upperZ+d.stairs.bounds.hi[2];
    if(d.shell.end+128>2048 || 3*s.height+40>2048)return Issue::Budget;
    for(int i=0;i<3;++i)d.shell.floors[i]={{-48,i*s.height-8,-48},{s.width+48,i*s.height,d.shell.end+64}};
    *out=d;return Issue::None;
}
inline bool demo_reserved(const Demo& d,int x,int y,int z){
    const extended::Box cell={{x,y,z},{x+8,y+8,z+8}};
    for(int level=0;level<2;++level)for(int i=0;i<d.stairs.count;++i){auto b=d.stairs.surfaces[i].body;
        b.lo[1]+=level*d.stairs.spec.height;b.hi[1]+=level*d.stairs.spec.height+d.stairs.spec.headroom;
        b.lo[2]+=level?d.upperZ:d.stairs.spec.landing;b.hi[2]+=level?d.upperZ:d.stairs.spec.landing;
        if(extended::intersects(cell,b))return true;
    }
    return false;
}
template<class Sink> bool emit_demo(const Demo& d,Sink& sink){
    struct ShellSink {const Demo& d;Sink& sink;
        bool module(int i,int kind,const extended::Box& b){return sink.module(i,kind,b);}
        bool cell(int x,int y,int z,int i,int kind){return kind==extended::Floor && demo_reserved(d,x,y,z) ? true : sink.cell(x,y,z,i,kind);}
    } shell{d,sink};
    if(!storeys::emit(d.shell,shell))return false;
    for(int level=0;level<2;++level){const int y=level*d.stairs.spec.height,z=level?d.upperZ:d.stairs.spec.landing;auto b=d.stairs.bounds;b.lo[1]+=y;b.hi[1]+=y;b.lo[2]+=z;b.hi[2]+=z;
        if(!sink.module(16+level,extended::Stairs,b))return false;
        if(!emit(d.stairs,[&](int x,int cy,int cz){return sink.cell(x,cy+y,cz+z,16+level,extended::Stairs);}))return false;
    }return true;
}
} // namespace voxel
