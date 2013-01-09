# Find external tools

# Git protocol
option(USE_GIT_PROTOCOL "If behind a firewall turn this off to use https instead." ON)
set(git_protocol "git")
if(NOT USE_GIT_PROTOCOL)
  set(git_protocol "https")
else(NOT USE_GIT_PROTOCOL)
  set(git_protocol "git")
endif()

#===== Macro set paths ===============================================
macro( SetPathsRecompile )
  foreach( tool ${Tools} )
 #   set(InstallPath ${CMAKE_INSTALL_PREFIX}) # Non cache variable so its value can change and be updated
    set(TOOL${tool} ${CMAKE_INSTALL_PREFIX}/${tool} CACHE STRING "Path to the ${tool} executable")
    get_filename_component(${tool}Path ${TOOL${tool}} REALPATH ABSOLUTE) # Set the real path in the config file
#    set(${tool}Path ${TOOL${tool}}) # ${proj}Path =  variable changed in the DTIAB config file (non cache)
    mark_as_advanced(CLEAR TOOL${tool}) # Show the option in the gui
    if(DEFINED TOOL${tool}Sys)
      mark_as_advanced(FORCE TOOL${tool}Sys) # Hide the unuseful option in the gui
    endif()
  endforeach()
endmacro( SetPathsRecompile )

macro( SetPathsSystem )
  foreach( tool ${Tools} )
    get_filename_component(${tool}Path ${TOOL${tool}}Sys REALPATH ABSOLUTE) # Set the real path in the config file
#    set(${tool}Path ${TOOL${tool}Sys})
    mark_as_advanced(CLEAR TOOL${tool}Sys) # Show the option in the gui
    if(DEFINED TOOL${tool})
      mark_as_advanced(FORCE TOOL${tool}) # Hide the option in the gui
    endif()
  endforeach()
endmacro( SetPathsSystem )

#===== Macro search tools ===============================================
macro( FindToolsMacro Proj )
  set( AllToolsFound ON )
  foreach( tool ${Tools} )
    find_program( TOOL${tool}Sys ${tool}) # search TOOL${tool}Sys in the PATH
    if(${TOOL${tool}Sys} STREQUAL "TOOL${tool}Sys-NOTFOUND") # If program not found, give a warning message and set AllToolsFound variable to OFF
      message( WARNING "${tool} not found. It will not be recompiled, so either set it to ON, or get ${Proj} manually." )
      set( AllToolsFound OFF )
    endif() # Found on system
  endforeach()
endmacro()

#===== Macro add tool ===============================================
 # if SourceCodeArgs or CMAKE_ExtraARGS passed to the macro as arguments, only the first word is used (each element of the list is taken as ONE argument) => use as "global variables"
macro( AddToolMacro Proj CLI) # CLI = Used if Slicer Extension : ON if CLI and OFF if NotCLI

  # Update and test tools
  if(COMPILE_EXTERNAL_${Proj}) # If need to recompile, just set the paths here
    SetPathsRecompile() # Uses the list "Tools"
  else(COMPILE_EXTERNAL_${Proj}) # If no need to recompile, search the tools on the system and need to recompile if some tool not found

    # search the tools on the system and warning if not found
    # If SlicerExtension, OFF packages are already in Slicer but can be not found -> don't recompile
    if( NOT DTIAtlasBuilder_BUILD_SLICER_EXTENSION )
      FindToolsMacro( ${Proj} )
    endif( NOT DTIAtlasBuilder_BUILD_SLICER_EXTENSION )

    # If some program not found, reset all tools to the recompiled path and recompile the whole package
    if(NOT AllToolsFound) # AllToolsFound set or reset in FindToolsMacro()
