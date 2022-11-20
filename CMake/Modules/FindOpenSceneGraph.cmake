find_package(OpenSceneGraph CONFIG)

if (OPENSCENEGRAPH_FOUND OR OpenSceneGraph_FOUND)
	if (NOT DEFINED OPENSCENEGRAPH_INCLUDE_DIRS)
		if (DEFINED openscenegraph_INCLUDE_DIRS)
			# Conan defaults to using lowercase - convert it to the case used in the "standard" CMake find_package module
			set(OPENSCENEGRAPH_FOUND TRUE)
			set(OPENSCENEGRAPH_VERSION  "${openscenegraph_VERSION_STRING}")
			set(OPENSCENEGRAPH_INCLUDE_DIRS "${openscenegraph_INCLUDE_DIRS}")
			set(OPENSCENEGRAPH_LIBRARIES "${openscenegraph_LIBRARIES}")
			set(OPENSCENEGRAPH_DEFINITIONS "${openscenegraph_DEFINITIONS}")
		else()
			message(FATAL_ERROR "OpenSceneGraph found, but cannot find expected variables")
		endif()
	endif()
else()
	# Fallback to default find module
	include("${CMAKE_ROOT}/Modules/FindOpenSceneGraph.cmake")
endif()