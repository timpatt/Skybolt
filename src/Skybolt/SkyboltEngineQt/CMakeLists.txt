set(TARGET_NAME SkyboltEngineQt)
add_source_group_tree(. SrcFiles)

FILE(GLOB UiSrc "*.ui")

include_directories("../")
include_directories("../../Skybolt")

find_package(skybolt-widgets)

# Qt
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
find_package(Qt6 COMPONENTS Core Gui Widgets OpenGL REQUIRED)

QT6_ADD_RESOURCES(SrcRccsSrc ${SrcRccs})

QT6_WRAP_UI(SrcUiHeaders ${UiSrc})
SET(SrcFiles ${SrcFiles} ${SrcUiHeaders} ${SrcRccsSrc})
SOURCE_GROUP("Generated Ui Files" FILES ${SrcUiHeaders} ${SrcRccsSrc})

set(LIBS
	qt::qt
	SkyboltEngine
	skybolt-widgets::skybolt-widgets
)

add_library(${TARGET_NAME} STATIC ${SrcFiles})

target_link_libraries (${TARGET_NAME} ${LIBS})

skybolt_install(${TARGET_NAME})

# TODO: If only we produced an executable!?
#qt_generate_deploy_app_script(
#    TARGET ${TARGET_NAME}_qt_app
#    OUTPUT_SCRIPT skybolt_install_qt_plugins_script
#	NO_COMPILER_RUNTIME # Don't install compiler runtime; we do this elsewhere
#	VERBOSE
#)
#install(SCRIPT "${skybolt_install_qt_plugins_script}" COMPONENT QtPlugins EXCLUDE_FROM_ALL)

# TODO: Remove this?
# Install Qt plugins, which are not tracked by CMake runtime dependency system
function(install_qt_plugins plugin_folder_name)

	# TODO: Only works with Conan


	# TODO: Get plugin dependencies and install them
	install(
		DIRECTORY ${qt_PACKAGE_FOLDER_RELEASE}/plugins/${plugin_folder_name}
		CONFIGURATIONS Release
		DESTINATION bin/qtPlugins
		COMPONENT QtPlugins
		EXCLUDE_FROM_ALL
		FILES_MATCHING PATTERN "*"
	)
	install(
		DIRECTORY ${qt_PACKAGE_FOLDER_RELWITHDEBINFO}/plugins/${plugin_folder_name}
		CONFIGURATIONS RelWithDebInfo
		DESTINATION bin/qtPlugins
		COMPONENT QtPlugins
		EXCLUDE_FROM_ALL
		FILES_MATCHING PATTERN "*"
	)

	# TODO: This didn't do anything!?
	# file(GLOB_RECURSE _plugins
	# 	LIST_DIRECTORIES TRUE
	# 	"${qt_PACKAGE_FOLDER_RELWITHDEBINFO}/plugins/${plugin_folder_name}/*${CMAKE_SHARED_LIBRARY_SUFFIX}"
	# )
	# install(
	# 	FILES ${_plugins}
	# 	CONFIGURATIONS RelWithDebInfo
	# 	DESTINATION bin/qtPlugins/${plugin_folder_name}
	# 	COMPONENT QtPlugins
	# 	EXCLUDE_FROM_ALL
	# )
	# string(CONCAT _getRuntimeDepsCommand [=[
	# 	file(GET_RUNTIME_DEPENDENCIES
	# 		LIBRARIES ]=]
	# 		"${_plugins}\n"
	# 	[=[
	# 		RESOLVED_DEPENDENCIES_VAR resolved
	# 		UNRESOLVED_DEPENDENCIES_VAR unresolved
	# 		CONFLICTING_DEPENDENCIES_PREFIX conflicting
	# 		PRE_EXCLUDE_REGEXES
	# 			[[api-ms-]] # VC Redistibutable DLLs
	# 			[[ext-ms-]] # Windows extension DLLs
	# 			[[kernel32\.dll]]
	# 			[[libc\.so\..*]] [[libgcc_s\.so\..*]] [[libm\.so\..*]] [[libstdc\+\+\.so\..*]]
	# 		POST_EXCLUDE_REGEXES
	# 			[[.*system32\/.*\.dll]] # Windows system DLLs
	# 			[[^\/(lib|usr\/lib|usr\/local\/lib)]] # Unix system libraries
	# 		DIRECTORIES
	# 	]=] 
	# 			"${CONAN_RUNTIME_LIB_DIRS}\n"
	# 	[=[
	# 	)
    #     if(1)
    #         message("file(GET_RUNTIME_DEPENDENCIES ]=] "${_plugins}" [=[ ...)")
    #         foreach(_file IN LISTS resolved)
    #             message("    resolved: ${_file}")
    #         endforeach()
    #         foreach(_file IN LISTS unresolved)
    #             message("    unresolved: ${_file}")
    #         endforeach()
    #         foreach(_file IN LISTS conflicting_FILENAMES)
    #             message("    conflicting: ${_file}")
    #             message("    with ${conflicting_${_file}}")
    #         endforeach()
    #     endif()
	# 	]=]
	# )
	# #message(FATAL_ERROR "_getRuntimeDepsCommand = ${_getRuntimeDepsCommand}")
	# install(CODE "${_getRuntimeDepsCommand}"
	# 	COMPONENT QtPlugins
	# 	EXCLUDE_FROM_ALL
	# )

	install(
		DIRECTORY ${qt_PACKAGE_FOLDER_MINSIZEREL}/plugins/${plugin_folder_name}
		CONFIGURATIONS MinSizeRel
		DESTINATION bin/qtPlugins
		COMPONENT QtPlugins
		EXCLUDE_FROM_ALL
		FILES_MATCHING PATTERN "*"
	)
	install(
		DIRECTORY ${qt_PACKAGE_FOLDER_DEBUG}/plugins/${plugin_folder_name}
		CONFIGURATIONS Debug
		DESTINATION bin/qtPlugins
		COMPONENT QtPlugins
		EXCLUDE_FROM_ALL
		FILES_MATCHING PATTERN "*"
	)
endfunction()

install_qt_plugins(generic)
install_qt_plugins(imageformats)
install_qt_plugins(platforms)
