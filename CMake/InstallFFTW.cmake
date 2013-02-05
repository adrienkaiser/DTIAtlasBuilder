## FFTWTYPE and TOP_BINARY_DIR given as args to this script

if(FFTWTYPE STREQUAL FFTWD)
  set(FFTW_CONFIGURE_EXTRA_ARG "")
elseif(FFTWTYPE STREQUAL FFTWF)
  set(FFTW_CONFIGURE_EXTRA_ARG --enable-float)
endif()

# Configure Step
message("[] Configuring ${FFTWTYPE}...")
execute_process(COMMAND sh ${TOP_BINARY_DIR}/FFTW/configure --prefix=${TOP_BINARY_DIR}/FFTW-install --enable-static --enable-threads ${FFTW_CONFIGURE_EXTRA_ARG} WORKING_DIRECTORY ${TOP_BINARY_DIR}/FFTW-build)

# Build Step
message("[] Building ${FFTWTYPE}...")
execute_process(COMMAND make WORKING_DIRECTORY ${TOP_BINARY_DIR}/FFTW-build)

# Install Step
message("[] Installing ${FFTWTYPE}...")
execute_process(COMMAND make install WORKING_DIRECTORY ${TOP_BINARY_DIR}/FFTW-build)
