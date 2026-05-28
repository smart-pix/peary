# Build flag for the Timepix3 telescope DAQ server
OPTION(BUILD_server "Build server for receiving commands from Run Control?" OFF)
IF(BUILD_server)
  # add target
  ADD_EXECUTABLE(pearysrv "pearysrv.cpp")
  ADD_EXECUTABLE(dummyclient "dummyclient.cpp")
  ADD_EXECUTABLE(dummysrv "dummysrv.cpp" "../peary/utils/log.cpp")
  TARGET_LINK_LIBRARIES(pearysrv ${PROJECT_NAME})
  INSTALL(TARGETS pearysrv dummyclient dummysrv
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
ENDIF(BUILD_server)

ADD_EXECUTABLE(peary_app "sample_application.cpp")
TARGET_LINK_LIBRARIES(peary_app ${PROJECT_NAME})

# Gather dependencies and build command line:
FIND_PACKAGE(Readline REQUIRED)
FIND_PACKAGE(Curses REQUIRED)

ADD_EXECUTABLE(pearycli "commandline/pearycli.cpp" "commandline/clicommands.cpp" "commandline/Console.cpp")
TARGET_LINK_LIBRARIES(pearycli ${PROJECT_NAME} ${Readline_LIBRARY} ${CURSES_LIBRARIES})

IF(BUILD_ExampleCaribou)
  ADD_EXECUTABLE(peary_direct "direct_device.cpp")
  TARGET_LINK_LIBRARIES(peary_direct ${PROJECT_NAME} PearyDeviceExampleCaribou)
  INSTALL(TARGETS peary_direct
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
ENDIF()

ADD_EXECUTABLE(pearyd pearyd.cpp)
TARGET_LINK_LIBRARIES(pearyd ${PROJECT_NAME})

INSTALL(TARGETS peary_app pearycli pearyd
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
