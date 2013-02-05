## TOP_BINARY_DIR given as argument to this script

if(WIN32)

  # Creating include and lib dirs
  file(MAKE_DIRECTORY ${TOP_BINARY_DIR}/FFTW-install/include)
  file(MAKE_DIRECTORY ${TOP_BINARY_DIR}/FFTW-install/lib)
  
  # Copying header file
  execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${TOP_BINARY_DIR}/FFTW/fftw3.h ${TOP_BINARY_DIR}/FFTW-install/include/fftw3.h)
  
  # Creating .lib from .dll : lib.exe program from VC++ (see http://fftw.org/install/windows.html)
  execute_process(COMMAND lib /machine:x64 /def:${TOP_BINARY_DIR}/FFTW/libfftw3l-3.def /out:${TOP_BINARY_DIR}/FFTW-install/lib/fftw3.lib)
  execute_process(COMMAND lib /machine:x64 /def:${TOP_BINARY_DIR}/FFTW/libfftw3f-3.def /out:${TOP_BINARY_DIR}/FFTW-install/lib/fftw3f.lib)

else(WIN32) # Unix-like : recompile

  ## FFTWD
  # Configure Step
  message("[] Configuring FFTWD...")
  execute_process(COMMAND sh ${TOP_BINARY_DIR}/FFTW/configure --prefix=${TOP_BINARY_DIR}/FFTW-install --enable-static --enable-threads WORKING_DIRECTORY ${TOP_BINARY_DIR}/FFTW-build)
  # Build Step
  message("[] Building FFTWD...")
  execute_process(COMMAND make WORKING_DIRECTORY ${TOP_BINARY_DIR}/FFTW-build)
  # Install Step
  message("[] Installing FFTWD...")
  execute_process(COMMAND make install WORKING_DIRECTORY ${TOP_BINARY_DIR}/FFTW-build)

  ## FFTWF
  # Configure Step
  message("[] Configuring FFTWF...")
  execute_process(COMMAND sh ${TOP_BINARY_DIR}/FFTW/configure --prefix=${TOP_BINARY_DIR}/FFTW-install --enable-static --enable-threads --enable-float WORKING_DIRECTORY ${TOP_BINARY_DIR}/FFTW-build)
  # Build Step
  message("[] Building FFTWF...")
  execute_process(COMMAND make WORKING_DIRECTORY ${TOP_BINARY_DIR}/FFTW-build)
  # Install Step
  message("[] Installing FFTWF...")
  execute_process(COMMAND make install WORKING_DIRECTORY ${TOP_BINARY_DIR}/FFTW-build)
  
endif(WIN32)
