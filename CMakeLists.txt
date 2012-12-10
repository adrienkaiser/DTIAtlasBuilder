cmake_minimum_required(VERSION 2.8)
CMAKE_POLICY(VERSION 2.8)

project(DTIAtlasBuilder)
set(Slicer_SKIP_PROJECT_COMMAND ON)

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

# Executable output path : folder with all the needed executables together
set (EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/bin CACHE PATH "Single output directory for building all executables.")

#===================================================================================
# Search needed libraries and packages fro DTIAtlasBuilder : ITK_DIR GenerateCLP_DIR ModuleDescriptionParser_DIR TCLAP_DIR QT_QMAKE_EXECUTABLE
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
    set( COMPILE_EXTERNAL_${tool} ON CACHE BOOL "" )
  endforeach()

  # Librairies needed only for external tools
  find_package(VTK REQUIRED)
  if (VTK_FOUND)
    set(VTK_USE_QVTK TRUE)
    set(VTK_USE_GUISUPPORT TRUE)
    include(${VTK_USE_FILE}) # creates VTK_DIR
  else(VTK_FOUND)
    message(FATAL_ERROR, "VTK not found. Please set VTK_DIR.")
  endif (VTK_FOUND)

  find_package(SlicerExecutionModel REQUIRED)
  if(SlicerExecutionModel_FOUND)
    include(${SlicerExecutionModel_USE_FILE}) # creates SlicerExecutionModel_DIR (DTI-Reg)
    include(${SlicerExecutionModel_CMAKE_DIR}/SEMMacroBuildCLI.cmake)
  else(SlicerExecutionModel_FOUND)
    message(FATAL_ERROR "SlicerExecutionModel not found. Please set SlicerExecutionModel_DIR")
  endif(SlicerExecutionModel_FOUND)

  if(COMPILE_EXTERNAL_AtlasWerks) # FFTW and FLTK only needed for AtlasWerks
    set(FFTW_DIR CACHE PATH "Path to the fftw install folder (./include, ./lib/libfftw3f.a)")
    if(NOT FFTW_DIR)
      message(FATAL_ERROR "FFTW not set. Please set FFTW_DIR manually")
    endif()

    find_package(FLTK REQUIRED)
    if(FLTK_FOUND)
      include_directories(${FLTK_INCLUDE_DIR}) # creates FLTK_DIR
    else(FLTK_FOUND)
      message(FATAL_ERROR "FLTK not found. Please set FLTK_DIR")
    endif(FLTK_FOUND)
  endif(COMPILE_EXTERNAL_AtlasWerks)

  # File containing add_external for all tools
  include( FindExternalTools.cmake ) # Go execute the given cmake script, and get back into this script when done

  # Update the paths to the program in the configuration file, and copy it to the executable directory
  configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/DTIAtlasBuilderSoftConfig.txt.in ${EXECUTABLE_OUTPUT_PATH}/DTIAtlasBuilderSoftConfig.txt ) # configure and copy with (tool)Path
#  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/bin/DTIAtlasBuilderSoftConfig.txt DESTINATION ${CMAKE_INSTALL_PREFIX}) # will create the install folder if doesn't exist and copy the file in it

endif(COMPILE_PACKAGE)

#======================================================================================
# Compile and Install step for DTIAtlasBuilder
QT4_WRAP_CPP(QtProject_HEADERS_MOC GUI.h)
QT4_WRAP_UI(UI_FILES GUIwindow.ui)
set(sources DTIAtlasBuilder.cxx GUI.h GUI.cxx ScriptWriter.h ScriptWriter.cxx ${QtProject_HEADERS_MOC} ${UI_FILES}) # define the variable "sources" that contains the names of files
GENERATECLP(sources DTIAtlasBuilder.xml) # include the GCLP file to the project
add_executable(DTIAtlasBuilder ${sources})  # add the files contained by "sources" to the project
target_link_libraries(DTIAtlasBuilder ${QT_LIBRARIES} ${ITK_LIBRARIES})
#install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/bin/DTIAtlasBuilder DESTINATION ${CMAKE_INSTALL_PREFIX}) # will create the install folder if doesn't exist and copy the program in it

if( DTIAtlasBuilder_BUILD_SLICER_EXTENSION )
  if(NOT Slicer_SOURCE_DIR)
    include(${Slicer_EXTENSION_CPACK})
  endif()
endif( DTIAtlasBuilder_BUILD_SLICER_EXTENSION )

