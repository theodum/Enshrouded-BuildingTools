#include "../src/extended_core.h"
#include <cmath>
#include <cstdio>
static int failures = 0, checks = 0;
#define CHECK(x) do { ++checks; if (!(x)) { std::printf("FAIL %d: %s\n", __LINE__, #x); ++failures; } } while (0)
int main() {
    using namespace extended;
    Grid grid; grid.enabled = 1; grid.height = 120 * 16;
    const double cam[] = {10.13, 128, -3.13}, ray[] = {0, -1, 0}; core::I3 hit;
    CHECK(grid_hit(grid, cam, ray, &hit)); CHECK(hit.x / core::kFix == 10.0 && hit.y / core::kFix == 120.0 && hit.z / core::kFix == -3.0);
    grid.x = 4; grid.z = -4; CHECK(grid_hit(grid, cam, ray, &hit)); CHECK(hit.x / core::kFix == 10.25 && hit.z / core::kFix == -3.25);
    const double parallel[] = {1, 0, 0}, away[] = {0, 1, 0}, nearParallel[] = {1, -0.00000001, 0};
    CHECK(!grid_hit(grid, cam, parallel, &hit)); CHECK(!grid_hit(grid, cam, away, &hit)); CHECK(!grid_hit(grid, cam, nearParallel, &hit));
    grid.enabled = 0; CHECK(!grid_hit(grid, cam, ray, &hit)); grid.enabled = 1;
    grid.height = -10000 * 16; CHECK(!grid_hit(grid, cam, ray, &hit));
    grid.height = -4; grid.spacing = 16; CHECK(grid_hit(grid, cam, ray, &hit)); CHECK(hit.y / core::kFix == -0.25);
    Proposal p; const int low[] = {100, 200, 300}, dimensions[] = {640, 640, 640};
    CHECK(suggest(low, dimensions, &p)); CHECK(contains(p.box, low)); int above[] = {100, 200 + 640, 300}; CHECK(!contains(p.box, above));
    int small[] = {0, 640, 640}; CHECK(!suggest(low, small, &p));
    core::BpHeader h{}; h.n[0] = 3; h.n[1] = 2; h.n[2] = 5;
    for (float& step : h.step) step = 0.5f;
    core::BpObject source{0x1234, {0.25f, 0.5f, 0.75f}, {0, 0, 0, 1}};
    project::Module module{}; module.offset[0] = -32; module.offset[1] = 16; module.offset[2] = 48; module.rotation = 1;
    Object object; CHECK(transform_object(module, 4, h, source, &object));
    CHECK(object.p[0] == -0.25 && object.p[1] == 1.5 && object.p[2] == 3.25 && object.module == 4);
    double point[3]; float rotation[4]; const int pivot[] = {0, 0, 0}; const core::I3 origin{};
    object_world(object, pivot, 1, origin, point, rotation); CHECK(point[0] == -3.25 && point[1] == 1.5 && point[2] == -0.25);
    CHECK(std::fabs(std::fabs(rotation[1]) - 1) < 1e-5); CHECK(std::fabs(rotation[3]) < 1e-5);
    source.rot[3] = 10; CHECK(!transform_object(module, 4, h, source, &object));
    project::CellEntry materials[3] = {{{0,0,0},0,0},{{8,0,0},1,0},{{16,0,0},1,1}};
    project::Plan materialPlan{materials,3}; materialPlan.count = 3; materialPlan.materialCount = 2;
    project::Manifest categories{}; categories.count = 2; categories.modules[1].category = Wall;
    CHECK(project::replace_material(&materialPlan,categories,0,1,1) == 1);
    CHECK(materials[0].material == 0 && materials[1].material == 1);
    CHECK(project::replace_material(&materialPlan,categories,1,0,-1,Wall) == 2);
    CHECK(materialPlan.materials[0].cells == 3 && materialPlan.materials[1].cells == 0);
    CHECK(project::replace_material(&materialPlan,categories,0,10) == -1);
    Component c; c.kind = Wall; c.size[0] = 12; c.size[1] = 8; c.size[2] = 2; c.thickness = 2;
    c.openingCount = 1; c.openings[0] = {4, 0, 4, 5}; CHECK(valid(c));
    CHECK(component_cell(c, 1, 0, 0)); CHECK(!component_cell(c, 5, 2, 0)); CHECK(component_cell(c, 5, 6, 0));
    c.kind = Floor; c.size[1] = 1; c.size[2] = 12; c.thickness = 1;
    c.openings[0] = {2, 3, 4, 5}; CHECK(valid(c)); CHECK(!component_cell(c, 3, 0, 4)); CHECK(component_cell(c, 1, 0, 4));
    c.kind = Tower; c.openingCount = 0; c.size[1] = 24; c.floors = 3; c.floorHeight = 8; c.round = 0;
    CHECK(valid(c)); CHECK(component_cell(c, 0, 3, 4)); CHECK(!component_cell(c, 5, 3, 5)); CHECK(component_cell(c, 5, 8, 5));
    c.round = 1; CHECK(!component_cell(c, 0, 3, 0)); CHECK(component_cell(c, 5, 8, 5));
    c.kind = Roof; c.round = 0; c.size[1] = 8;
    for (int shape = 0; shape < core::ROOF_COUNT; ++shape) {
        c.roofShape = shape; int count = 0;
        for (int y = 0; y < c.size[1]; ++y) for (int z = 0; z < c.size[2]; ++z) for (int x = 0; x < c.size[0]; ++x) count += component_cell(c, x, y, z);
        CHECK(count > 0);
    }
    std::printf("extended: %d checks, %d failures\n", checks, failures); return failures != 0;
}
