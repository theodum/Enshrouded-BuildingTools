// Blueprint objects (issue #49 stage 2): the props in a saved box (doors, chests, beds,
// stations, furniture, lights, ladders) with their item, position and rotation, and placing
// them again when the blueprint is built.
//
// Finding them (docs/reverse-engineering-notes.md, "Blueprints stage 2 research"): the engine
// finds entities in an area with asynchronous range queries. 9600c0 queues one (box, component
// type hash = the low 32 bits of the type's reflection hash, i.e. FNV-1a of "keen::ecs::<Name>";
// only types the query system registered: an unknown one crashes it), the query system resolves
// it, and 9b83d0 hands back up to 256 entity ids a frame or more later. Queries must be written
// from inside a system: the mod takes over the one call of `comfort_energy_gathering` (server)
// that reads its own query's results, and from there writes its queries (the system's writer
// [[sys]+8], the entity it is processing), reads their results on later runs and frees them
// (960390). An entity's components are read from its record (+0x18 archetype: bit mask, +0x84
// offsets, +0xa84 strides; +0x20 data; +0x30 row): CurrentTransform through the comfort system's
// own handle (its rbp+0x60), the item from the 4-byte component that holds the item id (the
// one the game's item lookup e94470 knows; index 610 in 1.0.1.2).
//
// Placing them: PlaceProp (objects.h's g_placePropOrig) with the blueprint system's building
// context (the same struct the prop system passes: item registry +0x28, undo +0xe8 / +0xf0).

static const char* SIG_BP_COMFORT_READ = "4C 8B 45 90 48 8D 95 A0 00 00 00 49 8B CF 45 8B 00 E8 ?? ?? ?? ?? 84 C0";        // call at +0x11 (9b83d0)
static const char* SIG_BP_QUERY_CREATE = "48 8B CB 48 89 7C 24 20 E8 ?? ?? ?? ?? 48 8B 4D B8 8B 9D 90 03 00 00 48 85 C9";   // call at +8 (9600c0)
static const char* SIG_BP_ENTITY_GET   = "42 8B 1C 2B 8B D3 E8 ?? ?? ?? ?? 48 8B F0 48 85 C0 0F 84 ?? ?? ?? ?? 48 8B 55 48"; // call at +6 (9b25f0)
static const char* SIG_BP_QUERY_FREE   = "44 8B C3 48 8B CF 8B 10 E8 ?? ?? ?? ?? 84 C0 0F 84 ?? ?? ?? ?? 48 8B 4D B8";        // call at +8 (960390)
static const char* SIG_BP_LOCAL_BOUNDS = "48 8B D7 48 8D 4C 24 40 E8 ?? ?? ?? ?? 0F 10 87 C0 03 00 00 48 8B D7 48 8D 8D C0 03 00 00";   // call at +8 (494210, place_prop)
static const char* SIG_BP_LOCA_TEXT    = "48 8D 48 48 E8 ?? ?? ?? ?? 8B D0 48 8D 4C 24 38 E8 ?? ?? ?? ?? 0F 10 44 24 38";   // call at +0x10 (aff9f0)
static const char* SIG_BP_ITEM_BY_ID   = "8B 7F 38 8B CF E8 ?? ?? ?? ?? 0F 57 C0 4C 89 65 58 0F 57 C9 66 0F 7F 45 60";        // call at +5 (e94470, dismantle)

struct BpQResult { uint8_t* entries; uint64_t count; };     // 16-byte entries, entity id first
typedef uint8_t  (*BpQResultsFn)(void* sys, BpQResult* out, uint32_t id);
typedef uint32_t* (*BpQCreateFn)(void* writer, uint32_t* outId, uint32_t entity, const float* aabb, const void* filter, uint32_t comp, uint8_t flag);
typedef uint8_t  (*BpQFreeFn)(void* writer, uint32_t entity, uint32_t id);
typedef uint8_t* (*BpEntityFn)(void* sys, uint32_t id);
typedef uint8_t* (*BpItemByIdFn)(uint32_t id, void* registry);
static BpQResultsFn g_bpQResults = nullptr;
static BpQCreateFn  g_bpQCreate = nullptr;
static BpQFreeFn    g_bpQFree = nullptr;
static BpEntityFn   g_bpEntity = nullptr;
static BpItemByIdFn g_bpItemById = nullptr;
static void* volatile g_bpItemReg = nullptr;          // item registry (building context +0x28), checked at each save / build

