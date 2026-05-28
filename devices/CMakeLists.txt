# Option to build all devices
OPTION(BUILD_ALL_DEVICES "Build all devices?" OFF)

# reset the saved libraries
SET(PEARY_DEVICE_LIBRARIES "" CACHE INTERNAL "Device libraries")

# Generate an interface library containing all devices:
ADD_LIBRARY(devices INTERFACE)

# Build all the modules
FILE(GLOB subdirs RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*)
FOREACH(subdir ${subdirs})
    IF(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${subdir})
        # Internal macros check if the device is actually enabled
        ADD_SUBDIRECTORY(${subdir})
    ENDIF()
ENDFOREACH()


INSTALL(TARGETS devices
    EXPORT Caribou
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

INSTALL(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/peary
    FILES_MATCHING
    PATTERN "*.hpp"
    PATTERN "*.h")
