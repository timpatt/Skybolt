# This module defines
#  muFFT_FOUND, if false, do not try to link
#  muFFT_LIBRARIES, libraries to link against
#  muFFT_INCLUDE_DIR, where to find headers

if(TARGET muFFT)
	set(muFFT_TARGET_FOUND TRUE)

	get_target_property(muFFT_INCLUDE_DIR muFFT INCLUDE_DIRECTORIES)

	add_library(muFFT::muFFT ALIAS muFFT)
	set(muFFT_LIBRARIES muFFT::muFFT)

	list(APPEND muFFT_FIND_COMPONENTS sse sse3 avx)
	list(REMOVE_DUPLICATES muFFT_FIND_COMPONENTS)
	foreach(_component ${muFFT_FIND_COMPONENTS})
		if (TARGET "muFFT-${_component}")
			add_library(muFFT::${_component} ALIAS "muFFT-${_component}")
			list(APPEND muFFT_LIBRARIES "muFFT::${_component}")
			set(muFFT_${_component}_FOUND TRUE)
		endif()
	endforeach()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(muFFT
	REQUIRED_VARS muFFT_TARGET_FOUND
	HANDLE_COMPONENTS
)
