
add_source_group_tree(. SOURCE)

include_directories("../")

find_package(OpenSceneGraph REQUIRED)
add_library(MapAttributesConverter STATIC ${SOURCE})

skybolt_install(MapAttributesConverter)