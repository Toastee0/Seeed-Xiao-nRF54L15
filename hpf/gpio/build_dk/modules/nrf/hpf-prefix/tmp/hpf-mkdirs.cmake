# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "C:/ncs/v3.2.0-preview2/nrf/applications/hpf/gpio")
  file(MAKE_DIRECTORY "C:/ncs/v3.2.0-preview2/nrf/applications/hpf/gpio")
endif()
file(MAKE_DIRECTORY
  "M:/nRF54L15/hpf/gpio/build_dk/hpf"
  "M:/nRF54L15/hpf/gpio/build_dk/modules/nrf/hpf-prefix"
  "M:/nRF54L15/hpf/gpio/build_dk/modules/nrf/hpf-prefix/tmp"
  "M:/nRF54L15/hpf/gpio/build_dk/modules/nrf/hpf-prefix/src/hpf-stamp"
  "M:/nRF54L15/hpf/gpio/build_dk/modules/nrf/hpf-prefix/src"
  "M:/nRF54L15/hpf/gpio/build_dk/modules/nrf/hpf-prefix/src/hpf-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "M:/nRF54L15/hpf/gpio/build_dk/modules/nrf/hpf-prefix/src/hpf-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "M:/nRF54L15/hpf/gpio/build_dk/modules/nrf/hpf-prefix/src/hpf-stamp${cfgdir}") # cfgdir has leading slash
endif()
