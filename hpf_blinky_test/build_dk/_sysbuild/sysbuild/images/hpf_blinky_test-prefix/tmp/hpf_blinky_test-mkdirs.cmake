# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "M:/nRF54L15/hpf_blinky_test")
  file(MAKE_DIRECTORY "M:/nRF54L15/hpf_blinky_test")
endif()
file(MAKE_DIRECTORY
  "M:/nRF54L15/hpf_blinky_test/build_dk/hpf_blinky_test"
  "M:/nRF54L15/hpf_blinky_test/build_dk/_sysbuild/sysbuild/images/hpf_blinky_test-prefix"
  "M:/nRF54L15/hpf_blinky_test/build_dk/_sysbuild/sysbuild/images/hpf_blinky_test-prefix/tmp"
  "M:/nRF54L15/hpf_blinky_test/build_dk/_sysbuild/sysbuild/images/hpf_blinky_test-prefix/src/hpf_blinky_test-stamp"
  "M:/nRF54L15/hpf_blinky_test/build_dk/_sysbuild/sysbuild/images/hpf_blinky_test-prefix/src"
  "M:/nRF54L15/hpf_blinky_test/build_dk/_sysbuild/sysbuild/images/hpf_blinky_test-prefix/src/hpf_blinky_test-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "M:/nRF54L15/hpf_blinky_test/build_dk/_sysbuild/sysbuild/images/hpf_blinky_test-prefix/src/hpf_blinky_test-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "M:/nRF54L15/hpf_blinky_test/build_dk/_sysbuild/sysbuild/images/hpf_blinky_test-prefix/src/hpf_blinky_test-stamp${cfgdir}") # cfgdir has leading slash
endif()
