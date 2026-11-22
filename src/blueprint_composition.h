#pragma once
#include "native_props.h"
namespace composition {
constexpr int MaxSupportCells=65536;
inline bool classic(const project::Manifest& m){return m.count>0&&m.modules[0].classicObjects;}
inline uint64_t bytes(uint64_t h,const void* p,size_t n){const auto* b=(const uint8_t*)p;while(n--){h^=*b++;h*=1099511628211ull;}return h;}
inline uint64_t fingerprint(const extended::Object* objects,int count){uint64_t h=14695981039346656037ull;
    for(int i=0;i<count;++i){const auto& o=objects[i];h=bytes(h,&o.item,4);h=bytes(h,o.p,24);h=bytes(h,o.rotation,16);h=bytes(h,&o.module,2);}return h;
}
inline bool support_cell(const project::Plan& source,int module,const int* cell){
    for(int i=0;i<source.count;++i){const auto& c=source.cells[i];if(c.module==module&&c.p[0]==cell[0]&&c.p[1]==cell[1]&&c.p[2]==cell[2])return true;}return false;
}
inline bool captured_parent(const props::Bounds* boxes,const extended::Object* objects,int count,int child){
    for(int i=0;i<count;++i)if(i!=child&&objects[i].module==objects[child].module&&props::atop(boxes[child],boxes[i]))return true;return false;
}
}
