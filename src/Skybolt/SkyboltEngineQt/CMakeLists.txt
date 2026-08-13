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

if (UNIX)
	# On Linux when using the X11 Qt backend, OsgWindow fails to acquire a GL context 
	# correctly and causes a crash.  This hack launches the GL view in a separate 
	# window (which looks crap, but works) to workaround the issue.
	target_compile_definitions(${TARGET_NAME} PRIVATE SKYBOLT_QT_OSG_WINDOW_HACK)
endif()

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

# TODO: On Linux platform/libxcb.so dynamically loads this file
file(GLOB _xcbFiles "${qt_PACKAGE_FOLDER_RELWITHDEBINFO}/lib/libQt6XcbQpa${CMAKE_SHARED_LIBRARY_SUFFIX}*")
if (_xcbFiles)
	install(
		FILES ${_xcbFiles}
		CONFIGURATIONS RelWithDebInfo
		DESTINATION lib
		COMPONENT QtPlugins
		EXCLUDE_FROM_ALL
	)
endif()
