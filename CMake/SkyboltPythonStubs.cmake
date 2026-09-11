# Adds a 'SkyboltPythonStubs' target that generates .pyi stub files for a compiled
# Python C extension module (e.g. built with pybind11) using Tools/BuildScripts/stubs.py.
function(skybolt_add_python_stubs_target)
	cmake_parse_arguments(ARG "" "TARGET;MODULE_NAME;OUTPUT_FILE" "EXTRA_MODULE_SEARCH_DIRS" ${ARGN})

	if (NOT ARG_TARGET)
		message(FATAL_ERROR "skybolt_add_python_stubs_target: missing required argument TARGET")
	endif()
	if (NOT ARG_MODULE_NAME)
		set(ARG_MODULE_NAME "${ARG_TARGET}")
	endif()
	if (NOT ARG_OUTPUT_FILE)
		set(ARG_OUTPUT_FILE "$<TARGET_FILE_DIR:${ARG_TARGET}>/${ARG_MODULE_NAME}.pyi")
	endif()
	set (_module_search_dirs "$<TARGET_FILE_DIR:${ARG_TARGET}>")
	if (ARG_EXTRA_MODULE_SEARCH_DIRS)
		list(APPEND _module_search_dirs ${ARG_EXTRA_MODULE_SEARCH_DIRS})
	endif()
	
	set(_target_name ${ARG_TARGET}_PythonStubs)

	find_package(Python3 COMPONENTS Interpreter QUIET)
	if (NOT Python3_Interpreter_FOUND)
		message(WARNING "Python3 interpreter not found. '${_target_name}' target will not be available.")
		return()
	endif()

	set(_script "${Skybolt_BUILD_SCRIPTS_DIR}/stubs.py")
	if (NOT EXISTS "${_script}")
		message(fatal_ERROR "Stub generation script not found at '${_script}'. '${_target_name}' target will not be available.")
		return()
	endif()

	set(_search_dir_args)
	foreach(_dir IN LISTS _module_search_dirs)
		list(APPEND _search_dir_args --module-search-dir "${_dir}")
	endforeach()

	add_custom_target(${_target_name}
		COMMAND ${Python3_EXECUTABLE} "${_script}"
			"${ARG_MODULE_NAME}"
			"${ARG_OUTPUT_FILE}"
			${_search_dir_args}
		COMMENT "Generating Python stubs for module '${ARG_MODULE_NAME}' into ${ARG_OUTPUT_FILE}"
		VERBATIM
	)
	add_dependencies(${_target_name} ${ARG_TARGET})

	set_target_properties(${_target_name} PROPERTIES FOLDER "PythonLibs")
endfunction()
