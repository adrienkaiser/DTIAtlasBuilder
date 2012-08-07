#include "itkImage.h"
#include "itkImageFileReader.h"

#include <iostream>
#include <string>
#include <vector>
#include <sstream> // to convert int to std::string
#include <math.h> // for the absolute value

#include "ScriptWriter.h"

void ScriptWriter::WriteScript()
{
	std::cout<<"|"<<std::endl; // command line display
	std::cout<<"| Number of Cases: "<<m_CasesPath.size()<<std::endl; // command line display
	std::cout<<"| Output Directory : "<<m_OutputPath<<"/DTIAtlas/Affine_Registration"<<std::endl; // command line display
	if(m_RegType==1) std::cout<<"| Using Case 1 as reference in the first Registration Loop"<<std::endl; // command line display
	else std::cout<<"| Using Template as reference for the Registration: "<<m_TemplatePath<<std::endl; // command line display
	std::cout<<"| Number of loops in the Registration Loop : "<<m_nbLoops<<std::endl; // command line display
	std::cout<<"| Writing begin: "; // command line display

	std::ostringstream out;
	out << m_nbLoops;
	m_nbLoops_str = out.str();

	if(m_RegType==1) PreprocessCase();
	else PreprocessTemplate();
	AtlasBuilding();
	MainScript();
}

