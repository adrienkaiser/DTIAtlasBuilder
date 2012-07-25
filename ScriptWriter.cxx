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
	std::cout<<"| Number of loops in the Registration Loop : "<<m_nbLoops-1<<std::endl; // command line display
	std::cout<<"| Writing begin: "; // command line display
	
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

	std::ostringstream out;
	out << m_nbLoops;
	std::string nbLoops_str = out.str();
	Script = Script + "nbLoops= " + nbLoops_str + "\n\n";

	std::ostringstream outO;
	outO << m_Overwrite;
	std::string Overwrite_str = outO.str();
	Script = Script + "Overwrite= " + Overwrite_str + "\n\n";

////////Create directory for temporary files
	Script = Script + "if not os.path.isdir(OutputPath):\n";
		Script = Script + "\tos.mkdir(OutputPath)\n";
		Script = Script + "\tprint(\"\\n => Creation of the temporary files directory = \" + OutputPath)\n\n";

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

//////////Cropping DTI image
	Script = Script + "print(\"\\n======== Cropping DTI Image =========\")\n";
	Script = Script + "case = 0\n";
	Script = Script + "while case < len(allcases):\n\n";//Script = Script + "for case in allcases\n"; // here 'case' will take the values of 'allcases' (carac strings)
			//Script = Script + "\tprint(\"Path = \" + allcases[case])\n\n";
			Script = Script + "\tcroppedDTI= OutputPath + \"/Case\" + str(case+1) + \"_croppedDTI.nrrd\"\n";
			Script = Script + "\tCropCommand=\"CropDTI \" + allcases[case] + \" -o \" + croppedDTI + \" -size \" + CropSize[0] + \",\" + CropSize[1] + \",\" + CropSize[2] + \" -v\"\n";
////CropDTI infile [-o outfile] [-region  px,py,pz,w,h,d | -size w,h,d] [-v]
			Script = Script + "\tprint(\"||Case \" + str(case+1) + \" => $ \" + CropCommand)\n";
			Script = Script + "\tif Overwrite==1 or not os.path.isfile(croppedDTI) : os.system(CropCommand)\n";
			Script = Script + "\telse : print(\"=> The file \\'\" + croppedDTI + \"\\' already exists so the command will not be executed\")\n";
			Script = Script + "\tcase += 1\n\n";
	}

//////////Generating FA
	Script = Script + "print(\"\\n======== Generating FA =========\")\n";
	Script = Script + "case = 0\n";
	Script = Script + "while case < len(allcases):\n\n";
		if(m_NeedToBeCropped==1) Script = Script + "\tDTI= OutputPath + \"/Case\" + str(case+1) + \"_croppedDTI.nrrd\"\n";
		else Script = Script + "\tDTI= allcases[case]\n";
		Script = Script + "\tFA= OutputPath + \"/Case\" + str(case+1) + \"_FA.nrrd\"\n";
//Script = Script + "\tcFA= OutputPath + \"/Case\" + str(case+1) + \"_cFA.nrrd\"\n";
		Script = Script + "\tGeneFACommand=\"dtiprocess --dti_image \" + DTI + \" -f \" + FA\n"; // color FA :  [ + \" --color_fa_output \" + cFA ]
		Script = Script + "\tprint(\"||Case \" + str(case+1) + \" => $ \" + GeneFACommand)\n";
		Script = Script + "\tif Overwrite==1 or not os.path.isfile(FA) : os.system(GeneFACommand)\n";
		Script = Script + "\telse : print(\"=> The file \\'\" + FA + \"\\' already exists so the command will not be executed\")\n";
		Script = Script + "\tcase += 1\n\n";

//////////Affine Registration and Normalization Loop
	Script = Script + "n = 0\n";
	Script = Script + "while n < nbLoops:\n";

