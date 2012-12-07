project (DTIAtlasBuilder)

cmake_minimum_required(VERSION 2.8)
CMAKE_POLICY(VERSION 2.8)

# Executable output path : folder with all the executables together
set (EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/bin CACHE PATH "Single output directory for building all executables.")

#===================================================================================
# Search needed libraries and packages : ITK_DIR VTK_DIR GenerateCLP_DIR ModuleDescriptionParser_DIR TCLAP_DIR QT_QMAKE_EXECUTABLE SlicerExecutionModel_DIR(DTI-Reg)
find_package(ITK REQUIRED) # creates ITK_DIR
if(ITK_FOUND)
  include(${ITK_USE_FILE})
else(ITK_FOUND)
  message(FATAL_ERROR "ITK not found. Please set ITK_DIR")
endif(ITK_FOUND)

find_package(VTK REQUIRED) # creates VTK_DIR
if (VTK_FOUND)
  set(VTK_USE_QVTK TRUE)
  set(VTK_USE_GUISUPPORT TRUE)
  include(${VTK_USE_FILE})
else(VTK_FOUND)
  message(FATAL_ERROR, "VTK not found. Please set VTK_DIR.")
endif (VTK_FOUND)

find_package(Qt4 REQUIRED) # creates QT_QMAKE_EXECUTABLE
if(QT_USE_FILE)
  include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${QT_INCLUDE_DIR})
  include(${QT_USE_FILE})
  add_definitions(-DQT_GUI_LIBS -DQT_CORE_LIB -DQT3_SUPPORT)
else(QT_USE_FILE)
  message(FATAL_ERROR, "QT not found. Please set QT_DIR.")
endif(QT_USE_FILE)

find_package(GenerateCLP REQUIRED) # creates GenerateCLP_DIR ModuleDescriptionParser_DIR TCLAP_DIR
if(GenerateCLP_FOUND)
  include(${GenerateCLP_USE_FILE})
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
    )
  foreach( tool ${ExtProjList})
    set( COMPILE_EXTERNAL_${tool} ON CACHE BOOL "" )
  endforeach()

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

