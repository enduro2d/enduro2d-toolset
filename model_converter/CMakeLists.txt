#
# external 3rd party
#

set(ASSIMP_NO_EXPORT ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
add_subdirectory(modules/assimp)

#
# model2mesh executable
#

file(GLOB MODEL2MESH_SOURCES
    sources/model2mesh.cpp)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES
    ${MODEL2MESH_SOURCES})

add_executable(model2mesh
    ${MODEL2MESH_SOURCES})

target_link_libraries(model2mesh
    assimp)

set_target_properties(model2mesh PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO)

#
# model2shape executable
#

file(GLOB MODEL2SHAPE_SOURCES
    sources/model2shape.cpp)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES
    ${MODEL2SHAPE_SOURCES})

add_executable(model2shape
    ${MODEL2SHAPE_SOURCES})

target_link_libraries(model2shape
    assimp)

set_target_properties(model2shape PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO)