void ScriptWriter::PreprocessCase ()
{
	std::string Script;

	std::cout<<"[Pre Processing]"; // command line display (no endl)

	Script = Script + "#!/usr/bin/python\n\n";
	Script = Script + "import os\n\n"; ///// To run a shell command : os.system("[shell command]")
	Script = Script + "print(\"\\n============ Pre processing =============\")\n\n";

	Script = Script + "allcases = [\"" + m_CasesPath[0];
	for (unsigned int i=1;i<m_CasesPath.size();i++) Script = Script + "\", \"" + m_CasesPath[i];
	Script = Script+ "\"]\n";
	
	Script = Script + "OutputPath= \"" + m_OutputPath + "/DTIAtlas/Affine_Registration\"\n";

	Script = Script + "AtlasFAref= OutputPath + \"/Case1_FA.nrrd\" #the reference will be the first case for the first loop\n";

////////Create directory for temporary files
	Script = Script + "if not os.path.isdir(OutputPath):\n";
		Script = Script + "\tos.mkdir(OutputPath)\n";
		Script = Script + "\tprint(\"\\n => Creation of the temporary files directory = \" + OutputPath)\n\n";

//////////Changing origin to (0,0,0)
	Script = Script + "print(\"\\n======== Changing origin of Images =========\")\n";
	Script = Script + "case = 0\n";
	Script = Script + "while case < len(allcases):\n";
		Script = Script + "\tDTIOrig= OutputPath + \"/Case\" + str(case+1) + \"_DTIOrig.nrrd\"\n";
		Script = Script + "\tChgOrigCommand = \"ImageMath \" + allcases[case] + \" -outfile \" + DTIOrig + \" -changeOrig 0,0,0 -type float\"\n";
		Script = Script + "\tprint(\"||Case \" + str(case+1) + \" => $ \" + ChgOrigCommand)\n";
		if(m_NeedToBeResampled==1)
		{
		Script = Script + "\tChgSpaceDirCommand = \"ResampleDTIlogEuclidean \" + DTIOrig + \" \" + DTIOrig + \" -R \" + OutputPath + \"/Case1_DTIOrig.nrrd\"\n";
		Script = Script + "\tif case>0 : print(\"||Case \" + str(case+1) + \" => $ \" + ChgSpaceDirCommand)\n";
		}
		if(m_Overwrite==1)
		{
		Script = Script + "\tos.system(ChgOrigCommand)\n";
if(m_NeedToBeResampled==1) Script = Script + "\tos.system(ChgSpaceDirCommand)\n";
		}
		else
		{
		Script = Script + "\tif not os.path.isfile(DTIOrig) :\n";
		Script = Script + "\t\tos.system(ChgOrigCommand)\n";
if(m_NeedToBeResampled==1) Script = Script + "\t\tos.system(ChgSpaceDirCommand)\n";
		Script = Script + "\telse : print(\"=> The file \\'\" + DTIOrig + \"\\' already exists so the commands will not be executed\")\n";
		}
		Script = Script + "\tcase += 1\n\n";

//////////Cropping DTI image
	if(m_NeedToBeCropped==1)
	{

	std::string CropSize_str [3];
	std::ostringstream out1;
	out1 << m_CropSize[0];
	CropSize_str[0] = out1.str();
	std::ostringstream out2;
	out2 << m_CropSize[1];
	CropSize_str[1] = out2.str();
	std::ostringstream out3;
	out3 << m_CropSize[2];
	CropSize_str[2] = out3.str();
	Script = Script + "CropSize=[\"" + CropSize_str[0] + "\",\"" + CropSize_str[1] + "\",\"" + CropSize_str[2] + "\"]\n\n";

	Script = Script + "print(\"\\n======== Cropping DTI Image =========\")\n";
	Script = Script + "case = 0\n";
	Script = Script + "while case < len(allcases):\n";
		Script = Script + "\tcroppedDTI= OutputPath + \"/Case\" + str(case+1) + \"_croppedDTI.nrrd\"\n";
		Script = Script + "\tDTIOrig= OutputPath + \"/Case\" + str(case+1) + \"_DTIOrig.nrrd\"\n";
		Script = Script + "\tCropCommand=\"CropDTI \" + DTIOrig + \" -o \" + croppedDTI + \" -size \" + CropSize[0] + \",\" + CropSize[1] + \",\" + CropSize[2] + \" -v\"\n";
		Script = Script + "\tprint(\"||Case \" + str(case+1) + \" => $ \" + CropCommand)\n";
if(m_Overwrite==1)Script = Script + "\tos.system(CropCommand)\n";
		else
		{
		Script = Script + "\tif not os.path.isfile(croppedDTI) : os.system(CropCommand)\n";
		Script = Script + "\telse : print(\"=> The file \\'\" + croppedDTI + \"\\' already exists so the command will not be executed\")\n";
		}
		Script = Script + "\tcase += 1\n\n";
	}

//////////Generating FA
	Script = Script + "print(\"\\n======== Generating FA =========\")\n";
	Script = Script + "case = 0\n";
	Script = Script + "while case < len(allcases):\n";
		if(m_NeedToBeCropped==1) Script = Script + "\tDTI= OutputPath + \"/Case\" + str(case+1) + \"_croppedDTI.nrrd\"\n";
		else Script = Script + "\tDTI= OutputPath + \"/Case\" + str(case+1) + \"_DTIOrig.nrrd\"\n";
		Script = Script + "\tFA= OutputPath + \"/Case\" + str(case+1) + \"_FA.nrrd\"\n";
		Script = Script + "\tGeneFACommand=\"dtiprocess --dti_image \" + DTI + \" -f \" + FA\n";
		Script = Script + "\tprint(\"||Case \" + str(case+1) + \" => $ \" + GeneFACommand)\n";
if(m_Overwrite==1)Script = Script + "\tos.system(GeneFACommand)\n";
		else
		{
		Script = Script + "\tif not os.path.isfile(FA) : os.system(GeneFACommand)\n";
		Script = Script + "\telse : print(\"=> The file \\'\" + FA + \"\\' already exists so the command will not be executed\")\n";
		}
		Script = Script + "\tcase += 1\n\n";

//////////Affine Registration and Normalization Loop
	Script = Script + "n = 0\n";
	Script = Script + "while n <= " + m_nbLoops_str + " :\n";
	//Script = Script + "\tif n == " + m_nbLoops_str + " : OutputPath= \"" + m_OutputPath + "/DTIAtlas/Affine_Registration\" # this will not be done for the last lap\n";

//////////Normalization
		Script = Script + "\tprint(\"\\n[LOOP \" + str(n) + \"/" + m_nbLoops_str + "] ======== Normalization =========\")\n";
		Script = Script + "\tif n == 0 : case = 1 # the first case is the reference for the first loop so it will not be normalized or registered\n";
		Script = Script + "\telse : case = 0\n";
		Script = Script + "\twhile case < len(allcases):\n";
			Script = Script + "\t\tFA= OutputPath + \"/Case\" + str(case+1) + \"_FA.nrrd\"\n";
			Script = Script + "\t\tNormFA= OutputPath + \"/Case\" + str(case+1) + \"_Loop\" + str(n) + \"_NormFA.nrrd\"\n";
			Script = Script + "\t\tNormFACommand=\"ImageMath \" + FA + \" -outfile \" + NormFA + \" -matchHistogram \" + AtlasFAref\n";
			Script = Script + "\t\tprint(\"||Case \" + str(case+1) + \" => $ \" + NormFACommand)\n";
if(m_Overwrite==1)	Script = Script + "\t\tos.system(NormFACommand)\n";
			else
			{
			Script = Script + "\t\tif not os.path.isfile(NormFA) : os.system(NormFACommand)\n";
			Script = Script + "\t\telse : print(\"=> The file \\'\" + NormFA + \"\\' already exists so the command will not be executed\")\n";
			}
			Script = Script + "\t\tcase += 1\n\n";

//////////Affine with BrainsFit registration
		Script = Script + "\tprint(\"\\n[LOOP \" + str(n) + \"/" + m_nbLoops_str + "] ======== Affine with BrainsFit registration =========\")\n";
		Script = Script + "\tif n == 0 : case = 1\n";
		Script = Script + "\telse : case = 0\n";
		Script = Script + "\twhile case < len(allcases):\n";
			Script = Script + "\t\tNormFA= OutputPath + \"/Case\" + str(case+1) + \"_Loop\" + str(n) + \"_NormFA.nrrd\"\n";
			Script = Script + "\t\tLinearTranstfm= OutputPath + \"/Case\" + str(case+1) + \"_Loop\" + str(n) + \"_LinearTrans.txt\"\n";
			Script = Script + "\t\tLinearTrans= OutputPath + \"/Case\" + str(case+1) + \"_Loop\" + str(n) + \"_LinearTrans_FA.nrrd\"\n";
			Script = Script + "\t\tAffineCommand=\"BRAINSFit --fixedVolume \" + AtlasFAref + \" --movingVolume \" + NormFA + \" --initializeTransformMode useCenterOfHeadAlign --useAffine --outputVolume \" + LinearTrans + \" --outputTransform \" + LinearTranstfm\n";
			Script = Script + "\t\tprint(\"||Case \" + str(case+1) + \" => $ \" + AffineCommand)\n";
if(m_Overwrite==1)		Script = Script + "\t\tif 1 :\n";
else				Script = Script + "\t\tif not os.path.isfile(LinearTranstfm) or not os.path.isfile(LinearTrans) :\n";
				Script = Script + "\t\t\tos.system(AffineCommand)\n";
				Script = Script + "\t\t\tif n == " + m_nbLoops_str + " : #changing into float only the last tfm files\n";
					Script = Script + "\t\t\t\tprint(\"=> Converting the transformation matrix from double to float for AtlasWerks\\n\")\n";
					Script = Script + "\t\t\t\ttxtFile = open(LinearTranstfm,'r')\n";
					Script = Script + "\t\t\t\tLinearTranstfmFloat = OutputPath + \"/Case\" + str(case+1) + \"_Loop\" + str(n) + \"_LinearTrans_float.txt\"\n";
					Script = Script + "\t\t\t\tnewFile = open(LinearTranstfmFloat,'w')\n";
					Script = Script + "\t\t\t\trd = 'string to enter the while loop'\n";
					Script = Script + "\t\t\t\twhile rd: # '' is equal to false in python => rd==0 is the end of th file\n";
						Script = Script + "\t\t\t\t\trd = txtFile.readline()\n";
						Script = Script + "\t\t\t\t\tnewFile.write(rd.replace('double','float'))\n";
					Script = Script + "\t\t\t\ttxtFile.close()\n";
					Script = Script + "\t\t\t\tnewFile.close()\n";
if(m_Overwrite==0)	Script = Script + "\t\telif os.path.isfile(LinearTranstfm) : print(\"=> The file \\'\" + LinearTranstfm + \"\\' already exists so the command will not be executed\")\n";
if(m_Overwrite==0)	Script = Script + "\t\telif os.path.isfile(LinearTrans) : print(\"=> The file \\'\" + LinearTrans + \"\\' already exists so the command will not be executed\")\n";
			Script = Script + "\t\tcase += 1\n\n";

//////////Implementing the affine registration
		Script = Script + "\tprint(\"\\n[LOOP \" + str(n) + \"/" + m_nbLoops_str + "] ======== Implementing the Affine registration =========\")\n";
		Script = Script + "\tif n == 0 : case = 1\n";
		Script = Script + "\telse : case = 0\n";
		Script = Script + "\twhile case < len(allcases):\n";
			Script = Script + "\t\tLinearTranstfm= OutputPath + \"/Case\" + str(case+1) + \"_Loop\" + str(n) + \"_LinearTrans.txt\"\n";
			Script = Script + "\t\tLinearTransDTI= OutputPath + \"/Case\" + str(case+1) + \"_Loop\" + str(n) + \"_LinearTrans_DTI.nrrd\"\n";
			if(m_NeedToBeCropped==1) Script = Script + "\t\toriginalDTI= OutputPath + \"/Case\" + str(case+1) + \"_croppedDTI.nrrd\"\n";
			else Script = Script + "\t\toriginalDTI= OutputPath + \"/Case\" + str(case+1) + \"_DTIOrig.nrrd\"\n";
			Script = Script + "\t\tImplementCommand=\"ResampleDTIlogEuclidean \" + originalDTI + \" \" + LinearTransDTI + \" -f \" + LinearTranstfm + \" -R \" + AtlasFAref\n";
			Script = Script + "\t\tprint(\"||Case \" + str(case+1) + \" => $ \" + ImplementCommand)\n";
if(m_Overwrite==1)	Script = Script + "\t\tos.system(ImplementCommand)\n";
			else
			{
			Script = Script + "\t\tif not os.path.isfile(LinearTransDTI) : os.system(ImplementCommand)\n";
			Script = Script + "\t\telse : print(\"=> The file \\'\" + LinearTransDTI + \"\\' already exists so the command will not be executed\")\n";
			}
			Script = Script + "\t\tcase += 1\n\n";

//////////Generating FA of registered images
		Script = Script + "\tprint(\"\\n[LOOP \" + str(n) + \"/" + m_nbLoops_str + "] ======== Generating FA of registered images =========\")\n";
		Script = Script + "\tif n == 0 : case = 1\n";
		Script = Script + "\telse : case = 0\n";
		Script = Script + "\twhile case < len(allcases):\n";
			Script = Script + "\t\tLinearTransDTI= OutputPath + \"/Case\" + str(case+1) + \"_Loop\" + str(n) + \"_LinearTrans_DTI.nrrd\"\n";
			Script = Script + "\t\tif n == " + m_nbLoops_str + " : LoopFA= OutputPath + \"/Case\" + str(case+1) + \"_Loop\" + str(n) + \"_FinalFA.nrrd\" # the last FA will be the Final output\n";
			Script = Script + "\t\telse : LoopFA= OutputPath + \"/Case\" + str(case+1) + \"_Loop\" + str(n) + \"_FA.nrrd\"\n";
			Script = Script + "\t\tGeneLoopFACommand=\"dtiprocess --dti_image \" + LinearTransDTI + \" -f \" + LoopFA\n";
			Script = Script + "\t\tprint(\"||Case \" + str(case+1) + \" => $ \" + GeneLoopFACommand)\n";
if(m_Overwrite==1)	Script = Script + "\t\tos.system(GeneLoopFACommand)\n";
			else
			{
			Script = Script + "\t\tif not os.path.isfile(LoopFA) : os.system(GeneLoopFACommand)\n";
			Script = Script + "\t\telse : print(\"=> The file \\'\" + LoopFA + \"\\' already exists so the command will not be executed\")\n";
			}
			Script = Script + "\t\tcase += 1\n\n";
		
//////////FA Average of registered images with ImageMath
		Script = Script + "\tif n != " + m_nbLoops_str + " : # this will not be done for the last lap\n";
			Script = Script + "\t\tprint(\"\\n[LOOP \" + str(n) + \"/" + m_nbLoops_str + "] ======== Computing FA Average of registered images =========\")\n";
			Script = Script + "\t\tFAAverage = OutputPath + \"/Loop\" + str(n) + \"_FAAverage.nrrd\"\n";
			Script = Script + "\t\tif n == 0 : FAforAVG= OutputPath + \"/Case1_FA.nrrd\"\n";
			Script = Script + "\t\telse : FAforAVG= OutputPath + \"/Case1_Loop\" + str(n) + \"_FA.nrrd\"\n";
			Script = Script + "\t\tAverageCommand = \"ImageMath \" + FAforAVG + \" -outfile \" + FAAverage + \" -avg \"\n";
			Script = Script + "\t\tcase = 1\n";
			Script = Script + "\t\twhile case < len(allcases):\n";
				Script = Script + "\t\t\tFAforAVG= OutputPath + \"/Case\" + str(case+1) + \"_Loop\" + str(n) + \"_FA.nrrd \"\n";
				Script = Script + "\t\t\tAverageCommand= AverageCommand + FAforAVG\n";
				Script = Script + "\t\t\tcase += 1\n";
			Script = Script + "\t\tprint(\"=> $ \" + AverageCommand)\n";
if(m_Overwrite==1)	Script = Script + "\t\tos.system(AverageCommand)\n";
			else
			{
			Script = Script + "\t\tif not os.path.isfile(FAAverage) : os.system(AverageCommand)\n";
			Script = Script + "\t\telse : print(\"=> The file \\'\" + FAAverage + \"\\' already exists so the command will not be executed\")\n";
			}
			Script = Script + "\t\tAtlasFAref = FAAverage # the average becomes the reference\n\n";

		Script = Script + "\tn += 1\n\n";

	Script = Script + "print(\"\\n============ End of Pre processing =============\")\n\n";

	m_Script_Preprocess=Script;
}

