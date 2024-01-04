#include "load_scene.hxx"
#include "stb/stb_image.h"
#include <ranges>

Scene load_scene(const std::filesystem::path & path) {
    int image_w, image_h, image_comp;
    const std::uint8_t * image_data = stbi_load(path.c_str(), &image_w, &image_h, &image_comp, STBI_rgb_alpha);
    if(image_data == nullptr) throw std::runtime_error{"Cannot load scene: " + path.string()};
    
    Scene scene;

    for(std::size_t i = 0; i < image_w * image_h; ++i) {
        std::uint8_t r = image_data[4 * i + 0];
        std::uint8_t g = image_data[4 * i + 1];
        std::uint8_t b = image_data[4 * i + 2];
        std::uint8_t a = image_data[4 * i + 3];

        scene.voxels.push_back(Voxel{
            .r = r,
            .g = g,
            .b = b,
            .a = a,
        });
    }

    scene.size = {8,8,4};

    return scene;
}