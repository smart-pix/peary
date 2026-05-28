########################################################
# CMake file for the peary caribou DAQ
CMAKE_MINIMUM_REQUIRED(VERSION 3.5 FATAL_ERROR)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW) # change linker path search behaviour
  CMAKE_POLICY(SET CMP0048 NEW) # set project version
  IF(${CMAKE_VERSION} VERSION_GREATER "3.13")
    CMAKE_POLICY(SET CMP0079 NEW) # Allow lookup of linking libraries in other directories
  ENDIF()
ENDIF(COMMAND CMAKE_POLICY)
########################################################

# Set default version
SET(PEARY_VERSION "v0.13.0")

# Set default build type
IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)

# Overwrite with the version from git if found
INCLUDE("cmake/tools.cmake")
GET_VERSION(PEARY_VERSION)

# Print version
MESSAGE(STATUS "Building Peary version ${PEARY_VERSION}")
MESSAGE(STATUS "Building \"${CMAKE_BUILD_TYPE}\"")

# Gather information about build time:
STRING(TIMESTAMP BUILD_TIME "%Y-%m-%d, %H:%M:%S UTC" UTC)

# Define the project with the simple version
STRING(REGEX MATCH "([0-9.]+)+" SIMPLE_VERSION "${PEARY_VERSION}")
# Set languages to NONE to allow the documentation to be built without CXX compiler:
PROJECT(peary VERSION ${SIMPLE_VERSION} LANGUAGES CXX)
# Access the project name (for install locations) in the source
ADD_DEFINITIONS(-DPEARY_PROJECT_NAME="${CMAKE_PROJECT_NAME}" -DPEARY_PROJECT_VERSION="${PEARY_VERSION}" -DPEARY_BUILD_TIME="${BUILD_TIME}")

# Include a generated configuration file
# FIXME: this should be combined with the ADD_DEFINITIONS
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.cmake.h" "${CMAKE_CURRENT_BINARY_DIR}/config.h" @ONLY)
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}")

###############################
# Setup the build environment #
###############################

# lock dir option must be available globally
SET(PEARY_LOCK_DIR "/var/lock" CACHE PATH "Path to lockfile directory")

# Include peary cmake functions
INCLUDE("cmake/PearyMacros.cmake")