void ScriptWriter::PreprocessTemplate ()
{
	std::string Script;

	std::cout<<"[Pre Processing]"; // command line display (no endl)

	Script = Script + "#!/usr/bin/python\n\n";
	Script = Script + "import os\n\n"; ///// To run a shell command : os.system("[shell command]")
	Script = Script + "print(\"\\n============ Pre processing =============\")\n\n";

	Script = Script + "allcases = [\"" + m_CasesPath[0];
	for (unsigned int i=1;i<m_CasesPath.size();i++) Script = Script + "\", \"" + m_CasesPath[i];
	Script = Script+ "\"]\n";
	
	Script = Script + "OutputPath= \"" + m_OutputPath + "/DTIAtlas/Affine_Registration\"\n";

	Script = Script + "AtlasFAref= \"" + m_TemplatePath + "\" #the reference will be the given template for the first loop\n";

////////Create directory for temporary files
	Script = Script + "if not os.path.isdir(OutputPath):\n";
		Script = Script + "\tos.mkdir(OutputPath)\n";
		Script = Script + "\tprint(\"\\n => Creation of the temporary files directory = \" + OutputPath)\n\n";

//////////Changing origin to (0,0,0)
	Script = Script + "print(\"\\n======== Changing origin of Images =========\")\n";
	Script = Script + "case = 0\n";
	Script = Script + "while case < len(allcases):\n";
		Script = Script + "\tDTIOrig= OutputPath + \"/Case\" + str(case+1) + \"_DTIOrig.nrrd\"\n";
		Script = Script + "\tChgOrigCommand = \"ImageMath \" + allcases[case] + \" -outfile \" + DTIOrig + \" -changeOrig 0,0,0 -type float\"\n";
		Script = Script + "\tprint(\"||Case \" + str(case+1) + \" => $ \" + ChgOrigCommand)\n";
		if(m_NeedToBeResampled==1)
		{
		Script = Script + "\tChgSpaceDirCommand = \"ResampleDTIlogEuclidean \" + DTIOrig + \" \" + DTIOrig + \" -R \" + OutputPath + \"/Case1_DTIOrig.nrrd\"\n";
		Script = Script + "\tif case>0 : print(\"||Case \" + str(case+1) + \" => $ \" + ChgSpaceDirCommand)\n";
		}
		if(m_Overwrite==1)
		{
		Script = Script + "\tos.system(ChgOrigCommand)\n";
if(m_NeedToBeResampled==1) Script = Script + "\tos.system(ChgSpaceDirCommand)\n";
		}
		else
		{
		Script = Script + "\tif not os.path.isfile(DTIOrig) :\n";
		Script = Script + "\t\tos.system(ChgOrigCommand)\n";
if(m_NeedToBeResampled==1) Script = Script + "\t\tos.system(ChgSpaceDirCommand)\n";
		Script = Script + "\telse : print(\"=> The file \\'\" + DTIOrig + \"\\' already exists so the commands will not be executed\")\n";
		}
		Script = Script + "\tcase += 1\n\n";

//////////Cropping DTI image
	if(m_NeedToBeCropped==1)
	{

	std::string CropSize_str [3];
	std::ostringstream out1;
	out1 << m_CropSize[0];
	CropSize_str[0] = out1.str();
	std::ostringstream out2;
	out2 << m_CropSize[1];
	CropSize_str[1] = out2.str();
	std::ostringstream out3;
	out3 << m_CropSize[2];
	CropSize_str[2] = out3.str();
	Script = Script + "CropSize=[\"" + CropSize_str[0] + "\",\"" + CropSize_str[1] + "\",\"" + CropSize_str[2] + "\"]\n\n";

	Script = Script + "print(\"\\n======== Cropping DTI Image =========\")\n";
	Script = Script + "case = 0\n";
	Script = Script + "while case < len(allcases):\n";
		Script = Script + "\tcroppedDTI= OutputPath + \"/Case\" + str(case+1) + \"_croppedDTI.nrrd\"\n";
		Script = Script + "\tDTIOrig= OutputPath + \"/Case\" + str(case+1) + \"_DTIOrig.nrrd\"\n";
		Script = Script + "\tCropCommand=\"CropDTI \" + DTIOrig + \" -o \" + croppedDTI + \" -size \" + CropSize[0] + \",\" + CropSize[1] + \",\" + CropSize[2] + \" -v\"\n";
		Script = Script + "\tprint(\"||Case \" + str(case+1) + \" => $ \" + CropCommand)\n";
if(m_Overwrite==1)Script = Script + "\tos.system(CropCommand)\n";
		else
		{
		Script = Script + "\tif not os.path.isfile(croppedDTI) : os.system(CropCommand)\n";
		Script = Script + "\telse : print(\"=> The file \\'\" + croppedDTI + \"\\' already exists so the command will not be executed\")\n";
		}
		Script = Script + "\tcase += 1\n\n";
	}

//////////Generating FA
	Script = Script + "print(\"\\n======== Generating FA =========\")\n";
	Script = Script + "case = 0\n";
	Script = Script + "while case < len(allcases):\n";
		if(m_NeedToBeCropped==1) Script = Script + "\tDTI= OutputPath + \"/Case\" + str(case+1) + \"_croppedDTI.nrrd\"\n";
		else Script = Script + "\tDTI= OutputPath + \"/Case\" + str(case+1) + \"_DTIOrig.nrrd\"\n";
		Script = Script + "\tFA= OutputPath + \"/Case\" + str(case+1) + \"_FA.nrrd\"\n";
		Script = Script + "\tGeneFACommand=\"dtiprocess --dti_image \" + DTI + \" -f \" + FA\n";
		Script = Script + "\tprint(\"||Case \" + str(case+1) + \" => $ \" + GeneFACommand)\n";
if(m_Overwrite==1)Script = Script + "\tos.system(GeneFACommand)\n";
		else
		{
		Script = Script + "\tif not os.path.isfile(FA) : os.system(GeneFACommand)\n";
		Script = Script + "\telse : print(\"=> The file \\'\" + FA + \"\\' already exists so the command will not be executed\")\n";
		}
		Script = Script + "\tcase += 1\n\n";

//////////Affine Registration and Normalization Loop
	Script = Script + "n = 0\n";
	Script = Script + "while n <= " + m_nbLoops_str + " :\n";

//////////Normalization
		Script = Script + "\tprint(\"\\n[LOOP \" + str(n) + \"/" + m_nbLoops_str + "] ======== Normalization =========\")\n";
		Script = Script + "\tcase = 0\n";
		Script = Script + "\twhile case < len(allcases):\n";
			Script = Script + "\t\tFA= OutputPath + \"/Case\" + str(case+1) + \"_FA.nrrd\"\n";
			Script = Script + "\t\tNormFA= OutputPath + \"/Case\" + str(case+1) + \"_Loop\" + str(n) + \"_NormFA.nrrd\"\n";
			Script = Script + "\t\tNormFACommand=\"ImageMath \" + FA + \" -outfile \" + NormFA + \" -matchHistogram \" + AtlasFAref\n";
			Script = Script + "\t\tprint(\"||Case \" + str(case+1) + \" => $ \" + NormFACommand)\n";
if(m_Overwrite==1)	Script = Script + "\t\tos.system(NormFACommand)\n";
			else
			{
			Script = Script + "\t\tif not os.path.isfile(NormFA) : os.system(NormFACommand)\n";
			Script = Script + "\t\telse : print(\"=> The file \\'\" + NormFA + \"\\' already exists so the command will not be executed\")\n";
			}
			Script = Script + "\t\tcase += 1\n\n";

//////////Affine with BrainsFit registration
		Script = Script + "\tprint(\"\\n[LOOP \" + str(n) + \"/" + m_nbLoops_str + "] ======== Affine with BrainsFit registration =========\")\n";
		Script = Script + "\tcase = 0\n";
		Script = Script + "\twhile case < len(allcases):\n";
			Script = Script + "\t\tNormFA= OutputPath + \"/Case\" + str(case+1) + \"_Loop\" + str(n) + \"_NormFA.nrrd\"\n";
			Script = Script + "\t\tLinearTranstfm= OutputPath + \"/Case\" + str(case+1) + \"_Loop\" + str(n) + \"_LinearTrans.txt\"\n";
			Script = Script + "\t\tLinearTrans= OutputPath + \"/Case\" + str(case+1) + \"_Loop\" + str(n) + \"_LinearTrans_FA.nrrd\"\n";
			Script = Script + "\t\tAffineCommand=\"BRAINSFit --fixedVolume \" + AtlasFAref + \" --movingVolume \" + NormFA + \" --initializeTransformMode useCenterOfHeadAlign --useAffine --outputVolume \" + LinearTrans + \" --outputTransform \" + LinearTranstfm\n";
			Script = Script + "\t\tprint(\"||Case \" + str(case+1) + \" => $ \" + AffineCommand)\n";
if(m_Overwrite==1)	Script = Script + "\t\tif 1 :\n";
else			Script = Script + "\t\tif not os.path.isfile(LinearTranstfm) or not os.path.isfile(LinearTrans) :\n";
				Script = Script + "\t\t\tos.system(AffineCommand)\n";
				Script = Script + "\t\t\tif n == " + m_nbLoops_str + " : #changing into float only the last tfm files\n";
					Script = Script + "\t\t\t\tprint(\"=> Converting the transformation matrix from double to float for AtlasWerks\\n\")\n";
					Script = Script + "\t\t\t\ttxtFile = open(LinearTranstfm,'r')\n";
					Script = Script + "\t\t\t\tLinearTranstfmFloat = OutputPath + \"/Case\" + str(case+1) + \"_Loop\" + str(n) + \"_LinearTrans_float.txt\"\n";
					Script = Script + "\t\t\t\tnewFile = open(LinearTranstfmFloat,'w')\n";
					Script = Script + "\t\t\t\trd = 'string to enter the while loop'\n";
					Script = Script + "\t\t\t\twhile rd: # '' is equal to false in python => rd==0 is the end of th file\n";
						Script = Script + "\t\t\t\t\trd = txtFile.readline()\n";
						Script = Script + "\t\t\t\t\tnewFile.write(rd.replace('double','float'))\n";
					Script = Script + "\t\t\t\ttxtFile.close()\n";
					Script = Script + "\t\t\t\tnewFile.close()\n";
if(m_Overwrite==0)	Script = Script + "\t\telif os.path.isfile(LinearTranstfm) : print(\"=> The file \" + LinearTranstfm + \" already exists so the command will not be executed\")\n";
if(m_Overwrite==0)	Script = Script + "\t\telif os.path.isfile(LinearTrans) : print(\"=> The file \\'\" + LinearTrans + \"\\' already exists so the command will not be executed\")\n";
			Script = Script + "\t\tcase += 1\n\n";

//////////Implementing the affine registration
		Script = Script + "\tprint(\"\\n[LOOP \" + str(n) + \"/" + m_nbLoops_str + "] ======== Implementing the Affine registration =========\")\n";
		Script = Script + "\tcase = 0\n";
		Script = Script + "\twhile case < len(allcases):\n";
			Script = Script + "\t\tLinearTranstfm= OutputPath + \"/Case\" + str(case+1) + \"_Loop\" + str(n) + \"_LinearTrans.txt\"\n";
			Script = Script + "\t\tLinearTransDTI= OutputPath + \"/Case\" + str(case+1) + \"_Loop\" + str(n) + \"_LinearTrans_DTI.nrrd\"\n";
			if(m_NeedToBeCropped==1) Script = Script + "\t\toriginalDTI= OutputPath + \"/Case\" + str(case+1) + \"_croppedDTI.nrrd\"\n";
			else Script = Script + "\t\toriginalDTI= OutputPath + \"/Case\" + str(case+1) + \"_DTIOrig.nrrd\"\n";
			Script = Script + "\t\tImplementCommand=\"ResampleDTIlogEuclidean \" + originalDTI + \" \" + LinearTransDTI + \" -f \" + LinearTranstfm + \" -R \" + AtlasFAref\n";
			Script = Script + "\t\tprint(\"||Case \" + str(case+1) + \" => $ \" + ImplementCommand)\n";
if(m_Overwrite==1)	Script = Script + "\t\tos.system(ImplementCommand)\n";
			else
			{
			Script = Script + "\t\tif not os.path.isfile(LinearTransDTI) : os.system(ImplementCommand)\n";
			Script = Script + "\t\telse : print(\"=> The file \\'\" + LinearTransDTI + \"\\' already exists so the command will not be executed\")\n";
			}
			Script = Script + "\t\tcase += 1\n\n";

//////////Generating FA of registered images
		Script = Script + "\tprint(\"\\n[LOOP \" + str(n) + \"/" + m_nbLoops_str + "] ======== Generating FA of registered images =========\")\n";
		Script = Script + "\tcase = 0\n";
		Script = Script + "\twhile case < len(allcases):\n";
			Script = Script + "\t\tLinearTransDTI= OutputPath + \"/Case\" + str(case+1) + \"_Loop\" + str(n) + \"_LinearTrans_DTI.nrrd\"\n";
			Script = Script + "\t\tif n == " + m_nbLoops_str + " : LoopFA= OutputPath + \"/Case\" + str(case+1) + \"_Loop\" + str(n) + \"_FinalFA.nrrd\" # the last FA will be the Final output\n";
			Script = Script + "\t\telse : LoopFA= OutputPath + \"/Case\" + str(case+1) + \"_Loop\" + str(n) + \"_FA.nrrd\"\n";
			Script = Script + "\t\tGeneLoopFACommand=\"dtiprocess --dti_image \" + LinearTransDTI + \" -f \" + LoopFA\n";
			Script = Script + "\t\tprint(\"||Case \" + str(case+1) + \" => $ \" + GeneLoopFACommand)\n";
if(m_Overwrite==1)	Script = Script + "\t\tos.system(GeneLoopFACommand)\n";
			else
			{
			Script = Script + "\t\tif not os.path.isfile(LoopFA) : os.system(GeneLoopFACommand)\n";
			Script = Script + "\t\telse : print(\"=> The file \\'\" + LoopFA + \"\\' already exists so the command will not be executed\")\n";
			}
			Script = Script + "\t\tcase += 1\n\n";

//////////FA Average of registered images with ImageMath
		Script = Script + "\tif n != " + m_nbLoops_str + " : # this will not be done for the last lap\n";
			Script = Script + "\t\tprint(\"\\n[LOOP \" + str(n) + \"/" + m_nbLoops_str + "] ======== Computing FA Average of registered images =========\")\n";
			Script = Script + "\t\tFAforAVG= OutputPath + \"/Case1_Loop\" + str(n) + \"_FA.nrrd\"\n";
			Script = Script + "\t\tFAAverage = OutputPath + \"/Loop\" + str(n) + \"_FAAverage.nrrd\"\n";
			Script = Script + "\t\tAverageCommand = \"ImageMath \" + FAforAVG + \" -outfile \" + FAAverage + \" -avg \"\n";
			Script = Script + "\t\tcase = 1\n";
			Script = Script + "\t\twhile case < len(allcases):\n";
				Script = Script + "\t\t\tFAforAVG= OutputPath + \"/Case\" + str(case+1) + \"_Loop\" + str(n) + \"_FA.nrrd \"\n";
				Script = Script + "\t\t\tAverageCommand= AverageCommand + FAforAVG\n";
				Script = Script + "\t\t\tcase += 1\n";
			Script = Script + "\t\tprint(\"=> $ \" + AverageCommand)\n";
if(m_Overwrite==1)	Script = Script + "\t\tos.system(AverageCommand)\n";
			else
			{
			Script = Script + "\t\tif not os.path.isfile(FAAverage) : os.system(AverageCommand)\n";
			Script = Script + "\t\telse : print(\"=> The file \\'\" + FAAverage + \"\\' already exists so the command will not be executed\")\n";
			}
			Script = Script + "\t\tAtlasFAref = FAAverage # the average becomes the reference\n\n";

		Script = Script + "\tn += 1\n\n";

	Script = Script + "print(\"\\n============ End of Pre processing =============\")\n\n";

	m_Script_Preprocess=Script;
}

