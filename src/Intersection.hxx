#pragma once
#include "stdxx/vector.hxx"

struct Intersection {
    stx::position3i coords;
    stx::position3f point;
    stx::vector3f normal;
    float depth;
    bool lost;
};