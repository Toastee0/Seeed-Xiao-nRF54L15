# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "M:/nRF54L15/blink")
  file(MAKE_DIRECTORY "M:/nRF54L15/blink")
endif()
file(MAKE_DIRECTORY
  "M:/nRF54L15/blink/build_debug/blink"
  "M:/nRF54L15/blink/build_debug/_sysbuild/sysbuild/images/blink-prefix"
  "M:/nRF54L15/blink/build_debug/_sysbuild/sysbuild/images/blink-prefix/tmp"
  "M:/nRF54L15/blink/build_debug/_sysbuild/sysbuild/images/blink-prefix/src/blink-stamp"
  "M:/nRF54L15/blink/build_debug/_sysbuild/sysbuild/images/blink-prefix/src"
  "M:/nRF54L15/blink/build_debug/_sysbuild/sysbuild/images/blink-prefix/src/blink-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "M:/nRF54L15/blink/build_debug/_sysbuild/sysbuild/images/blink-prefix/src/blink-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "M:/nRF54L15/blink/build_debug/_sysbuild/sysbuild/images/blink-prefix/src/blink-stamp${cfgdir}") # cfgdir has leading slash
endif()