static uint32_t fnv1a32(const char* s) { uint32_t h = 0x811c9dc5u; for (; *s; s++) { h ^= (uint8_t)*s; h *= 0x01000193u; } return h; }
// Component types the game itself queries (so the query system has them registered).
static const char* const kBpProbeTypes[] = {
    "keen::ecs::InteractionOfferRuntime", "keen::ecs::ComfortProvider", "keen::ecs::Ladder",
};
static const int kBpProbeN = sizeof kBpProbeTypes / sizeof kBpProbeTypes[0];
alignas(16) static float g_bpProbeBox[8];            // min xyz, 0, max xyz, 0 (metres)
alignas(16) static const uint8_t kBpProbeFilter[0x18] = { 1 };   // as the comfort system's (0x14 bytes used)
static volatile LONG g_bpProbeReq = 0;
static volatile LONG g_bpScanning = 0;               // the entity table scan runs (the save watchdog waits)
static uint32_t g_bpProbeIds[kBpProbeN]; static int g_bpProbeLive = 0;
static uint32_t g_bpProbeOwner = 0; static DWORD g_bpProbeTick = 0;
static volatile LONG g_bpProbeHits = 0;              // the comfort system was seen (its read call ran)
static volatile DWORD g_bpProbeSeen = 0;             // ... last time
static uint32_t g_bpItemComp = 0xffffffff;           // component index holding the item id (found on the first prop)
// found objects of the current probe (world positions; the save turns them into box offsets)
#include "blueprint_object_data.h"
static BpFound g_bpFound[core::BP_MAX_OBJS]; static int g_bpFoundN = 0;
static bool g_bpFoundOverflow=false;
static volatile LONG g_bpSnapshotState=0; // 0 idle, 1 requested, 2 scanning, 3 complete, -1 failed
static props::WorldObject g_bpSnapshot[props::MaxWorldObjects];static int g_bpSnapshotCount=0;
static props::Bounds g_bpSnapshotRegion;
static DWORD g_bpSnapshotTick=0,g_bpSnapshotCompleted=0;static uint32_t g_bpSnapshotBadItem=0;
static nativescan::Trace g_bpSnapshotTrace;
static bool g_bpWorldAbi=false;
static bool bp_snapshot_fail(nativescan::Error,uint32_t=0,uint32_t=0);
static bool bp_calibration_capture(const BpFound&,const core::BpObject&);
static int g_calibrateNextCapture=0;
static void bp_snapshot_step(uint8_t*,uint8_t*);
static bool bp_classic_scan_requested();
static void bp_classic_snapshot_step(uint8_t*,uint8_t*);
static void bp_probe_finished(bool answered);        // blueprint.h: writes the pending save