//////////Normalization
		Script = Script + "\tprint(\"\\n[LOOP \" + str(n) + \"] ======== Normalization =========\")\n";
		Script = Script + "\tif n == 0 : case = 1 # the first case is the reference for the first loop so it will not be normalized or registered\n";
		Script = Script + "\telse : case = 0\n";
		Script = Script + "\twhile case < len(allcases):\n\n";
			Script = Script + "\t\tFA= OutputPath + \"/Case\" + str(case+1) + \"_FA.nrrd\"\n";
			Script = Script + "\t\tNormFA= OutputPath + \"/Case\" + str(case+1) + \"_Loop\" + str(n) + \"_NormFA.nrrd\"\n";
			Script = Script + "\t\tNormFACommand=\"ImageMath \" + FA + \" -outfile \" + NormFA + \" -matchHistogram \" + AtlasFAref\n";
			Script = Script + "\t\tprint(\"||Case \" + str(case+1) + \" => $ \" + NormFACommand)\n";
			Script = Script + "\t\tif Overwrite==1 or not os.path.isfile(NormFA) : os.system(NormFACommand)\n";
			Script = Script + "\t\telse : print(\"=> The file \\'\" + NormFA + \"\\' already exists so the command will not be executed\")\n";
			Script = Script + "\t\tcase += 1\n\n";

//////////Affine with BrainsFit registration
		Script = Script + "\tprint(\"\\n[LOOP \" + str(n) + \"] ======== Affine with BrainsFit registration =========\")\n";
		Script = Script + "\tif n == 0 : case = 1\n";
		Script = Script + "\telse : case = 0\n";
		Script = Script + "\twhile case < len(allcases):\n\n";
			Script = Script + "\t\tNormFA= OutputPath + \"/Case\" + str(case+1) + \"_Loop\" + str(n) + \"_NormFA.nrrd\"\n";
			Script = Script + "\t\tLinearTranstfm= OutputPath + \"/Case\" + str(case+1) + \"_Loop\" + str(n) + \"_LinearTrans.tfm\"\n";
			Script = Script + "\t\tLinearTrans= OutputPath + \"/Case\" + str(case+1) + \"_Loop\" + str(n) + \"_LinearTrans_FA.nrrd\"\n";
			Script = Script + "\t\tAffineCommand=\"BRAINSFit --fixedVolume \" + AtlasFAref + \" --movingVolume \" + NormFA + \" --initializeTransformMode useCenterOfHeadAlign --useAffine --outputVolume \" + LinearTrans + \" --outputTransform \" + LinearTranstfm\n";
			Script = Script + "\t\tprint(\"||Case \" + str(case+1) + \" => $ \" + AffineCommand)\n";
			Script = Script + "\t\tif Overwrite==1 or not os.path.isfile(LinearTranstfm) or not os.path.isfile(LinearTrans) : os.system(AffineCommand)\n";
			Script = Script + "\t\telif os.path.isfile(LinearTranstfm) : print(\"=> The file \\'\" + LinearTranstfm + \"\\' already exists so the command will not be executed\")\n";
			Script = Script + "\t\telif os.path.isfile(LinearTrans) : print(\"=> The file \\'\" + LinearTrans + \"\\' already exists so the command will not be executed\")\n";
			Script = Script + "\t\tcase += 1\n\n";

//////////Implementing the affine registration
		Script = Script + "\tprint(\"\\n[LOOP \" + str(n) + \"] ======== Implementing the Affine registration =========\")\n";
		Script = Script + "\tif n == 0 : case = 1\n";
		Script = Script + "\telse : case = 0\n";
		Script = Script + "\twhile case < len(allcases):\n\n";
			Script = Script + "\t\tLinearTranstfm= OutputPath + \"/Case\" + str(case+1) + \"_Loop\" + str(n) + \"_LinearTrans.tfm\"\n";
			Script = Script + "\t\tLinearTransDTI= OutputPath + \"/Case\" + str(case+1) + \"_Loop\" + str(n) + \"_LinearTrans_DTI.nrrd\"\n";
			if(m_NeedToBeCropped==1) Script = Script + "\t\toriginalDTI= OutputPath + \"/Case\" + str(case+1) + \"_croppedDTI.nrrd\"\n";
			else Script = Script + "\t\toriginalDTI= allcases[case]\n";
			Script = Script + "\t\tImplementCommand=\"ResampleDTIlogEuclidean \" + originalDTI + \" \" + LinearTransDTI + \" -f \" + LinearTranstfm + \" -R \" + AtlasFAref\n";
			Script = Script + "\t\tprint(\"||Case \" + str(case+1) + \" => $ \" + ImplementCommand)\n";
			Script = Script + "\t\tif Overwrite==1 or not os.path.isfile(LinearTransDTI) : os.system(ImplementCommand)\n";
			Script = Script + "\t\telse : print(\"=> The file \\'\" + LinearTransDTI + \"\\' already exists so the command will not be executed\")\n";
			Script = Script + "\t\tcase += 1\n\n";