void ScriptWriter::AtlasBuilding()
{
	std::string Script;

	std::cout<<"[AtlasBuilding]"; // command line display (no endl)

	Script = Script + "#!/usr/bin/python\n\n";
	Script = Script + "import os\n\n"; ///// To run a shell command : os.system("[shell command]")
	Script = Script + "print(\"\\n============ Atlas Building =============\")\n\n";

	Script = Script + "DeformPath= \"" + m_OutputPath + "/DTIAtlas/NonLinear_Registration\"\n";
	Script = Script + "AffinePath= \"" + m_OutputPath + "/DTIAtlas/Affine_Registration\"\n";
	Script = Script + "FinalPath= \"" + m_OutputPath + "/DTIAtlas/Final_Atlas\"\n";

//////////Scale Levels: -scaleLevel --numberOfIterations --alpha --beta --gamma --maxPerturbation
	std::ostringstream outSL;
	outSL << m_AtlasWerksScaleLevels[0][0];
	std::string SL_str = outSL.str();
	Script = Script + "ScaleLevels = [[" + SL_str;
	for(unsigned int j=1;j<6;j++)
	{
		std::ostringstream outSL;
		outSL << m_AtlasWerksScaleLevels[0][j];
		std::string SL_str = outSL.str();
		Script = Script + "," + SL_str;
	}
	Script = Script + "]";

	for (unsigned int i=1;i<m_AtlasWerksScaleLevels.size();i++) 
	{
		std::ostringstream outSL;
		outSL << m_AtlasWerksScaleLevels[i][0];
		std::string SL_str = outSL.str();
		Script = Script + ",[" + SL_str;
		for(unsigned int j=1;j<6;j++)
		{
			std::ostringstream outSL;
			outSL << m_AtlasWerksScaleLevels[i][j];
			std::string SL_str = outSL.str();
			Script = Script + "," + SL_str;
		}
		Script = Script + "]";
	}	
	Script = Script+ "]\n\n";

////////Create directory for temporary files and final
	Script = Script + "if not os.path.isdir(DeformPath):\n";
		Script = Script + "\tprint(\"\\n => Creation of the Deformation transform directory = \" + DeformPath)\n";
		Script = Script + "\tos.mkdir(DeformPath)\n\n";
	Script = Script + "if not os.path.isdir(FinalPath):\n";
		Script = Script + "\tprint(\"\\n => Creation of the Final Atlas directory = \" + FinalPath)\n";
		Script = Script + "\tos.mkdir(FinalPath)\n\n";

//////////Cases variables:
	Script = Script + "allFAs = [AffinePath + \"/Case1_Loop" + m_nbLoops_str + "_NormFA.nrrd\"";
	for (unsigned int i=1;i<m_CasesPath.size();i++) 
	{
		std::ostringstream outi;
		outi << i+1;
		std::string i_str = outi.str();
		Script = Script + ", AffinePath + \"/Case" + i_str + "_Loop" + m_nbLoops_str + "_NormFA.nrrd\"";
	}	
	Script = Script+ "]\n\n";

	Script = Script + "alltfms = [AffinePath + \"/Case1_Loop" + m_nbLoops_str + "_LinearTrans_float.txt\"";
	for (unsigned int i=1;i<m_CasesPath.size();i++) 
	{
		std::ostringstream outi0;
		outi0 << i+1;
		std::string i0_str = outi0.str();
		Script = Script + ", AffinePath + \"/Case" + i0_str + "_Loop" + m_nbLoops_str + "_LinearTrans_float.txt\"";
	}	
	Script = Script+ "]\n\n";

	if(m_NeedToBeCropped==1) Script = Script + "allcases = [AffinePath + \"/Case1_croppedDTI.nrrd\"";
	else Script = Script + "allcases = [AffinePath + \"/Case1_DTIOrig.nrrd\"";
	for (unsigned int i=1;i<m_CasesPath.size();i++) 
	{
		std::ostringstream outi2;
		outi2 << i+1;
		std::string i2_str = outi2.str();
		if(m_NeedToBeCropped==1) Script = Script + ", AffinePath + \"/Case" + i2_str + "_croppedDTI.nrrd\"";
		else Script = Script + ", AffinePath + \"/Case" + i2_str + "_DTIOrig.nrrd\"";
	}
	Script = Script + "]\n\n";	

//////////AtlasWerks Command:
	Script = Script + "print(\"\\n======== Computing the Deformation Fields =========\")\n";
	Script = Script + "FinalAtlasPrefix= DeformPath + \"/AverageImage_\"\n";
	Script = Script + "FinalAtlasDefPrefix= DeformPath + \"/DeformedImage_\"\n";
	Script = Script + "FinalAtlasDefFieldPrefix= DeformPath + \"/DeformationField_\"\n";
	Script = Script + "FinalAtlasInvDefFieldPrefix= DeformPath + \"/InverseDeformationField_\"\n";
/*	Script = Script + "AtlasBCommand= \"AtlasWerks --outputImageFilenamePrefix \" + FinalAtlasPrefix + \" --outputDeformedImageFilenamePrefix \" + FinalAtlasDefPrefix + \" --outputHFieldFilenamePrefix \" + FinalAtlasDefFieldPrefix + \" --outputHInvFieldFilenamePrefix \" + FinalAtlasInvDefFieldPrefix\n";
	Script = Script + "scale = 0\n";
	Script = Script + "while scale < len(ScaleLevels): #ScaleLevels[scale][1]= nb of iterations\n";
		Script = Script + "\tif ScaleLevels[scale][1] > 0 : AtlasBCommand = AtlasBCommand + \" --scaleLevel \" + str(ScaleLevels[scale][0]) + \" --numberOfIterations \" + str(ScaleLevels[scale][1]) + \" --alpha \" + str(ScaleLevels[scale][2]) + \" --beta \" + str(ScaleLevels[scale][3]) + \" --gamma \" + str(ScaleLevels[scale][4]) + \" --maxPerturbation \" + str(ScaleLevels[scale][5])\n";
		Script = Script + "\tscale += 1\n";
	Script = Script + "case = 0\n";
	Script = Script + "while case < len(allcases):\n";
		Script = Script + "\tAtlasBCommand = AtlasBCommand + \" \" + allFAs[case]\n";
		Script = Script + "\tcase += 1\n";
	Script = Script + "case = 0\n";
	Script = Script + "while case < len(allcases):\n";
		Script = Script + "\tAtlasBCommand = AtlasBCommand + \" \" + alltfms[case]\n";
		Script = Script + "\tcase += 1\n";
*/
	Script = Script + "XMLFile= DeformPath + \"/AtlasWerksParameters.xml\"\n";
	Script = Script + "ParsedFile= DeformPath + \"/ParsedXML.xml\"\n";
	Script = Script + "AtlasBCommand= \"/biomed-resimg/tools/AtlasWerks/AtlasWerks_0.1.4_linux64/AtlasWerks -f \" + XMLFile + \" -o \" + ParsedFile\n";
	Script = Script + "print(\"=> $ \" + AtlasBCommand)\n";
if(m_Overwrite==1)Script = Script + "if 1 :\n";
//else	Script = Script + "if not os.path.isfile(FinalAtlasPrefix + str( len(ScaleLevels)-1 ) + \".mhd\") :\n";
else	Script = Script + "if not os.path.isfile(FinalAtlasPrefix + \".mhd\") :\n";
		Script = Script + "\tos.system(AtlasBCommand)\n";
		Script = Script + "\tprint(\"\\n======== Renaming the files generated by AtlasWerks =========\")\n";
		Script = Script + "\tcase = 0\n";
		Script = Script + "\twhile case < len(allcases):\n";
			Script = Script + "\t\tif case<10 :\n";
//				Script = Script + "\t\t\toriginalImage=DeformPath + \"/DeformedImage_\" + str( len(ScaleLevels)-1 ) + \"_000\" + str(case) + \".mhd\"\n";
				Script = Script + "\t\t\toriginalImage=DeformPath + \"/DeformedImage_000\" + str(case) + \".mhd\"\n";
				Script = Script + "\t\t\toriginalHField=DeformPath + \"/DeformationField_000\" + str(case) + \".mhd\"\n";
				Script = Script + "\t\t\toriginalInvHField=DeformPath + \"/InverseDeformationField_000\" + str(case) + \".mhd\"\n";
			Script = Script + "\t\tif case>10 and case <100 :\n";
//				Script = Script + "\t\t\toriginalImage=DeformPath + \"/DeformedImage_\" + str(len(ScaleLevels)-1) + \"_00\" + str(case) + \".mhd\"\n";
				Script = Script + "\t\t\toriginalImage=DeformPath + \"/DeformedImage_00\" + str(case) + \".mhd\"\n";
				Script = Script + "\t\t\toriginalHField=DeformPath + \"/DeformationField_00\" + str(case) + \".mhd\"\n";
				Script = Script + "\t\t\toriginalInvHField=DeformPath + \"/InverseDeformationField_00\" + str(case) + \".mhd\"\n";
			Script = Script + "\t\tif case>100 and case <1000 :\n";
//				Script = Script + "\t\t\toriginalImage=DeformPath + \"/DeformedImage_\" + str(len(ScaleLevels)-1) + \"_0\" + str(case) + \".mhd\"\n";
				Script = Script + "\t\t\toriginalImage=DeformPath + \"/DeformedImage_0\" + str(case) + \".mhd\"\n";
				Script = Script + "\t\t\toriginalHField=DeformPath + \"/DeformationField_0\" + str(case) + \".mhd\"\n";
				Script = Script + "\t\t\toriginalInvHField=DeformPath + \"/InverseDeformationField_0\" + str(case) + \".mhd\"\n";
			Script = Script + "\t\tif case>1000 :\n";
//				Script = Script + "\t\t\toriginalImage=DeformPath + \"/DeformedImage_\" + str(len(ScaleLevels)-1) + \"_\" + str(case) + \".mhd\"\n";
				Script = Script + "\t\t\toriginalImage=DeformPath + \"/DeformedImage_\" + str(case) + \".mhd\"\n";
				Script = Script + "\t\t\toriginalHField=DeformPath + \"/DeformationField_\" + str(case) + \".mhd\"\n";
				Script = Script + "\t\t\toriginalInvHField=DeformPath + \"/InverseDeformationField_\" + str(case) + \".mhd\"\n";
			Script = Script + "\t\tNewImage= DeformPath + \"/Case\" + str(case+1) + \"_NonLinearTrans_FA.mhd\"\n";
			Script = Script + "\t\tNewHField=DeformPath + \"/Case\" + str(case+1) + \"_DeformationField.mhd\"\n";
			Script = Script + "\t\tNewInvHField=DeformPath + \"/Case\" + str(case+1) + \"_InverseDeformationField.mhd\"\n";
			Script = Script + "\t\tprint(\"||Case \" + str(case+1) + \" => Renaming \\'\" + originalImage + \"\\' to \\'\" + NewImage + \"\\'\")\n";
			Script = Script + "\t\tprint(\"||Case \" + str(case+1) + \" => Renaming \\'\" + originalHField + \"\\' to \\'\" + NewHField + \"\\'\")\n";
			Script = Script + "\t\tprint(\"||Case \" + str(case+1) + \" => Renaming \\'\" + originalInvHField + \"\\' to \\'\" + NewInvHField + \"\\'\")\n";
			Script = Script + "\t\tos.rename(originalImage,NewImage)\n";
			Script = Script + "\t\tos.rename(originalHField,NewHField)\n";
			Script = Script + "\t\tos.rename(originalInvHField,NewInvHField)\n";
			Script = Script + "\t\tcase += 1\n";
//if(m_Overwrite==0)Script = Script + "else : print(\"=> The file \\'\" + FinalAtlasPrefix + str( len(ScaleLevels)-1 ) + \".mhd\\' already exists so the command will not be executed\")\n\n";
if(m_Overwrite==0)Script = Script + "else : print(\"=> The file \\'\" + FinalAtlasPrefix + \".mhd\\' already exists so the command will not be executed\")\n\n";

///////////Apply deformation fields
	Script = Script + "print(\"\\n======== Applying deformation fields to original DTIs =========\")\n";
	Script = Script + "case = 0\n";
	Script = Script + "while case < len(allcases):\n";
		Script = Script + "\tFinalDTI= FinalPath + \"/Case\" + str(case+1) + \"_FinalDTI.nrrd\"\n";
		if(m_NeedToBeCropped==1) Script = Script + "\toriginalDTI= AffinePath + \"/Case\" + str(case+1) + \"_croppedDTI.nrrd\"\n";
		else Script = Script + "\toriginalDTI= allcases[case]\n";
		if(m_nbLoops==0)
		{
			if(m_RegType==1) Script = Script + "\tRef= AffinePath + \"/Case1_FA.nrrd\"\n";
			else Script = Script + "\tRef= \"" + m_TemplatePath + "\"\n";
		}
		else 
		{
			std::ostringstream out;
			out << m_nbLoops-1;
			std::string nbLoops1_str = out.str();
		Script = Script + "\tRef = AffinePath + \"/Loop" + nbLoops1_str + "_FAAverage.nrrd\"\n"; // an average image has been generated in the loops of affine reg for reference
		}
		Script = Script + "\tHField= DeformPath + \"/Case\" + str(case+1) + \"_DeformationField.mhd\"\n";
		Script = Script + "\tFinalReSampCommand=\"ResampleDTIlogEuclidean -R \" + Ref + \" -H \" + HField + \" \" + originalDTI + \" \" + FinalDTI\n";
if(m_InterpolType.compare("Linear")==0)			Script = Script + "\tFinalReSampCommand = FinalReSampCommand + \" -i linear\"\n";
if(m_InterpolType.compare("Nearest Neighborhoor")==0)	Script = Script + "\tFinalReSampCommand = FinalReSampCommand + \" -i nn\"\n";
if(m_InterpolType.compare("Windowed Sinc")==0)		Script = Script + "\tFinalReSampCommand = FinalReSampCommand + \" -i ws\"\n";
if(m_InterpolType.compare("BSpline")==0)		Script = Script + "\tFinalReSampCommand = FinalReSampCommand + \" -i bs\"\n";
		Script = Script + "\tprint(\"||Case \" + str(case+1) + \" => $ \" + FinalReSampCommand)\n";
if(m_Overwrite==1)Script = Script + "\tos.system(FinalReSampCommand)\n";
		else
		{
		Script = Script + "\tif not os.path.isfile(FinalDTI) : os.system(FinalReSampCommand)\n";
		Script = Script + "\telse : print(\"=> The file \\'\" + FinalDTI + \"\\' already exists so the command will not be executed\")\n";
		}
		Script = Script + "\tcase += 1\n\n";

/////////// dtiaverage computing
	Script = Script + "print(\"\\n======== Computing the final DTI average =========\")\n";
	Script = Script + "DTIAverage = FinalPath + \"/FinalAtlasDTI.nrrd\"\n";
	Script = Script + "AverageCommand = \"dtiaverage \"\n";
	Script = Script + "case = 0\n";
	Script = Script + "while case < len(allcases):\n";
		Script = Script + "\tDTIforAVG= \"--inputs \" + FinalPath + \"/Case\" + str(case+1) + \"_FinalDTI.nrrd \"\n";
		Script = Script + "\tAverageCommand = AverageCommand + DTIforAVG\n";
		Script = Script + "\tcase += 1\n";
	Script = Script + "AverageCommand = AverageCommand + \"--tensor_output \" + DTIAverage\n";
if(m_AverageStatMethod.compare("PGA")==0)		Script = Script + "AverageCommand = AverageCommand + \" -m pga\"\n";
if(m_AverageStatMethod.compare("Euclidean")==0) 	Script = Script + "AverageCommand = AverageCommand + \" -m euclidean\"\n";
if(m_AverageStatMethod.compare("Log Euclidean")==0)	Script = Script + "AverageCommand = AverageCommand + \" -m log-euclidean\"\n";
	Script = Script + "print(\"=> $ \" + AverageCommand)\n";
if(m_Overwrite==1)Script = Script + "if 1 : \n";
else	Script = Script + "if not os.path.isfile(DTIAverage) : \n";
		Script = Script + "\tos.system(AverageCommand)\n";
////////////computing some images from the final DTI with dtiprocess
		Script = Script + "\tFA= FinalPath + \"/FinalAtlasFA.nrrd\"\n";
		Script = Script + "\tcFA= FinalPath + \"/FinalAtlasColorFA.nrrd\"\n";
		Script = Script + "\tRD= FinalPath + \"/FinalAtlasRD.nrrd\"\n"; // Radial Diffusivity
		Script = Script + "\tMD= FinalPath + \"/FinalAtlasMD.nrrd\"\n"; // Mean Diffusivity
		Script = Script + "\tAD= FinalPath + \"/FinalAtlasAD.nrrd\"\n"; // Axial Diffusivity
		Script = Script + "\tGeneFACommand=\"dtiprocess --dti_image \" + DTIAverage + \" -f \" + FA + \" -m \" + MD + \" --color_fa_output \" + cFA + \" --RD_output \" + RD + \" --lambda1_output \" + AD\n";
		Script = Script + "\tprint(\"=> $ \" + GeneFACommand)\n";
		Script = Script + "\tos.system(GeneFACommand)\n";
		Script = Script + "\tDbleToFloatCommand=\"unu convert -t float -i \" + DTIAverage + \" | unu save -f nrrd -e gzip -o \" + FinalPath + \"/FinalAtlasDTI_float.nrrd\"\n";
		Script = Script + "\tprint(\"=> $ \" + DbleToFloatCommand)\n";
		Script = Script + "\tos.system(DbleToFloatCommand)\n";
if(m_Overwrite==0)Script = Script + "else : print(\"=> The file \\'\" + DTIAverage + \"\\' already exists so the command will not be executed\")\n\n";

	Script = Script + "print(\"\\n============ End of Atlas Building =============\")\n\n";

	m_Script_AtlasBuilding=Script;
}

