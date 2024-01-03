#pragma once
#include "stdxx/vector.hxx"

struct Voxel {
	stx::position3i positon;
};

auto squared(auto x) {
	return x * x;
}

stx::vector3f ray_cast(stx::vector3f start, stx::vector3f end, auto process_voxel) {
	const stx::vector3f dir = stx::normalized(end - start);
	const stx::vector3f scale {
		std::sqrt(1                      + squared(dir.y / dir.x) + squared(dir.z / dir.x)),
		std::sqrt(squared(dir.x / dir.y) + 1                      + squared(dir.z / dir.y)),
		std::sqrt(squared(dir.x / dir.z) + squared(dir.y / dir.z) + 1),
	};

	std::cout << dir << "\n";
	std::cout << scale << "\n";

	stx::position3i voxel_coord = stx::position3i{start};
	stx::vector3f ray_length_1d {0,0,0}; 
	stx::position3i step {0,0,0};

	if(dir.x < 0) {
		step.x = -1;
		ray_length_1d.x = (start.x - voxel_coord.x) * scale.x;
	}
	else {
		step.x = +1;
		ray_length_1d.x = (voxel_coord.x + 1 - start.x) * scale.x;
	}

	if(dir.y < 0) {
		step.y = -1;
		ray_length_1d.y = (start.y - voxel_coord.y) * scale.y;
	}
	else {
		step.y = +1;
		ray_length_1d.y = (voxel_coord.y + 1 - start.y) * scale.y;
	}

	if(dir.z < 0) {
		step.z = -1;
		ray_length_1d.z = (start.z - voxel_coord.z) * scale.z;
	}
	else {
		step.z = +1;
		ray_length_1d.z = (voxel_coord.z + 1 - start.z) * scale.z;
	}

	const float max_dist = 100.f;
	bool running = true;
	float dist = 0.f;
	while(running && (dist < max_dist)) {
		const float shortest = std::min({
			ray_length_1d.x,
			ray_length_1d.y,
			ray_length_1d.z
		});

		if(shortest == ray_length_1d.x) {
			voxel_coord.x += step.x;
			dist = ray_length_1d.x;
			ray_length_1d.x += scale.x;
		} 
		if(shortest == ray_length_1d.y) {
			voxel_coord.y += step.y;
			dist = ray_length_1d.y;
			ray_length_1d.y += scale.y;
		} 
		if(shortest == ray_length_1d.z) {
			voxel_coord.z += step.z;
			dist = ray_length_1d.z;
			ray_length_1d.z += scale.z;
		} 

		running = process_voxel(Voxel{
			.positon = voxel_coord,
		});
	}

	return start + dir * dist;
}