//////////Generating FA of registered images
		Script = Script + "\tprint(\"\\n[LOOP \" + str(n) + \"] ======== Generating FA of registered images =========\")\n";
		if(m_nbLoops==1) // if there is no loop, we copy the first case so that Case1_Loop0_FinalFA.nrrd exists for AtlasWerks
		{
		Script = Script + "\tif n == 0 :\n";
			Script = Script + "\t\tcpCommand=\"cp \" + OutputPath + \"/Case1_FA.nrrd \" + OutputPath + \"/Case1_Loop0_FinalFA.nrrd\"\n";
			Script = Script + "\t\tprint(\"||Case1 => $ \" + cpCommand)\n";
			Script = Script + "\t\tif Overwrite==1 or not os.path.isfile(OutputPath + \"/Case1_Loop0_FinalFA.nrrd\") : os.system(cpCommand)\n";
			Script = Script + "\t\telse : print(\"=> The file \\'\" + OutputPath + \"/Case1_Loop0_FinalFA.nrrd\\' already exists so the command will not be executed\")\n";
			Script = Script + "\t\tcase = 1\n";
		}
		else Script = Script + "\tif n == 0 : case = 1\n";
		Script = Script + "\telse : case = 0\n";
		Script = Script + "\twhile case < len(allcases):\n\n";
			Script = Script + "\t\tLinearTransDTI= OutputPath + \"/Case\" + str(case+1) + \"_Loop\" + str(n) + \"_LinearTrans_DTI.nrrd\"\n";
			Script = Script + "\t\tif n == nbLoops-1 : LoopFA= OutputPath + \"/Case\" + str(case+1) + \"_Loop\" + str(n) + \"_FinalFA.nrrd\" # the last FA will be the Final output\n";
			Script = Script + "\t\telse : LoopFA= OutputPath + \"/Case\" + str(case+1) + \"_Loop\" + str(n) + \"_FA.nrrd\"\n";
			Script = Script + "\t\tGeneLoopFACommand=\"dtiprocess --dti_image \" + LinearTransDTI + \" -f \" + LoopFA\n";
			Script = Script + "\t\tprint(\"||Case \" + str(case+1) + \" => $ \" + GeneLoopFACommand)\n";
			Script = Script + "\t\tif Overwrite==1 or not os.path.isfile(LoopFA) : os.system(GeneLoopFACommand)\n";
			Script = Script + "\t\telse : print(\"=> The file \\'\" + LoopFA + \"\\' already exists so the command will not be executed\")\n";
			Script = Script + "\t\tcase += 1\n\n";
		