void ScriptWriter::MainScript()
{
	std::string Script;

	std::cout<<"[Main]"<<std::endl; // command line display (no endl)

	Script = Script + "#!/usr/bin/python\n\n";
	Script = Script + "import os\n\n"; ///// To run a shell command : os.system("[shell command]")
	Script = Script + "print(\"\\n=============== Main Script ================\")\n\n";

	Script = Script + "OutputPath= \"" + m_OutputPath + "/DTIAtlas\"\n";

//////////Call the other scripts  
	Script = Script + "PrePScriptCommand= OutputPath + \"/Script/DTIAtlasBuilder_Preprocess.script\"\n";
	Script = Script + "print(\"\\n=> $ \" + PrePScriptCommand)\n";
	Script = Script + "os.system(PrePScriptCommand)\n";

	Script = Script + "AtlasBuildingCommand= OutputPath + \"/Script/DTIAtlasBuilder_AtlasBuilding.script\"\n";
	Script = Script + "print(\"\\n=> $ \" + AtlasBuildingCommand)\n";
	Script = Script + "os.system(AtlasBuildingCommand)\n";

	Script = Script + "print(\"\\n============ End of execution =============\\n\")\n";

	m_Script_Main=Script;
}

std::string ScriptWriter::getScript_Preprocess()
{
	return m_Script_Preprocess;
}

