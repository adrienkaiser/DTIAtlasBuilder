cmake_minimum_required(VERSION 2.8)
CMAKE_POLICY(VERSION 2.8)

enable_testing()

project(DTIAtlasBuilder)
set(Slicer_SKIP_PROJECT_COMMAND ON) # Useful for Slicer Extension

#===================================================================================
# Slicer Extension
option( DTIAtlasBuilder_BUILD_SLICER_EXTENSION "Build DTIAtlasBuilder as a Slicer extension" OFF )
if( DTIAtlasBuilder_BUILD_SLICER_EXTENSION )

  if(NOT Slicer_SOURCE_DIR)
    set(EXTENSION_NAME DTIAtlasBuilder)
    set(EXTENSION_HOMEPAGE "https://github.com/NIRALUser/DTIAtlasBuilder")
    set(EXTENSION_CATEGORY "Diffusion")
    set(EXTENSION_CONTRIBUTORS "Adrien Kaiser")
    set(EXTENSION_DESCRIPTION "DTI Atlas Builder")
    set(EXTENSION_ACKNOWLEDGEMENTS "")
    set(EXTENSION_STATUS Beta)
  endif()

  set(MODULE_NAME DTIAtlasBuilder)

  if(NOT Slicer_SOURCE_DIR)
    find_package(Slicer REQUIRED)
    include(${Slicer_USE_FILE})
  endif()
endif( DTIAtlasBuilder_BUILD_SLICER_EXTENSION )

#===================================================================================
# Search needed libraries and packages for DTIAtlasBuilder : ITK_DIR GenerateCLP_DIR ModuleDescriptionParser_DIR TCLAP_DIR QT_QMAKE_EXECUTABLE
find_package(ITK REQUIRED)
if(ITK_FOUND)
  include(${ITK_USE_FILE}) # creates ITK_DIR
else(ITK_FOUND)
  message(FATAL_ERROR "ITK not found. Please set ITK_DIR")
endif(ITK_FOUND)

find_package(Qt4 REQUIRED)
if(QT_USE_FILE)
  include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${QT_INCLUDE_DIR})
  include(${QT_USE_FILE}) # creates QT_QMAKE_EXECUTABLE
  add_definitions(-DQT_GUI_LIBS -DQT_CORE_LIB -DQT3_SUPPORT)
else(QT_USE_FILE)
  message(FATAL_ERROR, "QT not found. Please set QT_DIR.")
endif(QT_USE_FILE)

find_package(GenerateCLP REQUIRED)
if(GenerateCLP_FOUND)
  include(${GenerateCLP_USE_FILE}) # creates GenerateCLP_DIR ModuleDescriptionParser_DIR TCLAP_DIR
else(GenerateCLP_FOUND)
  message(FATAL_ERROR "GenerateCLP not found. Please set GenerateCLP_DIR")
endif(GenerateCLP_FOUND)

#======================================================================================
# Compile package
set( COMPILE_PACKAGE OFF CACHE BOOL "Compiles all the external projects and tools" )
if(COMPILE_PACKAGE)

  # Executable output path : folder with all the needed executables together
  set (EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/bin CACHE PATH "Single output directory for building all executables.")

  # External packages to compile
  set( ExtProjList
    dtiprocessTK # dtiprocess, dtiaverage
    AtlasWerks # GreedyAtlas
    BRAINS # BRAINSFit, BRAINSDemonWarp
    ANTS # ANTS, WarpImageMultiTransform, WarpTensorImageMultiTransform
    ResampleDTI # ResampleDTIlogEuclidean
    DTIReg # DTI-Reg, DTI-Reg_Scalar_ANTS.bms, DTI-Reg_Scalar_BRAINS.bms
    teem # unu
    MriWatcher # MriWatcher
    NIRALUtilities # ImageMath, CropDTI
    )
  foreach( tool ${ExtProjList})
    set( COMPILE_EXTERNAL_${tool} ON CACHE BOOL "Compile external ${tool}" )
  endforeach()

  # File containing add_external for all tools
  include( FindExternalTools.cmake ) # Go execute the given cmake script, and get back into this script when done

  # Update the paths to the program in the configuration file, and copy it to the executable directory
  configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/DTIAtlasBuilderSoftConfig.txt.in ${EXECUTABLE_OUTPUT_PATH}/DTIAtlasBuilderSoftConfig.txt ) # configure and copy with (tool)Path