//////////FA Average of registered images with ImageMath
		Script = Script + "\tif n != nbLoops-1 : # this will not be done for the last lap\n";
			Script = Script + "\t\tprint(\"\\n[LOOP \" + str(n) + \"] ======== FA Average of registered images =========\")\n";
			Script = Script + "\t\tFAAverage = OutputPath + \"/Loop\" + str(n) + \"_FAAverage.nrrd\"\n";
			Script = Script + "\t\tif n == 0 : FAforAVG= OutputPath + \"/Case1_FA.nrrd\"\n";
			Script = Script + "\t\telse : FAforAVG= OutputPath + \"/Case1_Loop\" + str(n) + \"_FA.nrrd\"\n";
			Script = Script + "\t\tAverageCommand = \"ImageMath \" + FAforAVG + \" -outfile \" + FAAverage + \" -avg \"\n";
			Script = Script + "\t\tcase = 1\n";
			Script = Script + "\t\twhile case < len(allcases):\n\n";
				Script = Script + "\t\t\tFAforAVG= OutputPath + \"/Case\" + str(case+1) + \"_Loop\" + str(n) + \"_FA.nrrd \"\n";
				Script = Script + "\t\t\tAverageCommand= AverageCommand + FAforAVG\n";
				Script = Script + "\t\t\tcase += 1\n";
			Script = Script + "\t\tprint(\"=> $ \" + AverageCommand)\n";
			Script = Script + "\t\tif Overwrite==1 or not os.path.isfile(FAAverage) : os.system(AverageCommand)\n";
			Script = Script + "\t\telse : print(\"=> The file \\'\" + FAAverage + \"\\' already exists so the command will not be executed\")\n";
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

	std::ostringstream out;
	out << m_nbLoops;
	std::string nbLoops_str = out.str();
	Script = Script + "nbLoops= " + nbLoops_str + "\n\n";

	std::ostringstream outO;
	outO << m_Overwrite;
	std::string Overwrite_str = outO.str();
	Script = Script + "Overwrite= " + Overwrite_str + "\n\n";

////////Create directory for temporary files
	Script = Script + "if not os.path.isdir(OutputPath):\n";
		Script = Script + "\tos.mkdir(OutputPath)\n";
		Script = Script + "\tprint(\"\\n => Creation of the temporary files directory = \" + OutputPath)\n\n";

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

//////////Cropping DTI image
	Script = Script + "print(\"\\n======== Cropping DTI Image =========\")\n";
	Script = Script + "case = 0\n";
	Script = Script + "while case < len(allcases):\n\n";//Script = Script + "for case in allcases\n"; // here 'case' will take the values of 'allcases' (carac strings)
		//Script = Script + "\tprint(\"Path = \" + allcases[case])\n\n";
		Script = Script + "\tcroppedDTI= OutputPath + \"/Case\" + str(case+1) + \"_croppedDTI.nrrd\"\n";
		Script = Script + "\tCropCommand=\"CropDTI \" + allcases[case] + \" -o \" + croppedDTI + \" -size \" + CropSize[0] + \",\" + CropSize[1] + \",\" + CropSize[2] + \" -v\"\n";
////CropDTI infile [-o outfile] [-region  px,py,pz,w,h,d | -size w,h,d] [-v]
		Script = Script + "\tprint(\"||Case \" + str(case+1) + \" => $ \" + CropCommand)\n";
		Script = Script + "\tif Overwrite==1 or not os.path.isfile(croppedDTI) : os.system(CropCommand)\n";
		Script = Script + "\telse : print(\"=> The file \\'\" + croppedDTI + \"\\' already exists so the command will not be executed\")\n";
		Script = Script + "\tcase += 1\n\n";
	}

//////////Generating FA
	Script = Script + "print(\"\\n======== Generating FA =========\")\n";
	Script = Script + "case = 0\n";
	Script = Script + "while case < len(allcases):\n\n";
		if(m_NeedToBeCropped==1) Script = Script + "\tDTI= OutputPath + \"/Case\" + str(case+1) + \"_croppedDTI.nrrd\"\n";
		else Script = Script + "\tDTI= allcases[case]\n";
		Script = Script + "\tFA= OutputPath + \"/Case\" + str(case+1) + \"_FA.nrrd\"\n";
