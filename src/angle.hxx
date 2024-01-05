#pragma once
#include <numbers>

template<typename T>
T deg_to_rad(T deg) {
	return deg / T{180} * std::numbers::pi_v<T>;
}