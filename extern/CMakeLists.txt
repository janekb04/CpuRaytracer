add_custom_target(dependencies)

# GLM
add_library(glm INTERFACE)
target_include_directories(glm INTERFACE glm)
# MINIFB
add_subdirectory(minifb)
# NFD-Ext
add_subdirectory(nativefiledialog-extended)
# STB
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE stb)
# Boxer
add_subdirectory(Boxer)