//Script = Script + "\tcFA= OutputPath + \"/Case\" + str(case+1) + \"_cFA.nrrd\"\n";
		Script = Script + "\tGeneFACommand=\"dtiprocess --dti_image \" + DTI + \" -f \" + FA\n"; // color FA :  [ + \" --color_fa_output \" + cFA ]
		Script = Script + "\tprint(\"||Case \" + str(case+1) + \" => $ \" + GeneFACommand)\n";
		Script = Script + "\tif Overwrite==1 or not os.path.isfile(FA) : os.system(GeneFACommand)\n";
		Script = Script + "\telse : print(\"=> The file \\'\" + FA + \"\\' already exists so the command will not be executed\")\n";
		Script = Script + "\tcase += 1\n\n";

//////////Affine Registration and Normalization Loop
	Script = Script + "n = 0\n";
	Script = Script + "while n < nbLoops:\n";

//////////Normalization
		Script = Script + "\tprint(\"\\n[LOOP \" + str(n) + \"] ======== Normalization =========\")\n";
		Script = Script + "\tcase = 0\n";
		Script = Script + "\twhile case < len(allcases):\n\n";
			Script = Script + "\t\tFA= OutputPath + \"/Case\" + str(case+1) + \"_FA.nrrd\"\n";
			Script = Script + "\t\tNormFA= OutputPath + \"/Case\" + str(case+1) + \"_Loop\" + str(n) + \"_NormFA.nrrd\"\n";
			Script = Script + "\t\tNormFACommand=\"ImageMath \" + FA + \" -outfile \" + NormFA + \" -matchHistogram \" + AtlasFAref\n";
			Script = Script + "\t\tprint(\"||Case \" + str(case+1) + \" => $ \" + NormFACommand)\n";
			Script = Script + "\t\tif Overwrite==1 or not os.path.isfile(NormFA) : os.system(NormFACommand)\n";
			Script = Script + "\t\telse : print(\"=> The file \\'\" + NormFA + \"\\' already exists so the command will not be executed\")\n";
			Script = Script + "\t\tcase += 1\n\n";

//////////Affine with BrainsFit registration
		Script = Script + "\tprint(\"\\n[LOOP \" + str(n) + \"] ======== Affine with BrainsFit registration =========\")\n";
		Script = Script + "\tcase = 0\n";
		Script = Script + "\twhile case < len(allcases):\n\n";
			Script = Script + "\t\tNormFA= OutputPath + \"/Case\" + str(case+1) + \"_Loop\" + str(n) + \"_NormFA.nrrd\"\n";
			Script = Script + "\t\tLinearTranstfm= OutputPath + \"/Case\" + str(case+1) + \"_Loop\" + str(n) + \"_LinearTrans.tfm\"\n";
			Script = Script + "\t\tLinearTrans= OutputPath + \"/Case\" + str(case+1) + \"_Loop\" + str(n) + \"_LinearTrans_FA.nrrd\"\n";
			Script = Script + "\t\tAffineCommand=\"BRAINSFit --fixedVolume \" + AtlasFAref + \" --movingVolume \" + NormFA + \" --initializeTransformMode useCenterOfHeadAlign --useAffine --outputVolume \" + LinearTrans + \" --outputTransform \" + LinearTranstfm\n";
			Script = Script + "\t\tprint(\"||Case \" + str(case+1) + \" => $ \" + AffineCommand)\n";
			Script = Script + "\t\tif Overwrite==1 or not os.path.isfile(LinearTranstfm) or not os.path.isfile(LinearTrans) : os.system(AffineCommand)\n";
			Script = Script + "\t\telif os.path.isfile(LinearTranstfm) : print(\"=> The file \" + LinearTranstfm + \" already exists so the command will not be executed\")\n";
			Script = Script + "\t\telif os.path.isfile(LinearTrans) : print(\"=> The file \\'\" + LinearTrans + \"\\' already exists so the command will not be executed\")\n";
			Script = Script + "\t\tcase += 1\n\n";

