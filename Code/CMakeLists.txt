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
QT4_WRAP_CPP(QtProject_HEADERS_MOC GUI.h)
QT4_WRAP_UI(UI_FILES GUIwindow.ui)
set(DTIABsources DTIAtlasBuilder.cxx GUI.h GUI.cxx ScriptWriter.h ScriptWriter.cxx ${QtProject_HEADERS_MOC} ${UI_FILES})
GENERATECLP(DTIABsources DTIAtlasBuilder.xml) # include the GCLP file to the project
add_executable(DTIAtlasBuilder ${DTIABsources})  # add the files contained by "DTIABsources" to the project
target_link_libraries(DTIAtlasBuilder ${QT_LIBRARIES} ${ITK_LIBRARIES})
#set_target_properties(DTIAtlasBuilder PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${EXECUTABLE_OUTPUT_PATH})
#install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/bin/DTIAtlasBuilder DESTINATION ${CMAKE_INSTALL_PREFIX}) # will create the install folder if doesn't exist and copy the program in it

#======================================================================================
# Testing for DTIAtlasBuilder
if(BUILD_TESTING)
  set(TestingSRCdirectory ${CMAKE_CURRENT_SOURCE_DIR}/Testing)
  set(TestingBINdirectory ${CMAKE_CURRENT_BINARY_DIR}/Testing)
  set(TestDataFolder ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Testing)
  add_library(DTIAtlasBuilderLib STATIC ${DTIABsources})
  set_target_properties(DTIAtlasBuilderLib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint") # replace the main in DTIAtlasBuilder.cxx by the itkTest function ModuleEntryPoint
  target_link_libraries(DTIAtlasBuilderLib ${QT_LIBRARIES} ${ITK_LIBRARIES})
  set_target_properties(DTIAtlasBuilderLib PROPERTIES LABELS DTIAtlasBuilder)
  # Create Tests
  enable_testing()
  add_subdirectory( ${TestingSRCdirectory} ) # contains a CMakeLists.txt
#  include_directories( ${TestingSRCdirectory} ) # contains a CMakeLists.txt
endif()
