#include "../src/project.h"
#include <cstdio>
#include <cstring>
#include <cmath>
#include <vector>
#include <string>
#include <fstream>
#include <random>
#include <algorithm>
#ifndef SRC_DIR
#define SRC_DIR "tests"
#endif
static int failures = 0, checks = 0;
#define CHECK(c) do { ++checks; if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); ++failures; } } while (0)
static const char* valid = R"({"format":"BuildingToolsExtendedProject","version":1,"name":"Citadelle","gridSize":1,"anchor":[0,0,0],"modules":[{"file":"hall.btbp","offset":[0,0,0],"rotation":0},{"file":"tower.btbp","offset":[-3,1,5],"rotation":90},{"file":"bridge.btbp","offset":[12,0,4],"rotation":270}]})";
static project::Manifest manifest;
static bool parse(const std::string& s) { project::Error e; return project::decode(s.data(), (int)s.size(), &manifest, &e); }
static std::string replace(std::string s, const std::string& a, const std::string& b) { s.replace(s.find(a), a.size(), b); return s; }
static core::BpHeader header(int x, int y, int z, float step = 0.5f) {
    core::BpHeader h{}; h.n[0] = x; h.n[1] = y; h.n[2] = z;
    for (auto& s : h.step) s = step;
    h.nm = 1; h.m[0] = {0xaabbccdd, 0x81, 0, 0}; return h; // test-only ID, never installed
}
int main() {
    using namespace project;
    CHECK(!safe_file("CON.btbp") && !safe_file("nul.extra.btbp") && !safe_file("Com1.btbp") && !safe_file("LPT9.btbp") && !safe_file("AUX .btbp"));
    CHECK(safe_file("Chateau_1.btbp") && safe_file("COMPOSITE.btbp"));
    CHECK(parse(valid)); CHECK(manifest.count == 3); CHECK(manifest.modules[1].offset[0] == -48); CHECK(manifest.modules[1].rotation == 1);
    CHECK(parse(replace(valid, "Citadelle", "Citadelle \\u00e9 \\ud83c\\udff0")));
    CHECK(std::strstr(manifest.name, "\xc3\xa9") != nullptr);
    CHECK(!parse(replace(valid, "Citadelle", "bad\\u0000name")));
    CHECK(!parse(replace(valid, "Citadelle", "bad\\ud800")));
    CHECK(!parse(replace(valid, "Citadelle", "bad\\udc00")));
    CHECK(!parse(replace(valid,"Citadelle",std::string("\xc0\xaf",2))));
    CHECK(!parse(replace(valid,"Citadelle",std::string("\xed\xa0\x80",3))));
    CHECK(!parse(replace(valid,"Citadelle",std::string("\xf4\x90\x80\x80",4))));
    CHECK(!parse(replace(valid, "\"version\":1", "\"version\":2")));
    CHECK(!parse(replace(valid, "\"version\":1", "\"version\":1,\"version\":1")));
    CHECK(!parse(replace(valid, "\"version\":1", "\"unknown\":1")));
    CHECK(!parse(replace(valid, "\"rotation\":90", "\"rotation\":45")));
    CHECK(!parse(replace(valid, "hall.btbp", "../hall.btbp")));
    CHECK(!parse(replace(valid, "hall.btbp", "C:hall.btbp")));
    CHECK(!parse(replace(valid, "hall.btbp", "dir\\\\hall.btbp")));
    CHECK(!parse(replace(valid, "hall.btbp", "hall.txt")));
    CHECK(!parse(replace(valid, "\"gridSize\":1", "\"gridSize\":0.3")));
    CHECK(!parse(replace(valid, "\"gridSize\":1", "\"gridSize\":1e9")));
    CHECK(!parse(replace(valid, "\"gridSize\":1", "\"gridSize\":01")));
    CHECK(!parse(replace(valid, "[-3,1,5]", "[-3.25,1,5]")));
    CHECK(!parse(replace(valid, "[-3,1,5]", "[-9999999,1,5]")));
    CHECK(!parse(std::string(valid) + "{}"));
    std::string native = R"({"format":"BuildingToolsExtendedProject","version":1,"name":"native","gridSize":0.5,"anchor":[0,0,0],"modules":[],"pieces":[{"item":4294967295,"material":42,"offset":[-1,0.5,2],"rotation":90}]})";
    CHECK(parse(native)); CHECK(manifest.pieceCount == 1 && manifest.pieces[0].item == 0xffffffff && manifest.pieces[0].material == 42);
    CHECK(!parse(replace(native,"4294967295","4294967296")));
    CHECK(!parse(replace(native,"4294967295","00012")));
    CHECK(!parse(replace(native,"4294967295","0")));
    for (size_t n = 0; n < std::strlen(valid); ++n) CHECK(!parse(std::string(valid, n)));
    std::string many = R"({"format":"BuildingToolsExtendedProject","version":1,"name":"many","gridSize":1,"anchor":[0,0,0],"modules":[)";
    for (int i = 0; i < MaxModules + 1; ++i) { if (i) many += ','; many += R"({"file":"a.btbp","offset":[0,0,0]})"; }
    many += "]}"; CHECK(!parse(many));

    CellEntry cells[256]; Plan plan{cells, 256}; Error e;
    core::BpHeader h = header(2, 1, 3); uint8_t filled[6] = {};
    Module m{}; m.offset[0] = -16; m.offset[1] = 32; m.offset[2] = 80; m.rotation = 1;
    CHECK(add(&plan, m, 0, h, filled, &e)); CHECK(finish(&plan, &e)); CHECK(plan.count == 6);
    CHECK(plan.lo[0] == -16 && plan.lo[1] == 32 && plan.lo[2] == 80);
    CHECK(plan.hi[0] == 8 && plan.hi[1] == 40 && plan.hi[2] == 96);
    int anchor[3] = {16, 8, -16};
    for (int i = 0; i < plan.count; ++i) {
        int lo[3]; cell_low(plan.cells[i], anchor, 1, lo);
        CHECK(lo[0] == -(plan.cells[i].p[2] - anchor[2]) - Cell);
        CHECK(lo[1] == plan.cells[i].p[1] - anchor[1]); CHECK(lo[2] == plan.cells[i].p[0] - anchor[0]);
        int p[3] = {plan.cells[i].p[0], plan.cells[i].p[1], plan.cells[i].p[2]}, original[3];
        std::memcpy(original, p, sizeof p);
        for (int r = 0; r < 4; ++r) turn(1, p, p);
        CHECK(std::memcmp(original, p, sizeof p) == 0);
    }
    // All rotations, rectangular axes, negative offsets and mixed grid refinement.
    for (int r = 0; r < 4; ++r) {
        Plan p{cells, 256}; m.rotation = r; h = header(2, 1, 3, 1.f); h.step[2] = 0.5f;
        CHECK(add(&p, m, 0, h, filled, &e)); CHECK(finish(&p, &e)); CHECK(p.count == 24);
        CHECK(p.hi[0] - p.lo[0] == (r & 1 ? 24 : 32));
        CHECK(p.hi[2] - p.lo[2] == (r & 1 ? 32 : 24));
    }
    Plan duplicates{cells, 256}; m.rotation = 0; h = header(2, 1, 3);
    CHECK(add(&duplicates, m, 0, h, filled, &e)); CHECK(add(&duplicates, m, 1, h, filled, &e));
    CHECK(finish(&duplicates, &e)); CHECK(duplicates.count == 6 && duplicates.duplicates == 6);
    CHECK(finish(&duplicates,&e)); CHECK(duplicates.materials[0].cells==6);
    Plan owner{cells,256}; m.category=2; CHECK(add(&owner,m,1,h,filled,&e)); m.category=1; CHECK(add(&owner,m,0,h,filled,&e));
    CHECK(finish(&owner,&e)); for(int i=0;i<owner.count;++i) CHECK(owner.cells[i].module==0 && owner.cells[i].category==1); m.category=0;
    std::vector<CellEntry> refined(512); const uint8_t one[]={0};
    for(float step : {0.5f,1.f,2.f,4.f}) for(int rotation=0;rotation<4;++rotation) {
        Plan test{refined.data(),512}; auto source=header(1,1,1,step); Module part{}; part.rotation=rotation; part.offset[0]=-16; part.offset[1]=32; part.offset[2]=64;
        CHECK(add(&test,part,0,source,one,&e)); CHECK(finish(&test,&e)); int side=(int)(step*2); CHECK(test.count==side*side*side);
        for(int a=0;a<3;++a) CHECK(test.hi[a]-test.lo[a]==(int)(step*16));
    }
    Plan conflict{cells, 256}; CHECK(add(&conflict, m, 0, h, filled, &e)); h.m[0].item++;
    CHECK(add(&conflict, m, 1, h, filled, &e)); CHECK(!finish(&conflict, &e));
    Plan capped{cells, 5}; CHECK(!add(&capped, m, 0, h, filled, &e));
    Plan unsupported{cells, 256}; h.step[0] = 0.75f; CHECK(!add(&unsupported, m, 0, h, filled, &e));
    // A source's empty border does not shift its authored offset or global pivot.
    Plan sparse{cells, 256}; h = header(3, 1, 1); uint8_t bordered[3] = {core::BP_EMPTY, 0, core::BP_EMPTY};
    Module border{}; border.offset[0] = -16;
    CHECK(add(&sparse, border, 0, h, bordered, &e)); CHECK(finish(&sparse, &e)); CHECK(sparse.lo[0] == -8);
    const int pivot[3] = {8, 0, -8}; const core::I3 zero{};
    const core::I3 translated = position(sparse.cells[0], pivot, 0, zero);
    CHECK(translated.x == -(int64_t)12 * ((int64_t)1 << 28));
    // Decoder keeps legacy files without an object extension, rejects a torn count
    // and zero-length RLE runs without touching a world or native pointers.
    uint8_t encoded[256], back[3]; int size = core::bp_encode(h, bordered, encoded, sizeof encoded);
    core::BpHeader decoded{};
    CHECK(size > 4 && core::bp_decode(encoded, size - 4, &decoded, back, 3));
    CHECK(!core::bp_decode(encoded, size - 1, &decoded, back, 3));
    encoded[52] = encoded[53] = 0; // first RLE length, after one material and run count
    CHECK(!core::bp_decode(encoded, size, &decoded, back, 3));
    // Regression: more than 65,536 cells without a dense encompassing blueprint.
    std::vector<CellEntry> big(70000); Plan large{big.data(), (int)big.size()};
    h = header(40, 1, 600); std::vector<uint8_t> source(24000, 0);
    for (int module = 0; module < 3; ++module) {
        m.offset[0] = module * 40 * Cell; m.offset[1] = m.offset[2] = 0;
        if (module == 2) h.n[2] = 550;
        CHECK(add(&large, m, module, h, source.data(), &e));
    }
    CHECK(finish(&large, &e)); CHECK(large.count == 70000);
    for (int i = 1; i < large.count; ++i) CHECK(compare(large.cells[i - 1], large.cells[i]) < 0);
    std::mt19937 random(19); std::shuffle(big.begin(), big.end(), random);
    Plan reordered{big.data(), (int)big.size()}; reordered.count = 70000; reordered.materialCount = 1;
    CHECK(finish(&reordered, &e)); CHECK(reordered.count == 70000);
    for (int i = 1; i < reordered.count; ++i) CHECK(compare(big[i - 1], big[i]) < 0);
    Job job; job.start(70000); int calls = 0;
    while (job.phase == Phase::Checking) { int n = job.prepare(257); CHECK(job.checked(n)); ++calls; }
    CHECK(calls == 273 && job.next == 0 && job.phase == Phase::Building);
    while (job.phase == Phase::Building) { int n = job.prepare(257); CHECK(job.committed(n)); }
    CHECK(job.completed == 70000 && job.phase == Phase::Complete);
    CHECK(job.prepare(10)==0); job.start(0); CHECK(job.phase==Phase::Stopped && job.prepare(1)==0);
    job.start(4); CHECK(job.prepare(0)==0 && job.next==0); CHECK(job.prepare(2)==2); CHECK(!job.checked(1) && job.completed==0 && job.next==0);
    job.start(4); CHECK(job.prepare(4)==4 && job.checked(4)); CHECK(job.prepare(2)==2 && job.committed(2));
    job.phase=Phase::Stopped; CHECK(job.prepare(2)==0 && job.completed==2 && !job.committed(2));
    job.start(3); CHECK(job.prepare(2) == 2); CHECK(!job.checked(1)); CHECK(job.phase == Phase::Stopped && job.next == 0);
    job.start(3); job.prepare(3); CHECK(job.checked(3)); job.prepare(3); CHECK(!job.committed(2)); CHECK(job.completed == 0);
    // Load the actual packaged manifest and all three BTBP files through the same
    // decoder/planner used by the DLL. Verify module bounds after both rotations.
    const std::string root = std::string(SRC_DIR) + "/../examples/three-modules/";
    std::ifstream mf(root + "Trois_modules_test.btproject", std::ios::binary);
    std::string json((std::istreambuf_iterator<char>(mf)), {});
    CHECK(parse(json)); CHECK(manifest.count == 3);
    std::vector<CellEntry> exampleCells(4096); Plan example{exampleCells.data(), (int)exampleCells.size()};
    for (int i = 0; i < manifest.count; ++i) {
        std::ifstream bf(root + manifest.modules[i].file, std::ios::binary);
        std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(bf)), {}), decoded(65536);
        core::BpHeader blueprint{};
        const bool validFile = core::bp_decode(bytes.data(), (int)bytes.size(), &blueprint, decoded.data(), (long)decoded.size());
        CHECK(validFile);
        if (validFile) CHECK(add(&example, manifest.modules[i], i, blueprint, decoded.data(), &e));
        if (bytes.size() >= 26) {
            for (int q = 20; q < 26; ++q) bytes[q] = 255;
            CHECK(!core::bp_decode(bytes.data(), (int)bytes.size(), &blueprint, decoded.data(), 65536));
        }
    }
    CHECK(finish(&example, &e)); CHECK(example.count == 312 && example.duplicates == 0);
    const int bounds[3][6] = {{0, 0, 0, 96, 48, 64}, {128, 16, 0, 144, 40, 16}, {32, 32, 80, 48, 40, 128}};
    for (int module = 0; module < 3; ++module) for (int r = 0; r < 4; ++r) {
        int low[3] = {}, high[3] = {}; bool first = true;
        const core::I3 origin{(int64_t)1024 << 32, (int64_t)128 << 32, (int64_t)2048 << 32};
        for (int i = 0; i < example.count; ++i) {
            const auto& c = example.cells[i]; if (c.module != module) continue;
            int p[3]; cell_low(c, manifest.anchor, r, p);
            const core::I3 w = position(c, manifest.anchor, r, origin);
            CHECK(w.x == origin.x + (int64_t)(p[0] + 4) * ((int64_t)1 << 28));
            CHECK(w.y == origin.y + (int64_t)(p[1] + 4) * ((int64_t)1 << 28));
            CHECK(w.z == origin.z + (int64_t)(p[2] + 4) * ((int64_t)1 << 28));
            for (int a = 0; a < 3; ++a) { if (first || p[a] < low[a]) low[a] = p[a]; if (first || p[a] + Cell > high[a]) high[a] = p[a] + Cell; }
            first = false;
        }
        int expectedLow[3] = {}, expectedHigh[3] = {};
        for (int corner = 0; corner < 8; ++corner) {
            int p[3], out[3]; for (int a = 0; a < 3; ++a) p[a] = bounds[module][a + ((corner >> a) & 1 ? 3 : 0)];
            turn(r, p, out);
            for (int a = 0; a < 3; ++a) { if (!corner || out[a] < expectedLow[a]) expectedLow[a] = out[a]; if (!corner || out[a] > expectedHigh[a]) expectedHigh[a] = out[a]; }
        }
        CHECK(std::memcmp(low, expectedLow, sizeof low) == 0); CHECK(std::memcmp(high, expectedHigh, sizeof high) == 0);
    }
    // Installable dev3 campaign fixtures, using the actual BTBP decoder/planner.
    const char* campaign[]={"Dev3_Trois_batiments_relies.btproject","Dev3_Batiment_etages.btproject","Dev3_Grille_vide_Y120.btproject","Dev3_Village_36_batiments.btproject"};
    const int expectedCells[]={800,1444,528,75648};
    const int expectedModules[]={5,7,3,96};
    for(int fixture=0;fixture<4;++fixture) {
        const std::string folder=std::string(SRC_DIR)+"/../examples/dev3/";
        std::ifstream input(folder+campaign[fixture],std::ios::binary); std::string data((std::istreambuf_iterator<char>(input)),{});
        CHECK(parse(data)); CHECK(manifest.count==expectedModules[fixture]);
        std::vector<CellEntry> storage(100000); Plan loaded{storage.data(),(int)storage.size()};
        for(int i=0;i<manifest.count;++i) {
            std::ifstream src(folder+manifest.modules[i].file,std::ios::binary); std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(src)),{}),decodedCells(65536); core::BpHeader source{};
            bool ok=core::bp_decode(bytes.data(),(int)bytes.size(),&source,decodedCells.data(),65536); CHECK(ok);
            if(ok) CHECK(add(&loaded,manifest.modules[i],i,source,decodedCells.data(),&e));
        }
        CHECK(finish(&loaded,&e)); CHECK(loaded.count==expectedCells[fixture]);
        for(int r=0;r<4;++r) {
            for(int index : {0,loaded.count/2,loaded.count-1}) {
                int low[3];cell_low(loaded.cells[index],manifest.anchor,r,low);const auto placed=position(loaded.cells[index],manifest.anchor,r,core::I3{});
                CHECK(placed.x==(int64_t)(low[0]+4)*((int64_t)1<<28)); CHECK(placed.y==(int64_t)(low[1]+4)*((int64_t)1<<28));
            }
        }
        if(fixture==0) { // passage floors touch both houses with the same top altitude
            auto occupied=[&](int x,int y,int z) { CellEntry key{{x,y,z},0,0,0}; return std::binary_search(storage.begin(),storage.begin()+loaded.count,key,[](const CellEntry& a,const CellEntry& b){return compare(a,b)<0;}); };
            for(int x=0;x<40;++x) CHECK(occupied(x*8,0,24));
        }
    }
    // Bounded parser fuzz, including arbitrary embedded NUL and high bytes.
    for (int n = 0; n < 3000; ++n) {
        std::string damaged(valid); damaged[random() % damaged.size()] = (char)random(); parse(damaged);
    }
    std::printf("project: %d checks, %d failures\n", checks, failures);
    return failures ? 1 : 0;
}
