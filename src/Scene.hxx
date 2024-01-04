#pragma once
#include <vector>
#include "stdxx/vector.hxx"
#include "Voxel.hxx"

struct Scene {
    std::vector<Voxel> voxels;
    stx::size3u size;

    const Voxel & operator()(std::int64_t x, std::int64_t y, std::int64_t z) const {
        if(x >= this->size.x) return voxel::transparent;
        if(y >= this->size.y) return voxel::transparent;
        if(z >= this->size.z) return voxel::transparent;

        if(x < 0) return voxel::transparent;
        if(y < 0) return voxel::transparent;
        if(z < 0) return voxel::transparent;

        return this->voxels[
            (z * this->size.x * this->size.y) +
            (y * this->size.x               ) +
            (x                              )
        ];
    } 
};