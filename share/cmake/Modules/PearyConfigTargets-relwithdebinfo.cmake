#----------------------------------------------------------------
# Generated CMake target import file for configuration "RelWithDebInfo".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Caribou::peary" for configuration "RelWithDebInfo"
set_property(TARGET Caribou::peary APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(Caribou::peary PROPERTIES
  IMPORTED_LOCATION_RELWITHDEBINFO "${_IMPORT_PREFIX}/lib64/libpeary.so"
  IMPORTED_SONAME_RELWITHDEBINFO "libpeary.so"
  )

list(APPEND _cmake_import_check_targets Caribou::peary )
list(APPEND _cmake_import_check_files_for_Caribou::peary "${_IMPORT_PREFIX}/lib64/libpeary.so" )

# Import target "Caribou::PearyDeviceSpacelyCaribouBasic" for configuration "RelWithDebInfo"
set_property(TARGET Caribou::PearyDeviceSpacelyCaribouBasic APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(Caribou::PearyDeviceSpacelyCaribouBasic PROPERTIES
  IMPORTED_LOCATION_RELWITHDEBINFO "${_IMPORT_PREFIX}/lib64/libPearyDeviceSpacelyCaribouBasic.so"
  IMPORTED_SONAME_RELWITHDEBINFO "libPearyDeviceSpacelyCaribouBasic.so"
  )

list(APPEND _cmake_import_check_targets Caribou::PearyDeviceSpacelyCaribouBasic )
list(APPEND _cmake_import_check_files_for_Caribou::PearyDeviceSpacelyCaribouBasic "${_IMPORT_PREFIX}/lib64/libPearyDeviceSpacelyCaribouBasic.so" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