static bool prop_item(const uint8_t* it);            // objects.h
static uint32_t bp_sys_entity(uint8_t* sys) {          // 9bf9e0: the entity the system is processing
    uint32_t idx = *(uint32_t*)(sys + 8);
    if (!idx) return 0;
    uint8_t* base = *(uint8_t**)sys;
    if (!readable(base + 0x830, 8)) return 0;
    uint8_t* arr = *(uint8_t**)(base + 0x830);
    if (!arr || !readable(arr + (size_t)(idx - 1) * 16, 4)) return 0;
    return *(uint32_t*)(arr + (size_t)(idx - 1) * 16);
}
#include "blueprint_object_components.h"
static void bp_probe_collect(uint8_t* sys, uint8_t* frame, int q, const BpQResult& r) {
    uint16_t* trH = frame && readable(frame + 0x60, 8) ? *(uint16_t**)(frame + 0x60) : nullptr;
    const uint32_t trIdx = trH && readable(trH, 2) ? *trH : 0xffff;
    int added = 0;
    for (uint64_t n = 0; n < r.count && n < 256; n++) {
        uint8_t* en = r.entries + n * 16;
        if (!readable(en, 16)) break;
        uint32_t id = *(uint32_t*)en;
        bool dup = false;
        for (int k = 0; k < g_bpFoundN && !dup; k++) dup = g_bpFound[k].entity == id;
        if (dup || g_bpFoundN >= core::BP_MAX_OBJS) continue;
        uint8_t* e = g_bpEntity(sys, id);
        uint32_t sz = 0;
        uint8_t* t = bp_comp(e, trIdx, &sz);
        if (!t || sz < 0x28) continue;
        const int64_t* pos = (const int64_t*)t;
        bool inside = true;
        for (int a = 0; a < 3; a++) {
            float v = (float)((double)pos[a] / core::kFix);
            if (v < g_bpProbeBox[a] - 0.01f || v > g_bpProbeBox[4 + a] + 0.01f) inside = false;
        }
        uint32_t item = inside ? bp_entity_item(e) : 0;
        if (!item) continue;
        BpFound& f = g_bpFound[g_bpFoundN++];f={};
        f.entity = id; f.item = item;
        memcpy(f.pos, pos, 24); memcpy(f.rot, t + 0x18, 16);
        added++;
        LOG(.str("  object ").x(item).str(" at (").fix(pos[0]).c(',').fix(pos[1]).c(',').fix(pos[2]).str(") rot (")
            .f(f.rot[0]).c(',').f(f.rot[1]).c(',').f(f.rot[2]).c(',').f(f.rot[3]).c(')'));
    }
    LOG(.str("blueprint objects: ").str(kBpProbeTypes[q] + 10).str(" -> ").u(r.count).str(" entities, ").i(added).str(" new objects"));
}
// All entities of the world, from the table 9b25f0 looks ids up in: [[sys]+0x30] + 0x148 is an
// open-addressing map (+0x08 occupancy bits, +0x10 slots, +0x28 u32 keys, +0x54 count), its
// values (entity record pointers) at +0x188 by slot. Every entity in the box whose 4-byte item
// component names a placeable object is kept: the range queries above only reach entities with
// a queried component (usable, comfort, ladders), this reaches windows, fences and decorations too.
#include "blueprint_object_scan.h"
// the comfort system's results call: the game's own read, then the mod's scan
static uint8_t bt_comfort_read_body(uint8_t* sys, BpQResult* out, uint32_t id, uint8_t* frame) {
    uint8_t r = g_bpQResults(sys, out, id);
    g_bpProbeSeen = GetTickCount();
    if (!InterlockedExchange(&g_bpProbeHits, 1)) LOG(.str("blueprint objects: comfort system seen"));
    if(g_bpSnapshotState==1||g_bpSnapshotState==2){
        EnterCriticalSection(&g_stateLock);if(bp_classic_scan_requested())bp_classic_snapshot_step(sys,frame);else bp_snapshot_step(sys,frame);LeaveCriticalSection(&g_stateLock);return r;
    }
    if (!sys || !readable(sys, 0x10) || !readable(*(uint8_t**)sys, 0x838)) return r;
    if (InterlockedExchange(&g_bpProbeReq, 0)) {           // a save asked: every entity in its box, now
        EnterCriticalSection(&g_stateLock);
        g_bpFoundN = 0;
        g_bpFoundOverflow=false;
        g_bpScanning = 1;
        bp_scan_entities(sys, frame);
        g_bpScanning = 0;
        bp_probe_finished(true);
        LeaveCriticalSection(&g_stateLock);
        return r;
    }
    void* writer = *(void**)(*(uint8_t**)sys + 8);
    if (false && writer) {                                  // the range queries (kept for reference: they only reach some objects)
        if (g_bpProbeLive) for (int q = 0; q < kBpProbeN; q++) if (g_bpProbeIds[q]) g_bpQFree(writer, g_bpProbeOwner, g_bpProbeIds[q]);
        g_bpProbeOwner = bp_sys_entity(sys);
        g_bpFoundN = 0;
        for (int q = 0; q < kBpProbeN; q++) {
            g_bpProbeIds[q] = 0;
            g_bpQCreate(writer, &g_bpProbeIds[q], g_bpProbeOwner, g_bpProbeBox, kBpProbeFilter, fnv1a32(kBpProbeTypes[q]), 0);
        }
        g_bpProbeLive = 1; g_bpProbeTick = GetTickCount();
        return r;
    }
    if (!g_bpProbeLive || GetTickCount() - g_bpProbeTick < 150) return r;
    bool all = true;
    for (int q = 0; q < kBpProbeN; q++) {
        if (!g_bpProbeIds[q]) continue;
        BpQResult res = {};
        if (g_bpQResults(sys, &res, g_bpProbeIds[q])) bp_probe_collect(sys, frame, q, res);
        else { all = false; continue; }
        g_bpQFree(writer, g_bpProbeOwner, g_bpProbeIds[q]);
        g_bpProbeIds[q] = 0;
    }
    if (all || GetTickCount() - g_bpProbeTick > 4000) {
        if (!all) for (int q = 0; q < kBpProbeN; q++) if (g_bpProbeIds[q]) { g_bpQFree(writer, g_bpProbeOwner, g_bpProbeIds[q]); g_bpProbeIds[q] = 0; }
        g_bpProbeLive = 0;
        bp_probe_finished(all);
    }
    return r;
}
extern "C" uint8_t bt_comfort_read(uint8_t* sys, BpQResult* out, uint32_t id, void*, uint8_t* frame) {
    return bt_comfort_read_body(sys, out, id, frame);
}
// rcx, rdx, r8, r9 pass through; the caller's rbp becomes the 5th argument
extern "C" uint8_t bp_comfort_thunk[];
asm(".text\n"
    ".globl bp_comfort_thunk\n"
    "bp_comfort_thunk:\n"
    "    push %rbp\n"
    "    sub $0x30, %rsp\n"
    "    mov %rbp, 0x20(%rsp)\n"
    "    call bt_comfort_read\n"
    "    add $0x30, %rsp\n"
    "    pop %rbp\n"
    "    ret\n");

