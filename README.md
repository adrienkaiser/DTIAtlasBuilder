DTIAtlasBuilder
===============

Find the tool on NITRC : http://www.nitrc.org/projects/dtiatlasbuilder

And GitHub : http://github.com/NIRALUser/DTIAtlasBuilder

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

If you do not have these softwares installed on your machine, you can use the COMPILE_PACKAGE CMake option to download and compile aumatically the tools you need. If you do so, please run "make install" after the compilation to copy the tools into the CMAKE_INSTALL_PREFIX.