#      set( COMPILE_EXTERNAL_${Proj} ON CACHE BOOL "" FORCE)
      SetPathsRecompile() # Uses the list "Tools"
    else()
      SetPathsSystem() # Uses the list "Tools"
    endif()

  endif(COMPILE_EXTERNAL_${Proj})

  # After the main if() because we could need to recompile after not having found all tools on system
  if(COMPILE_EXTERNAL_${Proj})
    # Add project
    ExternalProject_Add(${Proj}
      ${SourceCodeArgs} # No difference between args passed separated with ';', spaces or return to line
      BINARY_DIR ${Proj}-build
      SOURCE_DIR ${Proj} # creates the folder if it doesn't exist
      CMAKE_GENERATOR ${gen}
      CMAKE_ARGS
        ${LOCAL_CMAKE_BUILD_OPTIONS}
        -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
        -DCMAKE_INSTALL_PREFIX:PATH=${Proj}-install
        ${CMAKE_ExtraARGS}
      INSTALL_COMMAND "" # So the install step of the external project is not done
    )

    # Install step : copy all needed executables to ${INSTALL_DIR} or ${NOCLI_INSTALL_DIR}
    if(NOT ${Proj} STREQUAL "MriWatcher") # MriWatcher is not in a ./bin directory -> install step specified manually after calling macro

      if( DTIAtlasBuilder_BUILD_SLICER_EXTENSION ) # check if variable if defined, and not differ if ON or OFF

        if(${CLI}) # Install in Extensions/DTIAtlaBuilder/lib/Slicer4.X/cli_module
          foreach( tool ${Tools} )
              install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${Proj}-build/bin/${tool} DESTINATION ${INSTALL_DIR})
          endforeach()
        else(${CLI}) # Install in Extensions/DTIAtlaBuilder/bin
          foreach( tool ${Tools} )
              install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${Proj}-build/bin/${tool} DESTINATION ${NOCLI_INSTALL_DIR})
          endforeach()
        endif(${CLI})

      else( DTIAtlasBuilder_BUILD_SLICER_EXTENSION )

        foreach( tool ${Tools} )
            install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${Proj}-build/bin/${tool} DESTINATION ${INSTALL_DIR})
        endforeach()

      endif( DTIAtlasBuilder_BUILD_SLICER_EXTENSION )

    endif() # NOT ${tool} STREQUAL "MriWatcher"

  endif(COMPILE_EXTERNAL_${Proj})
endmacro( AddToolMacro )

#====================================================================
#====================================================================
## Libraries for tools =============================================

# VTK
find_package(VTK REQUIRED)
if (VTK_FOUND)
  set(VTK_USE_QVTK TRUE)
  set(VTK_USE_GUISUPPORT TRUE)
  include(${VTK_USE_FILE}) # creates VTK_DIR
else(VTK_FOUND)
  message(FATAL_ERROR, "VTK not found. Please set VTK_DIR.")
endif (VTK_FOUND)

# FFTW and LAPACK for GreedyAtlas
if(COMPILE_EXTERNAL_AtlasWerks) # FFTW D + F build one on(after) another
  # FFTW
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CMake/InstallFFTW.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/FFTW-install/InstallFFTW.cmake @ONLY) # @ONLY to ignore ${} variables
  set(FFTW_URL "http://www.fftw.org/fftw-3.3.3.tar.gz")
  set(FFTW_URL_MD5 0a05ca9c7b3bfddc8278e7c40791a1c2)
  ExternalProject_Add(FFTWD    # FFTW has no CMakeLists.txt # Example : Slicer/SuperBuild/External_python.cmake
    URL ${FFTW_URL}
    URL_MD5 ${FFTW_URL_MD5}
    DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}/FFTW-install
    SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/FFTW
    BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/FFTW-build
    CONFIGURE_COMMAND ""
    INSTALL_COMMAND ""
    BUILD_COMMAND ${CMAKE_COMMAND} -DFFTWTYPE:STRING=FFTWD -P ${CMAKE_CURRENT_BINARY_DIR}/FFTW-install/InstallFFTW.cmake # -DARGNAME:TYPE=VALUE -P <cmake file> = Process script mode
    )
  ExternalProject_Add(FFTWF    # FFTW has no CMakeLists.txt # Example : Slicer/SuperBuild/External_python.cmake
    URL ${FFTW_URL}
    URL_MD5 ${FFTW_URL_MD5}
    DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}/FFTW-install # put the archive in the install directory
    SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/FFTW
    BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/FFTW-build
    CONFIGURE_COMMAND ""
    INSTALL_COMMAND ""
    BUILD_COMMAND ${CMAKE_COMMAND} -DFFTWTYPE:STRING=FFTWF -P ${CMAKE_CURRENT_BINARY_DIR}/FFTW-install/InstallFFTW.cmake # -DARGNAME:TYPE=VALUE -P <cmake file> = Process script mode
    DEPENDS FFTWD # So they are not done at the same time if threads
    )
  set(FFTW_DIR ${CMAKE_CURRENT_BINARY_DIR}/FFTW-install)

  # LAPACK (from http://www.nitrc.org/projects/spharm-pdm)
  ExternalProject_Add(LAPACK
    URL ${lapack_file}
    URL_MD5 ${lapack_md5}
    DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}/LAPACK-build
    SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/LAPACK
    BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/LAPACK-build
    CMAKE_GENERATOR ${gen}
    CMAKE_ARGS