// The item registry from a building context: +0x28 (the piece removal 48fab0 and PlaceProp read
// it there), checked with the item in hand (the shape state is its definition).
static bool bp_item_registry(void* ctx, void* state) {
    void* reg = ctx && readable((uint8_t*)ctx + 0x28, 8) ? *(void**)((uint8_t*)ctx + 0x28) : nullptr;
    uint8_t* self = g_bpItemById && reg && state ? g_bpItemById(*(uint32_t*)state, reg) : nullptr;
    if (self == (uint8_t*)state) { g_bpItemReg = reg; return true; }
    LOG(.str("blueprint objects: item registry not confirmed (").x((uint64_t)self).str(" for ").x((uint64_t)state).c(')'));
    return false;
}
// The save asks for the objects in its box (server thread). False = they cannot be found now
// (signatures missing, no item registry, or the comfort system is not running).
static bool bp_probe_request(const I3* c, const float* step, void* ctx, void* state) {
    if(g_bpSnapshotState==1||g_bpSnapshotState==2)return false;
    if (!g_bpQResults || !bp_item_registry(ctx, state)) return false;
    if (!g_bpProbeHits || GetTickCount() - g_bpProbeSeen > 3000) { LOG(.str("blueprint objects: the comfort system is not running here")); return false; }
    for (int a = 0; a < 3; a++) {
        const int64_t v0 = a == 0 ? c[0].x : a == 1 ? c[0].y : c[0].z, v1 = a == 0 ? c[1].x : a == 1 ? c[1].y : c[1].z;
        double lo = (double)(v0 < v1 ? v0 : v1) / core::kFix - step[a] * 0.5, hi = (double)(v0 < v1 ? v1 : v0) / core::kFix + step[a] * 0.5;
        g_bpProbeBox[a] = (float)lo; g_bpProbeBox[4 + a] = (float)hi;
    }
    g_bpProbeBox[3] = g_bpProbeBox[7] = 0.f;
    LOG(.str("blueprint objects: query for (").f(g_bpProbeBox[0]).c(',').f(g_bpProbeBox[1]).c(',').f(g_bpProbeBox[2]).str(") - (")
        .f(g_bpProbeBox[4]).c(',').f(g_bpProbeBox[5]).c(',').f(g_bpProbeBox[6]).c(')'));
    g_bpProbeReq = 1;
    return true;
}