std::string ScriptWriter::getScript_AtlasBuilding()
{
	return m_Script_AtlasBuilding;
}

std::string ScriptWriter::getScript_Main()
{
	return m_Script_Main;
}

int ScriptWriter::setCroppingSize() // returns 0 if no cropping , 1 if cropping needed
{	
	m_NeedToBeCropped=0;

	int MaxSize [3] = {-1,-1,-1};

/////////itk type definitions
	typedef itk::Image < double , 4 > ImageType; //itk type for image
	typedef itk::ImageFileReader <ImageType> ReaderType; //itk reader class to open an image
	ReaderType::Pointer reader=ReaderType::New();
	ImageType::RegionType region;

//////////Testing all the cases
	for (unsigned int i=0;i<m_CasesPath.size();i++) // read the headers of all files
	{
		reader->SetFileName( m_CasesPath[i] ); //Label is a path => open the image
		reader->UpdateOutputInformation(); // get the informations in the header
		region = reader->GetOutput()->GetLargestPossibleRegion();

		if((int)region.GetSize()[0]!=MaxSize[0]) //x coordinate
		{
			if((int)region.GetSize()[0]>MaxSize[0]) MaxSize[0]=region.GetSize()[0];
			if(i>0) m_NeedToBeCropped=1; 
		}
		if((int)region.GetSize()[1]!=MaxSize[1]) //y coordinate
		{
			if((int)region.GetSize()[1]>MaxSize[1]) MaxSize[1]=region.GetSize()[1];
			if(i>0) m_NeedToBeCropped=1; 
		}
		if((int)region.GetSize()[2]!=MaxSize[2]) //z coordinate
		{
			if((int)region.GetSize()[2]>MaxSize[2]) MaxSize[2]=region.GetSize()[2];
			if(i>0) m_NeedToBeCropped=1; 
		}
	}

	if(m_NeedToBeCropped==1)
	{
		m_CropSize[0] = MaxSize[0];
		m_CropSize[1] = MaxSize[1];
		m_CropSize[2] = MaxSize[2];
		std::cout<<"| Crop size computed : ["<<m_CropSize[0]<<";"<<m_CropSize[1]<<";"<<m_CropSize[2]<<"]"<<std::endl;
		return 1;
	}

	else return 0;
}

