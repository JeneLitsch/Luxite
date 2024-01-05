#include "load_camera.hxx"
#include "angle.hxx"

namespace {
	stx::position3f load_camera_position(const stx::json::iterator json) {
		if(!json) throw stx::json::format_error{"Cannot load camera position"};
		const std::optional<float> x = stx::static_opt_cast<float>(json[0].number());
		const std::optional<float> y = stx::static_opt_cast<float>(json[1].number());
		const std::optional<float> z = stx::static_opt_cast<float>(json[2].number());
		if(!x)throw stx::json::format_error{"Cannot load camera position.x"};
		if(!y)throw stx::json::format_error{"Cannot load camera position.y"};
		if(!z)throw stx::json::format_error{"Cannot load camera position.z"};
		return {*x, *y, *z};
	};



	stx::vector3f load_camera_rotation(const stx::json::iterator json) {
		if(!json) throw stx::json::format_error{"Cannot load camera rotation"};
		const std::optional<float> x = stx::static_opt_cast<float>(json[0].number());
		const std::optional<float> y = stx::static_opt_cast<float>(json[1].number());
		const std::optional<float> z = stx::static_opt_cast<float>(json[2].number());
		if(!x)throw stx::json::format_error{"Cannot load camera rotation.x"};
		if(!y)throw stx::json::format_error{"Cannot load camera rotation.y"};
		if(!z)throw stx::json::format_error{"Cannot load camera rotation.z"};
		return {*x, *y, *z};
	};
}



Camera load_camera(const stx::json::iterator json_manifest, const std::string & config_name) {
	return Camera {
		.position = {-2,-2, 4},
		.rotation
			= stx::quatf::from_axis_angle(stx::vector3f{1,0,0}, deg_to_rad(25.f))
			* stx::quatf::from_axis_angle(stx::vector3f{0,0,1}, deg_to_rad(45.f)),
	};
	
	const stx::json::iterator json_config = json_manifest["config"][config_name];
    if(!json_config) throw stx::json::format_error {"Cannot load config " + config_name};
    const stx::json::iterator json_camera = json_config["camera"];
	const stx::position3f position = load_camera_position(json_camera["position"]);
	const stx::vector3f rotation = load_camera_rotation(json_camera["rotation"]);

	std::cout << position << rotation << "\n"; 
    
    return Camera {
		.position = position,
		.rotation
			= stx::quatf::from_axis_angle(stx::vector3f{1,0,0}, deg_to_rad(rotation.x))
			* stx::quatf::from_axis_angle(stx::vector3f{0,0,1}, deg_to_rad(rotation.z)),
	};
}