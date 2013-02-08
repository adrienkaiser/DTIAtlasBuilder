cmake_minimum_required(VERSION 2.8)
CMAKE_POLICY(VERSION 2.8)

set(BUILD_TESTING ON CACHE BOOL "Build, configure and copy testing files")

project(DTIAtlasBuilder)
set(Slicer_SKIP_PROJECT_COMMAND ON) # Useful for Slicer Extension because include(${Slicer_USE_FILE}) wants to make a project() too

# External Projects
include(ExternalProject) # "ExternalProject" is the module that will allow to compile tools
if(CMAKE_EXTRA_GENERATOR) # CMake Generator = make, nmake..
  set(gen "${CMAKE_EXTRA_GENERATOR} - ${CMAKE_GENERATOR}")
else()
  set(gen "${CMAKE_GENERATOR}")
endif()

set(COMMON_BUILD_OPTIONS_FOR_EXTERNALPACKAGES
  -DMAKECOMMAND:STRING=${MAKECOMMAND}
  -DCMAKE_SKIP_RPATH:BOOL=${CMAKE_SKIP_RPATH}
  -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
  -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}
  -DCMAKE_CXX_COMPILER:PATH=${CMAKE_CXX_COMPILER}
  -DCMAKE_CXX_FLAGS_RELEASE:STRING=${CMAKE_CXX_FLAGS_RELEASE}
  -DCMAKE_CXX_FLAGS_DEBUG:STRING=${CMAKE_CXX_FLAGS_DEBUG}
  -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
  -DCMAKE_C_COMPILER:PATH=${CMAKE_C_COMPILER}
  -DCMAKE_C_FLAGS_RELEASE:STRING=${CMAKE_C_FLAGS_RELEASE}
  -DCMAKE_C_FLAGS_DEBUG:STRING=${CMAKE_C_FLAGS_DEBUG}
  -DCMAKE_C_FLAGS:STRING=${CMAKE_C_FLAGS}
  -DCMAKE_SHARED_LINKER_FLAGS:STRING=${CMAKE_SHARED_LINKER_FLAGS}
  -DCMAKE_EXE_LINKER_FLAGS:STRING=${CMAKE_EXE_LINKER_FLAGS}
  -DCMAKE_MODULE_LINKER_FLAGS:STRING=${CMAKE_MODULE_LINKER_FLAGS}
  -DCMAKE_GENERATOR:STRING=${CMAKE_GENERATOR}
  -DCMAKE_EXTRA_GENERATOR:STRING=${CMAKE_EXTRA_GENERATOR}
  -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
  -DCMAKE_LIBRARY_OUTPUT_DIRECTORY:PATH=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
  -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY:PATH=${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
  -DCMAKE_RUNTIME_OUTPUT_DIRECTORY:PATH=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  -DCMAKE_BUNDLE_OUTPUT_DIRECTORY:PATH=${CMAKE_BUNDLE_OUTPUT_DIRECTORY}
  -DCTEST_NEW_FORMAT:BOOL=${CTEST_NEW_FORMAT}
  -DMEMORYCHECK_COMMAND_OPTIONS:STRING=${MEMORYCHECK_COMMAND_OPTIONS}
  -DMEMORYCHECK_COMMAND:PATH=${MEMORYCHECK_COMMAND}
  -DCMAKE_SHARED_LINKER_FLAGS:STRING=${CMAKE_SHARED_LINKER_FLAGS}
  -DCMAKE_EXE_LINKER_FLAGS:STRING=${CMAKE_SHARED_LINKER_FLAGS}
  -DCMAKE_MODULE_LINKER_FLAGS:STRING=${CMAKE_MODULE_LINKER_FLAGS}
)