int ScriptWriter::CheckVoxelSize() // returns 0 if voxel size OK , otherwise 1
{	
/////////itk definitions
	typedef itk::Image < double , 4 > ImageType; //itk type for image
	typedef itk::ImageFileReader <ImageType> ReaderType; //itk reader class to open an image

	ReaderType::Pointer reader=ReaderType::New();
	reader->SetFileName( m_CasesPath[0] ); //Label is a path => open the image
	reader->UpdateOutputInformation();
	const ImageType::SpacingType& sp = reader-> GetOutput()->GetSpacing();
	double RefSpacing [3]; // the spacing of the first case is the reference for this computation
	RefSpacing[0]=sp[0];
	RefSpacing[1]=sp[1];
	RefSpacing[2]=sp[2];

//////////Testing all the cases
	for (unsigned int i=1;i<m_CasesPath.size();i++) // read the headers of all files
	{
		reader->SetFileName( m_CasesPath[i] ); //Label is a path => open the image
		reader->UpdateOutputInformation(); // get the informations in the header
		const ImageType::SpacingType& sp = reader-> GetOutput() -> GetSpacing();

		if( fabs(sp[0]-RefSpacing[0])/RefSpacing[0] > 0.05) return 1;
		if( fabs(sp[1]-RefSpacing[1])/RefSpacing[1] > 0.05) return 1;
		if( fabs(sp[2]-RefSpacing[2])/RefSpacing[2] > 0.05) return 1;
	}

	return 0;
}

