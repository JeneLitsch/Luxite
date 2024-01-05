#include "load_scene.hxx"
#include "stb/stb_image.h"

namespace {
    stx::size3u load_size(const stx::json::iterator json) {
        if(!json) throw stx::json::format_error{"Cannot load scene size"};
        const std::optional<std::uint32_t> x = json[0].u32();
        const std::optional<std::uint32_t> y = json[1].u32();
        const std::optional<std::uint32_t> z = json[2].u32();
        if(!x)throw stx::json::format_error{"Cannot load scene size.x"};
        if(!y)throw stx::json::format_error{"Cannot load scene size.y"};
        if(!z)throw stx::json::format_error{"Cannot load scene size.z"};
        return {*x, *y, *z};
    };
}


Scene load_scene(const std::filesystem::path & path, const stx::json::iterator manifest) {
    const std::filesystem::path albedo_path = path/"albedo.png";

    int image_w, image_h, image_comp;
    std::uint8_t * image_data = stbi_load(albedo_path.c_str(), &image_w, &image_h, &image_comp, STBI_rgb_alpha);
    if(image_data == nullptr) throw std::runtime_error{"Cannot load scene image: " + albedo_path.string()};
    
    Scene scene;

    for(std::size_t i = 0; i < image_w * image_h; ++i) {
        std::uint8_t r = image_data[4 * i + 0];
        std::uint8_t g = image_data[4 * i + 1];
        std::uint8_t b = image_data[4 * i + 2];
        std::uint8_t a = image_data[4 * i + 3];

        scene.voxels.push_back(Voxel{
            .r = static_cast<float>(r) / 255.f,
            .g = static_cast<float>(g) / 255.f,
            .b = static_cast<float>(b) / 255.f,
            .a = static_cast<float>(a) / 255.f,
        });
    }

    scene.size = load_size(manifest["size"]);

    stbi_image_free(image_data);

    return scene;
}