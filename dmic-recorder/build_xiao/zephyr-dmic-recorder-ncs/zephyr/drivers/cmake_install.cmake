# Install script for directory: C:/ncs/v3.1.0/zephyr/drivers

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
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-dmic-recorder-ncs/build_xiao/zephyr-dmic-recorder-ncs/zephyr/drivers/disk/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-dmic-recorder-ncs/build_xiao/zephyr-dmic-recorder-ncs/zephyr/drivers/firmware/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-dmic-recorder-ncs/build_xiao/zephyr-dmic-recorder-ncs/zephyr/drivers/interrupt_controller/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-dmic-recorder-ncs/build_xiao/zephyr-dmic-recorder-ncs/zephyr/drivers/misc/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-dmic-recorder-ncs/build_xiao/zephyr-dmic-recorder-ncs/zephyr/drivers/pcie/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-dmic-recorder-ncs/build_xiao/zephyr-dmic-recorder-ncs/zephyr/drivers/usb/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-dmic-recorder-ncs/build_xiao/zephyr-dmic-recorder-ncs/zephyr/drivers/usb_c/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-dmic-recorder-ncs/build_xiao/zephyr-dmic-recorder-ncs/zephyr/drivers/audio/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-dmic-recorder-ncs/build_xiao/zephyr-dmic-recorder-ncs/zephyr/drivers/cache/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-dmic-recorder-ncs/build_xiao/zephyr-dmic-recorder-ncs/zephyr/drivers/clock_control/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-dmic-recorder-ncs/build_xiao/zephyr-dmic-recorder-ncs/zephyr/drivers/console/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-dmic-recorder-ncs/build_xiao/zephyr-dmic-recorder-ncs/zephyr/drivers/gpio/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-dmic-recorder-ncs/build_xiao/zephyr-dmic-recorder-ncs/zephyr/drivers/pinctrl/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-dmic-recorder-ncs/build_xiao/zephyr-dmic-recorder-ncs/zephyr/drivers/regulator/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-dmic-recorder-ncs/build_xiao/zephyr-dmic-recorder-ncs/zephyr/drivers/serial/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("M:/projects/Seeed-Xiao-nRF54L15/zephyr-dmic-recorder-ncs/build_xiao/zephyr-dmic-recorder-ncs/zephyr/drivers/timer/cmake_install.cmake")
endif()