// ---- placing (server, the blueprint commit; building context of the blueprint system)
typedef uint8_t (*PlacePropFn)(void* ctx, uint8_t* info, uint32_t item);
static PlacePropFn g_placePropOrig = nullptr;        // objects.h finds it
static volatile LONG g_bpObjRecord = 0;              // obj_rec_check (objects.h): record the blueprint's objects
// Logic objects (item flag 0x80000 at +0x5d8: doors, windows, wirable things) make PlaceProp send
// a LogicEntityPlacedEvent through the context's +0xe0: the event writer of the prop system's
// dependencies, which the blueprint system does not have (null: crash at 9c3760, 0.32.0-dev10).
// Writing into another system's buffer is not safe (961470 appends without locks), so the mod
// hands PlaceProp a writer of its own: +0xb8 the event type index (u16), +0x1b8 the buffer
// {data, capacity, used}; 961470 returns null when it is full and PlaceProp writes through
// that pointer, so the buffer is emptied before every call. The event (it links the object into
// the base's logic network) is dropped.
struct BpEventBuf { uint8_t* data; uint64_t cap; uint32_t used; uint32_t pad; };
alignas(16) static uint8_t g_bpEvData[512];
static BpEventBuf g_bpEvBuf = { g_bpEvData, sizeof g_bpEvData, 0, 0 };
alignas(16) static uint8_t g_bpEvWriter[0x200];
static bool bp_logic_item(const uint8_t* it) { return (*(const uint32_t*)(it + 0x5d8) & 0x80000) != 0; }
static bool extended_prop_item(const uint8_t* it, bool nativeContext = false) {
    return g_nativeObjectAbi && prop_item(it) && readable(it + 0x5d8, 4) && extended::object_flags_allowed(*(const uint32_t*)(it + 0x5d8), nativeContext);
}
static int g_projectBlueprintLogic=0; // explicit session opt-in; upstream logic-link event is not copied
static nativecatalog::Outcomes g_nativeItemOutcomes;
static bool project_prop_item(const uint8_t* it){return extended_prop_item(it)||
    (g_projectBlueprintLogic&&extended_prop_item(it,true));}
