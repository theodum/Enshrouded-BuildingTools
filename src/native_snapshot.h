#pragma once
#include "native_props.h"
#include "native_item_index.h"
namespace nativescan {
enum Error {Ok,SavePending,HookMissing,RegistryMissing,ComfortInactive,ReaderBusy,AbiUnsupported,SystemUnreadable,
    TransformHandle,TableUnreadable,TableBudget,TableChanged,EntityUnreadable,ComponentUnreadable,
    ItemBoundsMissing,BoundsCorrupt,TransformCorrupt,ScaleCorrupt,DegenerateNear,Capacity,Timeout,
    SnapshotBudget,EntityBudget,ItemComponentUnknown,ItemUnresolved,SnapshotStale,Cancelled,TableInvalid,ClassicBudget};
inline const char* code(Error e){const char* names[]={"OK","E_SAVE_PENDING","E_HOOK_MISSING","E_REGISTRY_MISSING","E_COMFORT_INACTIVE","E_READER_BUSY","E_WORLD_ABI_UNSUPPORTED","E_SYSTEM_UNREADABLE",
    "E_TRANSFORM_HANDLE","E_TABLE_UNREADABLE","E_TABLE_BUDGET","E_TABLE_CHANGED","E_ENTITY_UNREADABLE","E_COMPONENT_UNREADABLE",
    "E_WORLD_PLACEMENT_BOUNDS_MISSING","E_WORLD_PLACEMENT_BOUNDS_CORRUPT","E_WORLD_TRANSFORM_CORRUPT","E_WORLD_SCALE_CORRUPT","E_DEGENERATE_PROXY_NEAR_TARGET","E_WORLD_OBJECT_CAPACITY","E_WORLD_READ_TIMEOUT",
    "E_WORLD_READ_CALLBACK_BUDGET","E_WORLD_ENTITY_BUDGET","E_ITEM_COMPONENT_UNOBSERVED","E_WORLD_ITEM_UNRESOLVED","E_WORLD_SNAPSHOT_STALE","E_WORLD_READ_CANCELLED","E_WORLD_TABLE_INVALID","E_CLASSIC_READ_BUDGET"};return names[(int)e];}
constexpr uint64_t MaxSlots=1u<<24;
constexpr int MaxEntities=131072,CallbackBudgetMs=200,SnapshotMaxAgeMs=250;
// Permission cache scoped to one native callback, never a distance/pointer
// heuristic. Query must return a validated readable memory region containing p.
struct ReadCache {
    struct Range {uintptr_t begin=0,end=0;};Range ranges[64]{};int count=0,next=0,last=-1;
    uint32_t queries=0,hits=0;
    void reset(){count=next=0;last=-1;queries=hits=0;}
    template<class Query> bool check(const void* p,size_t n,Query query){
        const auto address=(uintptr_t)p;if(!address||n>(uintptr_t)-1-address)return false;
        const auto end=address+n;
        auto contains=[&](int i){return address>=ranges[i].begin&&end<=ranges[i].end&&address<ranges[i].end;};
        if(last>=0&&contains(last)){++hits;return true;}
        for(int i=0;i<count;++i)if(contains(i)){last=i;++hits;return true;}
        Range region;++queries;if(!query(p,&region.begin,&region.end)||region.end<=region.begin||address<region.begin||end>region.end||address>=region.end)return false;
        const int index=count<64?count++:next++%64;ranges[index]=region;last=index;return true;
    }
};
struct Header {uint64_t world=0,table=0,bits=0,keys=0,values=0,slots=0;uint32_t count=0;};
enum Change {Unchanged,World,Table,Bits,Keys,Values,Slots,Count,Occupancy,Entry,Transform,Item,Population};
inline const char* change_name(Change c){const char* n[]={"none","world","table","occupancy_pointer","keys_pointer","values_pointer","capacity","entity_count","occupancy_content","entity_entry","prop_transform","item_component","population_count"};return n[(int)c];}
inline Change difference(const Header& a,const Header& b){
    if(a.world!=b.world)return World;if(a.table!=b.table)return Table;if(a.bits!=b.bits)return Bits;
    if(a.keys!=b.keys)return Keys;if(a.values!=b.values)return Values;if(a.slots!=b.slots)return Slots;
    return a.count!=b.count?Count:Unchanged;
}
struct Trace {Error error=Ok;uint64_t slot=0,slots=0;uint32_t entity=0,item=0;int visited=0,props=0,excludedDegenerate=0,restarts=0;uint32_t transformIndex=0xffff;
    Header before,after;Change change=Unchanged;const char* field="none";uint64_t expected=0,observed=0;int element=-1;
    int attempts=0,words=0,relevant=0,discovery=0;uint32_t durationMs=0;bool objectData=false,classicReader=false;
    double position[3]{};float rotation[4]{},scale[3]{},low[3]{},high[3]{};};
enum Proxy {Outside,Solid,Unresolved,Corrupt};
// World reads use the native PLACEMENT AABB contract, not guessed mesh extents.
// Ordered finite flat/point proxies can be excluded ONLY by transforming their
// actual native bounds. They remain unresolved when they touch the target area.
// Missing/nonfinite bounds cannot be excluded by distance or pivot heuristics.
inline Proxy proxy(const double* p,const float* q,const float* scale,const float* low,const float* high,
                   const props::Bounds& region,props::Bounds* world,Error* error,bool* degenerate=nullptr){
    float lo[3],hi[3];bool flat=false;*error=Ok;
    for(int a=0;a<3;++a){
        if(!extended::finite(low[a])||!extended::finite(high[a])||high[a]<low[a]){*error=BoundsCorrupt;return Corrupt;}
        if(!extended::finite(scale[a])){*error=ScaleCorrupt;return Corrupt;}
        const double x=(double)low[a]*scale[a],y=(double)high[a]*scale[a];
        if(!extended::finite(x)||!extended::finite(y)){*error=ScaleCorrupt;return Corrupt;}
        lo[a]=(float)(x<y?x:y);hi[a]=(float)(x>y?x:y);if(lo[a]==hi[a])flat=true;
    }
    if(degenerate)*degenerate=flat;
    if(!props::bounds(p,q,lo,hi,world,true)){*error=TransformCorrupt;return Corrupt;}
    if(!props::intersects(*world,region,0))return Outside;
    if(flat){*error=DegenerateNear;return Unresolved;}
    return Solid;
}
}
