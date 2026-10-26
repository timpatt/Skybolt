cmake_minimum_required (VERSION 3.26.0)

project(Skybolt)

include(CTest)
include(GNUInstallDirs)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

list(APPEND CMAKE_MODULE_PATH
	"${CMAKE_SOURCE_DIR}/CMake/"
	"${CMAKE_SOURCE_DIR}/CMake/Modules/"
	"${CMAKE_BINARY_DIR}"
)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(Skybolt_BUILD_SCRIPTS_DIR "${CMAKE_SOURCE_DIR}/Tools/BuildScripts")

#-------------------------------------------------------------
# Compiler flags
#-------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

OPTION(DISABLE_OPTIMIZATION_RELWITHDEBINFO "Disable Optimization in RelWithDebInfo")
if(DISABLE_OPTIMIZATION_RELWITHDEBINFO)
	if(WIN32)
	set(OPT_OFF Od)
	else()
	set(OPT_OFF O0)
	endif()

	set(CMAKE_C_FLAGS_RELWITHDEBINFO "-${OPT_OFF} -Zi")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-${OPT_OFF} -Zi")
endif()

if ( MSVC )
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /we4715") # Treat missing return as error
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd5033") # disable warnings about use of 'register' in glm
	add_definitions(-D_SILENCE_CXX17_OLD_ALLOCATOR_MEMBERS_DEPRECATION_WARNING) # disable deprecated member warnings from boost
	add_definitions(-D_CRT_SECURE_NO_WARNINGS) # disable warnings about 'unsafe' functions
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-register") # disable warnings about use of 'register' in glm
endif()

if(Skybolt_VERSION)
	add_definitions(-DSKYBOLT_VERSION=${Skybolt_VERSION})
endif()

#-------------------------------------------------------------
# Dependency options
#-------------------------------------------------------------
OPTION(Boost_STATIC_LIBS "Boost use static libs")
if (Boost_STATIC_LIBS)
	set(Boost_USE_STATIC_LIBS ON)
endif()

OPTION(OSG_STATIC_LIBS "OSG use static libs")
if (OSG_STATIC_LIBS)
	add_definitions(-DOSG_LIBRARY_STATIC)
endif()

#-------------------------------------------------------------
# Build options
#-------------------------------------------------------------
OPTION(SKYBOLT_PLUGINS_STATIC_BUILD "Build skybolt plugins as static libraries" OFF)
if (SKYBOLT_PLUGINS_STATIC_BUILD)
	set (SKYBOLT_PLUGINS_LIBRARY_TYPE STATIC)
else()
	set (SKYBOLT_PLUGINS_LIBRARY_TYPE SHARED)
endif()

OPTION(BUILD_WITH_OSG "Use OpenSceneGraph" ON)
if (BUILD_WITH_OSG)
	add_definitions(-DBUILD_WITH_OSG)
endif()

OPTION(BUILD_PYTHON_BINDINGS "Build Python Bindings")

# This option is provided to allow the application to opt-out of using boost as the logging backend,
# in cases where we want to minimize the number of dependencies. If this option is disabled, the application will
# fall back to a simple logging backend that writes to stdout.
OPTION(BUILD_WITH_BOOST_LOG "Use Boost as the logging backend" ON)
if (BUILD_WITH_BOOST_LOG)
	add_definitions(-DUSE_BOOST_LOG)
endif()

#-------------------------------------------------------------
# Build tree
#-------------------------------------------------------------
include(AddSourceGroup)
include(SetPluginTargetProperties)
if(BUILD_TESTING)
	find_package(Catch2 3 REQUIRED)
	include(Catch)
endif()
include(SkyboltInstall)
include(SkyboltPythonStubs)

add_subdirectory(src/Skybolt)

option(SKYBOLT_BUILD_EXAMPLES "Build Skybolt Examples" ON)
if (SKYBOLT_BUILD_EXAMPLES AND BUILD_WITH_OSG)
	add_subdirectory(src/SkyboltExamples)
endif()

#-------------------------------------------------------------
# Set package layout variables, used by documentation generation and downstream conan packages
#-------------------------------------------------------------
set(_runtime_lib_path ${CMAKE_INSTALL_LIBDIR})
set(_runtime_bin_path ${CMAKE_INSTALL_BINDIR})
# No config suffix on Linux
set(_runtime_lib_path_with_config ${CMAKE_INSTALL_LIBDIR})
set(_runtime_bin_path_with_config ${CMAKE_INSTALL_BINDIR})
if (WIN32)
	set(_runtime_lib_path ${CMAKE_INSTALL_BINDIR})
	set(_runtime_bin_path ${CMAKE_INSTALL_BINDIR})
	set(_runtime_lib_path_with_config ${CMAKE_INSTALL_BINDIR}/$<CONFIG>)
	set(_runtime_bin_path_with_config ${CMAKE_INSTALL_BINDIR}/$<CONFIG>)
endif()