//////////Implementing the affine registration
		Script = Script + "\tprint(\"\\n[LOOP \" + str(n) + \"] ======== Implementing the Affine registration =========\")\n";
		Script = Script + "\tcase = 0\n";
		Script = Script + "\twhile case < len(allcases):\n\n";
			Script = Script + "\t\tLinearTranstfm= OutputPath + \"/Case\" + str(case+1) + \"_Loop\" + str(n) + \"_LinearTrans.tfm\"\n";
			Script = Script + "\t\tLinearTransDTI= OutputPath + \"/Case\" + str(case+1) + \"_Loop\" + str(n) + \"_LinearTrans_DTI.nrrd\"\n";
			if(m_NeedToBeCropped==1) Script = Script + "\t\toriginalDTI= OutputPath + \"/Case\" + str(case+1) + \"_croppedDTI.nrrd\"\n";
			else Script = Script + "\t\toriginalDTI= allcases[case]\n";
			Script = Script + "\t\tImplementCommand=\"ResampleDTIlogEuclidean \" + originalDTI + \" \" + LinearTransDTI + \" -f \" + LinearTranstfm + \" -R \" + AtlasFAref\n";
			Script = Script + "\t\tprint(\"||Case \" + str(case+1) + \" => $ \" + ImplementCommand)\n";
			Script = Script + "\t\tif Overwrite==1 or not os.path.isfile(LinearTransDTI) : os.system(ImplementCommand)\n";
			Script = Script + "\t\telse : print(\"=> The file \\'\" + LinearTransDTI + \"\\' already exists so the command will not be executed\")\n";
			Script = Script + "\t\tcase += 1\n\n";

//////////Generating FA of registered images
		Script = Script + "\tprint(\"\\n[LOOP \" + str(n) + \"] ======== Generating FA of registered images =========\")\n";
		Script = Script + "\tcase = 0\n";
		Script = Script + "\twhile case < len(allcases):\n\n";
			Script = Script + "\t\tLinearTransDTI= OutputPath + \"/Case\" + str(case+1) + \"_Loop\" + str(n) + \"_LinearTrans_DTI.nrrd\"\n";
			Script = Script + "\t\tif n == nbLoops-1 : LoopFA= OutputPath + \"/Case\" + str(case+1) + \"_Loop\" + str(n) + \"_FinalFA.nrrd\" # the last FA will be the Final output\n";
			Script = Script + "\t\telse : LoopFA= OutputPath + \"/Case\" + str(case+1) + \"_Loop\" + str(n) + \"_FA.nrrd\"\n";
			Script = Script + "\t\tGeneLoopFACommand=\"dtiprocess --dti_image \" + LinearTransDTI + \" -f \" + LoopFA\n";
			Script = Script + "\t\tprint(\"||Case \" + str(case+1) + \" => $ \" + GeneLoopFACommand)\n";
			Script = Script + "\t\tif Overwrite==1 or not os.path.isfile(LoopFA) : os.system(GeneLoopFACommand)\n";
			Script = Script + "\t\telse : print(\"=> The file \\'\" + LoopFA + \"\\' already exists so the command will not be executed\")\n";
			Script = Script + "\t\tcase += 1\n\n";

