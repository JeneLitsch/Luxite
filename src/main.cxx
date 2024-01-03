#include <iostream>
#include "ray_cast.hxx"

bool print_voxel(const Voxel & voxel) {
	std::cout << voxel.positon << "\n";
	return true;
}

int main() {
	const stx::vector3f end = ray_cast({0,0,0}, {1,2,-3}, print_voxel);
	std::cout << "END: " << end << "\n";
}
