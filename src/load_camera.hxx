#pragma once
#include "Camera.hxx"
#include "stdxx/json.hxx"

Camera load_camera(const stx::json::iterator manifest, const std::string & config_name);
