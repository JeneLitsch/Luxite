#include <iostream>
#include <numbers>
#include <tuple>
#include <string_view>
#include <chrono>
#include <span>
#include <iomanip>
#include <future>

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



struct Options {
	bool threaded = false;
};



Options parse_options(std::span<char *> rest) {
	Options options;
	for(char * option : rest) {
		if(std::string_view{option} == "--threaded") {
			options.threaded = true;
		}
	}
	return options;
}



std::vector<std::uint8_t> render(const stx::size2u resolution, const Scene & scene, const Camera & camera, const Options & options) {
	std::vector<std::uint8_t> data;
	data.resize(resolution.x * resolution.y * 4);
	const float fov = 45.f;
	const stx::position3f start = camera.position;
	const stx::quatf orientation = camera.rotation;

	constexpr static std::size_t max_bounce = 4;
	constexpr static std::size_t split = 3;

	const auto f = [&resolution, &scene, &camera, &start, &data](std::int32_t y_start, std::int32_t y_end) {
		for(std::int32_t y = y_start; y < y_end; ++y){
			for(std::int32_t x = 0; x < resolution.x; ++x){
				const float dx = (static_cast<float>(x) / static_cast<float>(resolution.x)) * 2.f - 1.f;
				const float dy = (static_cast<float>(y) / static_cast<float>(resolution.y)) * 2.f - 1.f;
				const stx::vector3f default_dir {dx, 1, -dy};
				const stx::vector3f dir = stx::dim_cast<3>(stx::matrix4f::from_quat(camera.rotation) * stx::dim_cast<4>(default_dir));
				const auto [r,g,b] = render_rec(max_bounce, false, split, scene, start, dir);
				const std::size_t i = 4 * (y * resolution.x + x);
				data[i + 0] = static_cast<std::uint8_t>(std::clamp(r * 255.f, 0.f, 255.f));
				data[i + 1] = static_cast<std::uint8_t>(std::clamp(g * 255.f, 0.f, 255.f));
				data[i + 2] = static_cast<std::uint8_t>(std::clamp(b * 255.f, 0.f, 255.f));
				data[i + 3] = 255;
			}
			if(y % (resolution.y / 20) == 0) {
				const float percentage = (static_cast<float>(y - y_start) / (y_end - y_start)) * 100;
				std::cout 
					<< std::round(percentage) << "% in chunk"
					<< "["<< y_start << ", " << y_end << "]"
					<< "\n";
			} 
		}
	};

	if(options.threaded) {
		constexpr static std::size_t num_of_threads = 4;
		std::array<std::future<void>, num_of_threads> chunks;

		for(std::size_t i = 0; i < num_of_threads; ++i) {
			chunks[i] = std::async(std::launch::async, [&, i] () {
				const std::int32_t y_start = resolution.y * i / num_of_threads;
				const std::int32_t y_end   = resolution.y * (i + 1) / num_of_threads;
				return f(y_start, y_end);
			});
		}

		for(const std::future<void> & chunk : chunks) {
			chunk.wait();
		}
	}
	else {
		f(0, resolution.y);
	}

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
	const std::string config = argv[2];
    const std::filesystem::path out_path {argv[3]};
	const Options options = parse_options(std::span<char*>{argv + 4, argv + argc});

    const stx::json::node data = stx::json::from_file(in_path/"manifest.json");
    const stx::json::iterator manifest {data};

	const Scene scene = load_scene(in_path, manifest);
	const stx::size2u resolution = load_resolution(manifest, config);
	const Camera camera = load_camera(manifest, config);

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

	stx::log[stx::INFO] << "Options";
	stx::log.indent_in();
	stx::log[stx::WRITE] << "--threaded: " << std::boolalpha << options.threaded;
	stx::log.indent_out();

	stx::log[stx::INFO] << "Output";
	stx::log.indent_in();
	stx::log[stx::WRITE] << "Format:     " << "PNG";
	stx::log[stx::WRITE] << "Resolution: " << resolution;
	stx::log.indent_out();

	stx::log[stx::INFO] << "Rendering...";
	std::chrono::steady_clock clock;
	std::chrono::time_point time_start = clock.now();
	auto rendered_image = render(resolution, scene, camera, options);
	std::chrono::time_point time_end= clock.now();
	std::chrono::duration<double> duration = std::chrono::duration_cast<std::chrono::duration<double>>(time_end - time_start);
	stx::log[stx::INFO] << "Renering done. Duration: " << duration;


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
