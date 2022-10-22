if (NOT TARGET cxxtimer::cxxtimer)
	add_library(cxxtimer::cxxtimer INTERFACE IMPORTED)
	set_target_properties(cxxtimer::cxxtimer PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}")
endif()
