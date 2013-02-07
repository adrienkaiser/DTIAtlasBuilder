cmake_minimum_required(VERSION 2.8)
CMAKE_POLICY(VERSION 2.8)

# As the external project gives this CMakeLists the paths to the needed libraries (*_DIR), find_package will just use the existing *_DIR
find_package(ITK REQUIRED)
if(ITK_FOUND)
  include(${ITK_USE_FILE}) # creates ITK_DIR
else(ITK_FOUND)
  message(FATAL_ERROR "ITK not found. Please set ITK_DIR")
endif(ITK_FOUND)

find_package(GenerateCLP REQUIRED)
if(GenerateCLP_FOUND)
  include(${GenerateCLP_USE_FILE}) # creates GenerateCLP_DIR ModuleDescriptionParser_DIR TCLAP_DIR
else(GenerateCLP_FOUND)
  message(FATAL_ERROR "GenerateCLP not found. Please set GenerateCLP_DIR")
endif(GenerateCLP_FOUND)

find_package(Qt4 REQUIRED) # For DTIAtlasBuilder
if(QT_USE_FILE)
  include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${QT_INCLUDE_DIR})
  include(${QT_USE_FILE}) # creates QT_QMAKE_EXECUTABLE
  add_definitions(-DQT_GUI_LIBS -DQT_CORE_LIB -DQT3_SUPPORT)
else(QT_USE_FILE)
  message(FATAL_ERROR, "QT not found. Please set QT_DIR.")
endif(QT_USE_FILE)

#======================================================================================
# Compile step for DTIAtlasBuilder
if(DTIAtlasBuilder_BUILD_SLICER_EXTENSION) # to configure GUI.cxx
  set(SlicerExtCXXVar "true")
else(DTIAtlasBuilder_BUILD_SLICER_EXTENSION)
  set(SlicerExtCXXVar "false")
endif(DTIAtlasBuilder_BUILD_SLICER_EXTENSION)

if(APPLE) # to configure GUI.cxx
  set(Platform "mac")
elseif(WIN32)
  set(Platform "win")
else()
  set(Platform "linux")
endif()

QT4_WRAP_CPP(QtProject_HEADERS_MOC GUI.h)
QT4_WRAP_UI(UI_FILES GUIwindow.ui)
configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/GUI.cxx.in ${CMAKE_CURRENT_BINARY_DIR}/GUI.cxx ) # configure and copy : to set SlicerExtCXXVar (DTIAtlasBuilder_BUILD_SLICER_EXTENSION is "ON" or "OFF" -> not in c++)
set(DTIABsources DTIAtlasBuilder.cxx GUI.h ${CMAKE_CURRENT_BINARY_DIR}/GUI.cxx ScriptWriter.h ScriptWriter.cxx ${QtProject_HEADERS_MOC} ${UI_FILES})
GENERATECLP(DTIABsources DTIAtlasBuilder.xml) # include the GCLP file to the project
add_executable(DTIAtlasBuilder ${DTIABsources})  # add the files contained by "DTIABsources" to the project
target_link_libraries(DTIAtlasBuilder ${QT_LIBRARIES} ${ITK_LIBRARIES})
install(TARGETS DTIAtlasBuilder DESTINATION ${INSTALL_DIR}) # same if Slicer Ext or not

#======================================================================================
# For Slicer Extension -> will create target "ExperimentalUpload" inside inner build. !! Needs to be before add testing
if( DTIAtlasBuilder_BUILD_SLICER_EXTENSION )
  # Unset variables because Slicer will try to set it again and ERROR
  set( ITK_DIR_TMP ${ITK_DIR} )
  unset( ITK_DIR CACHE )
  unset( ITK_DIR )
  set( ITK_LIBRARIES_TMP ${ITK_LIBRARIES} )
  unset( ITK_LIBRARIES CACHE )
  unset( ITK_LIBRARIES )
  set( SlicerExecutionModel_DIR_TMP ${SlicerExecutionModel_DIR} )
  unset( SlicerExecutionModel_DIR )
  unset( SlicerExecutionModel_DIR CACHE )

  find_package(Slicer REQUIRED)
  include(${Slicer_USE_FILE})

  set( ITK_DIR ${ITK_DIR_TMP} CACHE PATH "ITK PATH" FORCE )
  set( ITK_LIBRARIES ${ITK_LIBRARIES_TMP} CACHE PATH "ITK PATH" FORCE )
  set( SlicerExecutionModel_DIR ${SlicerExecutionModel_DIR_TMP} CACHE PATH "SlicerExecutionModel PATH" FORCE )

  # Python # Needed to use the python compiled with Slicer # "PYTHON_EXECUTABLE" given in SlicerConfig.cmake when find_package(Slicer REQUIRED)
  if(WIN32)
    set(SlicerPythonExec ${PYTHON_EXECUTABLE})
  else(WIN32) # On Unix, "PYTHON_EXECUTABLE" is customPython* -> replace "customPython" by "python" (which also exists)
    get_filename_component(SlicerPythonExecDir ${PYTHON_EXECUTABLE} PATH) # get the directory
    set(SlicerPythonExec ${SlicerPythonExecDir}/python)
  endif(WIN32)
  install(PROGRAMS ${SlicerPythonExec} DESTINATION ${NOCLI_INSTALL_DIR})

  # Create sym links during install step
  if(APPLE)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/CMake/lib DESTINATION ${INSTALL_DIR}/..)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/CMake/Frameworks DESTINATION ${INSTALL_DIR}/..)
  endif(APPLE)

