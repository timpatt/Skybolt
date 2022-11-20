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

find_package(OpenSceneGraph MODULE REQUIRED COMPONENTS osg osgDB osgGA osgText osgUtil osgViewer)# ${OSG_STATIC_LINK_PLUGIN_NAMES})
include_directories(${OPENSCENEGRAPH_INCLUDE_DIRS})

find_package(OpenThreads)

find_package(cxxtimer REQUIRED)
find_package(earcut REQUIRED)
find_package(httplib REQUIRED)
find_package(px REQUIRED)

OPTION(SKYBOLT_USE_DELL_XPS_RTT_FIX "Workaround for render-to-texture driver bug on Dell XPS")
if (SKYBOLT_USE_DELL_XPS_RTT_FIX)
	add_definitions(-USE_DELL_XPS_RTT_FIX)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(LIBRARIES)

if (FALSE AND OSG_STATIC_LIBS)
	# Statically link to dependencies of OSG dependencies
	# (Surely OSG should already link these in if it requires them??)
	find_package(CURL REQUIRED)
	find_package(Freetype REQUIRED)
	find_package(JPEG REQUIRED)
	find_package(OpenGL REQUIRED)

	list(APPEND LIBRARIES
		${CMAKE_DL_LIBS}
		${CURL_LIBRARIES}
		${Freetype_LIBRARIES}
		${JPEG_LIBRARIES}
		${OPENGL_LIBRARIES}
	)
endif()

add_library(${LIB_NAME} STATIC ${SOURCE_FILES} ${HEADER_FILES})

target_include_directories(${LIB_NAME} PUBLIC ${OPENSCENEGRAPH_INCLUDE_DIRS})
#message("OSG_STATIC_LIBS = ${OSG_STATIC_LIBS}")
#message(FATAL_ERROR "OPENSCENEGRAPH_LIBRARIES= ${OPENSCENEGRAPH_LIBRARIES}")
target_link_libraries(${LIB_NAME}
	SkyboltCommon
	SkyboltSim
	MapAttributesConverter
	Boost::log
	${OPENSCENEGRAPH_LIBRARIES}
	${OPENTHREADS_LIBRARIES}
	${muFFT_LIBRARIES}
	cxxtimer::cxxtimer
	earcut::earcut
	httplib::httplib
	px::px
)

skybolt_install(${LIB_NAME})