#===================================================================================
# Slicer Extension
option( DTIAtlasBuilder_BUILD_SLICER_EXTENSION "Build DTIAtlasBuilder as a Slicer extension" OFF )
if( DTIAtlasBuilder_BUILD_SLICER_EXTENSION )

  if(NOT Slicer_SOURCE_DIR)
    set(EXTENSION_NAME DTIAtlasBuilder)
    set(EXTENSION_HOMEPAGE "http://www.nitrc.org/projects/dtiatlasbuilder")
    set(EXTENSION_CATEGORY "Diffusion")
    set(EXTENSION_CONTRIBUTORS "Adrien Kaiser (UNC)")
    set(EXTENSION_DESCRIPTION "A tool to create a DTI Atlas Image from a set of DTI Images")
    set(EXTENSION_ICONURL "http://www.nitrc.org/project/screenshot.php?group_id=636&screenshot_id=607")
    set(EXTENSION_SCREENSHOTURLS "http://www.slicer.org/slicerWiki/images/thumb/b/b8/DTIEstim-B0-crop.png/193px-DTIEstim-B0-crop.png http://www.slicer.org/slicerWiki/images/thumb/9/90/FiberTrack-fibers.png/138px-FiberTrack-fibers.png")
    set(EXTENSION_STATUS Beta)
    set(EXTENSION_DEPENDS "NA")
    set(EXTENSION_BUILD_SUBDIRECTORY DTIAtlasBuilder-build)
  endif()

  set(MODULE_NAME DTIAtlasBuilder)

  if(NOT Slicer_SOURCE_DIR)
    find_package(Slicer REQUIRED)
    include(${Slicer_USE_FILE})
  endif()

  # SlicerExecutionModel_DEFAULT_CLI_RUNTIME_OUTPUT_DIRECTORY and SlicerExecutionModel_DEFAULT_CLI_INSTALL_RUNTIME_DESTINATION defined in Slicer_USE_FILE
  # SlicerExecutionModel_DEFAULT_CLI_INSTALL_RUNTIME_DESTINATION is [sthg]/cli_module : must contain only CLIs
  # If build as Slicer Extension, CMAKE_INSTALL_PREFIX is set to [ExtensionsFolder]/DTIAtlaBuilder
  set(INSTALL_DIR ${SlicerExecutionModel_DEFAULT_CLI_INSTALL_RUNTIME_DESTINATION}) # Set for DTIAtlasBuilder and other cli modules
  if(APPLE) # On mac, Ext/cli_modules/DTIAtlasBuilder so Ext/ExternalBin is ../ExternalBin
    set(NOCLI_INSTALL_DIR ${SlicerExecutionModel_DEFAULT_CLI_INSTALL_RUNTIME_DESTINATION}/../ExternalBin)
  elseif(WIN32) # On Windows : idem Linux : Ext/lib/Slicer4.2/cli_modules/DTIAtlasBuilder so Ext/ExternalBin is ../../../ExternalBin
    set(NOCLI_INSTALL_DIR ${SlicerExecutionModel_DEFAULT_CLI_INSTALL_RUNTIME_DESTINATION}/../../../ExternalBin)
  else() # Linux : Ext/lib/Slicer4.2/cli_modules/DTIAtlasBuilder so Ext/ExternalBin is ../../../ExternalBin
    set(NOCLI_INSTALL_DIR ${SlicerExecutionModel_DEFAULT_CLI_INSTALL_RUNTIME_DESTINATION}/../../../ExternalBin)
  endif()

else( DTIAtlasBuilder_BUILD_SLICER_EXTENSION )

  if(APPLE OR WIN32) # If not Slicer ext, not compile because will fail at run time
    message(FATAL_ERROR "DTIAtlasBuilder has known issues and will not run on Mac or Windows")
  endif()

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

