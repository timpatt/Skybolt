
add_source_group_tree(. SOURCE)

include_directories("../")

find_package(OpenSceneGraph REQUIRED)
include_directories(${OPENSCENEGRAPH_INCLUDE_DIRS})

add_library(MapAttributesConverter STATIC ${SOURCE})

skybolt_install(MapAttributesConverter)