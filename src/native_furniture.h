// Reusable measured furniture arrangement. All IDs/bounds supplied by the live
// native registry; test fixtures supply explicitly synthetic definitions.
#pragma once
#include "native_props.h"
namespace props {
enum Role {Bed,Table,Chair,Torch,Painting,Chest,SmallDecoration,FloorDecoration,RoleCount};
struct Definition {uint32_t item=0;float low[3]{},high[3]{};};
struct Demo {extended::Object objects[11]{};Bounds boxes[11]{};int count=0,badRole=-1;};
inline bool furniture_demo(const Definition* definitions,Demo* output){
    Demo d;
    auto place=[&](int role,double x,double y,double z,int turn,int wall){
        d.badRole=role;const auto& def=definitions[role];if(!def.item)return false;
        auto& o=d.objects[d.count];o.item=def.item;o.module=1;const float identity[]={0,0,0,1};core::bp_turn_rot(turn,identity,o.rotation);
        const double zero[]={0,0,0};Bounds b;if(!bounds(zero,o.rotation,def.low,def.high,&b))return false;
        o.p[0]=x-(b.lo[0]+b.hi[0])*.5;o.p[1]=y-b.lo[1];o.p[2]=z-(b.lo[2]+b.hi[2])*.5;
        if(wall==1){o.p[0]=.5-b.lo[0];o.p[1]=y-(b.lo[1]+b.hi[1])*.5;}
        if(wall==2){o.p[2]=9.5-b.hi[2];o.p[1]=y-(b.lo[1]+b.hi[1])*.5;}
        if(!bounds(o.p,o.rotation,def.low,def.high,&d.boxes[d.count]))return false;
        const auto& world=d.boxes[d.count];
        const double lo[]={.5,0,.5},hi[]={11.5,4.5,9.5};
        for(int a=0;a<3;++a)if(world.lo[a]<lo[a]-Contact||world.hi[a]>hi[a]+Contact)return false;
        for(int i=0;i<d.count;++i)if(intersects(world,d.boxes[i]))return false;
        Bounds aisle{{5,0,.5},{7,3,8.5}};if(intersects(world,aisle))return false;
        ++d.count;return true;
    };
    bool ok=place(Bed,2.5,0,3,0,0)&&place(Table,8.5,0,4.5,0,0);
    if(ok){const auto b=d.boxes[1];ok=place(Chair,8.5,0,b.lo[2]-.8,0,0)&&place(Chair,8.5,0,b.hi[2]+.8,2,0);}
    ok=ok&&place(Torch,0,2.5,2.5,1,1)&&place(Torch,0,2.5,7.5,1,1)&&place(Painting,6,2.6,0,2,2)&&place(Chest,2.5,0,8,0,0);
    if(ok){const auto b=d.boxes[1];const double dx=(b.hi[0]-b.lo[0])*.25;
        ok=place(SmallDecoration,8.5-dx,b.hi[1],4.5,0,0)&&place(SmallDecoration,8.5+dx,b.hi[1],4.5,0,0);}
    ok=ok&&place(FloorDecoration,10,0,8,0,0);
    if(ok)d.badRole=-1;*output=d;return ok;
}
template<class Sink> bool demo_room(Sink sink){
    for(int y=-1;y<10;++y)for(int z=0;z<20;++z)for(int x=0;x<24;++x){
        bool occupied=y==-1||y==9||x==0||x==23||z==0||z==19;
        if(z==0&&x>=10&&x<14&&y>=0&&y<6)occupied=false;
        if(occupied&&!sink(x*8,y*8,z*8))return false;
    }
    return true;
}
} // namespace props
