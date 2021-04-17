add_executable(Engine "main.cpp" "array_wrapper.h" "window.h" "camera_controller.h" "transform.h" "duplex.h" "ray.h" "utility.h" "pixel.h"  "camera.h" "scheduler.h" "holder_or_void.h" "raytraceable.h" "world.h" "material.h" "framebuffer.h")
target_link_libraries(Engine PRIVATE glm)
target_link_libraries(Engine PRIVATE minifb)