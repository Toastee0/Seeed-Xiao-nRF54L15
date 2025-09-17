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
  set(CMAKE_OBJDUMP "C:/ncs/toolchains/b8b84efebd/opt/zephyr-sdk/riscv64-zephyr-elf/bin/riscv64-zephyr-elf-objdump.exe")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/old_nrfprojects/WS2812_nrf54l15/sdk-nrf/applications/hpf/mspi/build_xiao/mspi/zephyr/arch/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/old_nrfprojects/WS2812_nrf54l15/sdk-nrf/applications/hpf/mspi/build_xiao/mspi/zephyr/lib/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/old_nrfprojects/WS2812_nrf54l15/sdk-nrf/applications/hpf/mspi/build_xiao/mspi/zephyr/soc/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/old_nrfprojects/WS2812_nrf54l15/sdk-nrf/applications/hpf/mspi/build_xiao/mspi/zephyr/boards/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/old_nrfprojects/WS2812_nrf54l15/sdk-nrf/applications/hpf/mspi/build_xiao/mspi/zephyr/subsys/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/old_nrfprojects/WS2812_nrf54l15/sdk-nrf/applications/hpf/mspi/build_xiao/mspi/zephyr/drivers/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/old_nrfprojects/WS2812_nrf54l15/sdk-nrf/applications/hpf/mspi/build_xiao/mspi/modules/nrf/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/old_nrfprojects/WS2812_nrf54l15/sdk-nrf/applications/hpf/mspi/build_xiao/mspi/modules/hostap/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/old_nrfprojects/WS2812_nrf54l15/sdk-nrf/applications/hpf/mspi/build_xiao/mspi/modules/mcuboot/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/old_nrfprojects/WS2812_nrf54l15/sdk-nrf/applications/hpf/mspi/build_xiao/mspi/modules/mbedtls/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/old_nrfprojects/WS2812_nrf54l15/sdk-nrf/applications/hpf/mspi/build_xiao/mspi/modules/trusted-firmware-m/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/old_nrfprojects/WS2812_nrf54l15/sdk-nrf/applications/hpf/mspi/build_xiao/mspi/modules/cjson/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/old_nrfprojects/WS2812_nrf54l15/sdk-nrf/applications/hpf/mspi/build_xiao/mspi/modules/azure-sdk-for-c/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/old_nrfprojects/WS2812_nrf54l15/sdk-nrf/applications/hpf/mspi/build_xiao/mspi/modules/cirrus-logic/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/old_nrfprojects/WS2812_nrf54l15/sdk-nrf/applications/hpf/mspi/build_xiao/mspi/modules/openthread/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/old_nrfprojects/WS2812_nrf54l15/sdk-nrf/applications/hpf/mspi/build_xiao/mspi/modules/memfault-firmware-sdk/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/old_nrfprojects/WS2812_nrf54l15/sdk-nrf/applications/hpf/mspi/build_xiao/mspi/modules/canopennode/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/old_nrfprojects/WS2812_nrf54l15/sdk-nrf/applications/hpf/mspi/build_xiao/mspi/modules/chre/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/old_nrfprojects/WS2812_nrf54l15/sdk-nrf/applications/hpf/mspi/build_xiao/mspi/modules/lz4/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/old_nrfprojects/WS2812_nrf54l15/sdk-nrf/applications/hpf/mspi/build_xiao/mspi/modules/nanopb/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/old_nrfprojects/WS2812_nrf54l15/sdk-nrf/applications/hpf/mspi/build_xiao/mspi/modules/zscilib/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/old_nrfprojects/WS2812_nrf54l15/sdk-nrf/applications/hpf/mspi/build_xiao/mspi/modules/cmsis/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/old_nrfprojects/WS2812_nrf54l15/sdk-nrf/applications/hpf/mspi/build_xiao/mspi/modules/cmsis-dsp/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/old_nrfprojects/WS2812_nrf54l15/sdk-nrf/applications/hpf/mspi/build_xiao/mspi/modules/cmsis-nn/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/old_nrfprojects/WS2812_nrf54l15/sdk-nrf/applications/hpf/mspi/build_xiao/mspi/modules/cmsis_6/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/old_nrfprojects/WS2812_nrf54l15/sdk-nrf/applications/hpf/mspi/build_xiao/mspi/modules/fatfs/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/old_nrfprojects/WS2812_nrf54l15/sdk-nrf/applications/hpf/mspi/build_xiao/mspi/modules/hal_nordic/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/old_nrfprojects/WS2812_nrf54l15/sdk-nrf/applications/hpf/mspi/build_xiao/mspi/modules/hal_st/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/old_nrfprojects/WS2812_nrf54l15/sdk-nrf/applications/hpf/mspi/build_xiao/mspi/modules/hal_tdk/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/old_nrfprojects/WS2812_nrf54l15/sdk-nrf/applications/hpf/mspi/build_xiao/mspi/modules/hal_wurthelektronik/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/old_nrfprojects/WS2812_nrf54l15/sdk-nrf/applications/hpf/mspi/build_xiao/mspi/modules/liblc3/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/old_nrfprojects/WS2812_nrf54l15/sdk-nrf/applications/hpf/mspi/build_xiao/mspi/modules/libmetal/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/old_nrfprojects/WS2812_nrf54l15/sdk-nrf/applications/hpf/mspi/build_xiao/mspi/modules/littlefs/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/old_nrfprojects/WS2812_nrf54l15/sdk-nrf/applications/hpf/mspi/build_xiao/mspi/modules/loramac-node/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/old_nrfprojects/WS2812_nrf54l15/sdk-nrf/applications/hpf/mspi/build_xiao/mspi/modules/lvgl/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/old_nrfprojects/WS2812_nrf54l15/sdk-nrf/applications/hpf/mspi/build_xiao/mspi/modules/mipi-sys-t/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/old_nrfprojects/WS2812_nrf54l15/sdk-nrf/applications/hpf/mspi/build_xiao/mspi/modules/nrf_wifi/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/old_nrfprojects/WS2812_nrf54l15/sdk-nrf/applications/hpf/mspi/build_xiao/mspi/modules/open-amp/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/old_nrfprojects/WS2812_nrf54l15/sdk-nrf/applications/hpf/mspi/build_xiao/mspi/modules/percepio/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/old_nrfprojects/WS2812_nrf54l15/sdk-nrf/applications/hpf/mspi/build_xiao/mspi/modules/picolibc/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/old_nrfprojects/WS2812_nrf54l15/sdk-nrf/applications/hpf/mspi/build_xiao/mspi/modules/segger/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/old_nrfprojects/WS2812_nrf54l15/sdk-nrf/applications/hpf/mspi/build_xiao/mspi/modules/tinycrypt/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/old_nrfprojects/WS2812_nrf54l15/sdk-nrf/applications/hpf/mspi/build_xiao/mspi/modules/uoscore-uedhoc/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/old_nrfprojects/WS2812_nrf54l15/sdk-nrf/applications/hpf/mspi/build_xiao/mspi/modules/zcbor/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/old_nrfprojects/WS2812_nrf54l15/sdk-nrf/applications/hpf/mspi/build_xiao/mspi/modules/nrfxlib/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/old_nrfprojects/WS2812_nrf54l15/sdk-nrf/applications/hpf/mspi/build_xiao/mspi/modules/nrf_hw_models/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/old_nrfprojects/WS2812_nrf54l15/sdk-nrf/applications/hpf/mspi/build_xiao/mspi/modules/connectedhomeip/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/old_nrfprojects/WS2812_nrf54l15/sdk-nrf/applications/hpf/mspi/build_xiao/mspi/zephyr/kernel/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/old_nrfprojects/WS2812_nrf54l15/sdk-nrf/applications/hpf/mspi/build_xiao/mspi/zephyr/cmake/flash/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/old_nrfprojects/WS2812_nrf54l15/sdk-nrf/applications/hpf/mspi/build_xiao/mspi/zephyr/cmake/usage/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/old_nrfprojects/WS2812_nrf54l15/sdk-nrf/applications/hpf/mspi/build_xiao/mspi/zephyr/cmake/reports/cmake_install.cmake")
endif()

