DTIAtlasBuilder
===============

Find the tool on NITRC : https://www.nitrc.org/projects/dtiatlasbuilder

And GitHub : https://github.com/NIRALUser/DTIAtlasBuilder

Contact : akaiser@unc.edu



A tool to create an atlas from several DTI images

These Softwares need to be installed before executing the tool :
- ImageMath
- ResampleDTIlogEuclidean
- CropDTI
- dtiprocess
- BRAINSFit
- GreedyAtlas
- dtiaverage
- DTI-Reg
- MriWatcher
- unu

If you do not have these softwares installed on your machine, you can use the COMPILE_PACKAGE CMake option to download and compile aumatically the tools you need. If you do so, please run "make install" after the compilation to copy the tools into the EXECUTABLE_OUTPUT_PATH.

The following librairies are needed:
- ITK (DTIAtlasBuilder, BRAINSFit, NIRALUtilities, AtlasWerks, ANTS)
- ITKv4 v3Compatible (dtiprocessTK)
- VTK (NIRALUtilities, dtiprocessTK, AtlasWerks, ANTS, BRAINSFit)
- SlicerExecutionModel (BRAINSFit, DTI-Reg, ANTS)
  -> GenerateCLP (DTIAtlasBuilder, dtiprocessTK, BRAINSFit, NIRALUtilities)
- Qt4 (DTIAtlasBuilder, MriWatcher)
- BatchMake (DTI-Reg)
- FFTW (AtlasWerks) ! path given manually in the CMake interface
