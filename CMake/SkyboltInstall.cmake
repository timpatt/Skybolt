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
		LIBRARY DESTINATION lib COMPONENT Runtime
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