# Additional packages to be searched for by cmake
LIST(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# Support deprecated INSTALL_PREFIX option
SET(INSTALL_PREFIX "" CACHE PATH "Prefix prepended to install directories (deprecated)")
MARK_AS_ADVANCED(INSTALL_PREFIX)
IF(NOT ${INSTALL_PREFIX} STREQUAL "")
  MESSAGE(DEPRECATION "Use CMAKE_INSTALL_PREFIX instead of INSTALL_PREFIX")
  SET(CMAKE_INSTALL_PREFIX "${INSTALL_PREFIX}" CACHE PATH "Install path prefix, prepended onto install directories." FORCE)
  SET(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT FALSE)
ENDIF()

# Configure the installation prefix to allow system-wide installation
IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  SET(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}" CACHE PATH "Install path prefix, prepended onto install directories." FORCE)
ENDIF()

# Get default lib/include dirs
INCLUDE("GNUInstallDirs")

# Set up the RPATH so executables find the libraries even when installed in non-default location
SET(CMAKE_MACOSX_RPATH 1)
SET(CMAKE_SKIP_BUILD_RPATH  FALSE)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")

# Add the automatically determined parts of the RPATH which point to directories outside the build tree to the install RPATH
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# the RPATH to be used when installing, but only if it's not a system directory
LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" isSystemDir)
IF("${isSystemDir}" STREQUAL "-1")
   SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
ENDIF("${isSystemDir}" STREQUAL "-1")


#################################
# Define build flags for peary  #
#################################

# Set standard build flags
SET(COMPILER_FLAGS
    -Wall
    -Wextra
    -Wcast-align
    -Wcast-qual
    -Wconversion
    -Wuseless-cast
    -Wctor-dtor-privacy
    -Wzero-as-null-pointer-constant
    -Wdisabled-optimization
    -Wformat=2
    -Winit-self
    -Wlogical-op
    -Wmissing-declarations
    -Wmissing-include-dirs
    -Wnoexcept
    -Wold-style-cast
    -Wredundant-decls
    -Wsign-conversion
    -Wsign-promo
    -Wstrict-null-sentinel
    -Wswitch-default
    -Wundef
    -Wshadow
    -Wformat-security
    -Wdeprecated
    -fdiagnostics-color=auto
    -Wheader-hygiene)

INCLUDE("cmake/compiler-flag-checks.cmake")

SET(CMAKE_CXX_STANDARD 17)
INCLUDE(cmake/Platform.cmake)

# Check if compiler version supports all features:
INCLUDE("cmake/compiler-version-checks.cmake")

SET(CMAKE_THREAD_PREFER_PTHREAD TRUE)
FIND_PACKAGE(Threads REQUIRED)

###################################
# Load cpp format and check tools #
###################################

# Set the clang-format version required by the CI for correct formatting:
SET(CLANG_FORMAT_VERSION "14")
SET(CLANG_TIDY_VERSION "14")

# Set the source files to clang-format
FILE(GLOB_RECURSE
     CHECK_CXX_SOURCE_FILES
     devices/*.[ch]pp devices/*.[ht]cc exec/*.[ch]pp exec/*.[ht]cc peary/*.[ch]pp peary/*.[ht]cc
     )
# Exclude some paths
SET(FORMAT_EXCLUDE_FILES "peary/interfaces/Media/linux/")

FOREACH(f ${CHECK_CXX_SOURCE_FILES})
  FOREACH(e ${FORMAT_EXCLUDE_FILES})
    IF(f MATCHES e)
      LIST(REMOVE_ITEM CHECK_CXX_SOURCE_FILES f)
    ENDIF()
  ENDFOREACH()
ENDFOREACH()

INCLUDE("cmake/clang-cpp-checks.cmake")


##################################
# Define build targets for peary #
##################################

# Always build main peary library;
INCLUDE_DIRECTORIES(peary)
ADD_SUBDIRECTORY(peary)
SET(PEARY_LIBRARIES ${PEARY_LIBRARIES} peary)

# Include individual device libraries:
INCLUDE_DIRECTORIES(devices)
ADD_SUBDIRECTORY(devices)

# Add targets for Doxygen code reference and LaTeX User manual
ADD_SUBDIRECTORY(doc)

################################################
# Define build targets for sample applications #
################################################
OPTION(BUILD_EXE "Build the peary executables" ON)
IF(BUILD_EXE)
	ADD_SUBDIRECTORY(exec)
ENDIF()
ADD_SUBDIRECTORY(etc) # pearyd service files

#######################################
# Generate CMake Configuration Module #
#######################################

INCLUDE(CMakePackageConfigHelpers)
SET(PEARY_INCLUDE_DIR "${CMAKE_INSTALL_INCLUDEDIR}")
SET(PEARY_LIBRARY_DIR "${CMAKE_INSTALL_LIBDIR}")
SET(PEARY_CMAKE_DIR "share/cmake/Modules")
CONFIGURE_PACKAGE_CONFIG_FILE(${CMAKE_CURRENT_SOURCE_DIR}/cmake/PearyConfig.cmake.in
                              ${CMAKE_CURRENT_BINARY_DIR}/PearyConfig.cmake
                              INSTALL_DESTINATION ${PEARY_CMAKE_DIR}
                              PATH_VARS PEARY_INCLUDE_DIR PEARY_LIBRARY_DIR)

WRITE_BASIC_PACKAGE_VERSION_FILE(PearyConfigVersion.cmake
                                 VERSION ${PROJECT_VERSION}
                                 COMPATIBILITY AnyNewerVersion)
INSTALL(FILES
    ${PROJECT_BINARY_DIR}/PearyConfig.cmake
    ${PROJECT_BINARY_DIR}/PearyConfigVersion.cmake
    ${PROJECT_SOURCE_DIR}/cmake/PearyMacros.cmake
    DESTINATION ${PEARY_CMAKE_DIR})

INSTALL(EXPORT Caribou
    NAMESPACE Caribou::
    FILE PearyConfigTargets.cmake
    DESTINATION ${PEARY_CMAKE_DIR})
