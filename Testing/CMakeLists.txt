cmake_minimum_required(VERSION 2.8)
CMAKE_POLICY(VERSION 2.8)

## Test 1 : Usage
add_test(NAME TestUsage COMMAND ${CMAKE_CURRENT_BINARY_DIR}/../DTIAtlasBuilder --help) # Should just display the Usage : ${CMAKE_CURRENT_BINARY_DIR} = DTIAtlasBuilder-build/Testing -> need to ../
set(EXECUTABLE_OUTPUT_PATH ${TestingBINdirectory}) # So the test executables go to the right place

## ITKtest Tests
# Compile the program that will be run to : run pipeline and then compare output with reference
if(ITK_VERSION_MAJOR EQUAL 3)
  add_executable(DTIAtlasBuilderTest DTIAtlasBuilderTestITKv${ITK_VERSION_MAJOR}.cxx) # This executable will run pipeline and then compare output with reference
  target_link_libraries(DTIAtlasBuilderTest DTIAtlasBuilderLib)
  set_target_properties(DTIAtlasBuilderTest PROPERTIES LABELS DTIAtlasBuilder)
  set(NoGUITestCommand ${Slicer_LAUNCH_COMMAND} ${TestingBINdirectory}/DTIAtlasBuilderTest)
  set(NameOfMain ModuleEntryPoint)
elseif(ITK_VERSION_MAJOR EQUAL 4) # BRAINS/BRAINSCommonLib/BuildScripts/CMakeBRAINS3BuildMacros.cmake : macro MakeTestDriverFromSEMTool
  set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "#include \"itkTestDriverBeforeTest.inc\"")
  set(CMAKE_TESTDRIVER_AFTER_TESTMAIN "#include \"itkTestDriverAfterTest.inc\"")
  create_test_sourcelist(DTIAtlasBuilder DTIAtlasBuilderTestDriver.cxx DTIAtlasBuilderTestITKv${ITK_VERSION_MAJOR}.cxx # Creates DTIAtlasBuilderTestDriver.cxx (= test source) into bin/Testing
    EXTRA_INCLUDE itkTestDriverIncludeRequiredIOFactories.h
    FUNCTION  ProcessArgumentsAndRegisterRequiredFactories
    )
  add_executable(DTIAtlasBuilderTestDriver DTIAtlasBuilderTestDriver.cxx DTIAtlasBuilderTestITKv${ITK_VERSION_MAJOR}.cxx)
  target_link_libraries(DTIAtlasBuilderTestDriver DTIAtlasBuilderLib ${ITKTestKernel_LIBRARIES}) # ITKTestKernel_LIBRARIES only defined in ITKv4
  set(NoGUITestCommand ${LAUNCH_EXE} $<TARGET_FILE:DTIAtlasBuilderTestDriver>) # Slicer_LAUNCH_COMMAND will launch the Slicer environment
  set(NameOfMain DTIAtlasBuilderTestITKv4)
endif()

set(CompareParametersImageMathFA
  --compareIntensityTolerance 200 # pixel OK if in range [value-200;value+200] => FA between 0 and 10000 (ImageMath)
  --compareRadiusTolerance 1 # pixel OK if one of neighbors in the given radius is in the tolerance range
  --compareNumberOfPixelsTolerance 50 # Set the number of pixels that are allowed to fail
  )

set(CompareParametersdtiprocessFA
  --compareIntensityTolerance 0.2 # pixel OK if in range [value-0.1;value+0.1] => FA between 0 and 1 (dtiprocess)
  --compareRadiusTolerance 1 # pixel OK if one of neighbors in the given radius is in the tolerance range
  --compareNumberOfPixelsTolerance 50 # Set the number of pixels that are allowed to fail
  )

set(DTIABParameters
  --nogui # no need if ITKv4 -> already in DTIAtlasBuilderTest.cxx
  --parameter_file ${TestingBINdirectory}/TestGUIparametersBRAINS.txt
  --configuration_file ${CMAKE_CURRENT_BINARY_DIR}/DTIAtlasBuilderSoftConfig.txt # Testing config file
  )

## Test 2 : Affine Atlas automatic QC
# Test with ITKtest that will compare the 2 given images after pipeline has been ran
add_test( NAME TestnoGUIAffineOutput COMMAND ${NoGUITestCommand}
  --compare # --compare is an cmd line argument from the itk test files 
    ${TestDataFolder}/TestAffineAtlasFA.nrrd # Ref : baseline
    ${TestingBINdirectory}/DTIAtlas/1_Affine_Registration/Loop0/Loop0_FAAverage.nrrd # Affine Output
  ${CompareParametersImageMathFA}
  ${NameOfMain} # After this line, give the parameters to use in DTIAtlasBuilder
    ${DTIABParameters}
  )