//////////FA Average of registered images with ImageMath
		Script = Script + "\tif n != nbLoops-1 : # this will not be done for the last lap\n";
			Script = Script + "\t\tprint(\"\\n[LOOP \" + str(n) + \"] ======== FA Average of registered images =========\")\n";
			Script = Script + "\t\tFAforAVG= OutputPath + \"/Case1_Loop\" + str(n) + \"_FA.nrrd\"\n";
			Script = Script + "\t\tFAAverage = OutputPath + \"/Loop\" + str(n) + \"_FAAverage.nrrd\"\n";
			Script = Script + "\t\tAverageCommand = \"ImageMath \" + FAforAVG + \" -outfile \" + FAAverage + \" -avg \"\n";
			Script = Script + "\t\tcase = 1\n";
			Script = Script + "\t\twhile case < len(allcases):\n\n";
				Script = Script + "\t\t\tFAforAVG= OutputPath + \"/Case\" + str(case+1) + \"_Loop\" + str(n) + \"_FA.nrrd \"\n";
				Script = Script + "\t\t\tAverageCommand= AverageCommand + FAforAVG\n";
				Script = Script + "\t\t\tcase += 1\n";
			Script = Script + "\t\tprint(\"=> $ \" + AverageCommand)\n";
			Script = Script + "\t\tif Overwrite==1 or not os.path.isfile(FAAverage) : os.system(AverageCommand)\n";
			Script = Script + "\t\telse : print(\"=> The file \\'\" + FAAverage + \"\\' already exists so the command will not be executed\")\n";
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

	Script = Script + "DeformPath= \"" + m_OutputPath + "/DTIAtlas/Deform_Registration\"\n";
	Script = Script + "AffinePath= \"" + m_OutputPath + "/DTIAtlas/Affine_Registration\"\n";
	Script = Script + "FinalPath= \"" + m_OutputPath + "/DTIAtlas/Final_Atlas\"\n";

	std::ostringstream out;
	out << m_nbLoops;
	std::string nbLoops_str = out.str();
	Script = Script + "nbLoops= " + nbLoops_str + "\n";

	std::ostringstream outO;
	outO << m_Overwrite;
	std::string Overwrite_str = outO.str();
	Script = Script + "Overwrite= " + Overwrite_str + "\n\n";

////////Create directory for temporary files and final
	Script = Script + "if not os.path.isdir(DeformPath):\n";
		Script = Script + "\tprint(\"\\n => Creation of the Deformation transform directory = \" + DeformPath)\n";
		Script = Script + "\tos.mkdir(DeformPath)\n\n";
	Script = Script + "if not os.path.isdir(FinalPath):\n";
		Script = Script + "\tprint(\"\\n => Creation of the Final Atlas directory = \" + FinalPath)\n";
		Script = Script + "\tos.mkdir(FinalPath)\n\n";

//////////Cases variables:
	Script = Script + "allFAs = [AffinePath + \"/Case1_Loop\" + str(nbLoops-1) + \"_FinalFA.nrrd\"";
	for (unsigned int i=1;i<m_CasesPath.size();i++) 
	{
		std::ostringstream outi;
		outi << i+1;
		std::string i_str = outi.str();
		Script = Script + ", AffinePath + \"/Case" + i_str + "_Loop\" + str(nbLoops-1) + \"_FinalFA.nrrd\"";
	}	
	Script = Script+ "]\n\n";

	if(m_NeedToBeCropped==1)
	{
		Script = Script + "allcases = [AffinePath + \"/Case1_croppedDTI.nrrd\"";
		for (unsigned int i=1;i<m_CasesPath.size();i++) 
		{
			std::ostringstream outi2;
			outi2 << i+1;
			std::string i2_str = outi2.str();
			Script = Script + ", AffinePath + \"/Case" + i2_str + "_croppedDTI.nrrd\"";
		}
		Script = Script + "]\n\n";	
	}

	else
	{
		Script = Script + "allcases = [\"" + m_CasesPath[0];
		for (unsigned int i=1;i<m_CasesPath.size();i++) Script = Script + "\", \"" + m_CasesPath[i];
		Script = Script + "\"]\n\n";
	}

//////////AtlasWerks Command:
	Script = Script + "print(\"\\n======== Computing the Deformation Fields =========\")\n";
	Script = Script + "FinalAtlasPrefix= DeformPath + \"/AverageImage_\"\n";
	Script = Script + "FinalAtlasDefPrefix= DeformPath + \"/DeformedImage_\"\n";
	Script = Script + "FinalAtlasDefFieldPrefix= DeformPath + \"/DeformationField_\"\n";
	Script = Script + "AtlasBCommand= \"AtlasWerks --outputImageFilenamePrefix \" + FinalAtlasPrefix + \" --outputDeformedImageFilenamePrefix \" + FinalAtlasDefPrefix + \" --outputHFieldFilenamePrefix \" + FinalAtlasDefFieldPrefix\n";	
	Script = Script + "case = 0\n";
	Script = Script + "while case < len(allcases):\n";
		Script = Script + "\tAtlasBCommand = AtlasBCommand + \" \" + allFAs[case]\n";
		Script = Script + "\tcase += 1\n";
	Script = Script + "print(\"=> $ \" + AtlasBCommand)\n";
	Script = Script + "if Overwrite==1 or not os.path.isfile(FinalAtlasPrefix + \"0.mhd\") : os.system(AtlasBCommand)\n";
	Script = Script + "else : print(\"=> The file \\'\" + FinalAtlasPrefix + \"0.mhd\\' already exists so the command will not be executed\")\n\n";

