#include <iostream>
#include <numbers>
#include <tuple>

#include "stdxx/matrix.hxx"
#include "stdxx/log.hxx"
#include "stdxx/json.hxx"

#include "stb/stb_image_write.h"

#include "ray_cast.hxx"
#include "load_scene.hxx"
#include "load_camera.hxx"
#include "load_resolution.hxx"

#include "Scene.hxx"
#include "Camera.hxx"
#include "angle.hxx"


stx::vector3f reflect(stx::vector3f normal, stx::vector3f ray) {
	return ray - 2 * stx::dot(ray, normal) * normal;
}



std::tuple<float, float, float> render_rec(std::size_t rec_counter, bool loose_energy, std::size_t split, const Scene & scene, stx::position3f start, stx::vector3f dir) {
	if(rec_counter <= 0) return {0,0,0};
	auto end = ray_cast(stx::vector3f{start}, stx::normalized(dir), [&] (const Intersection & intersection) {
		const Voxel & v = scene(intersection.coords.x, intersection.coords.y, intersection.coords.z);
		return v.a == 0;
	});

	float bounce_r = 0;
	float bounce_g = 0;
	float bounce_b = 0;

	const Voxel & v = scene(end.coords.x, end.coords.y, end.coords.z);
	float brightness = std::clamp(stx::dot(end.normal, stx::normalized(stx::vector3f{0,-0.5f,1})), 0.1f, 1.f);

	for(std::size_t i = 0; i < split; ++i) {
		const float dx = (rand() % 1000) / 500.f - 1.f;
		const float dy = (rand() % 1000) / 500.f - 1.f;
		const float dz = (rand() % 1000) / 500.f - 1.f;
		const stx::vector3f rand_dir{dx, dy, dz};
		const stx::vector3f hemi_dir = stx::dot(end.normal, rand_dir) >= 0 ? rand_dir : -rand_dir;

		const stx::vector3f new_dir = stx::normalized(hemi_dir);
		const auto [ bounce_r_comp, bounce_g_comp, bounce_b_comp ] = render_rec(rec_counter-1, split, true, scene, end.point, new_dir);
		bounce_r += bounce_r_comp / split;
		bounce_g += bounce_g_comp / split;
		bounce_b += bounce_b_comp / split;
	}

	return {
		(v.r * (brightness + 0.5f * bounce_r)) * (loose_energy ? (1.f - end.depth) : 1.f),
		(v.g * (brightness + 0.5f * bounce_g)) * (loose_energy ? (1.f - end.depth) : 1.f),
		(v.b * (brightness + 0.5f * bounce_b)) * (loose_energy ? (1.f - end.depth) : 1.f),
	};
}



std::vector<std::uint8_t> render(stx::size2u size, const Scene & scene, const Camera & camera) {
	std::vector<std::uint8_t> data;
	const float fov = 45.f;
	const stx::position3f start = camera.position;
	const stx::quatf orientation = camera.rotation;

	constexpr static std::size_t max_bounce = 4;
	constexpr static std::size_t split = 3;

	for(std::int32_t y = 0; y < size.y; ++y){
		for(std::int32_t x = 0; x < size.x; ++x){
			float dx = (static_cast<float>(x) / static_cast<float>(size.x)) * 2.f - 1.f;
			float dy = (static_cast<float>(y) / static_cast<float>(size.y)) * 2.f - 1.f;
			const stx::vector3f default_dir {dx, 1, -dy};
			const stx::vector3f dir = stx::dim_cast<3>(stx::matrix4f::from_quat(camera.rotation) * stx::dim_cast<4>(default_dir));
			auto [r,g,b] = render_rec(max_bounce, false, split, scene, start, dir);
			data.push_back(static_cast<std::uint8_t>(std::clamp(r * 255.f, 0.f, 255.f)));
			data.push_back(static_cast<std::uint8_t>(std::clamp(g * 255.f, 0.f, 255.f)));
			data.push_back(static_cast<std::uint8_t>(std::clamp(b * 255.f, 0.f, 255.f)));
			data.push_back(255);
		}
		if(y % 10 == 0) {
			std::cout << (static_cast<float>(y) / size.y) * 100 << "%" << "\n";
		} 
	}
	std::cout << "100%" << "\n";
	return data;
}



int main(int argc, char ** argv) {
	stx::log.register_output(std::cout);

	if(argc < 4) {
		stx::log[stx::ERROR] 
			<< "To few arguments were provided. Usage: "
			<< argv[0] << " <project> <config> <output_path>";
		return EXIT_FAILURE;
	}

    const std::filesystem::path in_path {argv[1]};
    const std::filesystem::path out_path {argv[3]};
	
    const stx::json::node data = stx::json::from_file(in_path/"manifest.json");
    const stx::json::iterator manifest {data};

	const Scene scene = load_scene(in_path, manifest);
	const stx::size2u resolution = load_resolution(manifest, argv[2]);
	const Camera camera = load_camera(manifest, argv[2]);

	stx::log[stx::WRITE] << "Luxite: Voxel Raytracer (c) 2024 Sera K. Litsch ";

	stx::log[stx::INFO] << "Scene";
	stx::log.indent_in();
	stx::log[stx::WRITE] << "Size:       " << scene.size;
	stx::log.indent_out();
	
	stx::log[stx::INFO] << "Camera";
	stx::log.indent_in();
	stx::log[stx::WRITE] << "Position:   " << camera.position;
	stx::log[stx::WRITE] << "Quaternion: " << camera.rotation;
	stx::log.indent_out();

	stx::log[stx::INFO] << "Output";
	stx::log.indent_in();
	stx::log[stx::WRITE] << "Format:     " << "PNG";
	stx::log[stx::WRITE] << "Resolution: " << resolution;
	stx::log.indent_out();

	stx::log[stx::INFO] << "Rendering...";
	auto rendered_image = render(resolution, scene, camera);
	stx::log[stx::INFO] << "Renering done";

	stx::log[stx::INFO] << "Writing image...";
	if(std::filesystem::create_directory(out_path.parent_path())) {
		stx::log[stx::INFO] 
			<< "Output directory "
			<< std::filesystem::canonical(out_path.parent_path())
			<< " was created.";
	}
	stbi_write_png(out_path.c_str(), resolution.x, resolution.y, 4, rendered_image.data(), resolution.x * 4);
	stx::log[stx::INFO] << "Writing image done!";
}
