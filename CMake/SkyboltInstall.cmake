macro(skybolt_install target)
	# Install target
	install(TARGETS ${target}
		EXPORT SkyboltTargets
		RUNTIME_DEPENDENCY_SET SkyboltDependencies # Adds runtime dependencies to SkyboltDependencies target
		LIBRARY DESTINATION lib COMPONENT Runtime
		ARCHIVE DESTINATION lib COMPONENT Development EXCLUDE_FROM_ALL
		RUNTIME DESTINATION bin COMPONENT Runtime
	)
endmacro()

macro(skybolt_plugin_install target)
	install(TARGETS ${target}
		EXPORT SkyboltTargets
		LIBRARY DESTINATION lib/plugins COMPONENT Runtime
		ARCHIVE DESTINATION lib COMPONENT Development EXCLUDE_FROM_ALL
		RUNTIME DESTINATION bin/plugins COMPONENT Runtime
	)
endmacro()

macro(skybolt_python_module_install target)
	install(TARGETS ${target}
		EXPORT SkyboltTargets
		LIBRARY DESTINATION lib COMPONENT Runtime
		ARCHIVE DESTINATION lib COMPONENT Development EXCLUDE_FROM_ALL
		RUNTIME DESTINATION bin COMPONENT Runtime
	)
endmacro()

function(skybolt_install_deps)
	cmake_parse_arguments(ARG "EXCLUDE_FROM_ALL" "COMPONENT;DESTINATION" "EXECUTABLES;LIBRARIES" ${ARGN})

	if (NOT CONAN_RUNTIME_LIB_DIRS)
		message(FATAL_ERROR "Function currently only works when used with the Conan CMakeDeps generator")
	endif()
	if (NOT ARG_DESTINATION)
		message(FATAL_ERROR "Missing required argument: 'DESTINATION'.")
	endif()
	
	set(_getDepArgs)
	if (NOT ARG_LIBRARIES AND NOT ARG_EXECUTABLES)
		message(FATAL_ERROR "Need to specify at least one of 'LIBRARIES' or 'EXECUTABLES'")
	endif()
	if (ARG_LIBRARIES)
		list(APPEND _getDepArgs " LIBRARIES ${ARG_LIBRARIES}\n")
	endif()
	if (ARG_EXECUTABLES)
		list(APPEND _getDepArgs " EXECUTABLES ${ARG_EXECUTABLES}\n")
	endif()

	string(CONCAT _getRuntimeDepsCommand [=[
		file(GET_RUNTIME_DEPENDENCIES
			]=] "${_getDepArgs}"
		[=[
			RESOLVED_DEPENDENCIES_VAR resolved
			UNRESOLVED_DEPENDENCIES_VAR unresolved
			CONFLICTING_DEPENDENCIES_PREFIX conflicting
			PRE_EXCLUDE_REGEXES
				[[api-ms-]] # VC Redistibutable DLLs
				[[ext-ms-]] # Windows extension DLLs
				[[kernel32\.dll]]
				[[libc\.so\..*]] [[libgcc_s\.so\..*]] [[libm\.so\..*]] [[libstdc\+\+\.so\..*]]
			POST_EXCLUDE_REGEXES
				[[.*system32\/.*\.dll]] # Windows system DLLs
				[[^\/(lib|usr\/lib|usr\/local\/lib)]] # Unix system libraries
			DIRECTORIES
		]=] 
				"${CONAN_RUNTIME_LIB_DIRS}\n"
		[=[
		)
        if(1)
            message("file(GET_RUNTIME_DEPENDENCIES ]=] "${_getDepArgs}" [=[ ...)")
            foreach(_file IN LISTS resolved)
                message("    resolved: ${_file}")
            endforeach()
            foreach(_file IN LISTS unresolved)
                message("    unresolved: ${_file}")
            endforeach()
            foreach(_file IN LISTS conflicting_FILENAMES)
                message("    conflicting: ${_file}")
                message("    with ${conflicting_${_file}}")
            endforeach()
        endif()

		file(INSTALL ${resolved}
			DESTINATION "${CMAKE_INSTALL_PREFIX}/]=] "${ARG_DESTINATION}" [=["
			FOLLOW_SYMLINK_CHAIN
		)
		]=]
	)
	#message(FATAL_ERROR "_getRuntimeDepsCommand = ${_getRuntimeDepsCommand}")

	set(_installArgs)
	if (ARG_COMPONENT)
		list(APPEND _installArgs COMPONENT ${ARG_COMPONENT})
	endif()
	if (ARG_EXCLUDE_FROM_ALL)
		list(APPEND _installArgs EXCLUDE_FROM_ALL)
	endif()

	install(CODE "${_getRuntimeDepsCommand}" ${_installArgs})
endfunction()