# Install script for directory: C:/ncs/v3.1.0/zephyr

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/Zephyr-Kernel")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "C:/ncs/toolchains/b8b84efebd/opt/zephyr-sdk/arm-zephyr-eabi/bin/arm-zephyr-eabi-objdump.exe")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-epaper-ncs/build_xiao/zephyr-epaper-ncs/zephyr/arch/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-epaper-ncs/build_xiao/zephyr-epaper-ncs/zephyr/lib/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-epaper-ncs/build_xiao/zephyr-epaper-ncs/zephyr/soc/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-epaper-ncs/build_xiao/zephyr-epaper-ncs/zephyr/boards/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-epaper-ncs/build_xiao/zephyr-epaper-ncs/zephyr/subsys/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-epaper-ncs/build_xiao/zephyr-epaper-ncs/zephyr/drivers/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-epaper-ncs/build_xiao/zephyr-epaper-ncs/modules/nrf/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-epaper-ncs/build_xiao/zephyr-epaper-ncs/modules/hostap/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-epaper-ncs/build_xiao/zephyr-epaper-ncs/modules/mcuboot/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-epaper-ncs/build_xiao/zephyr-epaper-ncs/modules/mbedtls/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-epaper-ncs/build_xiao/zephyr-epaper-ncs/modules/trusted-firmware-m/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-epaper-ncs/build_xiao/zephyr-epaper-ncs/modules/cjson/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-epaper-ncs/build_xiao/zephyr-epaper-ncs/modules/azure-sdk-for-c/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-epaper-ncs/build_xiao/zephyr-epaper-ncs/modules/cirrus-logic/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-epaper-ncs/build_xiao/zephyr-epaper-ncs/modules/openthread/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-epaper-ncs/build_xiao/zephyr-epaper-ncs/modules/memfault-firmware-sdk/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-epaper-ncs/build_xiao/zephyr-epaper-ncs/modules/canopennode/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-epaper-ncs/build_xiao/zephyr-epaper-ncs/modules/chre/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-epaper-ncs/build_xiao/zephyr-epaper-ncs/modules/lz4/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-epaper-ncs/build_xiao/zephyr-epaper-ncs/modules/nanopb/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-epaper-ncs/build_xiao/zephyr-epaper-ncs/modules/zscilib/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-epaper-ncs/build_xiao/zephyr-epaper-ncs/modules/cmsis/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-epaper-ncs/build_xiao/zephyr-epaper-ncs/modules/cmsis-dsp/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-epaper-ncs/build_xiao/zephyr-epaper-ncs/modules/cmsis-nn/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-epaper-ncs/build_xiao/zephyr-epaper-ncs/modules/cmsis_6/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-epaper-ncs/build_xiao/zephyr-epaper-ncs/modules/fatfs/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-epaper-ncs/build_xiao/zephyr-epaper-ncs/modules/hal_nordic/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-epaper-ncs/build_xiao/zephyr-epaper-ncs/modules/hal_st/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-epaper-ncs/build_xiao/zephyr-epaper-ncs/modules/hal_tdk/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-epaper-ncs/build_xiao/zephyr-epaper-ncs/modules/hal_wurthelektronik/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-epaper-ncs/build_xiao/zephyr-epaper-ncs/modules/liblc3/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-epaper-ncs/build_xiao/zephyr-epaper-ncs/modules/libmetal/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-epaper-ncs/build_xiao/zephyr-epaper-ncs/modules/littlefs/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-epaper-ncs/build_xiao/zephyr-epaper-ncs/modules/loramac-node/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-epaper-ncs/build_xiao/zephyr-epaper-ncs/modules/lvgl/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-epaper-ncs/build_xiao/zephyr-epaper-ncs/modules/mipi-sys-t/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-epaper-ncs/build_xiao/zephyr-epaper-ncs/modules/nrf_wifi/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-epaper-ncs/build_xiao/zephyr-epaper-ncs/modules/open-amp/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-epaper-ncs/build_xiao/zephyr-epaper-ncs/modules/percepio/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-epaper-ncs/build_xiao/zephyr-epaper-ncs/modules/picolibc/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-epaper-ncs/build_xiao/zephyr-epaper-ncs/modules/segger/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-epaper-ncs/build_xiao/zephyr-epaper-ncs/modules/tinycrypt/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-epaper-ncs/build_xiao/zephyr-epaper-ncs/modules/uoscore-uedhoc/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-epaper-ncs/build_xiao/zephyr-epaper-ncs/modules/zcbor/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-epaper-ncs/build_xiao/zephyr-epaper-ncs/modules/nrfxlib/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-epaper-ncs/build_xiao/zephyr-epaper-ncs/modules/nrf_hw_models/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-epaper-ncs/build_xiao/zephyr-epaper-ncs/modules/connectedhomeip/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-epaper-ncs/build_xiao/zephyr-epaper-ncs/zephyr/kernel/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-epaper-ncs/build_xiao/zephyr-epaper-ncs/zephyr/cmake/flash/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-epaper-ncs/build_xiao/zephyr-epaper-ncs/zephyr/cmake/usage/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-epaper-ncs/build_xiao/zephyr-epaper-ncs/zephyr/cmake/reports/cmake_install.cmake")
endif()

