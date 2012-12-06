PROJECT (DTIAtlasBuilder)

cmake_minimum_required(VERSION 2.8)
CMAKE_POLICY(VERSION 2.8)
#External Projects

include(ExternalProject) # "ExternalProject" is the module that will allow to compile tools
if(CMAKE_EXTRA_GENERATOR) # Extra generator ??
  set(gen "${CMAKE_EXTRA_GENERATOR} - ${CMAKE_GENERATOR}")
else()
  set(gen "${CMAKE_GENERATOR}")
endif()

# Executable output path : folder with all the executables together
SET (EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/bin CACHE PATH "Single output directory for building all executables.")

# Git protocol
option(USE_GIT_PROTOCOL "If behind a firewall turn this off to use https instead." ON)
set(git_protocol "git")
if(NOT USE_GIT_PROTOCOL)
  set(git_protocol "https")
else(NOT USE_GIT_PROTOCOL)
  set(git_protocol "git")
endif()

# Search needed libraries and packages : ITK_DIR VTK_DIR GenerateCLP_DIR ModuleDescriptionParser_DIR TCLAP_DIR QT_QMAKE_EXECUTABLE SlicerExecutionModel_DIR(DTI-Reg)
FIND_PACKAGE(ITK REQUIRED) # creates ITK_DIR
IF(ITK_FOUND)
	INCLUDE(${ITK_USE_FILE})
ELSE(ITK_FOUND)
	MESSAGE(FATAL_ERROR "ITK not found. Please set ITK_DIR")
ENDIF(ITK_FOUND)

FIND_PACKAGE(VTK REQUIRED) # creates VTK_DIR
IF (VTK_FOUND)
  SET(VTK_USE_QVTK TRUE)
  SET(VTK_USE_GUISUPPORT TRUE)
  INCLUDE(${VTK_USE_FILE})
ELSE(VTK_FOUND)
   MESSAGE(FATAL_ERROR, "VTK not found. Please set VTK_DIR.")
ENDIF (VTK_FOUND)

FIND_PACKAGE(Qt4 REQUIRED) # creates QT_QMAKE_EXECUTABLE
IF(QT_USE_FILE)
	INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${QT_INCLUDE_DIR})
	INCLUDE(${QT_USE_FILE})
	ADD_DEFINITIONS(-DQT_GUI_LIBS -DQT_CORE_LIB -DQT3_SUPPORT)
ELSE(QT_USE_FILE)
	MESSAGE(FATAL_ERROR, "QT not found. Please set QT_DIR.")
ENDIF(QT_USE_FILE)

FIND_PACKAGE(GenerateCLP REQUIRED) # creates GenerateCLP_DIR ModuleDescriptionParser_DIR TCLAP_DIR
IF(GenerateCLP_FOUND)
	INCLUDE(${GenerateCLP_USE_FILE})
ELSE(GenerateCLP_FOUND)
	MESSAGE(FATAL_ERROR "GenerateCLP not found. Please set GenerateCLP_DIR")
ENDIF(GenerateCLP_FOUND)

# Compile package ?
set( COMPILE_PACKAGE OFF CACHE BOOL "Compiles all the external projects and tools" )
if(COMPILE_PACKAGE)
	# External tools variables
	set( ExtToolList
		dtiprocess # + dtiaverage
		GreedyAtlas
		BRAINSFit # + BRAINSDemonWarp
		ANTS # + WarpImageMultiTransform, WarpTensorImageMultiTransform
		ResampleDTIlogEuclidean
		)
	foreach( tool ${ExtToolList})
		set( COMPILE_EXTERNAL_${tool} ON CACHE BOOL "" )
	endforeach()

	# File containing add_external for all tools
	include( FindExternalTools.cmake ) # Go execute the given cmake script, and get back into this script when done

endif(COMPILE_PACKAGE)

# Compile DTIAtlasBuilder
QT4_WRAP_CPP(QtProject_HEADERS_MOC GUI.h)
QT4_WRAP_UI(UI_FILES GUIwindow.ui)
SET(sources DTIAtlasBuilder.cxx GUI.h GUI.cxx ScriptWriter.h ScriptWriter.cxx ${QtProject_HEADERS_MOC} ${UI_FILES}) # define the variable "sources" that contains the names of files
GENERATECLP(sources DTIAtlasBuilder.xml) # include the GCLP file to the project
ADD_EXECUTABLE(DTIAtlasBuilder ${sources})  # add the files contained by "sources" to the project
TARGET_LINK_LIBRARIES(DTIAtlasBuilder ${QT_LIBRARIES} ${ITK_LIBRARIES})

# Update the paths to the program in the configuration file, and copy it to the executable directory
CONFIGURE_FILE( ${CMAKE_CURRENT_SOURCE_DIR}/DTIAtlasBuilderSoftConfig.txt.in ${EXECUTABLE_OUTPUT_PATH}/DTIAtlasBuilderSoftConfig.txt ) # configure and copy with (tool)Path

