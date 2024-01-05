#include "load_resolution.hxx"

stx::size2u load_resolution(const stx::json::iterator json_manifest, const std::string & config_name) {
    const stx::json::iterator json_config = json_manifest["config"][config_name];
    if(!json_config) throw stx::json::format_error {"Cannot load config " + config_name};
    const stx::json::iterator json = json_config["resolution"];
    if(!json) throw stx::json::format_error{"Cannot load camera resolution"};
    const std::optional<std::uint32_t> x = json[0].u32();
    const std::optional<std::uint32_t> y = json[1].u32();
    if(!x)throw stx::json::format_error{"Cannot load camera resolution.x"};
    if(!y)throw stx::json::format_error{"Cannot load camera resolution.y"};
    return {*x, *y};
}