#  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/bin/DTIAtlasBuilderSoftConfig.txt DESTINATION ${CMAKE_INSTALL_PREFIX}) # will create the install folder if doesn't exist and copy the file in it

  message("When CMake done, run \"make\" to download and compile tools, and then run \"make install\" to copy all the needed executables in the output folder : ${EXECUTABLE_OUTPUT_PATH}.\nEnter e to exit this message, and then g to generate and get CMake done.")

endif(COMPILE_PACKAGE)

#======================================================================================
# Compile step for DTIAtlasBuilder
set(DTIABsourcesFolder ${CMAKE_CURRENT_SOURCE_DIR}/Code)
QT4_WRAP_CPP(QtProject_HEADERS_MOC ${DTIABsourcesFolder}/GUI.h)
QT4_WRAP_UI(UI_FILES ${DTIABsourcesFolder}/GUIwindow.ui)
set(sources ${DTIABsourcesFolder}/DTIAtlasBuilder.cxx ${DTIABsourcesFolder}/GUI.h ${DTIABsourcesFolder}/GUI.cxx ${DTIABsourcesFolder}/ScriptWriter.h ${DTIABsourcesFolder}/ScriptWriter.cxx ${QtProject_HEADERS_MOC} ${UI_FILES})
GENERATECLP(sources ${DTIABsourcesFolder}/DTIAtlasBuilder.xml) # include the GCLP file to the project
add_executable(DTIAtlasBuilder ${sources})  # add the files contained by "sources" to the project
target_link_libraries(DTIAtlasBuilder ${QT_LIBRARIES} ${ITK_LIBRARIES})
#install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/bin/DTIAtlasBuilder DESTINATION ${CMAKE_INSTALL_PREFIX}) # will create the install folder if doesn't exist and copy the program in it

#======================================================================================
# Testing for DTIAtlasBuilder # add_test(testName Command)
add_test(TestGUI ${EXECUTABLE_OUTPUT_PATH}/DTIAtlasBuilder) # Should just display the gui
add_test(TestGUIparameters ${EXECUTABLE_OUTPUT_PATH}/DTIAtlasBuilder --parameter_file ${CMAKE_CURRENT_BINARY_DIR}/Testing/TestGUIparameters.txt) # Should just display the gui with the parameters in it
add_test(TestGUIrun ${EXECUTABLE_OUTPUT_PATH}/DTIAtlasBuilder --parameter_file ${CMAKE_CURRENT_SOURCE_DIR}/Testing/TestGUIparameters.txt) # Should display the window and user should run the pipeline
configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/Testing/ReadMe.txt ${CMAKE_CURRENT_BINARY_DIR}/Testing/ReadMe.txt ) # copy the readme
configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/Testing/TestGUIparameters.txt.in ${CMAKE_CURRENT_BINARY_DIR}/Testing/TestGUIparameters.txt ) # configure and copy
configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/Testing/TestGUIparameters.csv.in ${CMAKE_CURRENT_BINARY_DIR}/Testing/TestGUIparameters.csv ) # configure and copy
if(COMPILE_PACKAGE) # Informative display
  message("When installed, run \"ctest\" in the build directory to launch tests and see if the program is working.")
else(COMPILE_PACKAGE)
  message("When CMake done, run \"make\" to compile the program, and then run \"ctest\" in the build directory to launch tests and see if the program is working.\nEnter e to exit this message, and then g to generate and get CMake done.")
endif(COMPILE_PACKAGE)

# For Slicer Extension
if( DTIAtlasBuilder_BUILD_SLICER_EXTENSION )
  if(NOT Slicer_SOURCE_DIR)
    include(${Slicer_EXTENSION_CPACK})
  endif()
endif( DTIAtlasBuilder_BUILD_SLICER_EXTENSION )

