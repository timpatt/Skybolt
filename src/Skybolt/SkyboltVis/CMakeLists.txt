set(LIB_NAME SkyboltVis)

add_source_group_tree(. SOURCE_FILES)

include_directories("../")

find_package(Boost COMPONENTS log REQUIRED)

if (OSG_STATIC_LIBS)
	# Names of plugins to statically link to
	# This should match the names defined by USE_OSGPLUGIN() in SkyboltVis/Window/Window.cpp
	set(OSG_STATIC_LINK_PLUGIN_NAMES
		osgdb_bmp
		osgdb_curl
		osgdb_dds
		osgdb_freetype
		osgdb_jpeg
		osgdb_osg
		osgdb_png
		osgdb_serializers_osg
		osgdb_tga
	)
endif()

find_package(OpenSceneGraph REQUIRED COMPONENTS osg osgDB osgGA osgText osgUtil osgViewer ${OSG_STATIC_LINK_PLUGIN_NAMES})
include_directories(${OSG_INCLUDE_DIR})

find_package(OpenThreads)

find_package(cxxtimer REQUIRED)
find_package(earcut_hpp REQUIRED)
find_package(httplib REQUIRED)
find_package(px_sched REQUIRED)

OPTION(SKYBOLT_USE_DELL_XPS_RTT_FIX "Workaround for render-to-texture driver bug on Dell XPS")
if (SKYBOLT_USE_DELL_XPS_RTT_FIX)
	add_definitions(-USE_DELL_XPS_RTT_FIX)
endif()

set(OSG_STATIC_LINK_PLUGIN_TARGET_NAMES ${OSG_STATIC_LINK_PLUGIN_NAMES})
list(TRANSFORM OSG_STATIC_LINK_PLUGIN_TARGET_NAMES PREPEND "openscenegraph-mr::")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(LIBRARIES
	SkyboltCommon
	SkyboltSim
	Boost::boost
	openscenegraph-mr::osg
	openscenegraph-mr::osgDB
	openscenegraph-mr::osgGA
	openscenegraph-mr::osgText
	openscenegraph-mr::osgUtil
	openscenegraph-mr::osgViewer
	${OSG_STATIC_LINK_PLUGIN_TARGET_NAMES}
	OpenThreads::OpenThreads
	${muFFT_LIBRARIES}
	cxxtimer::cxxtimer
	earcut_hpp::earcut_hpp
	httplib::httplib
	px_sched::px_sched
)

add_library(${LIB_NAME} STATIC ${SOURCE_FILES} ${HEADER_FILES})

if (OSG_STATIC_LIBS)
	# Statically link to dependencies of OSG dependencies
	find_package(Freetype REQUIRED)
	find_package(JPEG REQUIRED)
	find_package(OpenGL REQUIRED)

	list(APPEND LIBRARIES
		${CMAKE_DL_LIBS}
		Freetype::Freetype
		JPEG::JPEG
		${OPENGL_LIBRARIES}
	)

	# The OSG curl plugin gets statically linked when using OSG static libraries.
	# For some applications we might not want curl to be compiled into the executable,
	# so this option allows it to be disabled.
	OPTION(BUILD_WITH_OSG_CURL_PLUGIN "Build with OSG Curl plugin")

	if (BUILD_WITH_OSG_CURL_PLUGIN)
		find_package(CURL REQUIRED)
		list(APPEND LIBRARIES ${CURL_LIBRARIES})
		add_compile_definitions(${LIB_NAME} PRIVATE BUILD_WITH_OSG_CURL_PLUGIN)
	endif()
endif()

target_include_directories(${LIB_NAME} PUBLIC ${OSG_INCLUDE_DIR})
target_link_libraries(${LIB_NAME} ${LIBRARIES})

skybolt_install(${LIB_NAME})

# Install OSG plugins, which are not tracked by CMake runtime dependency system
cmake_path(GET OpenSceneGraph_INCLUDE_DIR PARENT_PATH PACKAGE_FOLDER) # Get OSG package folder

# TODO: Move this up?
include(GNUInstallDirs)
set(_runtime_lib_path ${CMAKE_INSTALL_LIBDIR})
if (WIN32)
	set(_runtime_lib_path ${CMAKE_INSTALL_BINDIR})
endif()
file(GLOB OSG_PLUGINS_DIR "${PACKAGE_FOLDER}/${_runtime_lib_path}/osgPlugins*")

if(OSG_PLUGINS_DIR)
	# TODO: Check OSG_PLUGINS_DIR holds a single path (not multiple!?)
	file(GLOB_RECURSE _plugins "${OSG_PLUGINS_DIR}/*${CMAKE_SHARED_LIBRARY_SUFFIX}")
	install(
		FILES ${_plugins}
#		CONFIGURATIONS RelWithDebInfo TODO???
		DESTINATION ${_runtime_lib_path}
		COMPONENT OsgPlugins
		EXCLUDE_FROM_ALL
	)

	# Not really the plugins; but the dependencies of the plugins!
	skybolt_install_deps(
		LIBRARIES "${_plugins}" 
		COMPONENT OsgPlugins 
		DESTINATION ${_runtime_lib_path}
		EXCLUDE_FROM_ALL
	)
endif()