#      ${COMMON_EXTERNAL_PROJECT_ARGS}
      -DBUILD_EXAMPLES:BOOL=OFF
      -DBUILD_TESTING:BOOL=OFF
#      ${${proj}_CMAKE_OPTIONS}
    INSTALL_COMMAND ""
    DEPENDS ""
    BUILD_COMMAND ${BUILD_COMMAND_STRING}
    )
  install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/LAPACK-build/lib/liblapack.so.3 DESTINATION ${NOCLI_INSTALL_DIR}) # liblapack.so will be found in the same directory than GreedyAtlas
endif(COMPILE_EXTERNAL_AtlasWerks)

# ITK and SlicerExecutionModel
set(RecompileITK OFF)
set(RecompileSEM OFF)

if(COMPILE_EXTERNAL_dtiprocessTK OR COMPILE_EXTERNAL_BRAINS OR COMPILE_EXTERNAL_ANTS OR COMPILE_EXTERNAL_ResampleDTI) # If one of these tools needed, need to recompile ITK so the version is OK
  set(RecompileITK ON) # Automatically recompile SlicerExecutionModel
else() # If no need, recompile ITK or SEM only if not found
  find_package(ITK) # Not required because will be recompiled if not found
  if(ITK_FOUND)
    include(${ITK_USE_FILE}) # set ITK_DIR

    # If ITK not recompiled, look for SlicerExecutionModel
    find_package(SlicerExecutionModel) # Not required because will be recompiled if not found
    if(SlicerExecutionModel_FOUND)
      include(${SlicerExecutionModel_USE_FILE}) # creates SlicerExecutionModel_DIR (DTI-Reg & BRAINSFit)
    else(SlicerExecutionModel_FOUND)
      message(WARNING "SlicerExecutionModel not found. It will be downloaded and recompiled.")
      set(RecompileSEM ON)
    endif(SlicerExecutionModel_FOUND)

  else(ITK_FOUND)
    message(WARNING "ITK not found. ITKv4.3.0 will be downloaded and recompiled.")
    set(RecompileITK ON) # Automatically recompile SlicerExecutionModel
  endif(ITK_FOUND)
endif() # COMPILE_EXTERNAL_dtiprocessTK OR COMPILE_EXTERNAL_BRAINS OR COMPILE_EXTERNAL_ANTS OR COMPILE_EXTERNAL_ResampleDTI

set(ITK_DEPEND "")
set(RecompileBatchMake OFF)
if(RecompileITK)
  # Download and compile ITKv4
  ExternalProject_Add(ITKv4 # BRAINSStandAlone/SuperBuild/External_ITKv4.cmake # !! All args needed as they are
    GIT_REPOSITORY "${git_protocol}://itk.org/ITK.git"
    GIT_TAG 555049f830d1c09f8d4d95904f429290467d39ab #2012-12-16 ITKv4.3.0
    SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/ITKv4
    BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/ITKv4-build
    CMAKE_GENERATOR ${gen}
    CMAKE_ARGS # !! ALL options need to be here for all tools to compile with this version of ITK
      -Wno-dev
      --no-warn-unused-cli
      -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
      -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}/ITKv4-install
      -DBUILD_EXAMPLES:BOOL=OFF
      -DBUILD_TESTING:BOOL=OFF
      -DBUILD_SHARED_LIBS:BOOL=OFF
      -DITK_LEGACY_REMOVE:BOOL=OFF
      -DITKV3_COMPATIBILITY:BOOL=ON
      -DITK_BUILD_ALL_MODULES:BOOL=ON
      -DITK_USE_REVIEW:BOOL=OFF # ON ok with BRAINSFit and ANTS not with dtiprocess and ResampleDTI # OFF ok with BRAINSFit
      -DKWSYS_USE_MD5:BOOL=ON # Required by SlicerExecutionModel
      -DUSE_WRAP_ITK:BOOL=OFF ## HACK:  QUICK CHANGE
      -DITK_USE_SYSTEM_DCMTK:BOOL=OFF
      -DCMAKE_LIBRARY_OUTPUT_DIRECTORY:PATH=${CMAKE_CURRENT_BINARY_DIR}/ITKv4-build/lib # Needed for BRAINSTools to compile
      -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY:PATH=${CMAKE_CURRENT_BINARY_DIR}/ITKv4-build/lib # Needed for BRAINSTools to compile
      -DCMAKE_RUNTIME_OUTPUT_DIRECTORY:PATH=${CMAKE_CURRENT_BINARY_DIR}/ITKv4-build/bin # Needed for BRAINSTools to compile
    INSTALL_COMMAND ""
    )
  set(ITK_DIR ${CMAKE_CURRENT_BINARY_DIR}/ITKv4-build) # Use the downloaded ITK for all tools
  set(ITK_DEPEND ITKv4)
  set(RecompileSEM ON) # If recompile ITK, recompile SlicerExecutionModel
  set(RecompileBatchMake ON) # If recompile ITK, recompile BatchMake
