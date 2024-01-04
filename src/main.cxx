#include <iostream>
#include <numbers>
#include "ray_cast.hxx"
#include "load_scene.hxx"
#include "stdxx/matrix.hxx"
#include "stdxx/quat.hxx"
#include "stb/stb_image_write.h"

#include "Scene.hxx"
#include "Camera.hxx"

bool print_voxel(stx::position3i coord) {
	std::cout << coord << "\n";
	return true;
}

float deg_to_rad(float deg) {
	return deg / 180 * std::numbers::pi_v<float>;
}


std::vector<std::uint8_t> render(stx::size2u size, const Scene & scene, const Camera & camera) {
	std::vector<std::uint8_t> data;
	const float fov = 45.f;
	const stx::position3f start = camera.position;
	const stx::quatf orientation = camera.rotation;
;

	for(std::int32_t y = 0; y < size.y; ++y){
		for(std::int32_t x = 0; x < size.x; ++x){
			float dx = (static_cast<float>(x) / static_cast<float>(size.x)) * 2.f - 1.f;
			float dy = (static_cast<float>(y) / static_cast<float>(size.y)) * 2.f - 1.f;
			
			const stx::vector3f default_dir {dx, 1, -dy};
			const stx::vector3f dir = stx::dim_cast<3>(stx::matrix4f::from_quat(camera.rotation) * stx::dim_cast<4>(default_dir));
			
			auto end = ray_cast(stx::vector3f{start}, stx::normalized(dir), [&] (const Intersection & intersection) {
				const Voxel & v = scene(intersection.coords.x, intersection.coords.y, intersection.coords.z);
				return v.a == 0;
			});

			const Voxel & v = scene(end.coords.x, end.coords.y, end.coords.z);
			float brightness = std::clamp(stx::dot(end.normal, stx::normalized(stx::vector3f{0,0.5f,-1})), 0.1f, 1.f);
			data.push_back(v.r * brightness);
			data.push_back(v.g * brightness);
			data.push_back(v.b * brightness);
			data.push_back(255);

		}
	}
	return data;
}

int main() {
	stx::size2u res {1024, 1024};
	const Scene scene = load_scene("test/scenes/a.png");
	const Camera camera {
		.position = {-4,-4, 4},
		.rotation
			= stx::quatf::from_axis_angle(stx::vector3f{1,0,0},  deg_to_rad(25.f))
			* stx::quatf::from_axis_angle(stx::vector3f{0,0,1},  deg_to_rad(45.f)),
	};

	auto data = render({res.x,res.y}, scene, camera);
	stbi_write_png("test/renders/a.png", res.x,res.y, 4, data.data(), res.x * 4);
}
