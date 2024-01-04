#pragma once
#include <vector>
#include "stdxx/vector.hxx"
#include "Voxel.hxx"

struct Scene {
    std::vector<Voxel> voxels;
    stx::size3u size;
};