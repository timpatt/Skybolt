if (NOT TARGET px::px)
	add_library(px::px INTERFACE IMPORTED)
	set_target_properties(px::px PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/../../external/px-4799d7e")
endif()