endif(RecompileITK)

if(RecompileSEM)
  # Download and compile SlicerExecutionModel with the downloaded ITKv4
  ExternalProject_Add(SlicerExecutionModel # BRAINSStandAlone/SuperBuild/External_SlicerExecutionModel.cmake
    GIT_REPOSITORY "${git_protocol}://github.com/Slicer/SlicerExecutionModel.git"
    GIT_TAG 7365853e2b88b832831fc0e9b90f1720ec0edbbb
    SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/SlicerExecutionModel
    BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/SlicerExecutionModel-build
    CMAKE_GENERATOR ${gen}
    CMAKE_ARGS
      -Wno-dev
      --no-warn-unused-cli
      -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
      -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}/SlicerExecutionModel-install
      -DBUILD_SHARED_LIBS:BOOL=OFF
      -DBUILD_EXAMPLES:BOOL=OFF
      -DBUILD_TESTING:BOOL=OFF
      -DITK_DIR:PATH=${ITK_DIR}
    INSTALL_COMMAND ""
    DEPENDS ${ITK_DEPEND} # either ITKv4 if recompiled, or empty
    )
  set(SlicerExecutionModel_DIR ${CMAKE_CURRENT_BINARY_DIR}/SlicerExecutionModel-build) # Use the downloaded SlicerExecutionModel for all tools
  set(GenerateCLP_DIR ${SlicerExecutionModel_DIR}/GenerateCLP)
  set(ModuleDescriptionParser_DIR ${SlicerExecutionModel_DIR}/ModuleDescriptionParser)
  set(TCLAP_DIR ${SlicerExecutionModel_DIR}/tclap)
  list(APPEND ITK_DEPEND SlicerExecutionModel)
endif(RecompileSEM)

# BatchMake for DTI-Reg (after ITK)
set(BatchMake_DEPEND "")
if(COMPILE_EXTERNAL_DTIReg) # BatchMake only needed for DTIReg
  find_package(BatchMake) # Not required because will be recompiled if not found
  if(BatchMake_FOUND)
    include(${BatchMake_USE_FILE})
  else(BatchMake_FOUND)
    message(WARNING "BatchMake not found. It will be downloaded and recompiled.")
    set(RecompileBatchMake ON) # If not found, recompile it
  endif(BatchMake_FOUND )
endif(COMPILE_EXTERNAL_DTIReg)

if(RecompileBatchMake)
    ExternalProject_Add(BatchMake
      GIT_REPOSITORY ${git_protocol}://batchmake.org/BatchMake.git
      GIT_TAG "0abb2faca1251f808ab3d0b820cc27b570a994f1" # 08-26-2012 updated for ITKv4 # "43d21fcccd09e5a12497bc1fb924bc6d5718f98c" # used in DTI-Reg 12-21-2012
      SOURCE_DIR BatchMake
      BINARY_DIR BatchMake-build
      CMAKE_GENERATOR ${gen}
      CMAKE_ARGS
        -DBUILD_SHARED_LIBS:BOOL=OFF
        -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
        -DBUILD_TESTING:BOOL=OFF
        -DUSE_FLTK:BOOL=OFF
        -DDASHBOARD_SUPPORT:BOOL=OFF
        -DGRID_SUPPORT:BOOL=OFF
        -DUSE_SPLASHSCREEN:BOOL=OFF
        -DITK_DIR:PATH=${ITK_DIR}
      INSTALL_COMMAND ""
      DEPENDS ${ITK_DEPEND}
      )
    set(BatchMake_DIR ${CMAKE_CURRENT_BINARY_DIR}/BatchMake-build)
    set(BatchMake_ITK_DIR ${ITK_DIR}) # If batchmake recompiled, no include(${BatchMake_USE_FILE}) has been done so BatchMake_ITK_DIR does not exist, and we used ${ITK_DIR} to compile it.
    set(BatchMake_DEPEND BatchMake)
endif(RecompileBatchMake)

#====================================================================
#===== TOOLS ========================================================
#       ||
#       VV

