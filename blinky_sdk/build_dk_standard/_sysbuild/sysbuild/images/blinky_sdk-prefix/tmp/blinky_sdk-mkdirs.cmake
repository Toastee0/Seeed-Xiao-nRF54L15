# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "M:/nRF54L15/blinky_sdk")
  file(MAKE_DIRECTORY "M:/nRF54L15/blinky_sdk")
endif()
file(MAKE_DIRECTORY
  "M:/nRF54L15/blinky_sdk/build_dk_standard/blinky_sdk"
  "M:/nRF54L15/blinky_sdk/build_dk_standard/_sysbuild/sysbuild/images/blinky_sdk-prefix"
  "M:/nRF54L15/blinky_sdk/build_dk_standard/_sysbuild/sysbuild/images/blinky_sdk-prefix/tmp"
  "M:/nRF54L15/blinky_sdk/build_dk_standard/_sysbuild/sysbuild/images/blinky_sdk-prefix/src/blinky_sdk-stamp"
  "M:/nRF54L15/blinky_sdk/build_dk_standard/_sysbuild/sysbuild/images/blinky_sdk-prefix/src"
  "M:/nRF54L15/blinky_sdk/build_dk_standard/_sysbuild/sysbuild/images/blinky_sdk-prefix/src/blinky_sdk-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "M:/nRF54L15/blinky_sdk/build_dk_standard/_sysbuild/sysbuild/images/blinky_sdk-prefix/src/blinky_sdk-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "M:/nRF54L15/blinky_sdk/build_dk_standard/_sysbuild/sysbuild/images/blinky_sdk-prefix/src/blinky_sdk-stamp${cfgdir}") # cfgdir has leading slash
endif()
