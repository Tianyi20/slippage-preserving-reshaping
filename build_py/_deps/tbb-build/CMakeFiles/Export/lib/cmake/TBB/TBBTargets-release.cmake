#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "TBB::tbb" for configuration "Release"
set_property(TARGET TBB::tbb APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(TBB::tbb PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libtbb.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS TBB::tbb )
list(APPEND _IMPORT_CHECK_FILES_FOR_TBB::tbb "${_IMPORT_PREFIX}/lib/libtbb.a" )

# Import target "TBB::tbbmalloc" for configuration "Release"
set_property(TARGET TBB::tbbmalloc APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(TBB::tbbmalloc PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libtbbmalloc.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS TBB::tbbmalloc )
list(APPEND _IMPORT_CHECK_FILES_FOR_TBB::tbbmalloc "${_IMPORT_PREFIX}/lib/libtbbmalloc.a" )

# Import target "TBB::tbbbind_2_5" for configuration "Release"
set_property(TARGET TBB::tbbbind_2_5 APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(TBB::tbbbind_2_5 PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libtbbbind_2_5.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS TBB::tbbbind_2_5 )
list(APPEND _IMPORT_CHECK_FILES_FOR_TBB::tbbbind_2_5 "${_IMPORT_PREFIX}/lib/libtbbbind_2_5.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