set(Skybolt_ASSETS_DIR "${CMAKE_SOURCE_DIR}/Assets")
set(Skybolt_BIN_DIR "${CMAKE_BINARY_DIR}/${_runtime_bin_path_with_config}")
set(Skybolt_DOCS_DIR "${CMAKE_BINARY_DIR}/docs")
set(Skybolt_PLUGIN_DIRS "${CMAKE_BINARY_DIR}/${_runtime_lib_path_with_config}/plugins")
set(Skybolt_PYTHON_MODULE_DIR "${CMAKE_BINARY_DIR}/${_runtime_lib_path_with_config}")

function(_skybolt_configure_conan_init_script)
	# TODO: Document this
	configure_file(
		"CMake/conan-skybolt-init.cmake.in" 
		"${CMAKE_BINARY_DIR}/conan-skybolt-init-buildtree.cmake"
		@ONLY
	)	

	set(Skybolt_ASSETS_DIR "\${CMAKE_CURRENT_LIST_DIR}/../Assets")
	set(Skybolt_BIN_DIR "\${CMAKE_CURRENT_LIST_DIR}/../${_runtime_bin_path}")
	set(Skybolt_BUILD_SCRIPTS_DIR "\${CMAKE_CURRENT_LIST_DIR}/../Tools/BuildScripts")
	set(Skybolt_DOCS_DIR "\${CMAKE_CURRENT_LIST_DIR}/../docs")
	set(Skybolt_PLUGIN_DIRS "\${CMAKE_CURRENT_LIST_DIR}/../${_runtime_lib_path}/plugins")
	set(Skybolt_PYTHON_MODULE_DIR "\${CMAKE_CURRENT_LIST_DIR}/../${_runtime_lib_path}")
	configure_file("CMake/conan-skybolt-init.cmake.in" "${CMAKE_BINARY_DIR}/conan-skybolt-init-installtree.cmake" @ONLY)
	install(FILES ${CMAKE_BINARY_DIR}/conan-skybolt-init-installtree.cmake
		DESTINATION CMake
		COMPONENT Development
		EXCLUDE_FROM_ALL
	)
endfunction()

_skybolt_configure_conan_init_script()

#-------------------------------------------------------------
# Documentation
#-------------------------------------------------------------
OPTION(SKYBOLT_BUILD_DOCS "Add a 'docs' target for generating documentation with MkDocs" OFF)

if (SKYBOLT_BUILD_DOCS)
	include(SkyboltDocs)
	skybolt_add_docs_target()
	add_dependencies(Docs SkyboltApp)
	add_dependencies(Docs skybolt_PythonStubs)
endif()

#-------------------------------------------------------------
# Installation
#-------------------------------------------------------------
# Install CMake scripts
install(DIRECTORY ${CMAKE_SOURCE_DIR}/CMake	
	DESTINATION .
	COMPONENT Development
	EXCLUDE_FROM_ALL
)

# Install headers
install(
	DIRECTORY ${CMAKE_SOURCE_DIR}/src/Skybolt/
	DESTINATION include
	COMPONENT Development
	EXCLUDE_FROM_ALL
	FILES_MATCHING PATTERN "*.h*"
)

# Install Build scripts
install(DIRECTORY ${CMAKE_SOURCE_DIR}/Tools/BuildScripts
	DESTINATION .
	COMPONENT Development
	EXCLUDE_FROM_ALL
)

# Install runtime dependencies
install(RUNTIME_DEPENDENCY_SET SkyboltDependencies
	PRE_EXCLUDE_REGEXES
		[=[api-ms-]=] # VC Redistibutable DLLs
		[=[ext-ms-]=] # Windows extension DLLs
		[[kernel32\.dll]]
		[[libc\.so\..*]] [[libgcc_s\.so\..*]] [[libm\.so\..*]] [[libstdc\+\+\.so\..*]]
	POST_EXCLUDE_REGEXES
		[=[.*system32\/.*\.dll]=] # Windows system DLLs
		[=[^\/(lib|usr\/lib|usr\/local\/lib)]=] # Unix system libraries
	DIRECTORIES ${CONAN_RUNTIME_LIB_DIRS}
	RUNTIME 
		COMPONENT SkyboltDependencies
		EXCLUDE_FROM_ALL
	LIBRARY
		COMPONENT SkyboltDependencies
		EXCLUDE_FROM_ALL
)

# Install assets
install(DIRECTORY ${CMAKE_SOURCE_DIR}/Assets	
	DESTINATION .
	COMPONENT Assets
	EXCLUDE_FROM_ALL
)

# Install docs
if (SKYBOLT_BUILD_DOCS)
	install(DIRECTORY ${CMAKE_BINARY_DIR}/docs/
		DESTINATION Docs
		COMPONENT Docs
		EXCLUDE_FROM_ALL
	)
endif()

install(FILES ${CMAKE_SOURCE_DIR}/License.txt ${CMAKE_SOURCE_DIR}/README.md
	DESTINATION .
	COMPONENT Runtime
)