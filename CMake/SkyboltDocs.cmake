# Adds a 'docs' target that generates project documentation using a python script.
function(skybolt_add_docs_target)
	cmake_parse_arguments(ARG "DEPLOY_TO_GITHUB" "SOURCE_DIR;ASSETS_DIR;BINARY_DIR;OUTPUT_DIR" "PYTHON_MODULE_DIRS;PLUGIN_DIRS" ${ARGN})

	if (NOT ARG_SOURCE_DIR)
		set(ARG_SOURCE_DIR "${CMAKE_SOURCE_DIR}")
	endif()
	if (NOT ARG_ASSETS_DIR)
		set(ARG_ASSETS_DIR "${Skybolt_ASSETS_DIR}")
	endif()
	if (NOT ARG_PYTHON_MODULE_DIRS)
		set(ARG_PYTHON_MODULE_DIRS "${Skybolt_PYTHON_MODULE_DIR}")
	endif()
	if (NOT ARG_BINARY_DIR)
		set(ARG_BINARY_DIR "${Skybolt_BIN_DIR}")
	endif()
	if (NOT ARG_PLUGIN_DIRS)
		set(ARG_PLUGIN_DIRS "${Skybolt_PLUGIN_DIRS}")
	endif()
	if (NOT ARG_OUTPUT_DIR)
		set(ARG_OUTPUT_DIR "${Skybolt_DOCS_DIR}")
	endif()

	find_package(Python3 COMPONENTS Interpreter QUIET)
	if (NOT Python3_Interpreter_FOUND)
		message(FATAL_ERROR "Python3 interpreter not found. Cannot build docs.")
	endif()

	set(_script "${Skybolt_BUILD_SCRIPTS_DIR}/docs/build_docs.py")
	if (NOT EXISTS "${_script}")
		message(FATAL_ERROR "Docs build script not found at '${_script}'.")
	endif()

	set(_deploy_arg)
	if (ARG_DEPLOY_TO_GITHUB)
		set(_deploy_arg --deploy-to-github)
	endif()

	add_custom_target(Docs
		COMMAND ${Python3_EXECUTABLE} "${_script}"
			--skybolt-source-dir "${ARG_SOURCE_DIR}"
			--skybolt-assets-dir "${ARG_ASSETS_DIR}"
			--skybolt-python-module-dirs ${ARG_PYTHON_MODULE_DIRS} # Should not be un quotes so that list is parsed correctly
			--skybolt-binary-dir ${ARG_BINARY_DIR}
			--skybolt-plugin-dirs ${ARG_PLUGIN_DIRS}
			--output-dir "${ARG_OUTPUT_DIR}"
			${_deploy_arg}
		COMMENT "Generating documentation with MkDocs into ${ARG_OUTPUT_DIR}"
		VERBATIM
	)

	set_target_properties(Docs PROPERTIES FOLDER "Docs")
endfunction()
