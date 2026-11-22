// Geometry/admission for native props. Placement remains bp_place_object/PlaceProp.
#pragma once
#include "extended_core.h"
namespace props {
constexpr int MaxObjects=2048,MaxWorldObjects=512;
constexpr double Contact=0.01; // tolerance for float model bounds and saved transforms
struct Bounds {double lo[3]{},hi[3]{};};
struct WorldObject {uint32_t item=0;Bounds box;uint32_t entity=0;};
inline bool bounds(const double* p,const float* q,const float* low,const float* high,Bounds* out,bool allowDegenerate=false) {
    if(!out)return false;double norm=0;for(int a=0;a<4;++a){if(!(q[a]>=-1.001f&&q[a]<=1.001f))return false;norm+=q[a]*q[a];}
    if(norm<0.99||norm>1.01)return false;
    for(int a=0;a<3;++a)if(!extended::finite(p[a])||!extended::finite(low[a])||!extended::finite(high[a])||high[a]<low[a]||(!allowDegenerate&&high[a]==low[a]))return false;
    const double x=q[0],y=q[1],z=q[2],w=q[3];
    for(int c=0;c<8;++c){double v[3];for(int a=0;a<3;++a)v[a]=(c>>a)&1?high[a]:low[a];
        const double t[]={p[0]+(1-2*(y*y+z*z))*v[0]+2*(x*y-w*z)*v[1]+2*(x*z+w*y)*v[2],
                          p[1]+2*(x*y+w*z)*v[0]+(1-2*(x*x+z*z))*v[1]+2*(y*z-w*x)*v[2],
                          p[2]+2*(x*z-w*y)*v[0]+2*(y*z+w*x)*v[1]+(1-2*(x*x+y*y))*v[2]};
        for(int a=0;a<3;++a){if(!extended::finite(t[a]))return false;if(!c||t[a]<out->lo[a])out->lo[a]=t[a];if(!c||t[a]>out->hi[a])out->hi[a]=t[a];}
    }return true;
}
inline bool intersects(const Bounds& a,const Bounds& b,double epsilon=Contact){
    for(int i=0;i<3;++i)if(a.hi[i]<=b.lo[i]+epsilon||b.hi[i]<=a.lo[i]+epsilon)return false;return true;
}
inline double abs(double x){return x<0?-x:x;}
inline bool atop(const Bounds& child,const Bounds& parent){
    return abs(child.lo[1]-parent.hi[1])<=Contact && child.lo[0]>=parent.lo[0]-Contact&&child.hi[0]<=parent.hi[0]+Contact
        &&child.lo[2]>=parent.lo[2]-Contact&&child.hi[2]<=parent.hi[2]+Contact;
}
inline void face_probe(const Bounds& b,int face,int index,double* point){
    const int axis=face/2;for(int a=0;a<3;++a){const double span=b.hi[a]-b.lo[a],inset=span<.1?span*.2:.02;
        if(a==axis)point[a]=face&1?b.hi[a]+Contact:b.lo[a]-Contact;
        else if(!index)point[a]=(b.lo[a]+b.hi[a])*.5;
        else {int k=a<axis?a:a-1;point[a]=((index-1)>>k)&1?b.hi[a]-inset:b.lo[a]+inset;}}
}
inline bool floor_support(const Bounds& b,const Bounds& parent){return atop(b,parent);}
inline int parent(const Bounds* boxes,int count,int child){
    int found=-1;for(int i=0;i<count;++i)if(i!=child&&atop(boxes[child],boxes[i])){if(found<0||boxes[i].hi[1]>boxes[found].hi[1])found=i;}return found;
}
// Contact proxy only: a model's native attachment rules are not exposed here.
// The caller's solid() is a read-only voxel query in production.
template<class Solid> bool supported(const Bounds& box,const Bounds* planned,int count,int index,
                                    const WorldObject* existing,int existingCount,Solid solid){
    int p=parent(planned,count,index);if(p>=0)return p<index;
    for(int i=0;i<existingCount;++i)if(atop(box,existing[i].box))return true;
    const int faces[]={2,0,1,4,5,3};
    for(int face:faces){bool all=true;for(int sample=0;sample<5&&all;++sample){double point[3];face_probe(box,face,sample,point);all=solid(point);}if(all)return true;}
    return false;
}
inline bool order(const Bounds* boxes,int count,int* indices,int* badA=nullptr,int* badB=nullptr){
    if(count<0||count>MaxObjects)return false;
    for(int i=0;i<count;++i){indices[i]=i;for(int j=0;j<i;++j)if(intersects(boxes[i],boxes[j])){if(badA)*badA=j;if(badB)*badB=i;return false;}}
    // A supported prop has a higher bottom than its parent. Stable sorting by
    // bottom altitude therefore places furniture before objects on its surface.
    for(int i=1;i<count;++i){int v=indices[i],j=i;while(j&&boxes[indices[j-1]].lo[1]>boxes[v].lo[1]+Contact){indices[j]=indices[j-1];--j;}indices[j]=v;}return true;
}
inline bool capture(core::BpObject* out,uint32_t item,const int64_t* position,const float* rotation,const double* origin){
    if(!out||!item)return false;double norm=0;
    for(int a=0;a<3;++a){double p=position[a]/core::kFix-origin[a];if(!extended::finite(p))return false;out->pos[a]=(float)p;}
    for(int a=0;a<4;++a){if(!(rotation[a]>=-1.001f&&rotation[a]<=1.001f))return false;norm+=rotation[a]*rotation[a];out->rot[a]=rotation[a];}
    if(norm<0.99||norm>1.01)return false;out->item=item;return true;
}
struct Counts {int attempted=0,placed=0,failed=0;void result(bool ok){++attempted;if(ok)++placed;else ++failed;}};
} // namespace props
