set(CompileSrc
${CMAKE_HOME_DIRECTORY}/src/acceleration_structure/grid/Container.cpp
${CMAKE_HOME_DIRECTORY}/src/acceleration_structure/grid/Geometry.cpp
${CMAKE_HOME_DIRECTORY}/src/acceleration_structure/grid/Grid.cpp
${CMAKE_HOME_DIRECTORY}/src/acceleration_structure/grid/Tools.cpp
main.cpp
)

add_executable(ray_triangle_intersect ${CompileSrc})
include_directories(${CMAKE_HOME_DIRECTORY}/src/acceleration_structure/grid)

