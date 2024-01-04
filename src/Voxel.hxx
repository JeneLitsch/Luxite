#pragma once
#include <cstdint>

struct Voxel {
    float r, g, b, a;
};

namespace voxel {
    constexpr inline static Voxel transparent {
        .r = 0,
        .g = 0,
        .b = 0,
        .a = 0,
    };
}