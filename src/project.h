// Building Tools Extended: platform-free composite manifest and sparse planner.
// No STL, allocation, Windows or game calls: also compiled by the host tests.
#pragma once
#include "core.h"

namespace project {
constexpr int MaxModules = 128, MaxCells = 1048576, MaxBytes = 524288, MaxPieces = 2048;
constexpr int MaxSpaces = 512, MaxSpaceColumns = 65536;
constexpr int Unit = 16, Cell = 8; // integer sixteenths of a metre; native 0.5 m voxels
constexpr int MaxCoord = 1048576;
struct Module { char file[192]; int offset[3]; int rotation; int category; bool objectsOnly=false,classicObjects=false; };
struct NativePiece { uint32_t item; int offset[3]; int rotation; uint32_t material; int clearance; };
struct Space { int lo[3],hi[3]; }; // reserved air above a complete voxel walking surface
struct Manifest { char name[192]; int grid; int anchor[3]; int count; Module modules[MaxModules]; int pieceCount; NativePiece pieces[MaxPieces]; int spaceCount; Space spaces[MaxSpaces]; };
struct Error { const char* message = nullptr; int at = 0; int module = -1; };
inline bool equal(const char* a, const char* b) { while (*a && *a == *b) { ++a; ++b; } return *a == *b; }
inline bool safe_file(const char* s) {
    int n = 0;
    for (; s[n]; ++n) {
        const unsigned char c = (unsigned char)s[n];
        if (c < 32 || c == '/' || c == '\\' || c == ':' || c == '*' || c == '?' || c == '"' || c == '<' || c == '>' || c == '|') return false;
    }
    if (n<=5 || s[0]=='.' || !equal(s+n-5,".btbp")) return false;
    char stem[8]={}; int length=0;
    for (;s[length] && s[length]!='.' && length<7;++length) stem[length]=s[length]>='a' && s[length]<='z' ? s[length]-'a'+'A' : s[length];
    while(length && stem[length-1]==' ') stem[--length]=0;
    if (equal(stem,"CON") || equal(stem,"PRN") || equal(stem,"AUX") || equal(stem,"NUL")) return false;
    if (length==4 && stem[3]>='0' && stem[3]<='9' && ((stem[0]=='C' && stem[1]=='O' && stem[2]=='M') || (stem[0]=='L' && stem[1]=='P' && stem[2]=='T'))) return false;
    return true;
}

// Strict, bounded JSON reader. Unknown/repeated fields and trailing data fail closed.
struct Reader {
    const char* s; int size, at = 0; Error* error;
    bool fail(const char* msg) { if (!error->message) { error->message = msg; error->at = at; } return false; }
    void space() { while (at < size && (s[at] == ' ' || s[at] == '\n' || s[at] == '\r' || s[at] == '\t')) ++at; }
    bool take(char c) { space(); if (at < size && s[at] == c) { ++at; return true; } return false; }
    bool want(char c) { return take(c) || fail("Expected JSON punctuation"); }
    bool hex4(unsigned* v) {
        *v = 0;
        for (int k = 0; k < 4; ++k) {
            if (at == size) return fail("Truncated Unicode escape");
            char c = s[at++]; int d = c >= '0' && c <= '9' ? c - '0' : c >= 'a' && c <= 'f' ? c - 'a' + 10 : c >= 'A' && c <= 'F' ? c - 'A' + 10 : -1;
            if (d < 0) return fail("Invalid Unicode escape");
            *v = (*v << 4) | (unsigned)d;
        }
        return true;
    }
    bool string(char* out, int cap) {
        if (!want('"')) return false;
        int n = 0;
        auto put = [&](unsigned char c) { if (n >= cap - 1) return fail("JSON string too long"); out[n++] = (char)c; return true; };
        while (at < size) {
            unsigned char c = (unsigned char)s[at++];
            if (c == '"') { out[n] = 0; return true; }
            if (c < 32) return fail("Control character in string");
            if (c != '\\') {
                if (c < 0x80) { if (!put(c)) return false; continue; }
                int count = c >= 0xc2 && c <= 0xdf ? 1 : c >= 0xe0 && c <= 0xef ? 2 : c >= 0xf0 && c <= 0xf4 ? 3 : -1;
                if (count < 0 || !put(c)) return fail("Invalid UTF-8 string");
                unsigned value = c & ((1u << (6 - count)) - 1);
                for (int k = 0; k < count; ++k) {
                    if (at == size) return fail("Truncated UTF-8 string"); unsigned char next = (unsigned char)s[at++];
                    if ((next & 0xc0) != 0x80 || !put(next)) return fail("Invalid UTF-8 continuation");
                    value = (value << 6) | (next & 63);
                }
                if ((count == 1 && value < 0x80) || (count == 2 && value < 0x800) || (count == 3 && value < 0x10000) || value > 0x10ffff || (value >= 0xd800 && value <= 0xdfff)) return fail("Invalid UTF-8 code point");
                continue;
            }
            if (at == size) return fail("Truncated escape");
            c = (unsigned char)s[at++];
            if (c == '"' || c == '\\' || c == '/') { if (!put(c)) return false; }
            else if (c == 'u') {
                unsigned v; if (!hex4(&v)) return false;
                if (v >= 0xd800 && v <= 0xdbff) {
                    if (at + 2 > size || s[at++] != '\\' || s[at++] != 'u') return fail("Missing low surrogate");
                    unsigned lo; if (!hex4(&lo) || lo < 0xdc00 || lo > 0xdfff) return fail("Invalid low surrogate");
                    v = 0x10000 + ((v - 0xd800) << 10) + lo - 0xdc00;
                } else if (v >= 0xdc00 && v <= 0xdfff) return fail("Unexpected low surrogate");
                if (v < 32) return fail("Control character in string");
                if (v < 0x80) { if (!put((unsigned char)v)) return false; }
                else if (v < 0x800) { if (!put(0xc0 | (v >> 6)) || !put(0x80 | (v & 63))) return false; }
                else if (v < 0x10000) { if (!put(0xe0 | (v >> 12)) || !put(0x80 | ((v >> 6) & 63)) || !put(0x80 | (v & 63))) return false; }
                else if (!put(0xf0 | (v >> 18)) || !put(0x80 | ((v >> 12) & 63)) || !put(0x80 | ((v >> 6) & 63)) || !put(0x80 | (v & 63))) return false;
            } else return fail("Unsupported string escape");
        }
        return fail("Unterminated string");
    }
    // Decimal numbers are converted exactly, without locale or floating point rounding.
    bool number(int* out) {
        space(); bool neg = take('-');
        if (at == size || s[at] < '0' || s[at] > '9') return fail("Expected decimal number");
        const bool zero = s[at] == '0'; int digits = 0; int64_t v = 0, den = 1;
        while (at < size && s[at] >= '0' && s[at] <= '9') {
            if (++digits > 7 || (zero && digits > 1)) return fail("Number out of range or leading zero");
            v = v * 10 + s[at++] - '0';
        }
        if (at < size && s[at] == '.') {
            ++at; int n = 0;
            while (at < size && s[at] >= '0' && s[at] <= '9') {
                if (++n > 4) return fail("At most four decimal places allowed");
                v = v * 10 + s[at++] - '0'; den *= 10;
            }
            if (!n) return fail("Missing fractional digits");
        }
        if ((v * Unit) % den || v * Unit / den > MaxCoord) return fail("Coordinate must be an exact sixteenth within range");
        *out = (int)(v * Unit / den) * (neg ? -1 : 1); return true;
    }
    bool vector(int* p) { return want('[') && number(p) && want(',') && number(p + 1) && want(',') && number(p + 2) && want(']'); }
    bool item_id(uint32_t* out) {
        space(); uint64_t value = 0; int digits = 0; const int start = at;
        while (at < size && s[at] >= '0' && s[at] <= '9') {
            value = value * 10 + s[at++] - '0'; if (++digits > 10 || value > 0xffffffffu) return fail("Item ID out of range");
        }
        if (!digits || !value || (digits > 1 && s[start] == '0')) return fail("Expected nonzero integer item ID");
        *out = (uint32_t)value; return true;
    }
    bool field(unsigned* seen, unsigned bit) { if (*seen & bit) return fail("Repeated JSON field"); *seen |= bit; return true; }
    bool module(Module* m) {
        if (!want('{')) return false;
        unsigned seen = 0; m->rotation = 0; m->category = 0;m->objectsOnly=m->classicObjects=false;
        do {
            char key[32]; if (!string(key, sizeof key) || !want(':')) return false;
            if (equal(key, "file")) { if (!field(&seen, 1) || !string(m->file, sizeof m->file) || !safe_file(m->file)) return fail("Expected a .btbp filename in the project directory"); }
            else if (equal(key, "offset")) { if (!field(&seen, 2) || !vector(m->offset)) return false; }
            else if (equal(key, "rotation")) {
                int r; if (!field(&seen, 4) || !number(&r)) return false;
                if (r < 0 || r > 270 * Unit || r % (90 * Unit)) return fail("Rotation must be 0, 90, 180 or 270 degrees");
                m->rotation = r / (90 * Unit);
            } else if (equal(key, "category")) {
                char value[24]; if (!field(&seen, 8) || !string(value, sizeof value)) return false;
                const char* names[] = {"general", "wall", "floor", "roof", "tower", "stairs", "furniture"};
                int at = 0; while (at < 7 && !equal(value, names[at])) ++at;
                if (at == 7) return fail("Unknown architectural category");
                m->category = at;
            } else if(equal(key,"content")){
                char value[24];if(!field(&seen,16)||!string(value,sizeof value))return false;
                if(!equal(value,"all")&&!equal(value,"objects"))return fail("Unknown module content");m->objectsOnly=equal(value,"objects");
            } else if(equal(key,"objectPlacement")){
                char value[24];if(!field(&seen,32)||!string(value,sizeof value))return false;
                if(!equal(value,"checked")&&!equal(value,"blueprint"))return fail("Unknown object placement method");m->classicObjects=equal(value,"blueprint");
            } else return fail("Unknown module field");
        } while (take(','));
        return want('}') && ((seen & 3) == 3 || fail("Module needs file and offset"))&&(!m->classicObjects||m->objectsOnly||fail("Blueprint placement requires objects-only content"));
    }
    bool piece(NativePiece* piece) {
        if (!want('{')) return false;
        unsigned seen = 0; piece->rotation = 0; piece->material = 0; piece->clearance = 0;
        do {
            char key[32]; if (!string(key, sizeof key) || !want(':')) return false;
            if (equal(key, "item")) {
                if (!field(&seen, 1)) return false;
                if (!item_id(&piece->item)) return false;
            } else if (equal(key, "offset")) { if (!field(&seen, 2) || !vector(piece->offset)) return false; }
            else if (equal(key, "rotation")) {
                int r; if (!field(&seen, 4) || !number(&r)) return false;
                if (r < 0 || r > 270 * Unit || r % (90 * Unit)) return fail("Invalid native-piece rotation");
                piece->rotation = r / (90 * Unit);
            } else if (equal(key, "material")) { if (!field(&seen, 8) || !item_id(&piece->material)) return false; }
            else if (equal(key, "clearance")) { if (!field(&seen, 16) || !number(&piece->clearance) || piece->clearance < 0 || piece->clearance > 128) return fail("Native clearance must be between 0 and 8 m"); }
            else return fail("Unknown native piece field");
        } while (take(','));
        return want('}') && ((seen & 3) == 3 || fail("Native piece needs item and offset"));
    }
    bool space_box(Space* box) {
        if(!want('{'))return false;unsigned seen=0;
        do {char key[32];if(!string(key,sizeof key)||!want(':'))return false;
            if(equal(key,"low")){if(!field(&seen,1)||!vector(box->lo))return false;}
            else if(equal(key,"high")){if(!field(&seen,2)||!vector(box->hi))return false;}
            else return fail("Unknown reserved-space field");
        }while(take(','));
        if(!want('}') || seen!=3)return fail("Reserved space needs low and high");
        for(int a=0;a<3;++a)if(box->lo[a]%Cell || box->hi[a]%Cell || box->hi[a]<=box->lo[a])return fail("Reserved space must be positive and align to 0.5 m");
        if(box->hi[1]-box->lo[1]<32 || box->hi[1]-box->lo[1]>128)return fail("Reserved headroom must be 2 to 8 m");
        if((int64_t)(box->hi[0]-box->lo[0])*(box->hi[2]-box->lo[2])/64>4096)return fail("Reserved surface too large");
        return true;
    }
};
inline bool decode(const char* data, int size, Manifest* out, Error* error) {
    *error = Error{}; out->count = out->pieceCount = out->spaceCount = 0; out->grid = Unit; out->name[0] = 0;
    for (int a = 0; a < 3; ++a) out->anchor[a] = 0;
    Reader r{data, size, 0, error}; unsigned seen = 0;
    if (!data || size < 2 || size > MaxBytes) return r.fail("Manifest size out of range");
    if (!r.want('{')) return false;
    do {
        char key[32]; if (!r.string(key, sizeof key) || !r.want(':')) return false;
        if (equal(key, "format")) {
            char value[64]; if (!r.field(&seen, 1) || !r.string(value, sizeof value)) return false;
            if (!equal(value, "BuildingToolsExtendedProject")) return r.fail("Unrecognized project format");
        } else if (equal(key, "version")) {
            int v; if (!r.field(&seen, 2) || !r.number(&v)) return false;
            if (v != Unit) return r.fail("Unsupported project version");
        } else if (equal(key, "name")) { if (!r.field(&seen, 4) || !r.string(out->name, sizeof out->name) || !out->name[0]) return r.fail("Project needs a name"); }
        else if (equal(key, "gridSize")) {
            if (!r.field(&seen, 8) || !r.number(&out->grid)) return false;
            if (out->grid < Cell || out->grid > 64 || out->grid % Cell) return r.fail("Grid must be a multiple of 0.5 m, from 0.5 to 4 m");
        } else if (equal(key, "anchor")) { if (!r.field(&seen, 16) || !r.vector(out->anchor)) return false; }
        else if (equal(key, "modules")) {
            if (!r.field(&seen, 32) || !r.want('[')) return false;
            if (r.take(']')) continue;
            do {
                if (out->count == MaxModules) return r.fail("Too many modules");
                error->module = out->count;
                if (!r.module(&out->modules[out->count])) return false;
                ++out->count;
            } while (r.take(','));
            if (!r.want(']')) return false;
            error->module = -1;
        } else if (equal(key, "pieces")) {
            if (!r.field(&seen, 64) || !r.want('[')) return false;
            if (r.take(']')) continue;
            do {
                if (out->pieceCount == MaxPieces) return r.fail("Too many native pieces");
                if (!r.piece(&out->pieces[out->pieceCount++])) return false;
            } while (r.take(','));
            if (!r.want(']')) return false;
        } else if(equal(key,"spaces")) {
            if(!r.field(&seen,128)||!r.want('['))return false;
            if(r.take(']'))continue;
            int64_t columns=0;
            do {if(out->spaceCount==MaxSpaces)return r.fail("Too many reserved spaces");
                auto& box=out->spaces[out->spaceCount++];if(!r.space_box(&box))return false;
                columns+=(int64_t)(box.hi[0]-box.lo[0])*(box.hi[2]-box.lo[2])/64;
                if(columns>MaxSpaceColumns)return r.fail("Reserved-space budget exceeded");
            }while(r.take(','));if(!r.want(']'))return false;
        } else return r.fail("Unknown project field");
    } while (r.take(','));
    if (!r.want('}')) return false;
    r.space(); if (r.at != size) return r.fail("Trailing data after manifest");
    if ((seen & 63) != 63 || (!out->count && !out->pieceCount)) return r.fail("Missing project field or empty project");
    for (int a = 0; a < 3; ++a) if (out->anchor[a] % Cell) return r.fail("Anchor must align to the 0.5 m grid");
    for (int m = 0; m < out->count; ++m)
        for (int a = 0; a < 3; ++a) if (out->modules[m].offset[a] % Cell) { error->module = m; return r.fail("Module offset must align to the 0.5 m grid"); }
    bool classic=false;for(int m=0;m<out->count;++m)classic|=out->modules[m].classicObjects;
    if(classic){if(out->pieceCount||out->spaceCount)return r.fail("Classic compositions require an independent objects-only project");
        for(int m=0;m<out->count;++m)if(!out->modules[m].classicObjects)return r.fail("Keep classic compositions separate from checked structure projects");}
    return true;
}

struct CellEntry { int p[3]; uint16_t module; uint8_t material; uint8_t category; };
struct Plan {
    CellEntry* cells; int capacity, count = 0;
    core::BpMat materials[core::BP_MAX_MATS] = {}; int materialCount = 0;
    int objects = 0, duplicates = 0, lo[3] = {}, hi[3] = {};
};
inline bool fail(Error* e, const char* msg, int module = -1) { e->message = msg; e->module = module; return false; }
inline void turn(int r, const int* p, int* out) {
    const int x = p[0], z = p[2]; out[1] = p[1];
    switch (r & 3) {
    case 0: out[0] = x; out[2] = z; break;
    case 1: out[0] = -z; out[2] = x; break;
    case 2: out[0] = -x; out[2] = -z; break;
    default: out[0] = z; out[2] = -x; break;
    }
}
// Rotate a cell's volume about the manifest anchor, not about the bounding box.
inline void cell_low(const CellEntry& c, const int* anchor, int rotation, int* out) {
    int p[3]; for (int a = 0; a < 3; ++a) p[a] = c.p[a] - anchor[a] + Cell / 2;
    turn(rotation, p, out);
    for (int a = 0; a < 3; ++a) out[a] -= Cell / 2;
}
inline core::I3 position(const CellEntry& c, const int* anchor, int rotation, const core::I3& origin) {
    int p[3]; cell_low(c, anchor, rotation, p);
    constexpr int64_t fixUnit = (int64_t)1 << 28;
    return core::I3{origin.x + (int64_t)(p[0] + Cell / 2) * fixUnit,
                    origin.y + (int64_t)(p[1] + Cell / 2) * fixUnit,
                    origin.z + (int64_t)(p[2] + Cell / 2) * fixUnit};
}
inline bool add(Plan* plan, const Module& module, int index, const core::BpHeader& h, const uint8_t* cells, Error* error) {
    int scale[3], turnedScale[3];
    for (int a = 0; a < 3; ++a) {
        const float v = h.step[a] * Unit;
        if (!(v >= Cell && v <= 64) || (int)v != v || (int)v % Cell) return fail(error, "Blueprint grid must be a multiple of 0.5 m, at most 4 m", index);
        scale[a] = (int)v / Cell;
    }
    turnedScale[0] = scale[module.rotation & 1 ? 2 : 0]; turnedScale[1] = scale[1]; turnedScale[2] = scale[module.rotation & 1 ? 0 : 2];
    if(module.objectsOnly){plan->objects+=h.objects;return true;}
    int mapping[core::BP_MAX_MATS];
    for (int m = 0; m < h.nm; ++m) {
        if (!h.m[m].item || h.m[m].mat < 0x80) return fail(error, "Blueprint contains an unidentified building material", index);
        int at = 0;
        while (at < plan->materialCount && (plan->materials[at].item != h.m[m].item || plan->materials[at].mat != h.m[m].mat)) ++at;
        if (at == core::BP_MAX_MATS) return fail(error, "Project exceeds 64 building materials", index);
        if (at == plan->materialCount) { plan->materials[at] = h.m[m]; plan->materials[at].cells = plan->materials[at].items = 0; ++plan->materialCount; }
        mapping[m] = at;
    }
    for (int j = 0; j < h.n[1]; ++j) for (int k = 0; k < h.n[2]; ++k) for (int i = 0; i < h.n[0]; ++i) {
        const uint8_t m = cells[core::bp_index(h.n, i, j, k)];
        if (m == core::BP_EMPTY) continue;
        if (m >= h.nm) return fail(error, "Invalid blueprint material index", index);
        int x, z; core::bp_turn(module.rotation, i, k, h.n[0], h.n[2], &x, &z);
        for (int yy = 0; yy < turnedScale[1]; ++yy) for (int zz = 0; zz < turnedScale[2]; ++zz) for (int xx = 0; xx < turnedScale[0]; ++xx) {
            if (plan->count == plan->capacity) return fail(error, "Project exceeds the expanded cell limit", index);
            CellEntry& c = plan->cells[plan->count++];
            c.p[0] = module.offset[0] + (x * turnedScale[0] + xx) * Cell;
            c.p[1] = module.offset[1] + (j * turnedScale[1] + yy) * Cell;
            c.p[2] = module.offset[2] + (z * turnedScale[2] + zz) * Cell;
            c.module = (uint16_t)index; c.material = (uint8_t)mapping[m]; c.category = (uint8_t)module.category;
            for (int a = 0; a < 3; ++a) if (c.p[a] < -MaxCoord || c.p[a] > MaxCoord - Cell) return fail(error, "Expanded project coordinate out of range", index);
        }
    }
    plan->objects += h.objects;
    return true;
}
inline int compare(const CellEntry& a, const CellEntry& b) {
    const int axes[3] = {1, 2, 0};
    for (int axis : axes) { if (a.p[axis] < b.p[axis]) return -1; if (a.p[axis] > b.p[axis]) return 1; }
    return 0;
}
// In-place heapsort: bounded memory even for a million occupied cells.
inline void sift(CellEntry* p, int n, int i) {
    for (;;) {
        int child = i * 2 + 1;
        if (child >= n) return;
        if (child + 1 < n && compare(p[child], p[child + 1]) < 0) ++child;
        if (compare(p[i], p[child]) >= 0) return;
        CellEntry t = p[i]; p[i] = p[child]; p[child] = t; i = child;
    }
}
inline bool finish(Plan* plan, Error* error) {
    if (!plan->count) return fail(error, "Project contains no building cells");
    for (int i = plan->count / 2; i-- > 0;) sift(plan->cells, plan->count, i);
    for (int n = plan->count; --n > 0;) { CellEntry t = plan->cells[0]; plan->cells[0] = plan->cells[n]; plan->cells[n] = t; sift(plan->cells, n, 0); }
    int n = 0;
    for (int i = 0; i < plan->count; ++i) {
        const CellEntry c = plan->cells[i];
        if (n && compare(plan->cells[n - 1], c) == 0) {
            if (plan->cells[n - 1].material != c.material) return fail(error, "Overlapping modules use different materials", c.module);
            if (c.module < plan->cells[n - 1].module) plan->cells[n - 1] = c;
            ++plan->duplicates; continue;
        }
        plan->cells[n++] = c;
    }
    plan->count = n;
    for (int a = 0; a < 3; ++a) { plan->lo[a] = plan->cells[0].p[a]; plan->hi[a] = plan->lo[a] + Cell; }
    for (int m = 0; m < plan->materialCount; ++m) plan->materials[m].cells = 0;
    for (int i = 0; i < n; ++i) {
        const CellEntry& c = plan->cells[i]; ++plan->materials[c.material].cells;
        for (int a = 0; a < 3; ++a) { if (c.p[a] < plan->lo[a]) plan->lo[a] = c.p[a]; if (c.p[a] + Cell > plan->hi[a]) plan->hi[a] = c.p[a] + Cell; }
    }
    return true;
}
inline int replace_material(Plan* plan, const Manifest& manifest, int source, int target, int module = -1, int category = -1) {
    if (source < 0 || target < 0 || source >= plan->materialCount || target >= plan->materialCount || module >= manifest.count || category >= 7) return -1;
    int changed = 0;
    for (int i = 0; i < plan->count; ++i) {
        auto& c = plan->cells[i];
        if (c.module >= manifest.count) return -1;
        const int type = c.category ? c.category : manifest.modules[c.module].category;
        if (c.material == source && (module < 0 || c.module == module) && (category < 0 || type == category)) {
            if (c.material != target) { c.material = (uint8_t)target; ++changed; }
        }
    }
    for (int m = 0; m < plan->materialCount; ++m) plan->materials[m].cells = 0;
    for (int i = 0; i < plan->count; ++i) ++plan->materials[plan->cells[i].material].cells;
    return changed;
}
// A transaction advances only after a complete matching batch acknowledgement.
enum class Phase { Ready, Checking, Building, Complete, Stopped };
struct Job {
    Phase phase = Phase::Ready; int total = 0, next = 0, batch = 0, completed = 0;
    void start(int cells) { total = cells > 0 ? cells : 0; next = completed = batch = 0; phase = cells > 0 ? Phase::Checking : Phase::Stopped; }
    int prepare(int limit) {
        if ((phase != Phase::Checking && phase != Phase::Building) || limit <= 0 || next >= total) return batch = 0;
        batch = total - next; if (batch > limit) batch = limit; return batch;
    }
    bool checked(int count) {
        if (phase != Phase::Checking || count != batch || batch <= 0) { phase = Phase::Stopped; return false; }
        next += count; batch = 0;
        if (next == total) { next = 0; phase = Phase::Building; }
        return true;
    }
    bool committed(int count) {
        if (phase != Phase::Building || count != batch || batch <= 0) { phase = Phase::Stopped; return false; }
        next += count; completed = next; batch = 0;
        if (next == total) phase = Phase::Complete;
        return true;
    }
};
} // namespace project
