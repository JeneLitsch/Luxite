#pragma once
#include "stdxx/json.hxx"
#include "stdxx/vector.hxx"

stx::size2u load_resolution(const stx::json::iterator manifest, const std::string & config_name);