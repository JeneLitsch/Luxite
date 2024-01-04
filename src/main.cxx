#include <iostream>
#include "ray_cast.hxx"
#include "load_scene.hxx"
#include "stdxx/matrix.hxx"
#include "stb/stb_image_write.h"

bool print_voxel(stx::position3i coord) {
	std::cout << coord << "\n";
	return true;
}


std::vector<std::uint8_t> generate_image(stx::size2u size, const Scene & scene) {
	std::vector<std::uint8_t> data;
	constexpr static float fov = 90.f;
	constexpr static stx::vector3f start {4,-4,2};
	constexpr static stx::matrix3f orientation{
		1,0,0,
		0,1,0,
		0,0,1,
	};

	for(std::int32_t y = 0; y < size.y; ++y){
		for(std::int32_t x = 0; x < size.x; ++x){
			float dx = (static_cast<float>(x) / static_cast<float>(size.x)) * 2.f - 1.f;
			float dy = (1 - static_cast<float>(y) / static_cast<float>(size.y)) * 2.f - 1.f;
			
			data.push_back(0);
			data.push_back(0);
			data.push_back(0);
			data.push_back(255);

			const stx::vector3f dir {dx, 1, dy};
			// std::cout << dir << " | \n";
			// std::cout << dx << " | \n";
			
			auto end = ray_cast(start, dir, [&] (stx::position3i coord, float depth) {
				if(coord.x >= scene.size.x) return true;
				if(coord.y >= scene.size.y) return true;
				if(coord.z >= scene.size.z) return true;

				if(coord.x < 0) return true;
				if(coord.y < 0) return true;
				if(coord.z < 0) return true;

				// std::cout << coord << " | ";
				const Voxel & v = scene.voxels[
					(coord.z * scene.size.x * scene.size.y) + (coord.y * scene.size.x) + coord.x
				];
				// std::cout << int(v.r) << "," << int(v.g) << "," << int(v.b) << "," << int(v.a) << "\n";
				if(v.a != 0) {
					data[(x + (y * size.x)) * 4 + 0] = (v.r * depth);
					data[(x + (y * size.x)) * 4 + 1] = (v.g * depth);
					data[(x + (y * size.x)) * 4 + 2] = (v.b * depth);
					return false;
				}
				return true;
			});
		}
	}
	return data;
}

int main() {
	auto scene = load_scene("test/scenes/a.png");
	// ray_cast({0,0,0}, {1,1,1}, [&] (stx::position3i coord) {
	// 	if(coord.x >= scene.size.x) return true;
	// 	if(coord.y >= scene.size.y) return true;
	// 	if(coord.z >= scene.size.z) return true;

	// 	if(coord.x < 0) return true;
	// 	if(coord.y < 0) return true;
	// 	if(coord.z < 0) return true;

	// 	const Voxel & v = scene.voxels[
	// 		(coord.z * scene.size.x * scene.size.y) + (coord.y * scene.size.x) + coord.x
	// 	];
	// 	std::cout << coord << " | ";
	// 	std::cout << int(v.r) << "," << int(v.g) << "," << int(v.b) << "," << int(v.a) << "\n";
	// 	return true;
	// });

	stx::size2u res {1024, 1024};

	auto data = generate_image({res.x,res.y}, scene);
	stbi_write_png("test/renders/a.png", res.x,res.y, 4, data.data(), res.x * 4);
}
