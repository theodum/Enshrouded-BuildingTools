// Pure geometry shared by native placement, preview and host tests.
#pragma once
#include "project.h"
namespace extended {
constexpr int Levels = 8, MaxLines = 4096;
inline int32_t elapsed(uint32_t now, uint32_t deadline) { return (int32_t)(now - deadline); }
struct Grid {
    int enabled = 0, height = 1920, spacing = 8, extent = 32;
    int x = 0, z = 0, axes = 1;
};
inline int snap(double value, int step) {
    double n = value / step;
    return (int)(n < 0 ? n - 0.5 : n + 0.5) * step;
}
// Cardinal quaternion rotations can put an exact grid boundary a few float ULPs
// outside it. Canonicalize only within 0.0001 sixteenths (= 6.25 micrometres).
// Genuine protrusions remain conservative; no 0.5 m voxel tolerance is added.
inline double native_bound16(float metres) {
    const double v=(double)metres*16;const int nearest=snap(v,1);
    const double delta=v-nearest;return delta>=-0.0001 && delta<=0.0001 ? nearest : v;
}
inline bool grid_hit(const Grid& g, const double* camera, const double* direction, core::I3* out) {
    if (!g.enabled || (g.spacing != 8 && g.spacing != 16) || !(direction[1] < -0.0001 || direction[1] > 0.0001)) return false;
    double t = (g.height / 16.0 - camera[1]) / direction[1];
    if (!(t >= 0 && t <= 512)) return false;
    double x = (camera[0] + t * direction[0]) * 16 - g.x;
    double z = (camera[2] + t * direction[2]) * 16 - g.z;
    if (!(x >= -project::MaxCoord && x <= project::MaxCoord && z >= -project::MaxCoord && z <= project::MaxCoord)) return false;
    constexpr int64_t unit = (int64_t)1 << 28;
    *out = core::I3{(int64_t)(g.x + snap(x, g.spacing)) * unit,
                   (int64_t)g.height * unit, (int64_t)(g.z + snap(z, g.spacing)) * unit};
    return true;
}
struct Box { int lo[3], hi[3]; };
inline Box turn_box(const Box& box, int rotation) {
    Box out{};
    for (int c = 0; c < 8; ++c) {
        int p[3], t[3]; for (int a = 0; a < 3; ++a) p[a] = (c >> a) & 1 ? box.hi[a] : box.lo[a];
        project::turn(rotation,p,t);
        for (int a = 0; a < 3; ++a) { if (!c || t[a] < out.lo[a]) out.lo[a] = t[a]; if (!c || t[a] > out.hi[a]) out.hi[a] = t[a]; }
    }
    return out;
}
inline void box_sample(const Box& box, int sample, int* low) {
    for (int a = 0; a < 3; ++a) {
        const int value = sample == 8 ? box.lo[a] + (box.hi[a] - box.lo[a]) / 2 : (sample >> a) & 1 ? box.hi[a] - 1 : box.lo[a];
        low[a] = value >= 0 ? value / 8 * 8 : -((-value + 7) / 8 * 8);
    }
}
inline int batch_limit(int requested) { return requested < 1 ? 128 : requested > 256 ? 256 : requested; }
inline int adapt_batch(int limit, uint32_t milliseconds) { return milliseconds > 50 && limit > 1 ? (limit+1)/2 : limit; }
inline uint64_t fingerprint_word(uint64_t hash, uint64_t value) {
    for (int k = 0; k < 8; ++k) { hash ^= value & 255; hash *= 1099511628211ull; value >>= 8; } return hash;
}
inline bool intersects(const Box& first, const Box& second, int headroom = 0) {
    for (int a = 0; a < 3; ++a)
        if ((int64_t)first.hi[a] + (a == 1 ? headroom : 0) <= second.lo[a] || first.lo[a] >= second.hi[a]) return false;
    return true;
}
// Sorted Y/Z/X plan: one binary search per intersecting Y/Z row, not a full
// million-cell scan for every native piece. Callers bound the probe volume.
inline bool plan_intersects(const project::Plan& plan, const Box& box) {
    for (int a=0;a<3;++a) if (box.hi[a]<=box.lo[a]) return false;
    int low[3]; for (int a = 0; a < 3; ++a) low[a] = box.lo[a] >= 0 ? box.lo[a] / 8 * 8 : -((-box.lo[a]+7)/8*8);
    for (int y = low[1]; y < box.hi[1]; y += 8) for (int z = low[2]; z < box.hi[2]; z += 8) {
        project::CellEntry key{{low[0],y,z},0,0,0}; int begin = 0, end = plan.count;
        while (begin < end) { int mid = begin + (end-begin)/2; if (project::compare(plan.cells[mid],key) < 0) begin = mid+1; else end = mid; }
        if (begin < plan.count) { const auto& c = plan.cells[begin]; if (c.p[1] == y && c.p[2] == z && c.p[0] < box.hi[0]) return true; }
    }
    return false;
}
inline bool contains(const Box& box, const int* low, int size = project::Cell) {
    for (int a = 0; a < 3; ++a) if (low[a] < box.lo[a] || low[a] + size > box.hi[a]) return false;
    return true;
}
struct Proposal { Box box; int center[3]; };
// Greedy assistance using the dimensions of an observed altar area. Does not
// claim permission, progression, terrain suitability or place anything.
inline bool suggest(const int* low, const int* dimensions, Proposal* out) {
    for (int a = 0; a < 3; ++a) {
        if (dimensions[a] < project::Cell || dimensions[a] > 4096 * 16) return false;
        out->center[a] = snap(low[a] + project::Cell / 2.0, project::Cell);
        out->box.lo[a] = out->center[a] - dimensions[a] / 2;
        out->box.hi[a] = out->box.lo[a] + dimensions[a];
    }
    return contains(out->box, low);
}
struct Object {
    uint32_t item; double p[3]; float rotation[4]; uint16_t module;
};
inline bool finite(double v) { return v >= -65536.0 && v <= 65536.0; }
inline bool object_box(const double* position, const float* rotation, const float* low, const float* high, Box* out) {
    const double x=rotation[0],y=rotation[1],z=rotation[2],w=rotation[3];
    for (int c = 0; c < 8; ++c) {
        double p[3]; for (int a = 0; a < 3; ++a) { if (!finite(low[a]) || !finite(high[a]) || high[a] < low[a]) return false; p[a] = (c>>a)&1 ? high[a] : low[a]; }
        const double q[] = {position[0]+(1-2*(y*y+z*z))*p[0]+2*(x*y-w*z)*p[1]+2*(x*z+w*y)*p[2], position[1]+2*(x*y+w*z)*p[0]+(1-2*(x*x+z*z))*p[1]+2*(y*z-w*x)*p[2], position[2]+2*(x*z-w*y)*p[0]+2*(y*z+w*x)*p[1]+(1-2*(x*x+y*y))*p[2]};
        for (int a = 0; a < 3; ++a) {
            if (!finite(q[a])) return false; double v=q[a]*16; int lo=(int)v,hi=(int)v; if (lo>v) --lo; if (hi<v) ++hi;
            if (!c || lo<out->lo[a]) out->lo[a]=lo; if (!c || hi>out->hi[a]) out->hi[a]=hi;
        }
    }
    return true;
}
// ItemFlag values read from the checked executable's keen::ItemFlag reflection.
inline bool object_flags_allowed(uint32_t flags, bool nativePropContext = false) {
    constexpr uint32_t special = (1u << 0) | (1u << 1) | (1u << 3) | (15u << 4) | (1u << 8) | (1u << 10) | (1u << 11) | (3u << 14) | (1u << 20);
    return !(flags & (special | (nativePropContext ? 0u : 1u << 19))) && !(flags & ~((1u<<22)-1));
}
inline bool transform_object(const project::Module& m, int module, const core::BpHeader& h, const core::BpObject& source, Object* out) {
    float p[3];
    for (int a = 0; a < 3; ++a) if (!finite(source.pos[a])) return false;
    double norm = 0;
    for (int a = 0; a < 4; ++a) { if (!(source.rot[a] >= -1.001f && source.rot[a] <= 1.001f)) return false; norm += source.rot[a] * source.rot[a]; }
    if (norm < 0.99 || norm > 1.01 || !source.item) return false;
    core::bp_turn_point(m.rotation, source.pos, h.n[0] * h.step[0], h.n[2] * h.step[2], p);
    core::bp_turn_rot(m.rotation, source.rot, out->rotation);
    for (int a = 0; a < 3; ++a) { out->p[a] = p[a] + m.offset[a] / 16.0; if (!finite(out->p[a])) return false; }
    out->item = source.item; out->module = (uint16_t)module; return true;
}
inline void object_world(const Object& o, const int* pivot, int turn, const core::I3& anchor, double* pos, float* rotation) {
    double x = o.p[0] - pivot[0] / 16.0, z = o.p[2] - pivot[2] / 16.0, rx, rz;
    switch (turn & 3) { case 0: rx = x; rz = z; break; case 1: rx = -z; rz = x; break; case 2: rx = -x; rz = -z; break; default: rx = z; rz = -x; break; }
    pos[0] = anchor.x / core::kFix + rx; pos[1] = anchor.y / core::kFix + o.p[1] - pivot[1] / 16.0; pos[2] = anchor.z / core::kFix + rz;
    core::bp_turn_rot(turn, o.rotation, rotation);
}
enum Category { General, Wall, Floor, Roof, Tower, Stairs, Furniture, CategoryCount };
inline int category(const char* text) {
    const char* names[] = {"general", "wall", "floor", "roof", "tower", "stairs", "furniture"};
    for (int i = 0; i < CategoryCount; ++i) if (project::equal(names[i], text)) return i;
    return -1;
}
struct Opening { int x, y, width, height; };
// All component sizes are native 0.5 m voxels. Native stair prefabs and the
// separate voxel_stairs.h generator do not use this simple Component shape.
struct Component {
    int kind = Wall, size[3] = {12, 8, 1}, thickness = 1, roofShape = core::ROOF_GABLE;
    Opening openings[16] = {}; int openingCount = 0;
    int round = 0, floors = 1, floorHeight = 8, crenels = 0;
    Opening floorHoles[16] = {}; int floorHoleCount = 0;
};
inline bool valid(const Component& c) {
    for (int a = 0; a < 3; ++a) if (c.size[a] < 1 || c.size[a] > 256) return false;
    if (c.thickness < 1 || c.thickness > 8 || c.openingCount < 0 || c.openingCount > 16) return false;
    if (c.kind < Wall || c.kind > Tower || c.floors < 1 || c.floorHeight < 2) return false;
    if (c.kind == Tower && c.floorHeight - c.thickness < 4) return false;
    if (c.floorHoleCount < 0 || c.floorHoleCount > 16) return false;
    for (int i = 0; i < c.openingCount; ++i) {
        auto o = c.openings[i]; if (o.x < 0 || o.y < 0 || o.width < 1 || o.height < 1 || o.x + o.width > c.size[0] || o.y + o.height > c.size[c.kind == Floor ? 2 : 1]) return false;
        if (c.kind == Tower && o.y + o.height > c.floorHeight) return false;
    }
    for (int i = 0; i < c.floorHoleCount; ++i) { auto o = c.floorHoles[i]; if (o.x < 0 || o.y < 0 || o.width < 1 || o.height < 1 || o.x + o.width > c.size[0] || o.y + o.height > c.size[2]) return false; }
    return true;
}
inline bool component_cell(const Component& c, int x, int y, int z) {
    if (c.kind == Wall) {
        for (int i = 0; i < c.openingCount; ++i) { auto o = c.openings[i]; if (x >= o.x && x < o.x + o.width && y >= o.y && y < o.y + o.height) return false; }
        return z < c.thickness;
    }
    if (c.kind == Floor) {
        for (int i = 0; i < c.openingCount; ++i) { auto o = c.openings[i]; if (x >= o.x && x < o.x + o.width && z >= o.y && z < o.y + o.height) return false; }
        return y < c.thickness;
    }
    if (c.kind == Roof) {
        core::Plan p{}; for (int a = 0; a < 3; ++a) { p.count[a] = c.size[a]; p.stepFix[a] = 1; }
        p.roofShape = c.roofShape; p.roofH = c.size[1]; p.roofBase = 0; p.thick = c.thickness;
        int low, high; core::roof_column(p, x, z, &low, &high); return y >= low && y <= high;
    }
    if (c.kind == Tower) {
        bool inside = true, wall = false;
        if (c.round) {
            inside = core::round_keep(core::ROUND_FULL, 0, false, 1, x, z, c.size[0], c.size[2]);
            wall = core::round_keep(core::ROUND_FULL, 0, true, c.thickness, x, z, c.size[0], c.size[2]);
        } else wall = x < c.thickness || z < c.thickness || x >= c.size[0] - c.thickness || z >= c.size[2] - c.thickness;
        if (!inside) return false;
        if (c.crenels && y >= c.size[1] - 2 && ((x + z) / 2) % 2) return false;
        if (wall) {
            for (int i = 0; i < c.openingCount; ++i) { auto o = c.openings[i]; if ((z < c.thickness || z >= c.size[2] - c.thickness) && x >= o.x && x < o.x + o.width && y % c.floorHeight >= o.y && y % c.floorHeight < o.y + o.height) return false; }
            return true;
        }
        for (int i = 0; i < c.floorHoleCount; ++i) { auto o = c.floorHoles[i]; if (x >= o.x && x < o.x + o.width && z >= o.y && z < o.y + o.height) return false; }
        return (y % c.floorHeight < c.thickness && y / c.floorHeight < c.floors) || (c.crenels && c.floors > 0 && y == c.size[1] - 3);
    }
    return false;
}
struct StairStep { int low[3], rotation; int width, depth; bool landing; };
inline StairStep turn_stair(const StairStep& step, int rotation) {
    const int width = !step.landing && (step.rotation & 1) ? step.depth : step.width;
    const int depth = !step.landing && (step.rotation & 1) ? step.width : step.depth;
    Box bounds = {{step.low[0],step.low[1],step.low[2]}, {step.low[0]+width,step.low[1]+8,step.low[2]+depth}};
    bounds = turn_box(bounds,rotation); StairStep out = step;
    for (int a = 0; a < 3; ++a) out.low[a] = bounds.lo[a];
    out.rotation = (step.rotation + rotation) & 3;
    if (step.landing) { out.width = bounds.hi[0]-bounds.lo[0]; out.depth = bounds.hi[2]-bounds.lo[2]; }
    return out;
}
inline int stair_layout(int kind, int flights, int width, int rise, int run, StairStep* out, int capacity) {
    if (kind < 0 || kind > 3 || flights < 1 || flights > 32 || width < 16 || width % 16 || rise < 8 || rise % 8 || run < 8 || run % 8) return 0;
    if ((kind == 1 && flights < 2) || (kind == 3 && flights < 4)) return 0;
    int count = 0; bool full = false;
    auto put = [&](int x, int y, int z, int rotation, bool landing = false, int w = 0, int d = 0) {
        if (count >= capacity) { full = true; return; }
        out[count++] = StairStep{{x,y,z},rotation,w ? w : width,d ? d : run,landing};
    };
    const int dx[] = {0,-1,0,1}, dz[] = {1,0,-1,0};
    auto flight = [&](int x, int y, int z, int r) {
        int lx = x, lz = z;
        if (r == 0) lx -= width / 2;
        if (r == 1) { lx -= run; lz -= width / 2; }
        if (r == 2) { lx -= width / 2; lz -= run; }
        if (r == 3) lz -= width / 2;
        put(lx,y,lz,r);
    };
    int x = width / 2, y = 0, z = 0, r = 0;
    for (int i = 0; i < flights; ++i) {
        flight(x,y,z,r); x += dx[r] * run; z += dz[r] * run; y += rise;
        bool turn = kind == 3 || (kind == 1 && i + 1 == (flights + 1) / 2);
        if (turn && i + 1 < flights) {
            x += dx[r] * width / 2; z += dz[r] * width / 2;
            put(x-width/2,y-8,z-width/2,0,true,width,width);
            r = (r + 1) & 3; x += dx[r] * width / 2; z += dz[r] * width / 2;
        }
    }
    if (kind == 2) {
        z += width / 2; put(x-width*3/2,y-8,z-width/2,0,true,width*3,width);
        const int sides[] = {-1,1};
        for (int side : sides) {
            const int rr = side < 0 ? 1 : 3; int bx = x + side * width * 3 / 2, by = y;
            for (int i = 0; i < flights; ++i) { flight(bx,by,z,rr); bx += side * run; by += rise; }
        }
    }
    return full ? 0 : count;
}
} // namespace extended
