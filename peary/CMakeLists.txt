# Add source files for the peary caribou core library

OPTION(INTERFACE_EMULATION "Use emulators for all interfaces" OFF)

# Allow to individually switch off interfaces
OPTION(INTERFACE_I2C "Build Caribou I2C interface?" ON)
OPTION(INTERFACE_SPI "Build Caribou SPI interface?" ON)
OPTION(INTERFACE_SPI_BUS "Build Caribou SPI bus interface?" ON)
OPTION(INTERFACE_SPI_CLICpix2 "Build Caribou SPI CLICpix2 interface?" ON)
OPTION(INTERFACE_IPSOCK "Build Caribou IP/Socket interface?" ON)
OPTION(INTERFACE_LOOP "Build Caribou Loopback interface?" ON)
OPTION(INTERFACE_MEM "Build Caribou Memory interface?" ON)
OPTION(INTERFACE_SERMEM "Build Caribou SerialMemory interface?" ON)
OPTION(INTERFACE_IIO "Build Caribou IIO interface?" OFF)
OPTION(INTERFACE_MEDIA "Build Caribou Media interface?" OFF)

SET(LIB_SOURCE_FILES
  # device manager
  "device/DeviceManager.cpp"
  "device/Device.cpp"
  # HAL base
  "hardware_abstraction/HAL.cpp"
  "hardware_abstraction/carboard/Carboard.cpp"
  "hardware_abstraction/falconboard/Falconboard.cpp"
  # interface manager
  "interfaces/InterfaceManager.cpp"
  "interfaces/interfaceConfiguration.cpp"
  # utilities
  "utils/log.cpp"
  "utils/lfsr.cpp"
  "utils/utils.cpp"
  "utils/configuration.cpp"
  )

IF(INTERFACE_I2C AND NOT INTERFACE_EMULATION)
  # add I2C source files
  SET(LIB_SOURCE_FILES ${LIB_SOURCE_FILES}
    "interfaces/I2C/i2c_config.cpp"
    "interfaces/I2C/i2c.cpp"
    )

  # add I2C library
  FIND_LIBRARY(i2clib NAMES i2c REQUIRED)
  SET(PEARY_DL_LIBS ${PEARY_DL_LIBS} ${i2clib})

  MESSAGE(STATUS "Caribou Interface I2C:\tON")
ELSE()
  SET(LIB_SOURCE_FILES ${LIB_SOURCE_FILES}
    "interfaces/I2C/i2c_config.cpp"
    "interfaces/I2C/emulator.cpp"
    )
  MESSAGE(STATUS "Caribou Interface I2C:\t(emulated)")
ENDIF()

IF(INTERFACE_SPI_CLICpix2 AND NOT INTERFACE_EMULATION)
  # add SPI source files
  SET(LIB_SOURCE_FILES ${LIB_SOURCE_FILES}
    "interfaces/SPI_CLICpix2/spi_CLICpix2.cpp"
    )
  MESSAGE(STATUS "Caribou Interface SPI CPx2:\tON")
ELSE()
  SET(LIB_SOURCE_FILES ${LIB_SOURCE_FILES}
    "interfaces/SPI_CLICpix2/emulator.cpp"
    )
  MESSAGE(STATUS "Caribou Interface SPI CPx2:\t(emulated)")
ENDIF()

IF(INTERFACE_IPSOCK)
  # add IP/Socket source files
  SET(LIB_SOURCE_FILES ${LIB_SOURCE_FILES}
    "interfaces/IPSocket/ipsocket.cpp"
    )
  MESSAGE(STATUS "Caribou Interface IP/Socket:\tON")
ELSE()
  MESSAGE(STATUS "Caribou Interface IP/Socket:\tOFF")
ENDIF()

# Loopback interface can be always built
IF(INTERFACE_LOOP)
  # add LOOP source files
  SET(LIB_SOURCE_FILES ${LIB_SOURCE_FILES}
    "interfaces/Loopback/loopback_config.cpp"
    "interfaces/Loopback/loopback.cpp"
    )
  MESSAGE(STATUS "Caribou Interface Loopback:\tON")
ELSE()
  MESSAGE(STATUS "Caribou Interface Loopback:\tOFF")
ENDIF()

IF(INTERFACE_MEM AND NOT INTERFACE_EMULATION)
  # add MEM source files
  SET(LIB_SOURCE_FILES ${LIB_SOURCE_FILES}
    "interfaces/Memory/memory_config.cpp"
    "interfaces/Memory/memory.cpp"
    )
  MESSAGE(STATUS "Caribou Interface MEM:\tON")
ELSE()
  SET(LIB_SOURCE_FILES ${LIB_SOURCE_FILES}
    "interfaces/Memory/memory_config.cpp"
    "interfaces/Memory/emulator.cpp"
    )
  MESSAGE(STATUS "Caribou Interface MEM:\t(emulated)")
ENDIF()

IF(INTERFACE_SERMEM AND INTERFACE_MEM AND NOT INTERFACE_EMULATION)
  # add SERMEM source files
  SET(LIB_SOURCE_FILES ${LIB_SOURCE_FILES}
    "interfaces/SerialMemory/serialmemory_config.cpp"
    "interfaces/SerialMemory/serialmemory.cpp"
    )
  MESSAGE(STATUS "Caribou Interface SERMEM:\tON")
