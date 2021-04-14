add_executable(Engine "main.cpp" "array_wrapper.h" "window.h" "camera_controller.h" "transform.h" "duplex.h" "ray.h" "utility.h" "pixel.h" "viewport.h" "camera.h")
target_link_libraries(Engine PRIVATE glm)
target_link_libraries(Engine PRIVATE minifb)