### For Slicer Extension:
## CLI Modules # CLI= (pgm) --xml exists # So the cli_modules go to Extensions/DTIAtlaBuilder/lib/Slicer4.2/cli_module
# dtiprocessTK
# ResampleDTI
# DTIReg
## No CLI Modules # So the non cli_modules don't go to Extensions/DTIAtlaBuilder/lib/Slicer4.2/cli_module but to Extensions/DTIAtlaBuilder/bin
# AtlasWerks
# BRAINS # BRAINS is a cli_module but needs to be in non cli_module to prevent conflict with actual Slicer's BRAINS
# ANTS
# MriWatcher
# NIRALUtilities
# teem # teem is in Slicer but not a cli_module

# ===== dtiprocessTK ==============================================================
set( SourceCodeArgs
  SVN_REPOSITORY "https://www.nitrc.org/svn/dtiprocess/branches/Slicer4Extension/dtiprocess"
  SVN_USERNAME slicerbot
  SVN_PASSWORD slicer
  SVN_REVISION -r 113 # 12/20/2012 updated for ITKv4.3.0
  )
set( CMAKE_ExtraARGS
  -DBUILD_TESTING:BOOL=OFF
  -DITK_DIR:PATH=${ITK_DIR}
  -DVTK_DIR:PATH=${VTK_DIR}
  -DGenerateCLP_DIR:PATH=${GenerateCLP_DIR}
  -DModuleDescriptionParser_DIR:PATH=${ModuleDescriptionParser_DIR}
  -DTCLAP_DIR:PATH=${TCLAP_DIR}
  -DSlicerExecutionModel_DIR:PATH=${SlicerExecutionModel_DIR}
  DEPENDS ${ITK_DEPEND}
  )
set( Tools
  dtiprocess
  dtiaverage
  )
AddToolMacro( dtiprocessTK OFF) # AddToolMacro( proj CLI) + uses SourceCodeArgs CMAKE_ExtraARGS Tools

# ===== AtlasWerks ================================================================
# code for external tools from https://github.com/Chaircrusher/AtlasWerksBuilder/blob/master/CMakeLists.txt
set( SourceCodeArgs
  URL "http://www.sci.utah.edu/releases/atlaswerks_v0.1.4/AtlasWerks_0.1.4_Linux.tgz"
  URL_MD5 05fc867564e3340d0d448dd0daab578a
  )
set( CMAKE_ExtraARGS
  -DITK_DIR:PATH=${ITK_DIR}
  -DVTK_DIR:PATH=${VTK_DIR}
#  -DFFTW_INSTALL_BASE_PATH:PATH=${FFTW_DIR} # will use find_library to find the libs
#  -DFFTWF_LIB:PATH=${FFTW_DIR}/lib/libfftw3f.a # FFTW in float
#  -DFFTWD_LIB:PATH=${FFTW_DIR}/lib/libfftw3.a # FFTW in double # needed for AtlasWerks to configure, not to compile with
#  -DFFTWF_THREADS_LIB:PATH=${FFTW_DIR}/lib/libfftw3f_threads.a
#  -DFFTWD_THREADS_LIB:PATH=${FFTW_DIR}/lib/libfftw3_threads.a
  -DFFTW_INCLUDE_PATH:PATH=${FFTW_DIR}/include # will be used to set FFTW_INSTALL_BASE_PATH by finding the path = remove the /include
  -DAtlasWerks_COMPILE_TESTING:BOOL=OFF
  -DatlasWerks_COMPILE_APP_Affine:BOOL=OFF
  -DatlasWerks_COMPILE_APP_AffineAtlas:BOOL=OFF
  -DatlasWerks_COMPILE_APP_ATLAS_WERKS:BOOL=OFF
  -DatlasWerks_COMPILE_APP_VECTOR_ATLAS_WERKS:BOOL=OFF
  -DatlasWerks_COMPILE_APP_FGROWTH:BOOL=OFF
  -DatlasWerks_COMPILE_APP_FWARP:BOOL=OFF
  -DatlasWerks_COMPILE_APP_ImageConvert:BOOL=OFF
  -DatlasWerks_COMPILE_APP_IMMAP:BOOL=OFF
  -DatlasWerks_COMPILE_APP_LDMM:BOOL=OFF
  -DatlasWerks_COMPILE_APP_GREEDY:BOOL=ON  # Compile Only GreedyAtlas
  -DatlasWerks_COMPILE_APP_TX_APPLY:BOOL=OFF
  -DatlasWerks_COMPILE_APP_TX_WERKS:BOOL=OFF
  -DatlasWerks_COMPILE_APP_UTILITIES:BOOL=OFF
  DEPENDS ${ITK_DEPEND} FFTWD FFTWF LAPACK # Not CMake Arg -> directly after CMakeArg in ExternalProject_Add()
  )