find_package(Git REQUIRED) # So error while configuring and not building if Git missing -> sets GIT_EXECUTABLE

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
  DTI-Reg_Scalar_ANTS.bms # doesn't exist anymore in the new DTI-Reg version
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

  else() # Slicer extension -> recompile only tools that are not in Slicer + not MriWatcher

   foreach( tool BRAINS teem MriWatcher ) # Already in Slicer -> not recompiled # MriWatcher needs GLUT so disable if Slicer Extension because glut not necesseraly installed
      set( COMPILE_EXTERNAL_${tool} OFF CACHE BOOL "Compile external ${tool}" )
      mark_as_advanced(CLEAR COMPILE_EXTERNAL_${tool}) # Show variable if been hidden
    endforeach()

    foreach( tool dtiprocessTK AtlasWerks ANTS ResampleDTI DTIReg NIRALUtilities ) # Not in Slicer -> recompile
      set( COMPILE_EXTERNAL_${tool} ON CACHE BOOL "Compile external ${tool}" )
      mark_as_advanced(CLEAR COMPILE_EXTERNAL_${tool}) # Show variable if been hidden
    endforeach()

    if(APPLE) # unu is not recompiled with Slicer on MacOS
      set( COMPILE_EXTERNAL_teem ON CACHE BOOL "Compile external teem" FORCE)
    endif(APPLE)

    if(WIN32) # AtlasWerks does not compile on windows yet, so do not try to compile it
      set( COMPILE_EXTERNAL_AtlasWerks OFF CACHE BOOL "Compile external AtlasWerks" FORCE)
    endif(WIN32)

  endif()

  # File containing add_external for all tools
  include( ${CMAKE_CURRENT_SOURCE_DIR}/CMake/FindExternalTools.cmake ) # Go execute the given cmake script, and get back into this script when done

  # Update the paths to the program in the configuration file, and copy it to the executable directory
  if( NOT DTIAtlasBuilder_BUILD_SLICER_EXTENSION ) # If Slicer Extension, compiled and executed not on same system so Config file useless
    configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/DTIAtlasBuilderSoftConfig.txt.in ${CMAKE_CURRENT_BINARY_DIR}/DTIAtlasBuilderSoftConfig.txt ) # configure and copy with (tool)Path
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/DTIAtlasBuilderSoftConfig.txt DESTINATION ${INSTALL_DIR}) # will create the install folder if doesn't exist and copy the file in it at install step
  endif( NOT DTIAtlasBuilder_BUILD_SLICER_EXTENSION )

  # Config file for testing (paths to the built softwares in build directory) = before installing
  if(BUILD_TESTING)
    if( DTIAtlasBuilder_BUILD_SLICER_EXTENSION ) # if Slicer Ext, BRAINSFit and unu not recompiled so set to Slicer binary dirs in test config file
      set(BRAINSFITpathTestingConfigFile ${Slicer_HOME}/${Slicer_CLIMODULES_BIN_DIR}/BRAINSFit) # BRAINSFit built in cli modules dir # Slicer_CLIMODULES_BIN_DIR is a relative path
      set(unupathTestingConfigFile ${Teem_DIR}/bin/unu) # Teem_DIR set when find_package(Slicer) in SlicerConfig.cmake 
    else( DTIAtlasBuilder_BUILD_SLICER_EXTENSION )
      set(BRAINSFITpathTestingConfigFile ${CMAKE_CURRENT_BINARY_DIR}/DTIAtlasBuilder-build/BRAINS-build/bin/BRAINSFit)
      set(unupathTestingConfigFile ${CMAKE_CURRENT_BINARY_DIR}/DTIAtlasBuilder-build/teem-build/bin/unu)
    endif( DTIAtlasBuilder_BUILD_SLICER_EXTENSION ) 
    configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/Testing/DTIAtlasBuilderSoftConfig.txt.in ${CMAKE_CURRENT_BINARY_DIR}/DTIAtlasBuilder-build/Testing/DTIAtlasBuilderSoftConfig.txt)
  endif(BUILD_TESTING)
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
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
  BINARY_DIR DTIAtlasBuilder-build
  CMAKE_GENERATOR ${gen}
  CMAKE_ARGS
    -DInnerBuildCMakeLists:BOOL=ON
    ${COMMON_BUILD_OPTIONS_FOR_EXTERNALPACKAGES}
    -DITK_DIR:PATH=${ITK_DIR}
    -DVTK_DIR:PATH=${VTK_DIR} # So the VTK path is defined in the inner build directory
    -DSlicerExecutionModel_DIR:PATH=${SlicerExecutionModel_DIR}
    -DGenerateCLP_DIR:PATH=${GenerateCLP_DIR}
    -DModuleDescriptionParser_DIR:PATH=${ModuleDescriptionParser_DIR}
    -DTCLAP_DIR:PATH=${TCLAP_DIR}
    -DQT_QMAKE_EXECUTABLE:PATH=${QT_QMAKE_EXECUTABLE}
    -DBUILD_TESTING:BOOL=${BUILD_TESTING}
    -DLIBRARY_OUTPUT_PATH:PATH=${LIBRARY_OUTPUT_PATH}
    -DGIT_EXECUTABLE:PATH=${GIT_EXECUTABLE} # needed when does include(Slicer) for Slicer ext
    # Installation step
    -DINSTALL_DIR:PATH=${INSTALL_DIR}
    -DNOCLI_INSTALL_DIR:PATH=${NOCLI_INSTALL_DIR}
    -DCOMPILE_EXTERNAL_dtiprocessTK:BOOL=${COMPILE_EXTERNAL_dtiprocessTK}
    -DCOMPILE_EXTERNAL_AtlasWerks:BOOL=${COMPILE_EXTERNAL_AtlasWerks}
    -DCOMPILE_EXTERNAL_BRAINS:BOOL=${COMPILE_EXTERNAL_BRAINS}
    -DCOMPILE_EXTERNAL_ANTS:BOOL=${COMPILE_EXTERNAL_ANTS}
    -DCOMPILE_EXTERNAL_ResampleDTI:BOOL=${COMPILE_EXTERNAL_ResampleDTI}
    -DCOMPILE_EXTERNAL_DTIReg:BOOL=${COMPILE_EXTERNAL_DTIReg}
    -DCOMPILE_EXTERNAL_teem:BOOL=${COMPILE_EXTERNAL_teem}
    -DCOMPILE_EXTERNAL_MriWatcher:BOOL=${COMPILE_EXTERNAL_MriWatcher}
    -DCOMPILE_EXTERNAL_NIRALUtilities:BOOL=${COMPILE_EXTERNAL_NIRALUtilities}
    # Slicer extension
    -DDTIAtlasBuilder_BUILD_SLICER_EXTENSION:BOOL=${DTIAtlasBuilder_BUILD_SLICER_EXTENSION}
    -DSlicer_DIR:PATH=${Slicer_DIR}
    -DSlicer_SKIP_PROJECT_COMMAND:BOOL=ON
    -DCLI_RUNTIME_OUTPUT_DIRECTORY:PATH=${SlicerExecutionModel_DEFAULT_CLI_RUNTIME_OUTPUT_DIRECTORY}
    -DCLI_INSTALL_RUNTIME_DESTINATION:PATH=${SlicerExecutionModel_DEFAULT_CLI_INSTALL_RUNTIME_DESTINATION}
    -DEXTENSION_SUPERBUILD_BINARY_DIR:PATH=${CMAKE_CURRENT_BINARY_DIR}
    -DEXTENSION_SOURCE_DIR:PATH=${EXTENSION_SOURCE_DIR}
    -DEXTENSION_NAME:STRING=${EXTENSION_NAME}
    -DEXTENSION_HOMEPAGE:STRING=${EXTENSION_HOMEPAGE}
    -DEXTENSION_CATEGORY:STRING=${EXTENSION_CATEGORY}
    -DEXTENSION_CONTRIBUTORS:STRING=${EXTENSION_CONTRIBUTORS}
    -DEXTENSION_DESCRIPTION:STRING=${EXTENSION_DESCRIPTION}
    -DEXTENSION_ICONURL:STRING=${EXTENSION_ICONURL}
    -DEXTENSION_SCREENSHOTURLS:STRING=${EXTENSION_SCREENSHOTURLS}
    -DEXTENSION_STATUS:STRING=${EXTENSION_STATUS}
    -DEXTENSION_DEPENDS:STRING=${EXTENSION_DEPENDS}
    -DEXTENSION_BUILD_SUBDIRECTORY:STRING=${EXTENSION_BUILD_SUBDIRECTORY}
    -DMIDAS_PACKAGE_EMAIL:STRING=${MIDAS_PACKAGE_EMAIL}
    -DMIDAS_PACKAGE_API_KEY:STRING=${MIDAS_PACKAGE_API_KEY}
  INSTALL_COMMAND ""
  DEPENDS ${ITK_DEPEND} ${DTIAtlasBuilderExternalToolsDependencies} # DTIAtlasBuilderExternalToolsDependencies contains the names of all the recompiles softwares so DTIAB is compiled last (for install)
)

