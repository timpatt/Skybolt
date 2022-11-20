
add_source_group_tree(. SOURCE)

include_directories("..")

find_package(OpenSceneGraph MODULE REQUIRED)

add_library(MapAttributesConverter STATIC ${SOURCE})
target_include_directories(MapAttributesConverter PUBLIC ${OPENSCENEGRAPH_INCLUDE_DIRS})
target_link_libraries(MapAttributesConverter PUBLIC ${OPENSCENEGRAPH_LIBRARIES})

skybolt_install(MapAttributesConverter)