set( Tools
  GreedyAtlas
  )
AddToolMacro( AtlasWerks OFF) # AddToolMacro( proj CLI) + uses SourceCodeArgs CMAKE_ExtraARGS Tools

# ===== BRAINSFit =============================================================
set( SourceCodeArgs
  GIT_REPOSITORY "${git_protocol}://github.com/BRAINSia/BRAINSStandAlone.git"
  GIT_TAG "dd7ad3926a01fbdd098ea858fb95012ca16fb236" # 12/18/2012
# "ff94032edafbc46a95f51db4bce894f0120b5992" : Slicer4 version # /devel/linux/Slicer4_linux64/Slicer/SuperBuild/External_BRAINSTools.cmake -> compiles but segfault
# "31dcba577ee1bac5c4680fc9d7c830d6074020a9" : 12/13/2012
# "98a46a2b08da882d46f04cbf0d539c2b73348049" # version from https://www.nitrc.org/svn/dtiprep/trunk/SuperBuild/External_BRAINSTools.cmake -> compiles but run error "undefined symbol: ModuleEntryPoint"
  )

set( CMAKE_ExtraARGS
  -DBUILD_TESTING:BOOL=OFF
  -DBUILD_SHARED_LIBS:BOOL=OFF
  -DBRAINSTools_SUPERBUILD:BOOL=OFF
  -DSuperBuild_BRAINSTools_USE_GIT:BOOL=${USE_GIT_PROTOCOL}
  -DITK_VERSION_MAJOR:STRING=4
  -DITK_DIR:PATH=${ITK_DIR}
  -DVTK_DIR:PATH=${VTK_DIR}
  -DUSE_SYSTEM_ITK=ON
  -DUSE_SYSTEM_SlicerExecutionModel=ON
  -DUSE_SYSTEM_VTK=ON
  -DGenerateCLP_DIR:PATH=${GenerateCLP_DIR}
  -DModuleDescriptionParser_DIR:PATH=${ModuleDescriptionParser_DIR}
  -DSlicerExecutionModel_DIR:PATH=${SlicerExecutionModel_DIR}
  -DBUILD_TESTING:BOOL=OFF
  -DUSE_BRAINSDemonWarp:BOOL=ON
  -DUSE_BRAINSFit:BOOL=ON
  -DUSE_BRAINSResample:BOOL=OFF
  -DUSE_AutoWorkup:BOOL=OFF
  -DUSE_ANTS:BOOL=OFF
  -DUSE_BRAINSContinuousClass:BOOL=OFF
  -DUSE_BRAINSFitEZ:BOOL=OFF
  -DUSE_BRAINSROIAuto:BOOL=OFF
  -DUSE_BRAINSSurfaceTools:BOOL=OFF
  -DUSE_DebugImageViewer:BOOL=OFF
  -DUSE_BRAINSABC:BOOL=OFF
  -DUSE_BRAINSConstellationDetector:BOOL=OFF
  -DUSE_BRAINSCreateLabelMapFromProbabilityMaps:BOOL=OFF
  -DUSE_BRAINSCut:BOOL=OFF
  -DUSE_BRAINSImageConvert:BOOL=OFF
  -DUSE_BRAINSInitializedControlPoints:BOOL=OFF
  -DUSE_BRAINSLandmarkInitializer:BOOL=OFF
  -DUSE_BRAINSMultiModeSegment:BOOL=OFF
  -DUSE_BRAINSMush:BOOL=OFF
  -DUSE_BRAINSSnapShotWriter:BOOL=OFF
  -DUSE_BRAINSTransformConvert:BOOL=OFF
  -DUSE_ConvertBetweenFileFormats:BOOL=OFF
  -DUSE_DWIConvert:BOOL=OFF
  -DUSE_DebugImageViewer:BOOL=OFF
  -DUSE_ICCDEF:BOOL=OFF
  -DUSE_ImageCalculator:BOOL=OFF
  -DUSE_GTRACT:BOOL=OFF
  -DLOCAL_SEM_EXECUTABLE_ONLY:BOOL=ON # Variable used in SlicerExecutionModel/CMake/SEMMacroBuildCLI.cmake:l.120 : if true, will only create executable without shared lib lib(exec)Lib.so
  DEPENDS ${ITK_DEPEND} # So ITK is compiled before
  PATCH_COMMAND patch -p0 -d ${CMAKE_CURRENT_BINARY_DIR} -i ${CMAKE_CURRENT_SOURCE_DIR}/CMake/BRAINS.patch # !! no "" # !! patch doesn't exist on windows !
  )
