cmake_minimum_required (VERSION 3.8)

add_compile_options("-std=c++20" "-g")

add_executable(app
    "main.cxx"
    "load_camera.cxx"
    "load_resolution.cxx"
    "load_scene.cxx"
    "stb_impl.cxx"
)

# target_link_libraries(app 
# 	"pthread"
# 	"sfml-graphics"
# 	"sfml-audio"
# 	"sfml-network"
# 	"sfml-system"
# 	"sfml-window")