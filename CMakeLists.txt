cmake_minimum_required(VERSION 2.8)
CMAKE_POLICY(VERSION 2.8)

set(BUILD_TESTING ON CACHE BOOL "Build, configure and copy testing files")

project(DTIAtlasBuilder)
set(Slicer_SKIP_PROJECT_COMMAND ON) # Useful for Slicer Extension

# External Projects
include(ExternalProject) # "ExternalProject" is the module that will allow to compile tools
if(CMAKE_EXTRA_GENERATOR) # Extra generator ??
  set(gen "${CMAKE_EXTRA_GENERATOR} - ${CMAKE_GENERATOR}")
else()
  set(gen "${CMAKE_GENERATOR}")
endif()

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

  # SlicerExecutionModel_DEFAULT_CLI_RUNTIME_OUTPUT_DIRECTORY and SlicerExecutionModel_DEFAULT_CLI_INSTALL_RUNTIME_DESTINATION defined in Slicer_USE_FILE
  # If build as Slicer Extension, CMAKE_INSTALL_PREFIX is set to Extensions/DTIAtlaBuilder
  # SlicerExecutionModel_DEFAULT_CLI_INSTALL_RUNTIME_DESTINATION is ./lib/Slicer4.2/cli_module -> not install not cli modules in it
  set(INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/${SlicerExecutionModel_DEFAULT_CLI_INSTALL_RUNTIME_DESTINATION}) # Set for DTIAtlasBuilder and config file
  set(NOCLI_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/bin)

else( DTIAtlasBuilder_BUILD_SLICER_EXTENSION )
  set(INSTALL_DIR ${CMAKE_INSTALL_PREFIX})
endif( DTIAtlasBuilder_BUILD_SLICER_EXTENSION )

set( COMPILE_PACKAGE ON CACHE BOOL "Compiles all the external projects and tools" )

#===================================================================================
# Search needed libraries and packages for DTIAtlasBuilder : ITK_DIR GenerateCLP_DIR ModuleDescriptionParser_DIR TCLAP_DIR QT_QMAKE_EXECUTABLE
if(NOT COMPILE_PACKAGE) # ITK and SlicerExecutionModel (GenerateCLP) are recompiled in the package
  find_package(ITK REQUIRED)
  if(ITK_FOUND)
    include(${ITK_USE_FILE}) # creates ITK_DIR
  else(ITK_FOUND)
    message(FATAL_ERROR "ITK not found. Please set ITK_DIR")
  endif(ITK_FOUND)

  find_package(SlicerExecutionModel REQUIRED)
  if(SlicerExecutionModel_FOUND)
    include(${SlicerExecutionModel_USE_FILE}) # creates GenerateCLP_DIR ModuleDescriptionParser_DIR TCLAP_DIR
  else(SlicerExecutionModel_FOUND)
    message(FATAL_ERROR "SlicerExecutionModel not found. Please set SlicerExecutionModel_DIR")
  endif(SlicerExecutionModel_FOUND)
endif(NOT COMPILE_PACKAGE)

find_package(Qt4 REQUIRED) # For DTIAtlasBuilder
if(QT_USE_FILE)
  include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${QT_INCLUDE_DIR})
  include(${QT_USE_FILE}) # creates QT_QMAKE_EXECUTABLE
  add_definitions(-DQT_GUI_LIBS -DQT_CORE_LIB -DQT3_SUPPORT)
else(QT_USE_FILE)
  message(FATAL_ERROR, "QT not found. Please set QT_DIR.")
endif(QT_USE_FILE)

#======================================================================================
# Compile package
set( ExtProjList # External packages to compile
  dtiprocessTK # dtiprocess, dtiaverage
  AtlasWerks # GreedyAtlas
  BRAINS # BRAINSFit, BRAINSDemonWarp
  ANTS # ANTS, WarpImageMultiTransform, WarpTensorImageMultiTransform
  ResampleDTI # ResampleDTIlogEuclidean
  DTIReg # DTI-Reg
  teem # unu
  MriWatcher # MriWatcher
  NIRALUtilities # ImageMath, CropDTI
  )
set( ToolsList # Needed tools -> to hide unuseful TOOL* variables
  dtiprocess
  dtiaverage
  GreedyAtlas
  BRAINSFit
  BRAINSDemonWarp
  ANTS
  WarpImageMultiTransform
  WarpTensorImageMultiTransform
  ResampleDTIlogEuclidean
  DTI-Reg
  DTI-Reg_Scalar_ANTS.bms
  DTI-Reg_Scalar_BRAINS.bms
  MriWatcher
  ImageMath
  CropDTI
  unu
  )