set( Tools
  BRAINSFit
  BRAINSDemonWarp
  )
AddToolMacro( BRAINS OFF) # AddToolMacro( proj CLI) + uses SourceCodeArgs CMAKE_ExtraARGS Tools

# ===== ANTS/WarpMultiTransform =====================================================
set( SourceCodeArgs
  SVN_REPOSITORY "https://advants.svn.sourceforge.net/svnroot/advants/trunk"
  SVN_USERNAME slicerbot
  SVN_PASSWORD slicer
  SVN_REVISION -r 1685 # 12/13/2012
  )
set( CMAKE_ExtraARGS
  -DBUILD_TESTING:BOOL=OFF
  -DBUILD_SHARED_LIBS:BOOL=OFF
  -DANTS_SUPERBUILD:BOOL=OFF
  -DSuperBuild_ANTS_USE_GIT_PROTOCOL:BOOL=${USE_GIT_PROTOCOL}
  -DUSE_SYSTEM_ITK:BOOL=ON
  -DITK_DIR:PATH=${ITK_DIR}
  -DITK_VERSION_MAJOR:STRING=4
  -DUSE_SYSTEM_SlicerExecutionModel:BOOL=ON
  -DSlicerExecutionModel_DIR:PATH=${SlicerExecutionModel_DIR}
  -DUSE_VTK:BOOL=OFF
  -DVTK_DIR:PATH=${VTK_DIR}
  DEPENDS ${ITK_DEPEND} 
  )
set( Tools
  ANTS
  WarpImageMultiTransform
  WarpTensorImageMultiTransform
  )
AddToolMacro( ANTS OFF) # AddToolMacro( proj CLI) + uses SourceCodeArgs CMAKE_ExtraARGS Tools

# ===== ResampleDTIlogEuclidean =====================================================
set( SourceCodeArgs
  GIT_REPOSITORY "${git_protocol}://github.com/booboo69500/ResampleDTIlogEuclidean.git" # Francois Budin's repositery
  GIT_TAG "e78a9ea00d73a11cc52b3c457e32f1302a3403d4" # 12/20/2012
#  URL "http://www.insight-journal.org/download/sourcecode/742/11/SourceCode11_ResampleDTIInsightJournal2.tar.gz"
# https://github.com/midas-journal/midas-journal-742/tree/master/ResampleDTIInsightJournal
  )
set( CMAKE_ExtraARGS
  -DBUILD_TESTING:BOOL=OFF
  -DBUILD_GENERATECLP:BOOL=OFF
  -DITK_DIR:PATH=${ITK_DIR}
  -DGenerateCLP_DIR:PATH=${GenerateCLP_DIR}
  DEPENDS ${ITK_DEPEND}
  )
set( Tools
  ResampleDTIlogEuclidean
  )
AddToolMacro( ResampleDTI ON) # AddToolMacro( proj CLI) + uses SourceCodeArgs CMAKE_ExtraARGS Tools

# ===== DTI-Reg =====================================================================
set( SourceCodeArgs
  SVN_REPOSITORY "https://www.nitrc.org/svn/dtireg/trunk"
  SVN_USERNAME slicerbot
  SVN_PASSWORD slicer
  SVN_REVISION -r 26 # 12/20/2012
  )
set( CMAKE_ExtraARGS
  -DANTSTOOL:PATH=${ANTSPath}
  -DBRAINSDemonWarpTOOL:PATH=${BRAINSDemonWarpPath}
  -DBRAINSFitTOOL:PATH=${BRAINSFitPath}
  -DBatchMake_DIR:PATH=${BatchMake_DIR}
  -DITK_DIR:PATH=${BatchMake_ITK_DIR} # Compile DTI-Reg with the same ITK than Batchmake (BatchMake_ITK_DIR is in BatchMake_USE_FILE) -> BatchMakeLib and ITK_LIBRARIES contain the same ITK libs
  -DSlicerExecutionModel_DIR:PATH=${SlicerExecutionModel_DIR}
  -DGenerateCLP_DIR:PATH=${GenerateCLP_DIR}
  -DModuleDescriptionParser_DIR:PATH=${ModuleDescriptionParser_DIR}
  -DTCLAP_DIR:PATH=${TCLAP_DIR}
  -DCOMPILE_EXTERNAL_dtiprocess:BOOL=OFF
  -DOPT_USE_SYSTEM_BatchMake:BOOL=ON
  -DOPT_USE_SYSTEM_ITK:BOOL=ON
  -DOPT_USE_SYSTEM_SlicerExecutionModel:BOOL=ON
  -DResampleDTITOOL:PATH=${ResampleDTIlogEuclideanPath}
  -DWARPIMAGEMULTITRANSFORMTOOL:PATH=${WarpImageMultiTransformPath}
  -DWARPTENSORIMAGEMULTITRANSFORMTOOL:PATH=${WarpTensorImageMultiTransformPath}
  -DdtiprocessTOOL:PATH=${dtiprocessPath}
  DEPENDS ${ITK_DEPEND} ${BatchMake_DEPEND}
  )