int ScriptWriter::CheckSpaceDir() // returns 0 if space dir OK , otherwise 1 => 
{
	m_NeedToBeResampled=0;

/////////itk definitions
	typedef itk::Image < double , 4 > ImageType; //itk type for image
	typedef itk::ImageFileReader <ImageType> ReaderType; //itk reader class to open an image

	ReaderType::Pointer reader=ReaderType::New();
	reader->SetFileName( m_CasesPath[0] ); //Label is a path => open the image
	reader->UpdateOutputInformation();
	const ImageType::DirectionType& dir = reader-> GetOutput()->GetDirection();
	double RefDir [3][3]; // the spacing of the first case is the reference for this computation
	RefDir[0][0]=dir[0][0];
	RefDir[0][1]=dir[0][1];
	RefDir[0][2]=dir[0][2];
	RefDir[1][0]=dir[1][0];
	RefDir[1][1]=dir[1][1];
	RefDir[1][2]=dir[1][2];
	RefDir[2][0]=dir[2][0];
	RefDir[2][1]=dir[2][1];
	RefDir[2][2]=dir[2][2];

//////////Testing all the cases
	for (unsigned int i=1;i<m_CasesPath.size();i++) // read the headers of all files
	{
		reader->SetFileName( m_CasesPath[i] ); //Label is a path => open the image
		reader->UpdateOutputInformation(); // get the informations in the header
		const ImageType::DirectionType& dir = reader-> GetOutput() -> GetDirection();

		if( (fabs(dir[0][0]-RefDir[0][0])/RefDir[0][0] > 0.05) || (fabs(dir[0][1]-RefDir[0][1])/RefDir[0][1] > 0.05) || (fabs(dir[0][2]-RefDir[0][2])/RefDir[0][2] > 0.05) ) m_NeedToBeResampled=1;
		if( (fabs(dir[1][0]-RefDir[1][0])/RefDir[1][0] > 0.05) || (fabs(dir[1][1]-RefDir[1][1])/RefDir[1][1] > 0.05) || (fabs(dir[1][2]-RefDir[1][2])/RefDir[1][2] > 0.05) ) m_NeedToBeResampled=1;
		if( (fabs(dir[2][0]-RefDir[2][0])/RefDir[2][0] > 0.05) || (fabs(dir[2][1]-RefDir[2][1])/RefDir[2][1] > 0.05) || (fabs(dir[2][2]-RefDir[2][2])/RefDir[2][2] > 0.05) ) m_NeedToBeResampled=1;
	}

	if(m_NeedToBeResampled==1) return 1;
	else return 0;
}

void ScriptWriter::setCasesPath(std::vector < std::string > CasesPath)
{
	for (unsigned int i=0;i<CasesPath.size();i++) m_CasesPath.push_back( CasesPath[i] );
}
void ScriptWriter::clearCasesPath()
{
	m_CasesPath.clear();
}

void ScriptWriter::setOutputPath(std::string OutputPath)
{
	m_OutputPath = OutputPath;
}

void ScriptWriter::setRegType(int RegType)
{
	m_RegType = RegType;
}

void ScriptWriter::setnbLoops(int nbLoops)
{
	m_nbLoops = nbLoops;
}

void ScriptWriter::setTemplatePath(std::string TemplatePath)
{
	m_TemplatePath = TemplatePath;
}

void ScriptWriter::setOverwrite(int Overwrite)
{
	m_Overwrite = Overwrite;
}

void ScriptWriter::setScaleLevels(std::vector< std::vector<double> > AtlasWerksScaleLevels)
{
	for(int i=0;i< (int) AtlasWerksScaleLevels.size();i++)
	{
		m_AtlasWerksScaleLevels.push_back( AtlasWerksScaleLevels[i] );
	}
}

void ScriptWriter::setInterpolType(std::string Type)
{
	m_InterpolType = Type;
}

void ScriptWriter::setAverageStatMethod(std::string Method)
{
	m_AverageStatMethod = Method;
}

