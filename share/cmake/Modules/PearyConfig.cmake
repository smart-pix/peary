
####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was PearyConfig.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)

macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
  endif()
endmacro()

macro(check_required_components _NAME)
  foreach(comp ${${_NAME}_FIND_COMPONENTS})
    if(NOT ${_NAME}_${comp}_FOUND)
      if(${_NAME}_FIND_REQUIRED_${comp})
        set(${_NAME}_FOUND FALSE)
      endif()
    endif()
  endforeach()
endmacro()

####################################################################################

SET_AND_CHECK(PEARY_INCLUDE_DIR "${PACKAGE_PREFIX_DIR}/include")
SET_AND_CHECK(PEARY_LIBRARY_DIR "${PACKAGE_PREFIX_DIR}/lib64")

INCLUDE("${CMAKE_CURRENT_LIST_DIR}/PearyConfigTargets.cmake")

SET(PEARY_DEVICE_EXTERNAL TRUE)
INCLUDE("${CMAKE_CURRENT_LIST_DIR}/PearyMacros.cmake")

# Check for presence of required components:
foreach(_comp ${Peary_FIND_COMPONENTS})
  find_library(Peary_${_comp}_LIBRARY ${_comp} HINTS ${PEARY_LIBRARY_DIR})
  if(Peary_${_comp}_LIBRARY)
    mark_as_advanced(Peary_${_comp}_LIBRARY)
    list(APPEND Peary_LIBRARIES ${Peary_${_comp}_LIBRARY})
    SET(Peary_${_comp}_FOUND TRUE)
  endif()
endforeach()
if(Peary_LIBRARIES)
  list(REMOVE_DUPLICATES Peary_LIBRARIES)
endif()

CHECK_REQUIRED_COMPONENTS(Peary)
