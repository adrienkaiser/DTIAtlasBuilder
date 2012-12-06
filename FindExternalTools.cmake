#Find external tools

#External Projects
include(ExternalProject)
if(CMAKE_EXTRA_GENERATOR) # Extra generator ??
  set(gen "${CMAKE_EXTRA_GENERATOR} - ${CMAKE_GENERATOR}")
else()
  set(gen "${CMAKE_GENERATOR}")
endif()

#===== Macro find program ===========================================
macro( FindExecutableMacro pathVariable ExecName CompileVariable)
	find_program( ${pathVariable} ${ExecName} ) # search for the program and put the result in a variable called ${pathVariable}
	if(NOT ${pathVariable})
		message( WARNING "${ExecName} not found. CMake external will download and compile it" )
		set( ${CompileVariable} ON CACHE BOOL "" FORCE)
	endif(NOT ${pathVariable} )
	set( ${pathVariable} ${${pathVariable}} CACHE STRING "Path to the system ${ExecName} executable")
endmacro( FindExecutableMacro )
#====================================================================

#===== Macro add tool ===============================================
 # if SourceCodeArgs or CMAKE_ExtraARGS passed to the macro as arguments, only the first word is used (each element of the list is taken as ONE argument) => use as "global variables"
macro( AddToolMacro proj tool ) # ex: proj = dtiprocessTK , tool = dtiprocess |  proj = AtlasWerks , tool = GreedyAtlas

	# If not asked to compile, search it : if not found, set COMPILE_EXTERNAL for next if()
	if(NOT COMPILE_EXTERNAL_${tool})
		FindExecutableMacro( TOOL${tool}Sys ${tool} COMPILE_EXTERNAL_${tool}) # pathVariable ExecName CompileVariable
	endif(NOT COMPILE_EXTERNAL_${tool})

	# Set paths for extra tools (tools in the same package)
	if(COMPILE_EXTERNAL_${tool})
		foreach( extratool ${extraTools})
			set(${extratool}Path ${EXECUTABLE_OUTPUT_PATH}/${extratool})
		endforeach()
	else(COMPILE_EXTERNAL_${tool})
		foreach( extratool ${extraTools})
			find_program( TOOL${extratool}Sys ${extratool} ) # If not asked to recompile, search the tool
			if(${TOOL${extratool}Sys} STREQUAL "TOOL${extratool}Sys-NOTFOUND") # If program not found, give a warning message, reset all extra tools to the recompiled path and recompile the whole package
				message( WARNING "${extratool} not found. CMake external will download and compile the whole ${proj} package" )
				set( COMPILE_EXTERNAL_${tool} ON CACHE BOOL "" FORCE)
				foreach( extratoolReset ${extraTools})
					set(${extratoolReset}Path ${EXECUTABLE_OUTPUT_PATH}/${extratoolReset})
				endforeach()
			elseif(${TOOL${extratool}Sys} STREQUAL "TOOL${extratool}Sys-NOTFOUND") # If program found set the NON CACHE "Path" variable and hide the find_program variable
				set(${extratool}Path ${TOOL${extratool}Sys})
				mark_as_advanced(TOOL${extratool}Sys) # Hide the option in the gui				
			endif(${TOOL${extratool}Sys} STREQUAL "TOOL${extratool}Sys-NOTFOUND")
		endforeach()
	endif(COMPILE_EXTERNAL_${tool})

	# Add the external project : If asked to compile, or not found on the system : no if/else because COMPILE_EXTERNAL_${tool} could have been set in the if (if not found)
	if(COMPILE_EXTERNAL_${tool})
		ExternalProject_Add(${proj}
			${SourceCodeArgs} # No difference between args passed separated with ';', spaces or return to line
			SOURCE_DIR ${proj} # creates the folder if it doesn't exist
			BINARY_DIR ${proj}-build
			UPDATE_COMMAND ""
			CMAKE_GENERATOR ${gen}
			CMAKE_ARGS
				${LOCAL_CMAKE_BUILD_OPTIONS}
				-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
				-DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
				-DCMAKE_RUNTIME_OUTPUT_DIRECTORY:PATH=${EXECUTABLE_OUTPUT_PATH}
				${CMAKE_ExtraARGS}
			INSTALL_COMMAND ""
#			DEPENDS ITK VTK FFTWF FFTWD CLAPACK ${FLTK_PREREQ}
#			DEPENDS  ${ITK_DEPEND} ${SlicerExecutionModel_DEPEND} ${VTK_DEPEND}
		)
		set( TOOL${tool} ${EXECUTABLE_OUTPUT_PATH}/${tool} CACHE STRING "Path to the ${tool} executable")
	endif(COMPILE_EXTERNAL_${tool})

	# ${proj}Path =  variable changed in the DTIAB config file :
	if(COMPILE_EXTERNAL_${tool})
		set(${tool}Path ${TOOL${tool}})
		mark_as_advanced(CLEAR TOOL${tool}) # Show the option in the gui
		if(DEFINED TOOL${tool}Sys)
			mark_as_advanced(FORCE TOOL${tool}Sys) # Hide the option in the gui
		endif()
	else(COMPILE_EXTERNAL_${tool})
		set(${tool}Path ${TOOL${tool}Sys})
		mark_as_advanced(CLEAR TOOL${tool}Sys) # Show the option in the gui
		if(DEFINED TOOL${tool})
			mark_as_advanced(FORCE TOOL${tool}) # Hide the option in the gui
		endif()
	endif(COMPILE_EXTERNAL_${tool})
endmacro( AddToolMacro )
#====================================================================

#====================================================================
#====================================================================
#===== TOOLS ========================================================
#       ||
#       VV