## Test 3 : Diffeomorphic Atlas automatic QC
# will not run the pipeline again because already ran : will just compare the images
add_test(NAME TestnoGUIDiffeomorphicOutput COMMAND ${NoGUITestCommand}
  --compare
    ${TestDataFolder}/TestDiffeomorphicAtlasFA.nrrd # Ref : baseline
    ${TestingBINdirectory}/DTIAtlas/3_Diffeomorphic_Atlas/DiffeomorphicAtlasFA.nrrd # Diffeomorphic Output
  ${CompareParametersdtiprocessFA}
  ${NameOfMain} # After this line, give the parameters to use in DTIAtlasBuilder
    ${DTIABParameters}
  )


## Test 4 : Final Atlas automatic QC
# will not run the pipeline again because already ran : will just compare the images
add_test(NAME TestnoGUIBRAINSFinalOutput COMMAND ${NoGUITestCommand}
  --compare
    ${TestDataFolder}/TestBRAINSFinalAtlasFA.nrrd # Ref : baseline
    ${TestingBINdirectory}/DTIAtlas/4_Final_Resampling/FinalAtlasFA.nrrd # Final Output
  ${CompareParametersdtiprocessFA}
  ${NameOfMain} # After this line, give the parameters to use in DTIAtlasBuilder
    ${DTIABParameters}
  )

## Test 5 : ANTS Final Atlas automatic QC
# will run the pipeline again to compute with ANTS
add_test(NAME TestnoGUIANTSFinalOutput COMMAND ${NoGUITestCommand}
  --compare
    ${TestDataFolder}/TestANTSFinalAtlasFA.nrrd # Ref : baseline
    ${TestingBINdirectory}/DTIAtlas/4_Final_Resampling/FinalAtlasFA.nrrd # Final Output with ANTS
  ${CompareParametersdtiprocessFA}
  ${NameOfMain} # After this line, give the parameters to use in DTIAtlasBuilder
    --nogui # no need if ITKv4 -> already in DTIAtlasBuilderTest.cxx
    --overwrite # Overwrite so it is computed with ANTS
    --parameter_file ${TestingBINdirectory}/TestGUIparametersANTS.txt
    --configuration_file ${CMAKE_CURRENT_BINARY_DIR}/DTIAtlasBuilderSoftConfig.txt # Testing config file
  )

## Test 6 : Open the GUI for 10 sec and close it
add_executable(DTIAtlasBuilderGUITest DTIAtlasBuilderGUITest.cxx)
target_link_libraries(DTIAtlasBuilderGUITest DTIAtlasBuilderLib)
set_target_properties(DTIAtlasBuilderGUITest PROPERTIES LABELS DTIAtlasBuilder)
add_test(NAME TestGUI COMMAND ${CMAKE_CURRENT_BINARY_DIR}/DTIAtlasBuilderGUITest ${CMAKE_CURRENT_BINARY_DIR}/bin/DTIAtlasBuilderSoftConfig.txt) # The config file will be read as argv[1] (no GenerateCLP)


## Test 7 : Open the GUI for 10 sec with a param file, check the content of the QLineEdit and close it
add_executable(DTIAtlasBuilderGUITestLabels DTIAtlasBuilderGUITestLabels.cxx)
target_link_libraries(DTIAtlasBuilderGUITestLabels DTIAtlasBuilderLib)
set_target_properties(DTIAtlasBuilderGUITestLabels PROPERTIES LABELS DTIAtlasBuilder)
add_test(NAME TestGUILabels COMMAND ${CMAKE_CURRENT_BINARY_DIR}/DTIAtlasBuilderGUITestLabels ${TestingBINdirectory}/TestGUIparametersBRAINS.txt ${CMAKE_CURRENT_BINARY_DIR}/DTIAtlasBuilderSoftConfig.txt ${TestingBINdirectory})
# The parameter file will be read as argv[1] (no GenerateCLP) and config file argv[2] # argv[3] is the folder to check to content of LineEdit


# Configure (Replace paths) and Copy needed files
configure_file( ${TestingSRCdirectory}/TestGUIparametersBRAINS.txt.in ${TestingBINdirectory}/TestGUIparametersBRAINS.txt )
configure_file( ${TestingSRCdirectory}/TestGUIparametersANTS.txt.in ${TestingBINdirectory}/TestGUIparametersANTS.txt )
configure_file( ${TestingSRCdirectory}/TestGUIparameters.csv.in ${TestingBINdirectory}/TestGUIparameters.csv )


# Informative display
if(COMPILE_PACKAGE)
#  message("When installed, run \"ctest\" in the build directory to launch tests and see if the program is working.")
else(COMPILE_PACKAGE)
#  message("When CMake done, run \"make\" to compile the program, and then run \"ctest\" in the build directory to launch tests and see if the program is working.\nEnter e to exit this message, and then g to generate and get CMake done.")
endif(COMPILE_PACKAGE)
