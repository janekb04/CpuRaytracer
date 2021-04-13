add_custom_target(dependencies)

# GLM
add_library(glm INTERFACE)
target_include_directories(glm INTERFACE glm)
# MINIFB
add_subdirectory(minifb)