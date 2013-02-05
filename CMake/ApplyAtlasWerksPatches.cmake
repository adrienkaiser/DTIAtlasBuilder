# TOP_BINARY_DIR is given as arg to this cmake script
# => TOP_BINARY_DIR= (topBuildDir)
# BASE_SOURCE_DIR is given as arg to this cmake script
# => BASE_SOURCE_DIR = DTIAB top source dir

#message("TOP_BINARY_DIR=${TOP_BINARY_DIR}")
#message("BASE_SOURCE_DIR=${BASE_SOURCE_DIR}")

# Patch to give AtlasWerks the paths to the recompiled LAPACK
execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${TOP_BINARY_DIR}/CMakeLists-AtlasWerksLAPACK-Patched.txt ${TOP_BINARY_DIR}/DTIAtlasBuilder-build/AtlasWerks/CMakeLists.txt)

# Patch so AtlasWerks searches FFTW libs in lib/Debug and lib/Release for Windows
execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${BASE_SOURCE_DIR}/CMake/AtlasWerksPatchedFindFFTW.cmake ${TOP_BINARY_DIR}/DTIAtlasBuilder-build/AtlasWerks/CMake/FindFFTW.cmake)