set( Tools
  DTI-Reg
  )
AddToolMacro( DTIReg ON ) # AddToolMacro( proj CLI) + uses SourceCodeArgs CMAKE_ExtraARGS Tools

# ===== teem (unu) =====================================================================
set( SourceCodeArgs
  SVN_REPOSITORY "https://teem.svn.sourceforge.net/svnroot/teem/teem/trunk"
  SVN_USERNAME slicerbot
  SVN_PASSWORD slicer
  SVN_REVISION -r 5888 # 12/13/2012
  )
set( CMAKE_ExtraARGS
  -DITK_DIR:PATH=${ITK_DIR}
  DEPENDS ${ITK_DEPEND}
  )
set( Tools
  unu
  )
AddToolMacro( teem OFF) # AddToolMacro( proj CLI) + uses SourceCodeArgs CMAKE_ExtraARGS Tools

# ===== MriWatcher =====================================================================
set( SourceCodeArgs
  SVN_REPOSITORY "https://www.nitrc.org/svn/mriwatcher/branches/mriwatcher_qt4"
  SVN_USERNAME slicerbot
  SVN_PASSWORD slicer
  SVN_REVISION -r 16 # 12/13/2012
  )
set( CMAKE_ExtraARGS
  -DQT_QMAKE_EXECUTABLE:PATH=${QT_QMAKE_EXECUTABLE}
  -DITK_DIR:PATH=${ITK_DIR}
  DEPENDS ${ITK_DEPEND}
  )
set( Tools
  MriWatcher
  )
AddToolMacro( MriWatcher OFF) # AddToolMacro( proj CLI) + uses SourceCodeArgs CMAKE_ExtraARGS Tools
if(COMPILE_EXTERNAL_MriWatcher)
  if( DTIAtlasBuilder_BUILD_SLICER_EXTENSION ) 
    install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/MriWatcher-build/MriWatcher DESTINATION ${NOCLI_INSTALL_DIR})
  else( DTIAtlasBuilder_BUILD_SLICER_EXTENSION ) 
    install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/MriWatcher-build/MriWatcher DESTINATION ${INSTALL_DIR}) # Specified manually because not in a ./bin directory
  endif( DTIAtlasBuilder_BUILD_SLICER_EXTENSION ) 
endif()

# ===== NIRALUtilities ===================================================================
set( SourceCodeArgs
  SVN_REPOSITORY "https://www.nitrc.org/svn/niral_utilities/trunk"
  SVN_USERNAME slicerbot
  SVN_PASSWORD slicer
  SVN_REVISION -r 38 # 12/13/2012
  )
set( CMAKE_ExtraARGS
  -DCOMPILE_CONVERTITKFORMATS:BOOL=OFF
  -DCOMPILE_CROPTOOLS:BOOL=ON
  -DCOMPILE_CURVECOMPARE:BOOL=OFF
  -DCOMPILE_DTIAtlasBuilder:BOOL=OFF
  -DCOMPILE_DWI_NIFTINRRDCONVERSION:BOOL=OFF
  -DCOMPILE_IMAGEMATH:BOOL=ON
  -DCOMPILE_IMAGESTAT:BOOL=OFF
  -DCOMPILE_POLYDATAMERGE:BOOL=OFF
  -DCOMPILE_POLYDATATRANSFORM:BOOL=OFF
  -DCOMPILE_TRANSFORMDEFORMATIONFIELD:BOOL=OFF
  -DITK_DIR:PATH=${ITK_DIR}
  -DVTK_DIR:PATH=${VTK_DIR}
  -DGenerateCLP_DIR:PATH=${GenerateCLP_DIR}
  -DModuleDescriptionParser_DIR:PATH=${ModuleDescriptionParser_DIR}
  -DTCLAP_DIR:PATH=${TCLAP_DIR}
  DEPENDS ${ITK_DEPEND}
  )
set( Tools
  ImageMath
  CropDTI
  )
AddToolMacro( NIRALUtilities OFF) # AddToolMacro( proj CLI) + uses SourceCodeArgs CMAKE_ExtraARGS Tools