static bool bp_logic_object(void*, const uint8_t*) { return false; }   // placed with the mod's writer (bp_place_object)
// An item's local bounds as the prop system gives them to PlaceProp (494210: float4 min, float4 max).
typedef void (*BpLocalBoundsFn)(float* out, uint8_t* item);
static BpLocalBoundsFn g_bpLocalBounds = nullptr;
static bool bp_item_bounds_raw(uint8_t* it, float* mn, float* mx) {
    if (!g_bpLocalBounds || !it) return false;
    alignas(16) float b[8] = {};
    g_bpLocalBounds(b, it);
    for (int a = 0; a < 3; a++) { mn[a] = b[a]; mx[a] = b[4 + a]; }
    return true;
}
static bool bp_item_bounds(uint8_t* it,float* mn,float* mx){
    return bp_item_bounds_raw(it,mn,mx)&&mx[0]>=mn[0]&&mx[1]>=mn[1]&&mx[2]>=mn[2];
}
#include "native_item_resolution.h"
// Fresh per-callback region checks avoid thousands of repeated VirtualQuery
// calls without carrying page permissions across world updates.
static bool bp_snapshot_memory_region(const void* p,uintptr_t* begin,uintptr_t* end){
    MEMORY_BASIC_INFORMATION region;if(!VirtualQuery(p,&region,sizeof region)||region.State!=MEM_COMMIT||(region.Protect&(PAGE_NOACCESS|PAGE_GUARD)))return false;
    *begin=(uintptr_t)region.BaseAddress;*end=*begin+region.RegionSize;return *end>*begin;
}
#include "native_snapshot_runtime.h"
#include "blueprint_classic_collision.h"
// Mirroring an object: the game has no mirrored models, so the copy is turned and one of the
// object's own horizontal axes comes out reversed against a true mirror image. That must be the
// axis along which the model is the same at both ends, whichever way it stands. r' = M r M
// reverses its local x: right for things the same on their left and right (chairs, lamps,
// paintings). Half a turn about the up axis on top (r' = M r M * (0, 1, 0, 0)) reverses its
// local z instead: right for things the same front and back but not along their length
// (railings, fences, windows, beds). The default comes from the model's bounds (much less deep
// than wide), except very thin comfort items (paintings, banners: they have a front); the
// window lets the player turn any kind of object the other way (BuildingTools-Blueprints\mirror.ini).
static int8_t g_bpMirrorPick[256]; static uint32_t g_bpMirrorPickItem[256]; static int g_bpMirrorPickN = 0;   // -1 = not read yet
static int bp_mirror_choice(uint32_t item) {          // 1 = half turn, 0 = as is, -1 = default
    for (int q = 0; q < g_bpMirrorPickN; q++) if (g_bpMirrorPickItem[q] == item) return g_bpMirrorPick[q];
    wchar_t key[16]; { Buf b; b.u(item, 16, 8); for (int k = 0; k <= b.n; k++) key[k] = (wchar_t)(uint8_t)b.z()[k]; }
    wchar_t path[MAX_PATH]; lstrcpyW(path, g_dir); lstrcatW(path, L"BuildingTools-Blueprints\\mirror.ini");
    int v = (int)GetPrivateProfileIntW(L"Mirror", key, (UINT)-1, path);
    if (g_bpMirrorPickN < 256) { g_bpMirrorPickItem[g_bpMirrorPickN] = item; g_bpMirrorPick[g_bpMirrorPickN++] = (int8_t)(v < -1 || v > 1 ? -1 : v); }
    return v < -1 || v > 1 ? -1 : v;
}
static void bp_mirror_set(uint32_t item, int v) {
    bp_mirror_choice(item);
    for (int q = 0; q < g_bpMirrorPickN; q++) if (g_bpMirrorPickItem[q] == item) g_bpMirrorPick[q] = (int8_t)v;
    wchar_t p[MAX_PATH]; lstrcpyW(p, g_dir); lstrcatW(p, L"BuildingTools-Blueprints\\");
    CreateDirectoryW(p, nullptr);
    lstrcatW(p, L"mirror.ini");                          // its own file: write_full_ini rewrites BuildingTools.ini
    wchar_t key[16], val[4] = { (wchar_t)('0' + v), 0 };
    { Buf b; b.u(item, 16, 8); for (int k = 0; k <= b.n; k++) key[k] = (wchar_t)(uint8_t)b.z()[k]; }
    WritePrivateProfileStringW(L"Mirror", key, val, p);
}
static bool bp_mirror_default(uint32_t item) {
    if (!g_bpItemById || !g_bpItemReg) return false;
    uint8_t* it = g_bpItemById(item, g_bpItemReg);
    float mn[3], mx[3];
    if (!bp_item_bounds(it, mn, mx)) return false;
    const float sx = mx[0] - mn[0], sz = mx[2] - mn[2];
    const bool comfort = readable(it + 0x790, 4) && *(uint32_t*)(it + 0x790) > 0;
    // paper-thin wall pieces (a painting: 3.5 m wide, 0.16 m deep) have a front, railings and
    // windows (depth 1/8 to 1/5 of the width) do not
    return sx > 0.05f && sz < 0.6f * sx && !(comfort && sz < 0.2f * sx) && !(sz < 0.08f * sx);
}
static bool bp_mirror_half_turn(uint32_t item) {
    const int c = bp_mirror_choice(item);
    const bool half = c >= 0 ? c == 1 : bp_mirror_default(item);
    LOG(.str("  mirror: object ").x(item).str(half ? " half turn" : " as is").str(c >= 0 ? " (chosen in the window)" : ""));
    return half;
}
#include "blueprint_object_place.h"