# ===== dtiprocess/dtiaverage =====================================================
set( SourceCodeArgs
	SVN_REPOSITORY "https://www.nitrc.org/svn/dtiprocess/trunk"
	SVN_USERNAME slicerbot
	SVN_PASSWORD slicer
	SVN_REVISION -r 82
	)
set( CMAKE_ExtraARGS
	-DBUILD_TESTING:BOOL=OFF
	-DITK_DIR:PATH=${ITK_DIR}
	-DVTK_DIR:PATH=${VTK_DIR}
	-DGenerateCLP_DIR:PATH=${GenerateCLP_DIR}
	-DModuleDescriptionParser_DIR:PATH=${ModuleDescriptionParser_DIR}
	-DTCLAP_DIR:PATH=${TCLAP_DIR}
	)
set( extraTools
	dtiaverage
	)
AddToolMacro( dtiprocessTK dtiprocess ) # AddToolMacro( proj tool )

# ===== AtlasWerks/GreedyAtlas =====================================================
# code from https://github.com/Chaircrusher/AtlasWerksBuilder/blob/master/CMakeLists.txt
set( SourceCodeArgs
	URL "http://www.sci.utah.edu/releases/atlaswerks_v0.1.4/AtlasWerks_0.1.4_Linux.tgz"
	URL_MD5 05fc867564e3340d0d448dd0daab578a
	)
set( CMAKE_ExtraARGS
	-DITK_DIR:PATH=${ITK_DIR}
	-DVTK_DIR:PATH=${VTK_DIR}
#	-DFFTWF_LIB:PATH=${Prereqs}/lib/libfftw3f.a
#	-DFFTWF_THREADS_LIB:PATH=${Prereqs}/lib/libfftw3f_threads.a
#	-DFFTWD_LIB:PATH=${Prereqs}/lib/libfftw3.a
#	-DFFTWD_THREADS_LIB:PATH=${Prereqs}/lib/libfftw3_threads.a
#	-DFFTW_INCLUDE:PATH=${Prereqs}/include
#	-DatlasWerks_COMPILE_APP_ImageConvert:BOOL=${AtlasWerks_BUILD_IMAGE_CONVERT}
#	${BuildGUIFlag}
#	"-DLAPACK_LIBS:STRING=lapack blas f2c"
#	"-DLAPACK_LIBS_SEARCH_DIRS:STRING=${Prereqs}/lib"
	)
set( extraTools
	""
	)
AddToolMacro( AtlasWerks GreedyAtlas ) # AddToolMacro( proj tool )

# ===== BRAINSFit/BRAINSDemonWarp =====================================================
set( SourceCodeArgs
	GIT_REPOSITORY ${git_protocol}://github.com/BRAINSia/BRAINSStandAlone.git
	GIT_TAG "12b6d41a74ec30465a07df9c361237f2b77c2955"
	)
set( CMAKE_ExtraARGS
	-DBUILD_TESTING:BOOL=OFF
	-DBUILD_SHARED_LIBS:BOOL=OFF # ${BUILD_SHARED_LIBS}
	-DSuperBuild_BRAINSTools_USE_GIT:BOOL=${USE_GIT_PROTOCOL}
	-DITK_VERSION_MAJOR:STRING=3
	-DITK_DIR:PATH=${ITK_DIR}
	-DGenerateCLP_DIR:PATH=${GenerateCLP_DIR}
	-DModuleDescriptionParser_DIR:PATH=${ModuleDescriptionParser_DIR}
	-DBUILD_TESTING:BOOL=OFF
	-DUSE_AutoWorkup:BOOL=OFF
	-DUSE_BRAINSContinuousClass:BOOL=OFF
	-DUSE_BRAINSDemonWarp:BOOL=ON
	-DUSE_BRAINSFit:BOOL=ON
	-DUSE_BRAINSFitEZ:BOOL=OFF
	-DUSE_BRAINSROIAuto:BOOL=OFF
	-DUSE_BRAINSResample:BOOL=OFF
	-DUSE_BRAINSSurfaceTools:BOOL=OFF
	-DUSE_DebugImageViewer:BOOL=OFF
	-DUSE_GTRACT:BOOL=OFF
	-DUSE_SYSTEM_ITK=ON
	-DUSE_SYSTEM_SlicerExecutionMode=ON
	-DUSE_SYSTEM_VTK=ON
	)
set( extraTools
	BRAINSDemonWarp
	)
AddToolMacro( BRAINS BRAINSFit ) # AddToolMacro( proj tool )

# ===== ANTS/WarpMultiTransform =====================================================
set( SourceCodeArgs
	SVN_REPOSITORY "https://advants.svn.sourceforge.net/svnroot/advants/trunk"
	SVN_REVISION -r 1395
	)
set( CMAKE_ExtraARGS
	-DBUILD_TESTING:BOOL=OFF
	-DBUILD_SHARED_LIBS:BOOL=OFF # ${BUILD_SHARED_LIBS}
	-DSuperBuild_ANTS_USE_GIT_PROTOC:BOOL=${USE_GIT_PROTOCOL}
	)
set( extraTools
	WarpImageMultiTransform
	WarpTensorImageMultiTransform
	)
AddToolMacro( ANTS ANTS ) # AddToolMacro( proj tool )

# ===== ResampleDTIlogEuclidean =====================================================
set( SourceCodeArgs
	SVN_REPOSITORY "http://svn.slicer.org/Slicer4/trunk/Modules/CLI/ResampleDTIVolume"
	SVN_REVISION -r 20422
	)
set( CMAKE_ExtraARGS
	""
	)
set( extraTools
	""
	)
AddToolMacro( ResampleDTI ResampleDTIlogEuclidean ) # AddToolMacro( proj tool )

