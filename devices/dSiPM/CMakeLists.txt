PEARY_ENABLE_DEFAULT(ON)

# Define device and return the generated name as DEVICE_NAME
PEARY_DEVICE(DEVICE_NAME)

# Add source files to library
PEARY_DEVICE_SOURCES(${DEVICE_NAME}
  dSiPMDevice.cpp
  dSiPMFrameDecoder.cpp
)

# Provide standard install target
PEARY_DEVICE_INSTALL(${DEVICE_NAME})


OPTION(BUILD_dSiPM_DECODER "Build the dSiPM standalone ROOT converter" OFF)
IF(BUILD_dSiPM_DECODER)

  # ROOT is required for vector and persistency etc
  FIND_PACKAGE(ROOT REQUIRED COMPONENTS GenVector Geom Core Tree Hist RIO NO_MODULE)
  IF(NOT ROOT_FOUND)
    MESSAGE(FATAL_ERROR "Could not find ROOT, make sure to source the ROOT environment\n"
    "$ source YOUR_ROOT_DIR/bin/thisroot.sh")
  ENDIF()

  INCLUDE_DIRECTORIES(SYSTEM ${PEARY_DEPS_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS})

  # Create executable for dSiPMBinaryDecoder
  ADD_EXECUTABLE(dSiPMBinaryDecoder)
  TARGET_SOURCES(dSiPMBinaryDecoder PRIVATE dSiPMFrameDecoder.cpp dSiPMbinDecoder.cpp)
  TARGET_LINK_LIBRARIES(dSiPMBinaryDecoder ${PROJECT_NAME} ROOT::Core ROOT::GenVector ROOT::Geom ROOT::RIO ROOT::Hist ROOT::Tree)

  INSTALL(TARGETS dSiPMBinaryDecoder
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

ENDIF()
