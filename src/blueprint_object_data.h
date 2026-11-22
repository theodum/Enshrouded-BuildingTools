#pragma once
#include "core.h"
// Runtime capture data; the BTBP format still writes only ID, position and quat.
struct BpFound { uint32_t entity,item;int64_t pos[3];float rot[4];float scale[3]{1,1,1};uint8_t templateId[16]{}; };