ELSE()
  SET(LIB_SOURCE_FILES ${LIB_SOURCE_FILES}
    "interfaces/SerialMemory/serialmemory_config.cpp"
    "interfaces/SerialMemory/emulator.cpp"
    )
  MESSAGE(STATUS "Caribou Interface SERMEM:\t(emulated)")
ENDIF()

IF(INTERFACE_IIO AND NOT INTERFACE_EMULATION)
  SET(LIB_SOURCE_FILES ${LIB_SOURCE_FILES}
    "interfaces/IIO/iio_config.cpp"
    "interfaces/IIO/iio.cpp"
  )
  # add iio library
  FIND_LIBRARY(iiolib NAMES iio REQUIRED)
  SET(PEARY_DL_LIBS ${PEARY_DL_LIBS} ${iiolib})
  MESSAGE(STATUS "Caribou Interface IIO:\tON")
ELSE()
  SET(LIB_SOURCE_FILES ${LIB_SOURCE_FILES}
    "interfaces/IIO/iio_config.cpp"
    "interfaces/IIO/emulator.cpp"
  )
  MESSAGE(STATUS "Caribou Interface IIO:\t(emulated)")
ENDIF()

IF(INTERFACE_SPI_BUS AND NOT INTERFACE_EMULATION)
  SET(LIB_SOURCE_FILES ${LIB_SOURCE_FILES}
    "interfaces/SPI_BUS/spi_bus_config.cpp"
    "interfaces/SPI_BUS/spi_bus.cpp"
  )
  MESSAGE(STATUS "Caribou Interface SPI bus:\tON")
ELSE()
  SET(LIB_SOURCE_FILES ${LIB_SOURCE_FILES}
    "interfaces/SPI_BUS/spi_bus_config.cpp"
    "interfaces/SPI_BUS/emulator.cpp"
  )
  MESSAGE(STATUS "Caribou Interface SPI bus:\t(emulated)")
ENDIF()

IF(INTERFACE_MEDIA AND NOT INTERFACE_EMULATION)
  SET(LIB_SOURCE_FILES ${LIB_SOURCE_FILES}
    "interfaces/Media/media_config.cpp"
    "interfaces/Media/media.cpp"
  )
  # add mediactl library
  FIND_LIBRARY(mediactllib NAMES mediactl REQUIRED)
  SET(PEARY_DL_LIBS ${PEARY_DL_LIBS} ${mediactllib})
  FIND_LIBRARY(v4l2subdevlib NAMES v4l2subdev REQUIRED)
  SET(PEARY_DL_LIBS ${PEARY_DL_LIBS} ${v4l2subdevlib})
  MESSAGE(STATUS "Caribou Interface Media:\tON")
ELSE()
  SET(LIB_SOURCE_FILES ${LIB_SOURCE_FILES}
    "interfaces/Media/media_config.cpp"
    "interfaces/Media/emulator.cpp"
  )
  MESSAGE(STATUS "Caribou Interface Media:\t(emulated)")
ENDIF()

ADD_LIBRARY(${PROJECT_NAME} SHARED ${LIB_SOURCE_FILES})

TARGET_LINK_LIBRARIES(${PROJECT_NAME} PUBLIC ${CMAKE_DL_LIBS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} PUBLIC Threads::Threads ${PEARY_DL_LIBS})
TARGET_COMPILE_DEFINITIONS(${PROJECT_NAME} PRIVATE SHARED_LIBRARY_SUFFIX="${CMAKE_SHARED_LIBRARY_SUFFIX}")
TARGET_COMPILE_DEFINITIONS(${PROJECT_NAME} PRIVATE "PEARY_LOCK_DIR=\"${PEARY_LOCK_DIR}\"")

IF(NOT INTERFACE_SPI OR INTERFACE_EMULATION)
  TARGET_COMPILE_DEFINITIONS(${PROJECT_NAME} PUBLIC SPI_EMULATED)
  MESSAGE(STATUS "Caribou Interface SPI:\t(emulated)")
ELSE()
  MESSAGE(STATUS "Caribou Interface SPI:\tON")
ENDIF()

IF(NOT INTERFACE_IIO OR INTERFACE_EMULATION)
    TARGET_COMPILE_DEFINITIONS(${PROJECT_NAME} PUBLIC IIO_EMULATED)
ENDIF()

IF(NOT INTERFACE_MEDIA OR INTERFACE_EMULATION)
  TARGET_COMPILE_DEFINITIONS(${PROJECT_NAME} PUBLIC MEDIA_EMULATED)
ENDIF()

TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/interfaces/Media>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/peary/peary>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/peary>)

INSTALL(TARGETS ${PROJECT_NAME}
  EXPORT Caribou
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/etc/pkg-config/peary.pc.in ${PROJECT_NAME}.pc @ONLY)
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

INSTALL(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/peary/"
    FILES_MATCHING
    PATTERN "*.hpp"
    PATTERN "*.h"
    PATTERN "*.tcc")
