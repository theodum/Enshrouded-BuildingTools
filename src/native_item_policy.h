#pragma once
#include "native_props.h"
namespace nativecatalog {
enum Status {Unobserved,Unknown,NotProp,Altar,Special,LogicDisabled,InvalidBounds,AbiUnavailable,Ready,ClassicCandidate,Unreadable};
inline const char* name(Status s){const char* names[]={"registry_unobserved","unknown_id","not_prop","altar_slot","special_flags","logic_disabled","invalid_bounds","abi_unavailable","policy_compatible","classic_blueprint_candidate","native_definition_unreadable"};return names[(int)s];}
inline bool admitted(Status s){return s==Ready||s==ClassicCandidate;}
struct Facts {bool registry=false,exists=false,abi=false,prop=false,altar=false,bounds=false;uint32_t flags=0;};
inline Status resolve(const Facts& f,bool logic,bool classic=false){
    if(!f.registry)return Unobserved;if(!f.exists)return Unknown;if(!f.abi)return AbiUnavailable;
    if(f.altar)return Altar;if(!f.prop)return NotProp;
    if(classic){if(f.flags&~((1u<<22)-1))return Special;return f.bounds?ClassicCandidate:InvalidBounds;}
    if(!extended::object_flags_allowed(f.flags,true))return Special;
    if((f.flags&0x80000)&&!logic)return LogicDisabled;
    if(!f.bounds)return InvalidBounds;return Ready;
}
struct Outcome {uint32_t item=0,accepted=0,failed=0;};
struct Outcomes {
    Outcome rows[4096]{};int count=0;bool truncated=false;
    const Outcome* find(uint32_t item)const{for(int i=0;i<count;++i)if(rows[i].item==item)return rows+i;return nullptr;}
    void record(uint32_t item,bool success){int i=0;while(i<count&&rows[i].item!=item)++i;
        if(i==count){if(count==4096){truncated=true;return;}rows[count++].item=item;}
        uint32_t& n=success?rows[i].accepted:rows[i].failed;if(n!=0xffffffffu)++n;
    }
};
}