// ---- item names (the window's material and object rows; UI thread)
// ItemInfo +0x48 is the name's loca reference (16 bytes); its LocaTagId is FNV-1a 32 over those
// bytes (42ea30), and aff9f0(out, id) gives the text in the game's language as {chars, length,
// hash}, the way the game's own item rows get it. Copied at once, cached per item.
struct BpText { const char* p; uint64_t n; uint64_t hash; };
typedef BpText* (*BpLocaTextFn)(BpText* out, uint32_t id);
static BpLocaTextFn g_bpLocaText = nullptr;
struct BpName { uint32_t item; char s[64]; };
static BpName g_bpNameCache[128]; static int g_bpNameCacheN = 0;
// only what the window already looked up (any thread: no game call, no new entry)
static const char* bp_item_name_cached(uint32_t item) {
    const int n = g_bpNameCacheN < 128 ? g_bpNameCacheN : 128;
    for (int q = 0; q < n; q++) if (g_bpNameCache[q].item == item) return g_bpNameCache[q].s[0] ? g_bpNameCache[q].s : nullptr;
    return nullptr;
}
static const char* bp_item_name(uint32_t item) {
    for (int q = 0; q < g_bpNameCacheN; q++) if (g_bpNameCache[q].item == item) return g_bpNameCache[q].s[0] ? g_bpNameCache[q].s : nullptr;
    if (!g_bpLocaText || !g_bpItemById || !g_bpItemReg || !item) return nullptr;
    uint8_t* it = g_bpItemById(item, g_bpItemReg);
    if (!it || !readable(it + 0x48, 16)) return nullptr;
    uint32_t id = 0x811c9dc5u;
    for (int k = 0; k < 16; k++) { id ^= it[0x48 + k]; id *= 0x01000193u; }
    BpText t = {};
    g_bpLocaText(&t, id);
    BpName& e = g_bpNameCacheN < 128 ? g_bpNameCache[g_bpNameCacheN++] : g_bpNameCache[127];
    e.item = item; e.s[0] = 0;
    if (t.p && t.n && t.n < 200 && readable(t.p, (size_t)t.n)) {
        size_t n = t.n < sizeof e.s - 1 ? (size_t)t.n : sizeof e.s - 1;
        while (n > 0 && n < t.n && ((uint8_t)t.p[n] & 0xC0) == 0x80) n--;      // whole UTF-8 characters
        memcpy(e.s, t.p, n); e.s[n] = 0;
    }
    return e.s[0] ? e.s : nullptr;
}

static void bp_objects_install() {
    if (uint8_t* lb = find_unique("BpLocalBounds", SIG_BP_LOCAL_BOUNDS)) g_bpLocalBounds = (BpLocalBoundsFn)(lb + 13 + *(int32_t*)(lb + 9));
    if (uint8_t* lt = find_unique("BpLocaText", SIG_BP_LOCA_TEXT)) g_bpLocaText = (BpLocaTextFn)(lt + 0x15 + *(int32_t*)(lt + 0x11));
    uint8_t* rd = find_unique("BpComfortRead", SIG_BP_COMFORT_READ);
    uint8_t* cr = find_unique("BpQueryCreate", SIG_BP_QUERY_CREATE);
    uint8_t* eg = find_unique("BpEntityGet", SIG_BP_ENTITY_GET);
    uint8_t* fr = find_unique("BpQueryFree", SIG_BP_QUERY_FREE);
    uint8_t* ib = find_unique("BpItemById", SIG_BP_ITEM_BY_ID);
    if (!rd || !cr || !eg || !fr || !ib) { LOG(.str("blueprint objects: not available (signatures)")); return; }
    auto tgt = [](uint8_t* call) { return call + 5 + *(int32_t*)(call + 1); };
    uint8_t* site = rd + 0x11;
    g_bpQCreate = (BpQCreateFn)tgt(cr + 8);
    g_bpEntity = (BpEntityFn)tgt(eg + 6);
    g_bpQFree = (BpQFreeFn)tgt(fr + 8);
    g_bpItemById = (BpItemByIdFn)tgt(ib + 5);
    g_bpQResults = (BpQResultsFn)tgt(site);
    const auto* pe=(const IMAGE_NT_HEADERS64*)(g_base+((const IMAGE_DOS_HEADER*)g_base)->e_lfanew);const uint64_t imageSize=pe->OptionalHeader.SizeOfImage;
    const auto* getter=(const uint8_t*)g_bpEntity;
    const auto* mapLookup=readable(getter,0x33)&&extended::entity_table_abi(getter)?tgt((uint8_t*)getter+0x1d):nullptr;
    g_bpWorldAbi=mapLookup&&readable(mapLookup,0x62)&&extended::entity_map_abi(mapLookup)&&extended::transform_abi((uint64_t)g_base,[&](uint64_t a,uint64_t n)->const uint8_t*{
        uint64_t off=a-(uint64_t)g_base;return off<imageSize&&n<=imageSize-off&&readable((void*)a,(size_t)n)?(const uint8_t*)a:nullptr;});
    LOG(.str("native world reader ABI: ").i(g_bpWorldAbi).str(" CurrentTransform=56 position=0 quaternion=24 scale=40"));
    if (hook_calls((uint8_t*)g_bpQResults, (void*)bp_comfort_thunk, &site, 1) != 1) { g_bpQResults = nullptr; LOG(.str("blueprint objects: hook failed")); return; }
    LOG(.str("blueprint objects: available"));
}
