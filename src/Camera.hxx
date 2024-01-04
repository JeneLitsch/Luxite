#pragma once
#include "stdxx/vector.hxx"
#include "stdxx/quat.hxx"
struct Camera {
    stx::position3f position;
    stx::quatf rotation;
};