endif( DTIAtlasBuilder_BUILD_SLICER_EXTENSION )

#===== Macro install tool ===============================================
macro( InstallToolMacro Proj CLI)
  
  if(COMPILE_EXTERNAL_${Proj})

    # Set Install directory
    if( DTIAtlasBuilder_BUILD_SLICER_EXTENSION )
      if(${CLI})
        set(MacroInstallDir ${INSTALL_DIR})
      else(${CLI})
        set(MacroInstallDir ${NOCLI_INSTALL_DIR})
      endif(${CLI})
    else( DTIAtlasBuilder_BUILD_SLICER_EXTENSION )
       set(MacroInstallDir ${INSTALL_DIR})
    endif( DTIAtlasBuilder_BUILD_SLICER_EXTENSION )

    # Find the tools and install commands
    foreach( tool ${Tools} )
       find_program( path_to_${tool} # Here all tools will be found because DTIAB is compiled after all tools
       NAMES ${tool}
       PATHS ${CMAKE_CURRENT_BINARY_DIR}/${Proj}-build/bin  # Here ${CMAKE_CURRENT_BINARY_DIR} is the inner build directory (build/DTIAtlasBuilder-build)
       PATH_SUFFIXES Debug Release RelWithDebInfo MinSizeRel # For Windows
       NO_DEFAULT_PATH
       NO_SYSTEM_ENVIRONMENT_PATH
      )
      install(PROGRAMS ${path_to_${tool}} DESTINATION ${MacroInstallDir})
    endforeach()

  endif(COMPILE_EXTERNAL_${Proj})

endmacro( InstallToolMacro )

## Install step for external projects: need to be here if SlicerExtension because make ExperimentalUpload done in inner build directory

set( Tools
  dtiprocess
  dtiaverage
  )
InstallToolMacro( dtiprocessTK ON) # proj CLI

set( Tools
  GreedyAtlas
  )
InstallToolMacro( AtlasWerks OFF)

set( Tools
  BRAINSFit
  BRAINSDemonWarp
  )
InstallToolMacro( BRAINS OFF)

set( Tools
  ANTS
  WarpImageMultiTransform
  WarpTensorImageMultiTransform
  )
InstallToolMacro( ANTS OFF)

set( Tools
  ResampleDTIlogEuclidean
  )
InstallToolMacro( ResampleDTI ON)

set( Tools
  DTI-Reg
  )
InstallToolMacro( DTIReg ON)

set( Tools
  unu
  )
InstallToolMacro( teem OFF)

set( Tools
  MriWatcher
  )
InstallToolMacro( MriWatcher OFF)

set( Tools
  ImageMath
  CropDTI
  )
InstallToolMacro( NIRALUtilities OFF)

#======================================================================================
# Testing for DTIAtlasBuilder
if(BUILD_TESTING)
  set(TestingSRCdirectory ${CMAKE_CURRENT_SOURCE_DIR}/Testing)
  set(TestingBINdirectory ${CMAKE_CURRENT_BINARY_DIR}/Testing)
  set(TestDataFolder ${CMAKE_CURRENT_SOURCE_DIR}/Data/Testing)
  add_library(DTIAtlasBuilderLib STATIC ${DTIABsources}) # STATIC is also the default
  set_target_properties(DTIAtlasBuilderLib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint") # replace the main in DTIAtlasBuilder.cxx by the itkTest function ModuleEntryPoint
  target_link_libraries(DTIAtlasBuilderLib ${QT_LIBRARIES} ${ITK_LIBRARIES})
  set_target_properties(DTIAtlasBuilderLib PROPERTIES LABELS DTIAtlasBuilder)
  # Create Tests
  include(CTest)
  add_subdirectory( ${TestingSRCdirectory} ) # contains a CMakeLists.txt
#  include_directories( ${TestingSRCdirectory} ) # contains a CMakeLists.txt
endif()

# For Slicer Extension
if( DTIAtlasBuilder_BUILD_SLICER_EXTENSION )
  if(NOT Slicer_SOURCE_DIR)
    set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${CMAKE_BINARY_DIR};${EXTENSION_NAME};ALL;/")
    include(${Slicer_EXTENSION_CPACK}) # So we can make package
  endif()
endif( DTIAtlasBuilder_BUILD_SLICER_EXTENSION )