if(COMPILE_PACKAGE)

  # Define COMPILE_EXTERNAL_* variables
  if( NOT DTIAtlasBuilder_BUILD_SLICER_EXTENSION ) # no Slicer extension -> recompile all

    foreach( tool ${ExtProjList})
      set( COMPILE_EXTERNAL_${tool} ON CACHE BOOL "Compile external ${tool}" )
      mark_as_advanced(CLEAR COMPILE_EXTERNAL_${tool}) # Show variable if been hidden
    endforeach()

  else() # Slicer extension -> recompile only tools that are not in Slicer

   foreach( tool BRAINS teem ) # Already in Slicer -> not recompiled
      set( COMPILE_EXTERNAL_${tool} OFF CACHE BOOL "Compile external ${tool}" )
      mark_as_advanced(CLEAR COMPILE_EXTERNAL_${tool}) # Show variable if been hidden
    endforeach()

    foreach( tool dtiprocessTK AtlasWerks ANTS ResampleDTI DTIReg MriWatcher NIRALUtilities ) # Not in Slicer -> recompile
      set( COMPILE_EXTERNAL_${tool} ON CACHE BOOL "Compile external ${tool}" )
      mark_as_advanced(CLEAR COMPILE_EXTERNAL_${tool}) # Show variable if been hidden
    endforeach()

  endif()

  # File containing add_external for all tools
  include( ${CMAKE_CURRENT_SOURCE_DIR}/CMake/FindExternalTools.cmake ) # Go execute the given cmake script, and get back into this script when done

  # Update the paths to the program in the configuration file, and copy it to the executable directory
  configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/DTIAtlasBuilderSoftConfig.txt.in ${INSTALL_DIR}/DTIAtlasBuilderSoftConfig.txt ) # configure and copy with (tool)Path
#  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/bin/DTIAtlasBuilderSoftConfig.txt DESTINATION ${INSTALL_DIR}) # will create the install folder if doesn't exist and copy the file in it

#  message("When CMake done, run \"make\" to download and compile tools, and then run \"make install\" to copy all the needed executables in the output folder : ${INSTALL_DIR}.\nEnter e to exit this message, and then g to generate and get CMake done.")

else(COMPILE_PACKAGE) # Hide unuseful variables
  foreach( proj ${ExtProjList})
    mark_as_advanced(FORCE COMPILE_EXTERNAL_${proj})
  endforeach()
#  get_cmake_property(CacheVars CACHE_VARIABLES) # put all the cache variables in the variable "CacheVars" -> search for all TOOL* variables : avoid ToolsList
  foreach( tool ${ToolsList})
    mark_as_advanced(FORCE TOOL${tool})
    mark_as_advanced(FORCE TOOL${tool}Sys)
  endforeach()
endif(COMPILE_PACKAGE)

#======================================================================================
ExternalProject_Add(DTIAtlasBuilder # DTIAtlasBuilder added as Externalproject in case of SlicerExecutionModel recompiled because needs it
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Code
  BINARY_DIR DTIAtlasBuilder-build
  CMAKE_GENERATOR ${gen}
  CMAKE_ARGS
    -DITK_DIR:PATH=${ITK_DIR}
    -DSlicerExecutionModel_DIR:PATH=${SlicerExecutionModel_DIR}
    -DGenerateCLP_DIR:PATH=${GenerateCLP_DIR}
    -DModuleDescriptionParser_DIR:PATH=${ModuleDescriptionParser_DIR}
    -DTCLAP_DIR:PATH=${TCLAP_DIR}
    -DQT_QMAKE_EXECUTABLE:PATH=${QT_QMAKE_EXECUTABLE}
    -DBUILD_TESTING:BOOL=${BUILD_TESTING}
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    -DLIBRARY_OUTPUT_PATH:PATH=${LIBRARY_OUTPUT_PATH}
  INSTALL_COMMAND ""
  DEPENDS ${ITK_DEPEND}
)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/DTIAtlasBuilder-build/DTIAtlasBuilder DESTINATION ${INSTALL_DIR})

# For Slicer Extension
if( DTIAtlasBuilder_BUILD_SLICER_EXTENSION )
  if(NOT Slicer_SOURCE_DIR)
    include(${Slicer_EXTENSION_CPACK})
  endif()
endif( DTIAtlasBuilder_BUILD_SLICER_EXTENSION )
