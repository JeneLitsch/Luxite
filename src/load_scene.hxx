#pragma once
#include <vector>
#include <filesystem>
#include "Scene.hxx"
#include "stdxx/json.hxx"

Scene load_scene(const std::filesystem::path & path, const stx::json::iterator manifest);