///////////Apply deformation fields and Affine transform
	Script = Script + "print(\"\\n======== Applying deformation fields and transform matrix to original DTIs =========\")\n";
	Script = Script + "case = 0\n";
	Script = Script + "while case < len(allcases):\n";
		Script = Script + "\ttfm= AffinePath + \"/Case\" + str(case+1) + \"_Loop\" + str(nbLoops-1) + \"_LinearTrans.tfm\"\n";
		Script = Script + "\tFinalDTI= FinalPath + \"/Case\" + str(case+1) + \"_FinalDTI.nrrd\"\n";
		if(m_NeedToBeCropped==1) Script = Script + "\toriginalDTI= AffinePath + \"/Case\" + str(case+1) + \"_croppedDTI.nrrd\"\n";
		else Script = Script + "\toriginalDTI= allcases[case]\n";
		if(m_nbLoops==1)
		{
			if(m_RegType==1) Script = Script + "\tRef= AffinePath + \"/Case1_FA.nrrd\"\n";
			else Script = Script + "\tRef= \"" + m_TemplatePath + "\"\n";
		}
		else Script = Script + "\tRef = AffinePath + \"/Loop\" + str(nbLoops-2) + \"_FAAverage.nrrd\"\n"; // an average image has been generated in the loops of affine reg for reference
		Script = Script + "\tif case < 10 : HField= FinalAtlasDefFieldPrefix + \"000\" + str(case) + \".mhd\"\n";
		Script = Script + "\telif case > 10 and case < 100 : HField= FinalAtlasDefFieldPrefix + \"00\" + str(case) + \".mhd\"\n";
		Script = Script + "\telif case > 100 and case < 1000 : HField= FinalAtlasDefFieldPrefix + \"0\" + str(case) + \".mhd\"\n";
		Script = Script + "\telif case > 1000 : HField= FinalAtlasDefFieldPrefix + \"\" + str(case) + \".mhd\"\n";
		Script = Script + "\tFinalReSampCommand=\"ResampleDTIlogEuclidean -f \" + tfm + \" -R \" + Ref + \" -H \" + HField + \" \" + originalDTI + \" \" + FinalDTI\n";
		Script = Script + "\tprint(\"||Case \" + str(case+1) + \" => $ \" + FinalReSampCommand)\n";
		Script = Script + "\tif Overwrite==1 or not os.path.isfile(FinalDTI) : os.system(FinalReSampCommand)\n";
		Script = Script + "\telse : print(\"=> The file \\'\" + FinalDTI + \"\\' already exists so the command will not be executed\")\n";
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
	Script = Script + "AverageCommand = AverageCommand + \"--tensor_output \" + DTIAverage + \" -m log-euclidean\"\n";
	Script = Script + "print(\"=> $ \" + AverageCommand)\n";
	Script = Script + "if Overwrite==1 or not os.path.isfile(DTIAverage) : os.system(AverageCommand)\n";
	Script = Script + "else : print(\"=> The file \\'\" + DTIAverage + \"\\' already exists so the command will not be executed\")\n\n";

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
	int RefSpacing [3]; // the spacing of the first case is the reference for this computation
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
	m_nbLoops = nbLoops + 1;
}

void ScriptWriter::setTemplatePath(std::string TemplatePath)
{
	m_TemplatePath = TemplatePath;
}

void ScriptWriter::setOverwrite(int Overwrite)
{
	m_Overwrite = Overwrite;
}

