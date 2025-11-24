# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "M:/nRF54L15/bluetooth/channel_sounding_ras_reflector_new")
  file(MAKE_DIRECTORY "M:/nRF54L15/bluetooth/channel_sounding_ras_reflector_new")
endif()
file(MAKE_DIRECTORY
  "M:/nRF54L15/bluetooth/channel_sounding_ras_reflector_new/build_dk/channel_sounding_ras_reflector_new"
  "M:/nRF54L15/bluetooth/channel_sounding_ras_reflector_new/build_dk/_sysbuild/sysbuild/images/channel_sounding_ras_reflector_new-prefix"
  "M:/nRF54L15/bluetooth/channel_sounding_ras_reflector_new/build_dk/_sysbuild/sysbuild/images/channel_sounding_ras_reflector_new-prefix/tmp"
  "M:/nRF54L15/bluetooth/channel_sounding_ras_reflector_new/build_dk/_sysbuild/sysbuild/images/channel_sounding_ras_reflector_new-prefix/src/channel_sounding_ras_reflector_new-stamp"
  "M:/nRF54L15/bluetooth/channel_sounding_ras_reflector_new/build_dk/_sysbuild/sysbuild/images/channel_sounding_ras_reflector_new-prefix/src"
  "M:/nRF54L15/bluetooth/channel_sounding_ras_reflector_new/build_dk/_sysbuild/sysbuild/images/channel_sounding_ras_reflector_new-prefix/src/channel_sounding_ras_reflector_new-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "M:/nRF54L15/bluetooth/channel_sounding_ras_reflector_new/build_dk/_sysbuild/sysbuild/images/channel_sounding_ras_reflector_new-prefix/src/channel_sounding_ras_reflector_new-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "M:/nRF54L15/bluetooth/channel_sounding_ras_reflector_new/build_dk/_sysbuild/sysbuild/images/channel_sounding_ras_reflector_new-prefix/src/channel_sounding_ras_reflector_new-stamp${cfgdir}") # cfgdir has leading slash
endif()
