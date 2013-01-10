/*std classes*/
#include <iostream>
#include <string>
#include <vector>
#include <sstream> // to convert int to std::string
#include <math.h> // for the absolute value

/*itk classes*/
#include "itkImage.h"
#include "itkImageFileReader.h"
#include <itksys/SystemTools.hxx> // for GetFilenamePath()

#include "ScriptWriter.h"

  /////////////////////////////////////////
 //         WRITING FUNCTIONS           //
/////////////////////////////////////////

void ScriptWriter::WriteScript(int MainPID)
{
	std::cout<<"|"<<std::endl; // command line display
	std::cout<<"| Number of Cases: "<<m_CasesPath.size()<<std::endl; // command line display
	std::cout<<"| Output Directory : "<<m_OutputPath<<"/DTIAtlas/1_Affine_Registration"<<std::endl; // command line display
	if( m_useGridProcess ) std::cout<<"| Using grid processing"<<std::endl; // command line display
	if(m_RegType==1) std::cout<<"| Using Case 1 as reference in the first Registration Loop"<<std::endl; // command line display
	else std::cout<<"| Using Template as reference for the Registration: "<<m_TemplatePath<<std::endl; // command line display
	std::cout<<"| Number of loops in the Registration Loop : "<<m_nbLoops<<std::endl; // command line display
	std::cout<<"| Writing begin: "; // command line display (no endl)

	std::ostringstream out;
	out << m_nbLoops;
	m_nbLoops_str = out.str();

	std::ostringstream outss;
	outss << MainPID;
	std::string MainPID_str = outss.str();

	Preprocess(MainPID_str);
	AtlasBuilding(MainPID_str);
	MainScript();
}

void ScriptWriter::Preprocess (std::string MainPID_str)
{
	std::string Script;

	std::cout<<"[Pre Processing]"; // command line display (no endl)

	Script = Script + "#!/usr/bin/python\n\n";
	Script = Script + "import os\n"; // To run a shell command : os.system("[shell command]")
	Script = Script + "import sys\n"; // to return an exit code
	Script = Script + "import signal\n";
	Script = Script + "import ctypes\n"; // for sigqueue : send a signal with a value in it to another process
	Script = Script + "libc = ctypes.CDLL(\"libc.so.6\")\n\n"; // load c lib

	Script = Script + "print(\"\\n============ Pre processing =============\")\n\n";

	Script = Script + "# Files Paths\n";
	Script = Script + "allcases = [\"" + m_CasesPath[0];
	for (unsigned int i=1;i<m_CasesPath.size();i++) Script = Script + "\", \"" + m_CasesPath[i];
	Script = Script+ "\"]\n";
	
	Script = Script + "OutputPath= \"" + m_OutputPath + "/DTIAtlas/1_Affine_Registration\"\n";

	if(m_RegType==1) Script = Script + "AtlasFAref= OutputPath + \"/Case1_FA.nrrd\" #the reference will be the first case for the first loop\n";
	else Script = Script + "AtlasFAref= \"" + m_TemplatePath + "\" #the reference will be the given template for the first loop\n";

	Script = Script + "ErrorList=[] #empty list\n\n";

	// nbSteps = (nbCases x CropDTI) + nbCases x Generate FA + (nbCases-1) x (Normalize, Affine reg, Apply transfm, generate FA) (1st loop if no template) + (nbLoops) x nbCases x (Normalize, Affine reg, Apply transfm, generate FA) + nbLoops x Compute average (not for the last loop) + nonLinear reg + nbCases x Apply tfm + DTI average + nbCases x (1st resamp 2nd resamp) + DTI average
	int nbSteps = m_CasesPath.size()*1; // nbCases x Generate FA
	if(m_NeedToBeCropped==1) nbSteps = nbSteps + m_CasesPath.size()*1; // (nbCases x CropDTI)
	if(m_RegType==1) nbSteps = nbSteps + (m_CasesPath.size()-1)*4; // (nbCases-1) x (Normalize, Affine reg, Apply transfm, generate FA) (1st loop if no template)
	else  nbSteps = nbSteps + m_CasesPath.size()*4; // nbCases x (Normalize, Affine reg, Apply transfm, generate FA) (1st loop if template)
	nbSteps = nbSteps + m_nbLoops*(m_CasesPath.size()*4) + m_nbLoops*1; // (nbLoops) x nbCases x (Normalize, Affine reg, Apply transfm, generate FA) + nbLoops x Compute average (not for the last loop)
	nbSteps = nbSteps + 1 + m_CasesPath.size()*1 + 1 + m_CasesPath.size()*2 + 1; // nonLinear reg + nbCases x Apply tfm + DTI average + nbCases x (1st resamp 2nd resamp) + DTI average

	std::ostringstream oss1;
	oss1 << nbSteps;
	std::string nbSteps_str = oss1.str();
	Script = Script + "nbSteps=" + nbSteps_str  + " # for the progress bar\n";
	Script = Script + "nbStepsDone=0\n\n";
// Script = Script + "\tos.kill(" + MainPID_str + ", signal.SIGUSR2) # send a signal to the progress bar\n";

/* Call script to run command on grid */
	std::string GridApostrophe = "";
	std::string GridProcessCmd = "";
	std::string GridProcessFileExistCmd = "";
	std::string GridProcessFileExistCmd1 = "";
	std::string GridProcessFileExistCmd2 = "";
	std::string GridProcessCmdNoCase = "";
	std::string GridProcessFileExistCmdNoCase = "";
	std::string GridProcessFileExistCmdNoCase1 = "";
	std::string GridProcessFileExistCmdNoCase2 = "";
	std::string GridProcessFileExistIndent = "";
	std::string GridProcessFileExistIndent1 = "";
	std::string GridProcessFileExistIndent2 = "";
	std::ostringstream oss;
	oss << m_RegType; // m_RegType = 0 if template (-> NoCase1=0) |  = 1 if case1=ref (-> NoCase1=1) => NoCase1 = (string) m_RegType
	std::string NoCase1 = oss.str();
	if( m_useGridProcess )
	{
		Script = Script + "# Call script to run command on server\n";
		Script = Script + "ScriptCommand = \"" + m_OutputPath + "/DTIAtlas/Script/RunCommandOnServer.script\"\n";
		Script = Script + "FilesFolder = \"" + m_OutputPath + "/DTIAtlas/GridProcessingFiles\"\n";
		Script = Script + "if not os.path.isdir(FilesFolder):\n";
			Script = Script + "\tos.mkdir(FilesFolder)\n";
		 	Script = Script + "\tprint(\"\\n=> Creation of the directory for the grid processing = \" + FilesFolder)\n";
		Script = Script + "else : os.system(\"rm \" + FilesFolder + \"/*\") # remove any previous file\n\n";

		//Test Function
		Script = Script + "# Function that tests if all cases have been processed on the grid\n";
		Script = Script + "def TestGridProcess ( FilesFolder, NbCases , NoCase1): # if NbCases == 0, then just search the file \'file\' (unique command)\n";
			Script = Script + "\tprint(\"\\n| Waiting for all cases (\" + str(NbCases-NoCase1) + \") to be processed on grid...\") # NoCase1 is 0 or 1\n";
			Script = Script + "\tfilesOK = 0\n";
			Script = Script + "\twhile not filesOK :\n";
				Script = Script + "\t\tfilesOK = 1\n";
				Script = Script + "\t\tif NbCases>0 : \n";
					Script = Script + "\t\t\tNbfilesOK = 0\n";
					Script = Script + "\t\t\tif NoCase1 == 1 : case = 1\n";
					Script = Script + "\t\t\telse : case = 0\n";
					Script = Script + "\t\t\twhile case < NbCases:\n";
						Script = Script + "\t\t\t\tif not os.path.isfile( FilesFolder + \"/Case\" + str(case+1) ) : filesOK = 0\n";
						Script = Script + "\t\t\t\telse : NbfilesOK = NbfilesOK + 1\n";
						Script = Script + "\t\t\t\tcase += 1\n";
					Script = Script + "\t\t\tprint(\"\\r| [\" + str(NbfilesOK) + \"\\t/ \" + str(NbCases) + \" ] Files processed\"), # the comma prevents the print from jumping line\n";
				Script = Script + "\t\telif not os.path.isfile( FilesFolder + \"/file\" ) : filesOK = 0\n";
			Script = Script + "\tprint(\"\\n| All files processed\")\n";
			Script = Script + "\tos.system(\"rm \" + FilesFolder + \"/*\")\n\n";

		GridApostrophe = " + \"\\'\"";
		std::string File = "FilesFolder + \"/Case\" + str(case+1)";
		GridProcessCmd = "\"" + m_GridCommand + " " + m_OutputPath + "/DTIAtlas/Script/RunCommandOnServer.script \" + " + File + " + \" \"" + GridApostrophe + " + ";
		GridProcessFileExistCmd = "\tf = open( " + File + ", 'w')\n\tf.close()\n"; // if the image already exists, create the "semaphore" file
		GridProcessFileExistCmd1 = "\t\tf = open( " + File + ", 'w')\n\t\tf.close()\n"; // if the image already exists, create the "semaphore" file
		GridProcessFileExistCmd2 = "\t\t\tf = open( " + File + ", 'w')\n\t\t\tf.close()\n"; // if the image already exists, create the "semaphore" file

		std::string FileNoCase = "FilesFolder + \"/file\""; //  for the commands executed only once = not once per case
		GridProcessCmdNoCase = "\"" + m_GridCommand + " " + m_OutputPath + "/DTIAtlas/Script/RunCommandOnServer.script \" + " + FileNoCase + " + \" \"" + GridApostrophe + " + ";
		GridProcessFileExistCmdNoCase = "\tf = open( " + FileNoCase + ", 'w')\n\tf.close()\n";
		GridProcessFileExistCmdNoCase1 = "\t\tf = open( " + FileNoCase + ", 'w')\n\t\tf.close()\n";
		GridProcessFileExistCmdNoCase2 = "\t\t\tf = open( " + FileNoCase + ", 'w')\n\t\t\tf.close()\n";

		GridProcessFileExistIndent = "\n\t"; // if the file already exists, several commands in the else (create file), so change line and indent
		GridProcessFileExistIndent1 = "\n\t\t"; // if the file already exists, several commands in the else (create file), so change line and indent (double)
		GridProcessFileExistIndent2 = "\n\t\t\t"; // if the file already exists, several commands in the else (create file), so change line and indent (3)

	}

/* Create directory for temporary files */
	Script = Script + "# Create directory for temporary files\n";
	Script = Script + "if not os.path.isdir(OutputPath):\n";
		Script = Script + "\tos.mkdir(OutputPath)\n";
	 	Script = Script + "\tprint(\"\\n=> Creation of the affine directory = \" + OutputPath)\n\n";

/* Rescaling template */
	if(m_RegType==0) // with template
	{
		Script = Script + "# Rescaling template\n";
		Script = Script + "print(\"\\n======== Rescaling FA template =========\")\n";
		Script = Script + "RescaleTemp= OutputPath + \"/FATemplate_Rescaled.nrrd\"\n";
		Script = Script + "RescaleTempCommand=" + GridProcessCmdNoCase + "\"" + m_SoftPath[0] + " \" + AtlasFAref + \" -outfile \" + RescaleTemp + \" -rescale 0,10000 \"" + GridApostrophe + "\n";
		Script = Script + "print(\"=> $ \" + RescaleTempCommand)\n";
		if(m_Overwrite==1) Script = Script + "if os.system(RescaleTempCommand)!=0 : ErrorList.append(\'ImageMath: Rescaling FA template\')\n";
		else
		{
			Script = Script + "if not os.path.isfile(RescaleTemp) :\n";
				Script = Script + "\tif os.system(RescaleTempCommand)!=0 : ErrorList.append(\'ImageMath:  Rescaling FA template\')\n";
			Script = Script + "else : " + GridProcessFileExistIndent + "print(\"=> The file \\'\" + RescaleTemp + \"\\' already exists so the command will not be executed\")\n" + GridProcessFileExistCmdNoCase; //either empty string or create file command
		}

if( m_useGridProcess ) Script = Script + "TestGridProcess( FilesFolder, 0, 0) # stays in the function until all process is done : 0 makes the function look for \'file\'\n\n";
		Script = Script + "AtlasFAref= RescaleTemp\n\n";
	}

/* Cropping DTI image */
	if(m_NeedToBeCropped==1)
	{
	Script = Script + "# Cropping DTI image\n";

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
		Script = Script + "\tCropCommand=" + GridProcessCmd + "\"" + m_SoftPath[2] + " \" + allcases[case] + \" -o \" + croppedDTI + \" -size \" + CropSize[0] + \",\" + CropSize[1] + \",\" + CropSize[2] + \" -v\"" + GridApostrophe + "\n";
		Script = Script + "\tprint(\"||Case \" + str(case+1) + \" => $ \" + CropCommand)\n";
		Script = Script + "\tnbStepsDone += 1\n";
		if(m_Overwrite==1)
		{
			Script = Script + "\tif os.system(CropCommand)!=0 : ErrorList.append(\'[Case \' + str(case+1) + \'] CropDTI: Cropping DTI image\')\n";
			Script = Script + "\tlibc.sigqueue(" + MainPID_str + ", signal.SIGUSR2, int(float(100*nbStepsDone)/float(nbSteps))) # send a signal to the progress bar, with the progress value in it\n";
		}
		else
		{
			Script = Script + "\tif not os.path.isfile(croppedDTI) :\n";
				Script = Script + "\t\tif os.system(CropCommand)!=0 : ErrorList.append(\'[Case \' + str(case+1) + \'] CropDTI: Cropping DTI image\')\n";
				Script = Script + "\t\tlibc.sigqueue(" + MainPID_str + ", signal.SIGUSR2, int(float(100*nbStepsDone)/float(nbSteps))) # send a signal to the progress bar, with the progress value in it\n";
			Script = Script + "\telse : " + GridProcessFileExistIndent1 + "print(\"=> The file \\'\" + croppedDTI + \"\\' already exists so the command will not be executed\")\n" + GridProcessFileExistCmd1;
		}
		Script = Script + "\tcase += 1\n\n";

if( m_useGridProcess ) Script = Script + "TestGridProcess( FilesFolder, len(allcases), 0) # stays in the function until all process is done : 0 makes the function look for \'file\'\n\n";
	}

/* Generating FA */
	Script = Script + "# Generating FA\n";
	Script = Script + "print(\"\\n======== Generating FA =========\")\n";
	Script = Script + "case = 0\n";
	Script = Script + "while case < len(allcases):\n";
		if(m_NeedToBeCropped==1) Script = Script + "\tDTI= OutputPath + \"/Case\" + str(case+1) + \"_croppedDTI.nrrd\"\n";
		else Script = Script + "\tDTI= allcases[case]\n";
		Script = Script + "\tFA= OutputPath + \"/Case\" + str(case+1) + \"_FA.nrrd\"\n";
		Script = Script + "\tGeneFACommand=" + GridProcessCmd + "\"" + m_SoftPath[3] + " --dti_image \" + DTI + \" -f \" + FA" + GridApostrophe + "\n";
		Script = Script + "\tprint(\"||Case \" + str(case+1) + \" => $ \" + GeneFACommand)\n";
		Script = Script + "\tnbStepsDone += 1\n";
		if(m_Overwrite==1)
		{
			Script = Script + "\tif os.system(GeneFACommand)!=0 : ErrorList.append(\'[Case \' + str(case+1) + \'] dtiprocess: Generating FA of DTI image\')\n";
			Script = Script + "\tlibc.sigqueue(" + MainPID_str + ", signal.SIGUSR2, int(float(100*nbStepsDone)/float(nbSteps))) # send a signal to the progress bar, with the progress value in it\n";
		}
		else
		{
			Script = Script + "\tif not os.path.isfile(FA) :\n";
				Script = Script + "\t\tif os.system(GeneFACommand)!=0 : ErrorList.append(\'[Case \' + str(case+1) + \'] dtiprocess: Generating FA of DTI image\')\n";
				Script = Script + "\t\tlibc.sigqueue(" + MainPID_str + ", signal.SIGUSR2, int(float(100*nbStepsDone)/float(nbSteps))) # send a signal to the progress bar, with the progress value in it\n";
			Script = Script + "\telse : " + GridProcessFileExistIndent1 + "print(\"=> The file \\'\" + FA + \"\\' already exists so the command will not be executed\")\n" + GridProcessFileExistCmd1;
		}

		Script = Script + "\tcase += 1\n\n";

if( m_useGridProcess ) Script = Script + "TestGridProcess( FilesFolder, len(allcases), 0) # stays in the function until all process is done : 0 makes the function look for \'file\'\n\n";

/* Affine Registration and Normalization Loop */
	Script = Script + "# Affine Registration and Normalization Loop\n";
	Script = Script + "n = 0\n";
	Script = Script + "while n <= " + m_nbLoops_str + " :\n";

	Script = Script + "\tif not os.path.isdir(OutputPath + \"/Loop\" + str(n)):\n";
		Script = Script + "\t\tprint(\"\\n => Creation of the Output directory for Loop \" + str(n) + \" = \" + OutputPath + \"/Loop\" + str(n))\n";
		Script = Script + "\t\tos.mkdir(OutputPath + \"/Loop\" + str(n))\n\n";

/* Normalization */
	Script = Script + "# Normalization\n";
		Script = Script + "\tprint(\"\\n[LOOP \" + str(n) + \"/" + m_nbLoops_str + "] ======== Normalization =========\")\n";
	if(m_RegType==1) Script = Script + "\tcase = (n==0) # (n==0) -> bool: =1(true) =0(false) : the first case is the reference for the first loop so it will not be normalized or registered\n";
	else 	Script = Script + "\tcase = 0\n";
		Script = Script + "\twhile case < len(allcases):\n";
			Script = Script + "\t\tFA= OutputPath + \"/Case\" + str(case+1) + \"_FA.nrrd\"\n";
			Script = Script + "\t\tNormFA= OutputPath + \"/Loop\" + str(n) + \"/Case\" + str(case+1) + \"_Loop\" + str(n) + \"_NormFA.nrrd\"\n";
			Script = Script + "\t\tNormFACommand=" + GridProcessCmd + "\"" + m_SoftPath[0] + " \" + FA + \" -outfile \" + NormFA + \" -matchHistogram \" + AtlasFAref" + GridApostrophe + "\n";
			Script = Script + "\t\tprint(\"||Case \" + str(case+1) + \" => $ \" + NormFACommand)\n";
			Script = Script + "\t\tnbStepsDone += 1\n";
			if(m_Overwrite==1)
			{
				Script = Script + "\t\tif os.system(NormFACommand)!=0 : ErrorList.append(\'[Loop \' + str(n) + \'][Case \' + str(case+1) + \'] ImageMath: Normalizing FA image\')\n";
				Script = Script + "\t\tlibc.sigqueue(" + MainPID_str + ", signal.SIGUSR2, int(float(100*nbStepsDone)/float(nbSteps))) # send a signal to the progress bar, with the progress value in it\n";
			}
			else
			{
				Script = Script + "\t\tif not os.path.isfile(NormFA) :\n";
					Script = Script + "\t\t\tif os.system(NormFACommand)!=0 : ErrorList.append(\'[Loop \' + str(n) + \'][Case \' + str(case+1) + \'] ImageMath: Normalizing FA image\')\n";
					Script = Script + "\t\t\tlibc.sigqueue(" + MainPID_str + ", signal.SIGUSR2, int(float(100*nbStepsDone)/float(nbSteps))) # send a signal to the progress bar, with the progress value in it\n";
				Script = Script + "\t\telse : " + GridProcessFileExistIndent2 + "print(\"=> The file \\'\" + NormFA + \"\\' already exists so the command will not be executed\")\n" + GridProcessFileExistCmd2;
			}
			Script = Script + "\t\tcase += 1\n\n";

if( m_useGridProcess ) Script = Script + "\tTestGridProcess( FilesFolder, len(allcases), " + NoCase1 + "*(n==0)) # stays in the function until all process is done : 0 makes the function look for \'file\'\n\n";

/* Affine registration with BrainsFit */
	Script = Script + "# Affine registration with BrainsFit\n";
		Script = Script + "\tprint(\"\\n[LOOP \" + str(n) + \"/" + m_nbLoops_str + "] ======== Affine registration with BrainsFit =========\")\n";
	if(m_RegType==1) Script = Script + "\tcase = (n==0) # (n==0) -> bool: =1(true) =0(false) : the first case is the reference for the first loop so it will not be normalized or registered\n";
	else 	Script = Script + "\tcase = 0\n";
		Script = Script + "\twhile case < len(allcases):\n";
			Script = Script + "\t\tNormFA= OutputPath + \"/Loop\" + str(n) + \"/Case\" + str(case+1) + \"_Loop\" + str(n) + \"_NormFA.nrrd\"\n";
			Script = Script + "\t\tLinearTranstfm= OutputPath + \"/Loop\" + str(n) + \"/Case\" + str(case+1) + \"_Loop\" + str(n) + \"_LinearTrans.txt\"\n";
			Script = Script + "\t\tLinearTrans= OutputPath + \"/Loop\" + str(n) + \"/Case\" + str(case+1) + \"_Loop\" + str(n) + \"_LinearTrans_FA.nrrd\"\n";
			Script = Script + "\t\tAffineCommand=" + GridProcessCmd + "\"" + m_SoftPath[4] + " --fixedVolume \" + AtlasFAref + \" --movingVolume \" + NormFA + \" --useAffine --outputVolume \" + LinearTrans + \" --outputTransform \" + LinearTranstfm\n";
			Script = Script + "\t\tInitLinearTrans= OutputPath + \"/Case\" + str(case+1) + \"_InitLinearTrans.nrrd\"\n";
			Script = Script + "\t\tif n==0 and os.path.isfile(InitLinearTrans) : AffineCommand= AffineCommand + \" --initialTransform \" + InitLinearTrans" + GridApostrophe + "\n";
			Script = Script + "\t\telse : AffineCommand= AffineCommand + \" --initializeTransformMode " + m_BFAffineTfmMode + "\"" + GridApostrophe + "\n";
			Script = Script + "\t\tprint(\"||Case \" + str(case+1) + \" => $ \" + AffineCommand)\n";
			Script = Script + "\t\tnbStepsDone += 1\n";
			if(m_Overwrite==1)
			{
				Script = Script + "\t\tif os.system(AffineCommand)!=0 : ErrorList.append(\'[Loop \' + str(n) + \'][Case \' + str(case+1) + \'] BRAINSFit: Affine Registration of FA image\')\n";
				Script = Script + "\t\tlibc.sigqueue(" + MainPID_str + ", signal.SIGUSR2, int(float(100*nbStepsDone)/float(nbSteps))) # send a signal to the progress bar, with the progress value in it\n";
			}
			else 
			{
				Script = Script + "\t\tif not os.path.isfile(LinearTranstfm) or not os.path.isfile(LinearTrans) :\n";
					Script = Script + "\t\t\tif os.system(AffineCommand)!=0 : ErrorList.append(\'[Loop \' + str(n) + \'][Case \' + str(case+1) + \'] BRAINSFit: Affine Registration of FA image\')\n";
					Script = Script + "\t\t\tlibc.sigqueue(" + MainPID_str + ", signal.SIGUSR2, int(float(100*nbStepsDone)/float(nbSteps))) # send a signal to the progress bar, with the progress value in it\n";
				Script = Script + "\t\telif os.path.isfile(LinearTranstfm) : " + GridProcessFileExistIndent2 + "print(\"=> The file \\'\" + LinearTranstfm + \"\\' already exists so the command will not be executed\")\n" + GridProcessFileExistCmd2;
				Script = Script + "\t\telif os.path.isfile(LinearTrans) : " + GridProcessFileExistIndent2 + "print(\"=> The file \\'\" + LinearTrans + \"\\' already exists so the command will not be executed\")\n" + GridProcessFileExistCmd2;
			}

			Script = Script + "\t\tcase += 1\n\n";

if( m_useGridProcess ) Script = Script + "\tTestGridProcess( FilesFolder, len(allcases), " + NoCase1 + "*(n==0)) # stays in the function until all process is done : 0 makes the function look for \'file\'\n\n";

/* Implementing the affine registration */
	Script = Script + "# Implementing the affine registration\n";
		Script = Script + "\tprint(\"\\n[LOOP \" + str(n) + \"/" + m_nbLoops_str + "] ======== Implementing the Affine registration =========\")\n";
	if(m_RegType==1) Script = Script + "\tcase = (n==0) # (n==0) -> bool: =1(true) =0(false) : the first case is the reference for the first loop so it will not be normalized or registered\n";
	else 	Script = Script + "\tcase = 0\n";
		Script = Script + "\twhile case < len(allcases):\n";
			Script = Script + "\t\tLinearTranstfm= OutputPath + \"/Loop\" + str(n) + \"/Case\" + str(case+1) + \"_Loop\" + str(n) + \"_LinearTrans.txt\"\n";
			Script = Script + "\t\tLinearTransDTI= OutputPath + \"/Loop\" + str(n) + \"/Case\" + str(case+1) + \"_Loop\" + str(n) + \"_LinearTrans_DTI.nrrd\"\n";
			if(m_NeedToBeCropped==1) Script = Script + "\t\toriginalDTI= OutputPath + \"/Case\" + str(case+1) + \"_croppedDTI.nrrd\"\n";
			else Script = Script + "\t\toriginalDTI= allcases[case]\n";
			Script = Script + "\t\tImplementCommand=" + GridProcessCmd + "\"" + m_SoftPath[1] + " \" + originalDTI + \" \" + LinearTransDTI + \" -f \" + LinearTranstfm + \" -R \" + AtlasFAref" + GridApostrophe + "\n";
			Script = Script + "\t\tprint(\"||Case \" + str(case+1) + \" => $ \" + ImplementCommand)\n";
			Script = Script + "\t\tnbStepsDone += 1\n";
			if(m_Overwrite==1)
			{
				Script = Script + "\t\tif os.system(ImplementCommand)!=0: ErrorList.append(\'[Loop \' + str(n) + \'][Case \' + str(case+1) + \'] BRAINSFit: Implementing the Affine Registration on FA image\')\n";
				Script = Script + "\t\tlibc.sigqueue(" + MainPID_str + ", signal.SIGUSR2, int(float(100*nbStepsDone)/float(nbSteps))) # send a signal to the progress bar, with the progress value in it\n";
			}
			else
			{
				Script = Script + "\t\tif not os.path.isfile(LinearTransDTI) :\n";
					Script = Script + "\t\t\tif os.system(ImplementCommand)!=0: ErrorList.append(\'[Loop \' + str(n) + \'][Case \' + str(case+1) + \'] ResampleDTIlogEuclidean: Implementing the Affine Registration on FA image\')\n";
					Script = Script + "\t\t\tlibc.sigqueue(" + MainPID_str + ", signal.SIGUSR2, int(float(100*nbStepsDone)/float(nbSteps))) # send a signal to the progress bar, with the progress value in it\n";
				Script = Script + "\t\telse : " + GridProcessFileExistIndent2 + "print(\"=> The file \\'\" + LinearTransDTI + \"\\' already exists so the command will not be executed\")\n" + GridProcessFileExistCmd2;
			}
			Script = Script + "\t\tcase += 1\n\n";

if( m_useGridProcess ) Script = Script + "\tTestGridProcess( FilesFolder, len(allcases), " + NoCase1 + "*(n==0)) # stays in the function until all process is done : 0 makes the function look for \'file\'\n\n";

/* Generating FA of registered images */
	Script = Script + "# Generating FA of registered images\n";
		Script = Script + "\tprint(\"\\n[LOOP \" + str(n) + \"/" + m_nbLoops_str + "] ======== Generating FA of registered images =========\")\n";
	if(m_RegType==1) Script = Script + "\tcase = (n==0) # (n==0) -> bool: =1(true) =0(false) : the first case is the reference for the first loop so it will not be normalized or registered\n";
	else 	Script = Script + "\tcase = 0\n";
		Script = Script + "\twhile case < len(allcases):\n";
			Script = Script + "\t\tLinearTransDTI= OutputPath + \"/Loop\" + str(n) + \"/Case\" + str(case+1) + \"_Loop\" + str(n) + \"_LinearTrans_DTI.nrrd\"\n";
			Script = Script + "\t\tif n == " + m_nbLoops_str + " : LoopFA= OutputPath + \"/Loop" + m_nbLoops_str + "/Case\" + str(case+1) + \"_Loop" + m_nbLoops_str + "_FinalFA.nrrd\" # the last FA will be the Final output\n";
			Script = Script + "\t\telse : LoopFA= OutputPath + \"/Loop\" + str(n) + \"/Case\" + str(case+1) + \"_Loop\" + str(n) + \"_FA.nrrd\"\n";
			Script = Script + "\t\tGeneLoopFACommand=" + GridProcessCmd + "\"" + m_SoftPath[3] + " --dti_image \" + LinearTransDTI + \" -f \" + LoopFA" + GridApostrophe + "\n";
			Script = Script + "\t\tprint(\"||Case \" + str(case+1) + \" => $ \" + GeneLoopFACommand)\n";
			Script = Script + "\t\tnbStepsDone += 1\n";
			if(m_Overwrite==1)
			{
				Script = Script + "\t\tif os.system(GeneLoopFACommand)!=0 : ErrorList.append(\'[Loop \' + str(n) + \'][Case \' + str(case+1) + \'] dtiprocess: Generating FA of affine registered images\')\n";
				Script = Script + "\t\tlibc.sigqueue(" + MainPID_str + ", signal.SIGUSR2, int(float(100*nbStepsDone)/float(nbSteps))) # send a signal to the progress bar, with the progress value in it\n";
			}
			else
			{
				Script = Script + "\t\tif not os.path.isfile(LoopFA) :\n";
					Script = Script + "\t\t\tif os.system(GeneLoopFACommand)!=0 : ErrorList.append(\'[Loop \' + str(n) + \'][Case \' + str(case+1) + \'] dtiprocess: Generating FA of affine registered images\')\n";
					Script = Script + "\t\t\tlibc.sigqueue(" + MainPID_str + ", signal.SIGUSR2, int(float(100*nbStepsDone)/float(nbSteps))) # send a signal to the progress bar, with the progress value in it\n";
				Script = Script + "\t\telse : " + GridProcessFileExistIndent2 + "print(\"=> The file \\'\" + LoopFA + \"\\' already exists so the command will not be executed\")\n" + GridProcessFileExistCmd2;
			}
			Script = Script + "\t\tcase += 1\n\n";

if( m_useGridProcess ) Script = Script + "\tTestGridProcess( FilesFolder, len(allcases), " + NoCase1 + "*(n==0)) # stays in the function until all process is done : 0 makes the function look for \'file\'\n\n";

/* FA Average of registered images with ImageMath */
	Script = Script + "# FA Average of registered images with ImageMath\n";
		Script = Script + "\tif n != " + m_nbLoops_str + " : # this will not be done for the last lap\n";
			Script = Script + "\t\tprint(\"\\n[LOOP \" + str(n) + \"/" + m_nbLoops_str + "] ======== Computing FA Average of registered images =========\")\n";
			Script = Script + "\t\tFAAverage = OutputPath + \"/Loop\" + str(n) + \"/Loop\" + str(n) + \"_FAAverage.nrrd\"\n";
		if(m_RegType==1) //use case as loop 1 ref
		{
			Script = Script + "\t\tif n == 0 : FAforAVG= OutputPath + \"/Case1_FA.nrrd\"\n";
			Script = Script + "\t\telse : FAforAVG= OutputPath + \"/Loop\" + str(n) + \"/Case1_Loop\" + str(n) + \"_FA.nrrd\"\n";
		}
		else 	Script = Script + "\t\tFAforAVG= OutputPath + \"/Loop\" + str(n) + \"/Case1_Loop\" + str(n) + \"_FA.nrrd\"\n";
			Script = Script + "\t\tAverageCommand = " + GridProcessCmdNoCase + "\"" + m_SoftPath[0] + " \" + FAforAVG + \" -outfile \" + FAAverage + \" -avg \"\n";
			Script = Script + "\t\tcase = 1\n";
			Script = Script + "\t\twhile case < len(allcases):\n";
				Script = Script + "\t\t\tFAforAVG= OutputPath + \"/Loop\" + str(n) + \"/Case\" + str(case+1) + \"_Loop\" + str(n) + \"_FA.nrrd \"\n";
				Script = Script + "\t\t\tAverageCommand= AverageCommand + FAforAVG\n";
				Script = Script + "\t\t\tcase += 1\n";
			Script = Script + "\t\tAverageCommand= AverageCommand" + GridApostrophe + "\n";
			Script = Script + "\t\tprint(\"=> $ \" + AverageCommand)\n";
			Script = Script + "\t\tnbStepsDone += 1\n";
			if(m_Overwrite==1)
			{
				Script = Script + "\t\tif os.system(AverageCommand)!=0 : ErrorList.append(\'[Loop \' + str(n) + \'] dtiaverage: Computing FA Average of registered images\')\n";
				Script = Script + "\t\tlibc.sigqueue(" + MainPID_str + ", signal.SIGUSR2, int(float(100*nbStepsDone)/float(nbSteps))) # send a signal to the progress bar, with the progress value in it\n";
			}
			else
			{
				Script = Script + "\t\tif not os.path.isfile(FAAverage) :\n";
					Script = Script + "\t\t\tif os.system(AverageCommand)!=0 : ErrorList.append(\'[Loop \' + str(n) + \'] dtiaverage: Computing FA Average of registered images\')\n";
					Script = Script + "\t\t\tlibc.sigqueue(" + MainPID_str + ", signal.SIGUSR2, int(float(100*nbStepsDone)/float(nbSteps))) # send a signal to the progress bar, with the progress value in it\n";
				Script = Script + "\t\telse : " + GridProcessFileExistIndent2 + "print(\"=> The file \\'\" + FAAverage + \"\\' already exists so the command will not be executed\")\n" + GridProcessFileExistCmdNoCase2;
			}

if( m_useGridProcess ) Script = Script + "\t\tTestGridProcess( FilesFolder, 0, 0) # stays in the function until all process is done : 0 makes the function look for \'file\'\n\n";

			Script = Script + "\t\tAtlasFAref = FAAverage # the average becomes the reference\n\n";

		Script = Script + "\tn += 1\n\n";

	Script = Script + "print(\"\\n============ End of Pre processing =============\")\n\n";

	Script = Script + "# Display errors\n";
	Script = Script + "if len(ErrorList) >0 :\n";
		Script = Script + "\tprint(\"\\n=> \" + len(ErrorList) + \" errors detected during the followind operations:\")\n";
		Script = Script + "\tfor error in ErrorList : print(\'\\n\' + error)\n";
		Script = Script + "\tsys.exit(1)\n";
	Script = Script + "else:\n";
		Script = Script + "\tprint(\"\\n=> No errors detected during preprocessing\")\n";
		Script = Script + "\tsys.exit(0)\n";

	m_Script_Preprocess=Script;
}

  /////////////////////////////////////////
 //           ATLAS BUILDING            //
/////////////////////////////////////////

void ScriptWriter::AtlasBuilding(std::string MainPID_str)
{
	std::string Script;

	std::cout<<"[AtlasBuilding]"; // command line display (no endl)

	Script = Script + "#!/usr/bin/python\n\n";
	Script = Script + "import os\n"; ///// To run a shell command : os.system("[shell command]")
	Script = Script + "import sys\n"; // to return an exit code
	Script = Script + "import signal\n";
	Script = Script + "import ctypes\n"; // for sigqueue : send a signal with a value in it to another process
	Script = Script + "libc = ctypes.CDLL(\"libc.so.6\")\n\n"; // load c lib

	Script = Script + "print(\"\\n============ Atlas Building =============\")\n\n";

	Script = Script + "# Files Paths\n";
	Script = Script + "DeformPath= \"" + m_OutputPath + "/DTIAtlas/2_NonLinear_Registration\"\n";
	Script = Script + "AffinePath= \"" + m_OutputPath + "/DTIAtlas/1_Affine_Registration\"\n";
	Script = Script + "FinalPath= \"" + m_OutputPath + "/DTIAtlas/3_Diffeomorphic_Atlas\"\n";
	Script = Script + "FinalResampPath= \"" + m_OutputPath + "/DTIAtlas/4_Final_Resampling\"\n";

	Script = Script + "ErrorList=[] #empty list\n\n";

	// nbSteps = (nbCases x CropDTI) + nbCases x Generate FA + (nbCases-1) x (Normalize, Affine reg, Apply transfm, generate FA) (1st loop if no template) + (nbLoops) x nbCases x (Normalize, Affine reg, Apply transfm, generate FA) + nbLoops x Compute average (not for the last loop) + nonLinear reg + nbCases x Apply tfm + DTI average + nbCases x (1st resamp 2nd resamp) + DTI average
	int nbSteps = m_CasesPath.size()*1; // nbCases x Generate FA
	if(m_NeedToBeCropped==1) nbSteps = nbSteps + m_CasesPath.size()*1; // (nbCases x CropDTI)
	if(m_RegType==1) nbSteps = nbSteps + (m_CasesPath.size()-1)*4; // (nbCases-1) x (Normalize, Affine reg, Apply transfm, generate FA) (1st loop if no template)
	else  nbSteps = nbSteps + m_CasesPath.size()*4; // nbCases x (Normalize, Affine reg, Apply transfm, generate FA) (1st loop if template)
	nbSteps = nbSteps + m_nbLoops*(m_CasesPath.size()*4) + m_nbLoops*1; // (nbLoops) x nbCases x (Normalize, Affine reg, Apply transfm, generate FA) + nbLoops x Compute average (not for the last loop)
	int nbStepsPreProc = nbSteps;
	nbSteps = nbSteps + 1 + m_CasesPath.size()*1 + 1 + m_CasesPath.size()*2 + 1; // nonLinear reg + nbCases x Apply tfm + DTI average + nbCases x (1st resamp 2nd resamp) + DTI average

	std::ostringstream oss1;
	oss1 << nbSteps;
	std::string nbSteps_str = oss1.str();
	Script = Script + "nbSteps=" + nbSteps_str  + " # for the progress bar\n";
	std::ostringstream oss2;
	oss2 << nbStepsPreProc;
	std::string nbStepsPreProc_str = oss2.str();
	Script = Script + "nbStepsDone=" + nbStepsPreProc_str  + " # because steps already done in preprocessing\n\n";

/* Call script to run command on grid */
	std::string GridApostrophe = "";
	std::string GridProcessCmd = "";
	std::string GridProcessFileExistCmd = "";
	std::string GridProcessFileExistCmd1 = "";
	std::string GridProcessFileExistCmd2 = "";
	std::string GridProcessCmdNoCase = "";
	std::string GridProcessFileExistCmdNoCase = "";
	std::string GridProcessFileExistCmdNoCase1 = "";
	std::string GridProcessFileExistCmdNoCase2 = "";
	std::string GridProcessFileExistIndent = "";
	std::string GridProcessFileExistIndent1 = "";
	std::string GridProcessFileExistIndent2 = "";
	if( m_useGridProcess )
	{
		Script = Script + "# Call script to run command on server\n";
		Script = Script + "ScriptCommand = \"" + m_OutputPath + "/DTIAtlas/Script/RunCommandOnServer.script\"\n";
		Script = Script + "FilesFolder = \"" + m_OutputPath + "/DTIAtlas/GridProcessingFiles\"\n\n";

		//Test Function
		Script = Script + "# Function that tests if all cases have been processed on the grid\n";
		Script = Script + "def TestGridProcess ( FilesFolder, NbCases ): # if NbCases == 0, then just search the file \'file\' (unique command)\n";
			Script = Script + "\tprint(\"\\n| Waiting for all cases (\" + str(NbCases) + \") to be processed on grid...\") # NoCase1 is 0 or 1\n";
			Script = Script + "\tfilesOK = 0\n";
			Script = Script + "\twhile not filesOK :\n";
				Script = Script + "\t\tfilesOK = 1\n";
				Script = Script + "\t\tif NbCases>0 : \n";
					Script = Script + "\t\t\tNbfilesOK = 0\n";
					Script = Script + "\t\t\tcase = 0\n";
					Script = Script + "\t\t\twhile case < NbCases:\n";
						Script = Script + "\t\t\t\tif not os.path.isfile( FilesFolder + \"/Case\" + str(case+1) ) : filesOK = 0\n";
						Script = Script + "\t\t\t\telse : NbfilesOK = NbfilesOK + 1\n";
						Script = Script + "\t\t\t\tcase += 1\n";
					Script = Script + "\t\t\tprint(\"\\r| [\" + str(NbfilesOK) + \"\\t/ \" + str(NbCases) + \" ] Files processed\"), # the comma prevents the print from jumping line\n";
				Script = Script + "\t\telif not os.path.isfile( FilesFolder + \"/file\" ) : filesOK = 0\n";
			Script = Script + "\tprint(\"\\n| All files processed\")\n";
			Script = Script + "\tos.system(\"rm \" + FilesFolder + \"/*\")\n\n";

		GridApostrophe = " + \"\\'\"";
		std::string File = "FilesFolder + \"/Case\" + str(case+1)";
		GridProcessCmd = "\"" + m_GridCommand + " " + m_OutputPath + "/DTIAtlas/Script/RunCommandOnServer.script \" + " + File + " + \" \"" + GridApostrophe + " + ";
		GridProcessFileExistCmd = "\tf = open( " + File + ", 'w')\n\tf.close()\n"; // if the image already exists, create the "semaphore" file
		GridProcessFileExistCmd1 = "\t\tf = open( " + File + ", 'w')\n\t\tf.close()\n"; // if the image already exists, create the "semaphore" file
		GridProcessFileExistCmd2 = "\t\t\tf = open( " + File + ", 'w')\n\t\t\tf.close()\n"; // if the image already exists, create the "semaphore" file

		std::string FileNoCase = "FilesFolder + \"/file\""; //  for the commands executed only once = not once per case
		GridProcessCmdNoCase = "\"" + m_GridCommand + " " + m_OutputPath + "/DTIAtlas/Script/RunCommandOnServer.script \" + " + FileNoCase + " + \" \"" + GridApostrophe + " + ";
		GridProcessFileExistCmdNoCase = "\tf = open( " + FileNoCase + ", 'w')\n\tf.close()\n";
		GridProcessFileExistCmdNoCase1 = "\t\tf = open( " + FileNoCase + ", 'w')\n\t\tf.close()\n";
		GridProcessFileExistCmdNoCase2 = "\t\t\tf = open( " + FileNoCase + ", 'w')\n\t\t\tf.close()\n";

		GridProcessFileExistIndent = "\n\t"; // if the file already exists, several commands in the else (create file), so change line and indent
		GridProcessFileExistIndent1 = "\n\t\t"; // if the file already exists, several commands in the else (create file), so change line and indent (double)
		GridProcessFileExistIndent2 = "\n\t\t\t"; // if the file already exists, several commands in the else (create file), so change line and indent (3)

	}

/* Create directory for temporary files and final */
	Script = Script + "# Create directory for temporary files and final\n";
	Script = Script + "if not os.path.isdir(DeformPath):\n";
		Script = Script + "\tprint(\"\\n=> Creation of the Deformation transform directory = \" + DeformPath)\n";
		Script = Script + "\tos.mkdir(DeformPath)\n\n";
	Script = Script + "if not os.path.isdir(FinalPath):\n";
		Script = Script + "\tprint(\"\\n=> Creation of the Final Atlas directory = \" + FinalPath)\n";
		Script = Script + "\tos.mkdir(FinalPath)\n\n";
	Script = Script + "if not os.path.isdir(FinalResampPath):\n";
		Script = Script + "\tprint(\"\\n=> Creation of the Final Resampling directory = \" + FinalResampPath)\n";
		Script = Script + "\tos.mkdir(FinalResampPath)\n\n";
	Script = Script + "if not os.path.isdir(FinalResampPath + \"/First_Resampling\"):\n";
		Script = Script + "\tprint(\"\\n=> Creation of the First Final Resampling directory = \" + FinalResampPath + \"/First_Resampling\")\n";
		Script = Script + "\tos.mkdir(FinalResampPath + \"/First_Resampling\")\n\n";
	Script = Script + "if not os.path.isdir(FinalResampPath + \"/Second_Resampling\"):\n";
		Script = Script + "\tprint(\"\\n => Creation of the Second Final Resampling directory = \" + FinalResampPath + \"/Second_Resampling\")\n";
		Script = Script + "\tos.mkdir(FinalResampPath + \"/Second_Resampling\")\n\n";

/* Cases variables: */
	Script = Script + "# Cases variables\n";
	Script = Script + "alltfms = [AffinePath + \"/Loop" + m_nbLoops_str + "/Case1_Loop" + m_nbLoops_str + "_LinearTrans.txt\"";
	for (unsigned int i=1;i<m_CasesPath.size();i++) 
	{
		std::ostringstream outi0;
		outi0 << i+1;
		std::string i0_str = outi0.str();
		Script = Script + ", AffinePath + \"/Loop" + m_nbLoops_str + "/Case" + i0_str + "_Loop" + m_nbLoops_str + "_LinearTrans.txt\"";
	}	
	Script = Script+ "]\n\n";

	if(m_NeedToBeCropped==1) Script = Script + "allcases = [AffinePath + \"/Case1_croppedDTI.nrrd\"";
	else Script = Script + "allcases = [\"" + m_CasesPath[0] + "\"";
	for (unsigned int i=1;i<m_CasesPath.size();i++) 
	{
		if(m_NeedToBeCropped==1)
		{
			std::ostringstream outi2;
			outi2 << i+1;
			std::string i2_str = outi2.str();
			Script = Script + ", AffinePath + \"/Case" + i2_str + "_croppedDTI.nrrd\"";
		}
		else Script = Script + ", \"" + m_CasesPath[i] + "\"";
	}
	Script = Script + "]\n\n";	

/* GreedyAtlas Command: */
	Script = Script + "# GreedyAtlas Command\n";
	Script = Script + "print(\"\\n======== Computing the Deformation Fields with GreedyAtlas =========\")\n";
	Script = Script + "XMLFile= DeformPath + \"/GreedyAtlasParameters.xml\"\n";
	Script = Script + "ParsedFile= DeformPath + \"/ParsedXML.xml\"\n";
	Script = Script + "AtlasBCommand= " + GridProcessCmdNoCase + "\"" + m_SoftPath[5] + " -f \" + XMLFile + \" -o \" + ParsedFile" + GridApostrophe + "\n";
	Script = Script + "print(\"=> $ \" + AtlasBCommand)\n";
	Script = Script + "nbStepsDone += 1\n";
	if(m_Overwrite==1)Script = Script + "if 1 :\n";
	else Script = Script + "if not os.path.isfile(DeformPath + \"/MeanImage.mhd\") :\n";
		Script = Script + "\tif os.system(AtlasBCommand)!=0 : ErrorList.append(\'GreedyAtlas: Computing non-linear atlas from affine registered images\')\n";
		Script = Script + "\tlibc.sigqueue(" + MainPID_str + ", signal.SIGUSR2, int(float(100*nbStepsDone)/float(nbSteps))) # send a signal to the progress bar, with the progress value in it\n";
if( m_useGridProcess ) Script = Script + "\tTestGridProcess( FilesFolder, 0) # stays in the function until all process is done : 0 makes the function look for \'file\'\n\n";

		Script = Script + "\tprint(\"\\n======== Renaming the files generated by GreedyAtlas =========\")\n";
		Script = Script + "\tcase = 0\n";
		Script = Script + "\twhile case < len(allcases): # Renaming\n";
			Script = Script + "\t\toriginalImage=DeformPath + \"/Case\" + str(case+1) + \"_Loop" + m_nbLoops_str + "_FinalFADefToMean.mhd\"\n";
			Script = Script + "\t\toriginalHField=DeformPath + \"/Case\" + str(case+1) + \"_Loop" + m_nbLoops_str + "_FinalFADefFieldImToMean.mhd\"\n";
			Script = Script + "\t\toriginalInvHField=DeformPath + \"/Case\" + str(case+1) + \"_Loop" + m_nbLoops_str + "_FinalFADefFieldMeanToIm.mhd\"\n";
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
if(m_Overwrite==0) Script = Script + "else : print(\"=> The file \\'\" + DeformPath + \"/MeanImage.mhd\\' already exists so the command will not be executed\")\n";

/* Apply deformation fields */
	Script = Script + "\n# Apply deformation fields\n";
	Script = Script + "print(\"\\n======== Applying deformation fields to original DTIs =========\")\n";
	Script = Script + "case = 0\n";
	Script = Script + "while case < len(allcases):\n";
		Script = Script + "\tFinalDTI= FinalPath + \"/Case\" + str(case+1) + \"_DiffeomorphicDTI.nrrd\"\n";
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
		Script = Script + "\tRef = AffinePath + \"/Loop" + nbLoops1_str + "/Loop" + nbLoops1_str + "_FAAverage.nrrd\"\n"; // an average image has been generated in the loops of affine reg for reference
		}
		Script = Script + "\tHField= DeformPath + \"/Case\" + str(case+1) + \"_DeformationField.mhd\"\n";
		Script = Script + "\tFinalReSampCommand=\"" + m_SoftPath[1] + " -R \" + Ref + \" -H \" + HField + \" -f \" + alltfms[case] + \" \" + originalDTI + \" \" + FinalDTI\n";
		/* options */
		if(m_InterpolType.compare("Linear")==0)			Script = Script + "\tFinalReSampCommand = FinalReSampCommand + \" -i linear\"\n";
		if(m_InterpolType.compare("Nearest Neighborhoor")==0)	Script = Script + "\tFinalReSampCommand = FinalReSampCommand + \" -i nn\"\n";
		if(m_InterpolType.compare("Windowed Sinc")==0)
		{
			if(m_InterpolOption.compare("Hamming")==0)	Script = Script + "\tFinalReSampCommand = FinalReSampCommand + \" -i ws -W h\"\n";
			if(m_InterpolOption.compare("Cosine")==0)	Script = Script + "\tFinalReSampCommand = FinalReSampCommand + \" -i ws -W c\"\n";
			if(m_InterpolOption.compare("Welch")==0)	Script = Script + "\tFinalReSampCommand = FinalReSampCommand + \" -i ws -W w\"\n";
			if(m_InterpolOption.compare("Lanczos")==0)	Script = Script + "\tFinalReSampCommand = FinalReSampCommand + \" -i ws -W l\"\n";
			if(m_InterpolOption.compare("Blackman")==0)	Script = Script + "\tFinalReSampCommand = FinalReSampCommand + \" -i ws -W b\"\n";
		}
		if(m_InterpolType.compare("BSpline")==0)
		{
			std::istringstream istr(m_InterpolOption);
			int i;
			istr >> i;
			if(i>=0 && i<=5) Script = Script + "\tFinalReSampCommand = FinalReSampCommand + \" -i bs -o " + m_InterpolOption + "\"\n";
		}
		if(m_TensInterpol.compare("Non Log Euclidean")==0)
		{
			if(m_InterpolLogOption.compare("Zero")==0)		Script = Script + "\tFinalReSampCommand = FinalReSampCommand + \" --nolog --correction zero\"\n";
			if(m_InterpolLogOption.compare("None")==0)		Script = Script + "\tFinalReSampCommand = FinalReSampCommand + \" --nolog --correction none\"\n";
			if(m_InterpolLogOption.compare("Absolute Value")==0)	Script = Script + "\tFinalReSampCommand = FinalReSampCommand + \" --nolog --correction abs\"\n";
			if(m_InterpolLogOption.compare("Nearest")==0)		Script = Script + "\tFinalReSampCommand = FinalReSampCommand + \" --nolog --correction nearest\"\n";
		}

		if(m_TensTfm.compare("Preservation of the Principal Direction (PPD)")==0)	Script = Script + "\tFinalReSampCommand = FinalReSampCommand + \" -T PPD\"\n";
		if(m_TensTfm.compare("Finite Strain (FS)")==0)	Script = Script + "\tFinalReSampCommand = FinalReSampCommand + \" -T FS\"\n";

		Script = Script + "\tprint(\"||Case \" + str(case+1) + \" => $ \" + FinalReSampCommand)\n";
		Script = Script + "\tnbStepsDone += 1\n";
		if(m_Overwrite==1) Script = Script + "\tif 1 :\n";
		else Script = Script + "\tif not os.path.isfile(FinalDTI) :\n";

			Script = Script + "\t\tDiffeomorphicCaseFA = FinalPath + \"/Case\" + str(case+1) + \"_DiffeomorphicFA.nrrd\"\n";
			Script = Script + "\t\tGeneDiffeomorphicCaseFACommand=\"" + m_SoftPath[3] + " --scalar_float --dti_image \" + FinalDTI + \" -f \" + DiffeomorphicCaseFA\n";

			Script = Script + "\t\tCaseDbleToFloatCommand=\"" + m_SoftPath[8] + " convert -t float -i \" + FinalDTI + \" | " + m_SoftPath[8] + " save -f nrrd -e gzip -o \" + FinalPath + \"/Case\" + str(case+1) + \"_DiffeomorphicDTI_float.nrrd\"\n\n";

			if( ! m_useGridProcess )
			{
			Script = Script + "\t\tif os.system(FinalReSampCommand)!=0 : ErrorList.append(\'[Case \' + str(case+1) + \'] ResampleDTIlogEuclidean: Applying deformation fields to original DTIs\')\n";

			Script = Script + "\t\tprint(\"||Case \" + str(case+1) + \" => $ \" + GeneDiffeomorphicCaseFACommand)\n";
			Script = Script + "\t\tif os.system(GeneDiffeomorphicCaseFACommand)!=0 : ErrorList.append(\'[Case \' + str(case+1) + \'] dtiprocess: Computing Diffeomorphic FA\')\n";

			Script = Script + "\t\tprint(\"||Case \" + str(case+1) + \" => $ \" + CaseDbleToFloatCommand + \"\\n\")\n";
			Script = Script + "\t\tif os.system(CaseDbleToFloatCommand)!=0 : ErrorList.append(\'[Case \' + str(case+1) + \'] unu: Converting the final DTI images from double to float DTI\')\n";
			}
			else // run all commands in the same time in the script
			{
			Script = Script + "\t\tFinalResampGridCommand=" + GridProcessCmd + "FinalReSampCommand + \"\\' \" + \"\\'\" + GeneDiffeomorphicCaseFACommand + \"\\' \" + \"\\'\" + CaseDbleToFloatCommand + \"\\'\"\n";
			Script = Script + "\t\tif os.system(FinalResampGridCommand)!=0 : ErrorList.append(\'[Case \' + str(case+1) + \'] Applying deformation fields to original DTIs\')\n";
			}

			Script = Script + "\t\tlibc.sigqueue(" + MainPID_str + ", signal.SIGUSR2, int(float(100*nbStepsDone)/float(nbSteps))) # send a signal to the progress bar, with the progress value in it\n";

if(m_Overwrite==0) Script = Script + "\telse : " + GridProcessFileExistIndent1 + "print(\"=> The file \\'\" + FinalDTI + \"\\' already exists so the command will not be executed\")\n" + GridProcessFileExistCmd1;
		Script = Script + "\tcase += 1\n\n";

if( m_useGridProcess ) Script = Script + "TestGridProcess( FilesFolder, len(allcases) ) # stays in the function until all process is done : 0 makes the function look for \'file\'\n\n";

/* dtiaverage computing */
	Script = Script + "# dtiaverage computing\n";
	Script = Script + "print(\"\\n======== Computing the Diffeomorphic DTI average =========\")\n";
	Script = Script + "DTIAverage = FinalPath + \"/DiffeomorphicAtlasDTI.nrrd\"\n";
	Script = Script + "AverageCommand = \"" + m_SoftPath[6] + " \"\n";
	Script = Script + "case = 0\n";
	Script = Script + "while case < len(allcases):\n";
		Script = Script + "\tDTIforAVG= \"--inputs \" + FinalPath + \"/Case\" + str(case+1) + \"_DiffeomorphicDTI.nrrd \"\n";
		Script = Script + "\tAverageCommand = AverageCommand + DTIforAVG\n";
		Script = Script + "\tcase += 1\n";
	Script = Script + "AverageCommand = AverageCommand + \"--tensor_output \" + DTIAverage\n";
	Script = Script + "print(\"=> $ \" + AverageCommand)\n";
	Script = Script + "nbStepsDone += 1\n";
	if(m_Overwrite==1)Script = Script + "if 1 : \n";
	else Script = Script + "if not os.path.isfile(DTIAverage) : \n";

/* Computing some images from the final DTI with dtiprocess */
	Script = Script + "# Computing some images from the final DTI with dtiprocess\n";
		Script = Script + "\tFA= FinalPath + \"/DiffeomorphicAtlasFA.nrrd\"\n";
		Script = Script + "\tcFA= FinalPath + \"/DiffeomorphicAtlasColorFA.nrrd\"\n";
		Script = Script + "\tRD= FinalPath + \"/DiffeomorphicAtlasRD.nrrd\"\n"; // Radial Diffusivity
		Script = Script + "\tMD= FinalPath + \"/DiffeomorphicAtlasMD.nrrd\"\n"; // Mean Diffusivity
		Script = Script + "\tAD= FinalPath + \"/DiffeomorphicAtlasAD.nrrd\"\n"; // Axial Diffusivity
		Script = Script + "\tGeneFACommand=\"" + m_SoftPath[3] + " --scalar_float --dti_image \" + DTIAverage + \" -f \" + FA + \" -m \" + MD + \" --color_fa_output \" + cFA + \" --RD_output \" + RD + \" --lambda1_output \" + AD\n\n";

		Script = Script + "\tDbleToFloatCommand=\"" + m_SoftPath[8] + " convert -t float -i \" + DTIAverage + \" | " + m_SoftPath[8] + " save -f nrrd -e gzip -o \" + FinalPath + \"/DiffeomorphicAtlasDTI_float.nrrd\"\n\n";

		if( ! m_useGridProcess )
		{
		Script = Script + "\tif os.system(AverageCommand)!=0 : ErrorList.append(\'dtiaverage: Computing the final DTI average\')\n";

		Script = Script + "\tprint(\"=> $ \" + GeneFACommand)\n";
		Script = Script + "\tif os.system(GeneFACommand)!=0 : ErrorList.append(\'dtiprocess: Computing Diffeomorphic FA, color FA, MD, RD and AD\')\n";

		Script = Script + "\tprint(\"=> $ \" + DbleToFloatCommand)\n";
		Script = Script + "\tif os.system(DbleToFloatCommand)!=0 : ErrorList.append(\'unu: Converting the final DTI atlas from double to float DTI\')\n";
		}
		else // run all commands in the same time in the script
		{
		Script = Script + "\tAverageGridCommand=" + GridProcessCmdNoCase + "AverageCommand + \"\\' \" + \"\\'\" + GeneFACommand + \"\\' \" + \"\\'\" + DbleToFloatCommand + \"\\'\"\n";
		Script = Script + "\tif os.system(AverageGridCommand)!=0 : ErrorList.append(\'Computing the final DTI average\')\n";
		}

		Script = Script + "\tlibc.sigqueue(" + MainPID_str + ", signal.SIGUSR2, int(float(100*nbStepsDone)/float(nbSteps))) # send a signal to the progress bar, with the progress value in it\n";
if(m_Overwrite==0) Script = Script + "else : " + GridProcessFileExistIndent + "print(\"=> The file \\'\" + DTIAverage + \"\\' already exists so the command will not be executed\")\n" + GridProcessFileExistCmdNoCase;

if( m_useGridProcess ) Script = Script + "TestGridProcess( FilesFolder, 0 ) # stays in the function until all process is done : 0 makes the function look for \'file\'\n\n";

/* Computing global deformation fields */
	Script = Script + "# Computing global deformation fields\n";
/* Threads
	if( m_DTIRegOptions[0].compare("ANTS")==0 )
	{

	Script = Script + "import thread\n";
	Script = Script + "nbCPUs = os.sysconf(\"SC_NPROCESSORS_ONLN\")\n";
	Script = Script + "nbRunningThreads=0;\n";
	Script = Script + "def thread_executeANTS (program) : # function to use for the htread is ANTS chosen\n";
		Script = Script + "\tglobal nbRunningThreads # to be able to use the variable nbRunningThreads as a global one\n";
		Script = Script + "\tglobal nbCPUs # to be able to use the variable nbCPUs as a global one\n";
		Script = Script + "\tglobal ErrorList # to be able to use the variable ErrorList as a global one\n";
		Script = Script + "\tnbRunningThreads = nbRunningThreads + 1\n";
//		Script = Script + "\tprint(str(nbRunningThreads) + \" threads running, \" + str(nbCPUs) + \" CPUs on the machine\")\n";
//		Script = Script + "\tprint(\"Executing: \" + program)\n";
		Script = Script + "\tif os.system(program)!=0 : ErrorList.append(\'[Case \' + str(case+1) + \'] DTI-Reg: Computing global deformation fields\')\n";
		Script = Script + "\tnbRunningThreads = nbRunningThreads - 1;\n\n";
//		Script = Script + "\tprint(str(nbRunningThreads) + \" threads running, \" + str(nbCPUs) + \" CPUs on the machine\")\n";
	}
*/

/*MULTITHREADING	if( m_DTIRegOptions[0].compare("BRAINS")==0 ) Script = Script + "\t\tif os.system(GlobalDefFieldCommand)!=0 : ErrorList.append(\'[Case \' + str(case+1) + \'] DTI-Reg: Computing global deformation fields\')\n";
			if( m_DTIRegOptions[0].compare("ANTS")==0 ) //use threads for ANTS because very slow
			{
				Script = Script + "\t\twhile nbRunningThreads >= nbCPUs : pass # waiting here for a thread to be free (pass = do nothing)\n";
				Script = Script + "\t\ttry:\n";
					Script = Script + "\t\t\tthread.start_new_thread( thread_executeANTS, (GlobalDefFieldCommand, ) )\n";
				Script = Script + "\t\texcept:\n";
					Script = Script + "\t\t\tprint \"Error: unable to start thread\"\n";
			}*/

	Script = Script + "print(\"\\n======== Computing global deformation fields =========\")\n";
	Script = Script + "case = 0\n";
	Script = Script + "while case < len(allcases):\n";
		if(m_NeedToBeCropped==1) Script = Script + "\torigDTI= AffinePath + \"/Case\" + str(case+1) + \"_croppedDTI.nrrd\"\n";
		else Script = Script + "\torigDTI= allcases[case]\n";
		Script = Script + "\tGlobalDefField = FinalResampPath + \"/First_Resampling/Case\" + str(case+1) + \"_GlobalDeformationField.nrrd\"\n";
		Script = Script + "\tFinalDef = FinalResampPath + \"/First_Resampling/Case\" + str(case+1) + \"_DeformedDTI.nrrd\"\n";
		Script = Script + "\tGlobalDefFieldCommand=\"" + m_SoftPath[7] + " --fixedVolume \" + DTIAverage + \" --movingVolume \" + origDTI + \" --outputDeformationFieldVolume \" + GlobalDefField + \" --outputVolume \" + FinalDef\n";

		// give DTI-Reg the paths to the binary directories of BRAINS (4), dtiprocess (3) and ResampleDTIlogEuclidean (1) (ANTS given in option in the GUI : DTIRegExtraPath)
		// m_SoftPath[] contains executables, and DTI-Reg only needs the paths
		std::string BRAINSExecDir = itksys::SystemTools::GetRealPath( itksys::SystemTools::GetFilenamePath(m_SoftPath[4]).c_str() );
		std::string dtiprocessExecDir = itksys::SystemTools::GetRealPath( itksys::SystemTools::GetFilenamePath(m_SoftPath[3]).c_str() );
		std::string ResampExecDir = itksys::SystemTools::GetRealPath( itksys::SystemTools::GetFilenamePath(m_SoftPath[1]).c_str() );
		Script = Script + "\tGlobalDefFieldCommand = GlobalDefFieldCommand + \" --ProgramsPathsVector " + m_DTIRegExtraPath + ";" + BRAINSExecDir + ";" + dtiprocessExecDir + ";" + ResampExecDir + "\"\n";
/* m_DTIRegOptions[]
0	RegMethod
	ANTS
	1	ARegType
	2	TfmStep
	3	Iter
	4	SimMet
	5	SimParam
	6	GSigma
	7	SmoothOff
	BRAINS
	1	BRegType
	2	TfmMode
	3	Sigma
	4	NbPyrLev
	5	PyrLevIt
*/
		if( m_DTIRegOptions[0].compare("BRAINS")==0 )
		{
			Script = Script + "\tGlobalDefFieldCommand= GlobalDefFieldCommand + \" --method useScalar-BRAINS\"\n";
			if(m_DTIRegOptions[1].compare("GreedyDiffeo (SyN)")==0) Script = Script + "\tGlobalDefFieldCommand= GlobalDefFieldCommand + \" --BRAINSRegistrationType GreedyDiffeo\"\n";
			else if(m_DTIRegOptions[1].compare("SpatioTempDiffeo (SyN)")==0) Script = Script + "\tGlobalDefFieldCommand= GlobalDefFieldCommand + \" --BRAINSRegistrationType SpatioTempDiffeo\"\n";
			else Script = Script + "\tGlobalDefFieldCommand= GlobalDefFieldCommand + \" --BRAINSRegistrationType " + m_DTIRegOptions[1] + "\"\n";
			Script = Script + "\tGlobalDefFieldCommand= GlobalDefFieldCommand + \" --BRAINSSmoothDefFieldSigma " + m_DTIRegOptions[3] + "\"\n";
			Script = Script + "\tGlobalDefFieldCommand= GlobalDefFieldCommand + \" --BRAINSnumberOfPyramidLevels " + m_DTIRegOptions[4] + "\"\n";
			Script = Script + "\tGlobalDefFieldCommand= GlobalDefFieldCommand + \" --BRAINSarrayOfPyramidLevelIterations " + m_DTIRegOptions[5] + "\"\n";
			if(m_DTIRegOptions[2].compare("Use computed affine transform")==0) Script = Script + "\tGlobalDefFieldCommand= GlobalDefFieldCommand + \" --initialAffine \" + alltfms[case]\n";
			else Script = Script + "\tGlobalDefFieldCommand= GlobalDefFieldCommand + \" --BRAINSinitializeTransformMode " + m_DTIRegOptions[2] + "\"\n";
			Script = Script + "\tBRAINSTempTfm = FinalResampPath + \"/First_Resampling/Case\" + str(case+1) + \"_FA_AffReg.txt\"\n"; // so that nothing is stored in the same dir than the Atlas
			Script = Script + "\tGlobalDefFieldCommand= GlobalDefFieldCommand + \" --outputTransform \" + BRAINSTempTfm\n";
		}
		if( m_DTIRegOptions[0].compare("ANTS")==0 )
		{
			Script = Script + "\tGlobalDefFieldCommand= GlobalDefFieldCommand + \" --method useScalar-ANTS\"\n";
			if(m_DTIRegOptions[1].compare("GreedyDiffeo (SyN)")==0) Script = Script + "\tGlobalDefFieldCommand= GlobalDefFieldCommand + \" --ANTSRegistrationType GreedyDiffeo\"\n";
			else if(m_DTIRegOptions[1].compare("SpatioTempDiffeo (SyN)")==0) Script = Script + "\tGlobalDefFieldCommand= GlobalDefFieldCommand + \" --ANTSRegistrationType SpatioTempDiffeo\"\n";
			else Script = Script + "\tGlobalDefFieldCommand= GlobalDefFieldCommand + \" --ANTSRegistrationType " + m_DTIRegOptions[1] + "\"\n";
			Script = Script + "\tGlobalDefFieldCommand= GlobalDefFieldCommand + \" --ANTSTransformationStep " + m_DTIRegOptions[2] + "\"\n";
			Script = Script + "\tGlobalDefFieldCommand= GlobalDefFieldCommand + \" --ANTSIterations " + m_DTIRegOptions[3] + "\"\n";
			if(m_DTIRegOptions[4].compare("Cross-Correlation (CC)")==0) Script = Script + "\tGlobalDefFieldCommand= GlobalDefFieldCommand + \" --ANTSSimilarityMetric CC\"\n";
			else if(m_DTIRegOptions[4].compare("Mutual Information (MI)")==0) Script = Script + "\tGlobalDefFieldCommand= GlobalDefFieldCommand + \" --ANTSSimilarityMetric MI\"\n";
			else if(m_DTIRegOptions[4].compare("Mean Square Difference (MSQ)")==0) Script = Script + "\tGlobalDefFieldCommand= GlobalDefFieldCommand + \" --ANTSSimilarityMetric MSQ\"\n";
			Script = Script + "\tGlobalDefFieldCommand= GlobalDefFieldCommand + \" --ANTSSimilarityParameter " + m_DTIRegOptions[5] + "\"\n";
			Script = Script + "\tGlobalDefFieldCommand= GlobalDefFieldCommand + \" --ANTSGaussianSigma " + m_DTIRegOptions[6] + "\"\n";
			if( m_DTIRegOptions[7].compare("1")==0 ) Script = Script + "\tGlobalDefFieldCommand= GlobalDefFieldCommand + \" --ANTSGaussianSmoothingOff\"\n";
			Script = Script + "\tGlobalDefFieldCommand= GlobalDefFieldCommand + \" --initialAffine \" + alltfms[case]\n";
			Script = Script + "\tANTSTempFileBase = FinalResampPath + \"/First_Resampling/Case\" + str(case+1) + \"_FA_\"\n"; // so that nothing is stored in the same dir than the Atlas
			Script = Script + "\tGlobalDefFieldCommand= GlobalDefFieldCommand + \" --ANTSOutbase \" + ANTSTempFileBase\n"; // no --outputTfm for ANTS because --ANTSOutbase is used for the tfm
		}
		Script = Script + "\tprint(\"\\n||Case \" + str(case+1) + \" => $ \" + GlobalDefFieldCommand)\n";
		Script = Script + "\tnbStepsDone += 1\n";

		if(m_Overwrite==1) Script = Script + "\tif 1 :\n";
		else Script = Script + "\tif not os.path.isfile(FinalDef) :\n";

			Script = Script + "\t\tGlobDbleToFloatCommand=\"" + m_SoftPath[8] + " convert -t float -i \" + FinalDef + \" | " + m_SoftPath[8] + " save -f nrrd -e gzip -o \" + FinalResampPath + \"/First_Resampling/Case\" + str(case+1) + \"_DeformedDTI_float.nrrd\"\n\n";

			if( ! m_useGridProcess )
			{
			Script = Script + "\t\tif os.system(GlobalDefFieldCommand)!=0 : ErrorList.append(\'[Case \' + str(case+1) + \'] DTI-Reg: Computing global deformation fields\')\n";

			Script = Script + "\t\tprint(\"\\n||Case \" + str(case+1) + \" => $ \" + GlobDbleToFloatCommand)\n";
			Script = Script + "\t\tif os.system(GlobDbleToFloatCommand)!=0 : ErrorList.append(\'[Case \' + str(case+1) + \'] unu: Converting the deformed images from double to float DTI\')\n";
			}
			else // run all commands in the same time in the script
			{
			Script = Script + "\t\tGlobDefFieldGridCommand=" + GridProcessCmd + "GlobalDefFieldCommand + \"\\' \" + \"\\'\" + GlobDbleToFloatCommand + \"\\'\"\n";
			Script = Script + "\t\tif os.system(GlobDefFieldGridCommand)!=0 : ErrorList.append(\'[Case \' + str(case+1) + \'] Computing global deformation fields\')\n";
			}

			Script = Script + "\t\tlibc.sigqueue(" + MainPID_str + ", signal.SIGUSR2, int(float(100*nbStepsDone)/float(nbSteps))) # send a signal to the progress bar, with the progress value in it\n";
if(m_Overwrite==0) Script = Script + "\telse : " + GridProcessFileExistIndent1 + "print(\"=> The file \\'\" + FinalDef + \"\\' already exists so the command will not be executed\")\n" + GridProcessFileExistCmd1;
		Script = Script + "\tcase += 1\n\n";

//if( m_DTIRegOptions[0].compare("ANTS")==0 )	Script = Script + "while nbRunningThreads > 0 : pass # waiting for all the threads to be finished\n\n";

if( m_useGridProcess ) Script = Script + "TestGridProcess( FilesFolder, len(allcases) ) # stays in the function until all process is done : 0 makes the function look for \'file\'\n\n";

/* dtiaverage recomputing */
	Script = Script + "# dtiaverage recomputing\n";
	Script = Script + "print(\"\\n======== Recomputing the final DTI average =========\")\n";
	Script = Script + "DTIAverage2 = FinalResampPath + \"/FinalAtlasDTI.nrrd\"\n";
	Script = Script + "AverageCommand2 = \"" + m_SoftPath[6] + " \"\n";
	Script = Script + "case = 0\n";
	Script = Script + "while case < len(allcases):\n";
		Script = Script + "\tDTIforAVG2= \"--inputs \" + FinalResampPath + \"/First_Resampling/Case\" + str(case+1) + \"_DeformedDTI.nrrd \"\n";
		Script = Script + "\tAverageCommand2 = AverageCommand2 + DTIforAVG2\n";
		Script = Script + "\tcase += 1\n";
	Script = Script + "AverageCommand2 = AverageCommand2 + \"--tensor_output \" + DTIAverage2\n";
	Script = Script + "print(\"=> $ \" + AverageCommand2)\n";
	Script = Script + "nbStepsDone += 1\n";
	if(m_Overwrite==1)Script = Script + "if 1 : \n";
	else Script = Script + "if not os.path.isfile(DTIAverage2) : \n";

/* Computing some images from the final DTI with dtiprocess */
	Script = Script + "# Computing some images from the final DTI with dtiprocess\n";
		Script = Script + "\tFA2= FinalResampPath + \"/FinalAtlasFA.nrrd\"\n";
		Script = Script + "\tcFA2= FinalResampPath + \"/FinalAtlasColorFA.nrrd\"\n";
		Script = Script + "\tRD2= FinalResampPath + \"/FinalAtlasRD.nrrd\"\n"; // Radial Diffusivity
		Script = Script + "\tMD2= FinalResampPath + \"/FinalAtlasMD.nrrd\"\n"; // Mean Diffusivity
		Script = Script + "\tAD2= FinalResampPath + \"/FinalAtlasAD.nrrd\"\n"; // Axial Diffusivity
		Script = Script + "\tGeneFACommand2=\"" + m_SoftPath[3] + " --scalar_float --dti_image \" + DTIAverage2 + \" -f \" + FA2 + \" -m \" + MD2 + \" --color_fa_output \" + cFA2 + \" --RD_output \" + RD2 + \" --lambda1_output \" + AD2\n\n";

		Script = Script + "\tDbleToFloatCommand2=\"" + m_SoftPath[8] + " convert -t float -i \" + DTIAverage2 + \" | " + m_SoftPath[8] + " save -f nrrd -e gzip -o \" + FinalResampPath + \"/FinalAtlasDTI_float.nrrd\"\n\n";

		if( ! m_useGridProcess )
		{
		Script = Script + "\tif os.system(AverageCommand2)!=0 : ErrorList.append(\'dtiaverage: Recomputing the final DTI average\')\n";

		Script = Script + "\tprint(\"=> $ \" + GeneFACommand2)\n";
		Script = Script + "\tif os.system(GeneFACommand2)!=0 : ErrorList.append(\'dtiprocess: Recomputing final FA, color FA, MD, RD and AD\')\n";

		Script = Script + "\tprint(\"=> $ \" + DbleToFloatCommand2)\n";
		Script = Script + "\tif os.system(DbleToFloatCommand2)!=0 : ErrorList.append(\'unu: Converting the final resampled DTI atlas from double to float DTI\')\n";
		}
		else // run all commands in the same time in the script
		{
		Script = Script + "\tAverage2GridCommand=" + GridProcessCmdNoCase + "AverageCommand2 + \"\\' \" + \"\\'\" + GeneFACommand2 + \"\\' \" + \"\\'\" + DbleToFloatCommand2 + \"\\'\"\n";
		Script = Script + "\tif os.system(Average2GridCommand)!=0 : ErrorList.append(\'Recomputing the final DTI average\')\n";
		}

		Script = Script + "\tlibc.sigqueue(" + MainPID_str + ", signal.SIGUSR2, int(float(100*nbStepsDone)/float(nbSteps))) # send a signal to the progress bar, with the progress value in it\n";
if(m_Overwrite==0) Script = Script + "else : " + GridProcessFileExistIndent + "print(\"=> The file \\'\" + DTIAverage2 + \"\\' already exists so the command will not be executed\")\n" + GridProcessFileExistCmdNoCase;

if( m_useGridProcess ) Script = Script + "TestGridProcess( FilesFolder, 0 ) # stays in the function until all process is done : 0 makes the function look for \'file\'\n\n";

/* Recomputing global deformation fields */
	Script = Script + "# Recomputing global deformation fields\n";
	Script = Script + "print(\"\\n======== Recomputing global deformation fields =========\")\n";
	Script = Script + "case = 0\n";
	Script = Script + "while case < len(allcases):\n";
		if(m_NeedToBeCropped==1) Script = Script + "\torigDTI2= AffinePath + \"/Case\" + str(case+1) + \"_croppedDTI.nrrd\"\n";
		else Script = Script + "\torigDTI2= allcases[case]\n";
		Script = Script + "\tGlobalDefField2 = FinalResampPath + \"/Second_Resampling/Case\" + str(case+1) + \"_GlobalDeformationField.nrrd\"\n";
		Script = Script + "\tFinalDef2 = FinalResampPath + \"/Second_Resampling/Case\" + str(case+1) + \"_FinalDeformedDTI.nrrd\"\n";
		Script = Script + "\tGlobalDefFieldCommand2=\"" + m_SoftPath[7] + " --fixedVolume \" + DTIAverage2 + \" --movingVolume \" + origDTI2 + \" --outputDeformationFieldVolume \" + GlobalDefField2 + \" --outputVolume \" + FinalDef2\n";

		// give DTI-Reg the paths to the binary directories of ANTS (6), BRAINS (5) and ResampleDTIlogEuclidean (2) | m_DTIRegExtraPath is supposed to contain ANTS
		// ANTSExecDir, BRAINSExecDir, ResampExecDir defined in the previous resampling
		Script = Script + "\tGlobalDefFieldCommand2 = GlobalDefFieldCommand2 + \" --ProgramsPathsVector " + m_DTIRegExtraPath + ";" + BRAINSExecDir + ";" + dtiprocessExecDir + ";" + ResampExecDir + "\"\n";

/* m_DTIRegOptions[]
0	RegMethod
	ANTS
	1	ARegType
	2	TfmStep
	3	Iter
	4	SimMet
	5	SimParam
	6	GSigma
	7	SmoothOff
	BRAINS
	1	BRegType
	2	TfmMode
	3	Sigma
	4	NbPyrLev
	5	PyrLevIt
*/
		if( m_DTIRegOptions[0].compare("BRAINS")==0 )
		{
			Script = Script + "\tGlobalDefFieldCommand2= GlobalDefFieldCommand2 + \" --method useScalar-BRAINS\"\n";
			if(m_DTIRegOptions[1].compare("GreedyDiffeo (SyN)")==0) Script = Script + "\tGlobalDefFieldCommand2= GlobalDefFieldCommand2 + \" --BRAINSRegistrationType GreedyDiffeo\"\n";
			else if(m_DTIRegOptions[1].compare("SpatioTempDiffeo (SyN)")==0) Script = Script + "\tGlobalDefFieldCommand2= GlobalDefFieldCommand2 + \" --BRAINSRegistrationType SpatioTempDiffeo\"\n";
			else Script = Script + "\tGlobalDefFieldCommand2= GlobalDefFieldCommand2 + \" --BRAINSRegistrationType " + m_DTIRegOptions[1] + "\"\n";
			Script = Script + "\tGlobalDefFieldCommand2= GlobalDefFieldCommand2 + \" --BRAINSSmoothDefFieldSigma " + m_DTIRegOptions[3] + "\"\n";
			Script = Script + "\tGlobalDefFieldCommand2= GlobalDefFieldCommand2 + \" --BRAINSnumberOfPyramidLevels " + m_DTIRegOptions[4] + "\"\n";
			Script = Script + "\tGlobalDefFieldCommand2= GlobalDefFieldCommand2 + \" --BRAINSarrayOfPyramidLevelIterations " + m_DTIRegOptions[5] + "\"\n";
			if(m_DTIRegOptions[2].compare("Use computed affine transform")==0) Script = Script + "\tGlobalDefFieldCommand2= GlobalDefFieldCommand2 + \" --initialAffine \" + alltfms[case]\n";
			else Script = Script + "\tGlobalDefFieldCommand2= GlobalDefFieldCommand2 + \" --BRAINSinitializeTransformMode " + m_DTIRegOptions[2] + "\"\n";
			Script = Script + "\tBRAINSTempTfm2 = FinalResampPath + \"/Second_Resampling/Case\" + str(case+1) + \"_FA_AffReg.txt\"\n"; // so that nothing is stored in the same dir than the Atlas
			Script = Script + "\tGlobalDefFieldCommand2= GlobalDefFieldCommand2 + \" --outputTransform \" + BRAINSTempTfm2\n";
		}
		if( m_DTIRegOptions[0].compare("ANTS")==0 )
		{
			Script = Script + "\tGlobalDefFieldCommand2= GlobalDefFieldCommand2 + \" --method useScalar-ANTS\"\n";
			if(m_DTIRegOptions[1].compare("GreedyDiffeo (SyN)")==0) Script = Script + "\tGlobalDefFieldCommand2= GlobalDefFieldCommand2 + \" --ANTSRegistrationType GreedyDiffeo\"\n";
			else if(m_DTIRegOptions[1].compare("SpatioTempDiffeo (SyN)")==0) Script = Script + "\tGlobalDefFieldCommand2= GlobalDefFieldCommand2 + \" --ANTSRegistrationType SpatioTempDiffeo\"\n";
			else Script = Script + "\tGlobalDefFieldCommand2= GlobalDefFieldCommand2 + \" --ANTSRegistrationType " + m_DTIRegOptions[1] + "\"\n";
			Script = Script + "\tGlobalDefFieldCommand2= GlobalDefFieldCommand2 + \" --ANTSTransformationStep " + m_DTIRegOptions[2] + "\"\n";
			Script = Script + "\tGlobalDefFieldCommand2= GlobalDefFieldCommand2 + \" --ANTSIterations " + m_DTIRegOptions[3] + "\"\n";
			if(m_DTIRegOptions[4].compare("Cross-Correlation (CC)")==0) Script = Script + "\tGlobalDefFieldCommand2= GlobalDefFieldCommand2 + \" --ANTSSimilarityMetric CC\"\n";
			else if(m_DTIRegOptions[4].compare("Mutual Information (MI)")==0) Script = Script + "\tGlobalDefFieldCommand2= GlobalDefFieldCommand2 + \" --ANTSSimilarityMetric MI\"\n";
			else if(m_DTIRegOptions[4].compare("Mean Square Difference (MSQ)")==0) Script = Script + "\tGlobalDefFieldCommand2= GlobalDefFieldCommand2 + \" --ANTSSimilarityMetric MSQ\"\n";
			Script = Script + "\tGlobalDefFieldCommand2= GlobalDefFieldCommand2 + \" --ANTSSimilarityParameter " + m_DTIRegOptions[5] + "\"\n";
			Script = Script + "\tGlobalDefFieldCommand2= GlobalDefFieldCommand2 + \" --ANTSGaussianSigma " + m_DTIRegOptions[6] + "\"\n";
			if( m_DTIRegOptions[7].compare("1")==0 ) Script = Script + "\tGlobalDefFieldCommand2= GlobalDefFieldCommand2 + \" --ANTSGaussianSmoothingOff\"\n";
			Script = Script + "\tGlobalDefFieldCommand2= GlobalDefFieldCommand2 + \" --initialAffine \" + alltfms[case]\n";
			Script = Script + "\tANTSTempFileBase2 = FinalResampPath + \"/Second_Resampling/Case\" + str(case+1) + \"_FA_\"\n"; // so that nothing is stored in the same dir than the Atlas
			Script = Script + "\tGlobalDefFieldCommand2= GlobalDefFieldCommand2 + \" --ANTSOutbase \" + ANTSTempFileBase2\n"; // no --outputTfm for ANTS because --ANTSOutbase is used for the tfm
		}
		Script = Script + "\tprint(\"\\n||Case \" + str(case+1) + \" => $ \" + GlobalDefFieldCommand2)\n";
		Script = Script + "\tnbStepsDone += 1\n";
		if(m_Overwrite==1) Script = Script + "\tif 1 :\n";
		else Script = Script + "\tif not os.path.isfile(FinalDef2) :\n";

			Script = Script + "\t\tDTIRegCaseFA = FinalResampPath + \"/Second_Resampling/Case\" + str(case+1) + \"_FinalDeformedFA.nrrd\"\n";
			Script = Script + "\t\tGeneDTIRegCaseFACommand=\"" + m_SoftPath[3] + " --scalar_float --dti_image \" + FinalDef2 + \" -f \" + DTIRegCaseFA\n";

			Script = Script + "\t\tGlobDbleToFloatCommand2=\"" + m_SoftPath[8] + " convert -t float -i \" + FinalDef2 + \" | " + m_SoftPath[8] + " save -f nrrd -e gzip -o \" + FinalResampPath + \"/Second_Resampling/Case\" + str(case+1) + \"_FinalDeformedDTI_float.nrrd\"\n";

			if( ! m_useGridProcess )
			{
			Script = Script + "\t\tif os.system(GlobalDefFieldCommand2)!=0 : ErrorList.append(\'[Case \' + str(case+1) + \'] DTI-Reg: Computing global deformation fields\')\n";

			Script = Script + "\t\tprint(\"\\n||Case \" + str(case+1) + \" => $ \" + GlobDbleToFloatCommand2)\n";
			Script = Script + "\t\tif os.system(GlobDbleToFloatCommand2)!=0 : ErrorList.append(\'[Case \' + str(case+1) + \'] unu: Converting the deformed images from double to float DTI\')\n";

			Script = Script + "\t\tprint(\"\\n||Case \" + str(case+1) + \" => $ \" + GeneDTIRegCaseFACommand)\n";
			Script = Script + "\t\tif os.system(GeneDTIRegCaseFACommand)!=0 : ErrorList.append(\'[Case \' + str(case+1) + \'] dtiprocess: Computing DTIReg FA\')\n";
			}
			else // run all commands in the same time in the script
			{
			Script = Script + "\t\tGlobDefField2GridCommand=" + GridProcessCmd + "GlobalDefFieldCommand2 + \"\\' \" + \"\\'\" + GlobDbleToFloatCommand2 + \"\\' \" + \"\\'\" + GeneDTIRegCaseFACommand + \"\\'\"\n";
			Script = Script + "\t\tif os.system(GlobDefField2GridCommand)!=0 : ErrorList.append(\'[Case \' + str(case+1) + \'] Recomputing global deformation fields\')\n";
			}

			Script = Script + "\t\tlibc.sigqueue(" + MainPID_str + ", signal.SIGUSR2, int(float(100*nbStepsDone)/float(nbSteps))) # send a signal to the progress bar, with the progress value in it\n";
if(m_Overwrite==0) Script = Script + "\telse : " + GridProcessFileExistIndent1 + "print(\"=> The file \\'\" + FinalDef2 + \"\\' already exists so the command will not be executed\")\n" + GridProcessFileExistCmd1;
		Script = Script + "\tcase += 1\n\n";

if( m_useGridProcess ) Script = Script + "TestGridProcess( FilesFolder, len(allcases) ) # stays in the function until all process is done : 0 makes the function look for \'file\'\n\n";

	Script = Script + "print(\"\\n============ End of Atlas Building =============\")\n\n";

	Script = Script + "# Display errors\n";
	Script = Script + "if len(ErrorList) >0 :\n";
		Script = Script + "\tprint(\"\\n=> \" + len(ErrorList) + \" errors detected during the followind operations:\")\n";
		Script = Script + "\tfor error in ErrorList : print(\'\\n\' + error)\n";
		Script = Script + "\tsys.exit(1)\n";
	Script = Script + "else:\n";
		Script = Script + "\tprint(\"\\n=> No errors detected during Atlas building\")\n";
		Script = Script + "\tsys.exit(0)\n";


	m_Script_AtlasBuilding=Script;
}

void ScriptWriter::MainScript()
{
	std::string Script;

	std::cout<<"[Main]"<<std::endl; // command line display (no endl)

	Script = Script + "#!/usr/bin/python\n\n";
	Script = Script + "import os\n"; ///// To run a shell command : os.system("[shell command]")
	Script = Script + "import time\n"; // to compute the execution time
	Script = Script + "import sys\n\n"; // to return an exit code
	Script = Script + "print(\"\\n=============== Main Script ================\")\n\n";

	Script = Script + "OutputPath= \"" + m_OutputPath + "/DTIAtlas\"\n";

	Script = Script + "ErrorList=[] #empty list\n\n";

	Script = Script + "time1=time.time()\n\n";

/* Call the other scripts */
	Script = Script + "# Call the Preprocess script\n";
	Script = Script + "PrePScriptCommand= OutputPath + \"/Script/DTIAtlasBuilder_Preprocess.script\"\n";
	Script = Script + "print(\"\\n=> $ \" + PrePScriptCommand)\n";
	Script = Script + "if os.system(PrePScriptCommand)!=0 : ErrorList.append(\'=> Errors detected in preprocessing\')\n\n";

	Script = Script + "# Call the Atlas Building script\n";
	Script = Script + "AtlasBuildingCommand= OutputPath + \"/Script/DTIAtlasBuilder_AtlasBuilding.script\"\n";
	Script = Script + "print(\"\\n=> $ \" + AtlasBuildingCommand)\n";
	Script = Script + "if os.system(AtlasBuildingCommand)!=0 : ErrorList.append(\'=> Errors detected in atlas building\')\n\n";

	Script = Script + "print(\"\\n============ End of execution =============\\n\")\n\n";

	Script = Script + "# Display errors\n";
	Script = Script + "for error in ErrorList : print(error + \'\\n\')\n";
	Script = Script + "if len(ErrorList)==0 : print(\"=> No errors detected during execution\\n\")\n\n";

	Script = Script + "# Display execution time\n";
	Script = Script + "time2=time.time()\n";
	Script = Script + "timeTot=time2-time1\n";
	Script = Script + "if timeTot<60 : print(\"| Execution time = \" + str(int(timeTot)) + \"s\")\n";
	Script = Script + "elif timeTot<3600 : print(\"| Execution time = \" + str(int(timeTot)) + \"s = \" + str(int(timeTot/60)) + \"m \" + str( int(timeTot) - (int(timeTot/60)*60) ) + \"s\")\n";
	Script = Script + "else : print(\"| Execution time = \" + str(int(timeTot)) + \"s = \" + str(int(timeTot/3600)) + \"h \" + str( int( (int(timeTot) - int(timeTot/3600)*3600) /60) ) + \"m \" + str( int(timeTot) - (int(timeTot/60)*60) ) + \"s\")\n\n";

	Script = Script + "if len(ErrorList)==0 : sys.exit(0)\n";
	Script = Script + "else : sys.exit(1)\n\n";

	m_Script_Main=Script;
}

  /////////////////////////////////////////
 //         SCRIPT ACCESSORS            //
/////////////////////////////////////////

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

  /////////////////////////////////////////
 //           CHECK DATASET             //
/////////////////////////////////////////

int ScriptWriter::setCroppingSize( bool SafetyMargin ) // returns 0 if no cropping , 1 if cropping needed
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

	if(SafetyMargin) // bool SafetyMargin (CropDTI will add 2 voxels each size : place the image in the center)
	{
		m_NeedToBeCropped=1; 
		MaxSize[0] = MaxSize[0] + 4; // add 2 voxels each side in each direction to the computed cropping size
		MaxSize[1] = MaxSize[1] + 4; // add 2 voxels each side in each direction to the computed cropping size
		MaxSize[2] = MaxSize[2] + 4; // add 2 voxels each side in each direction to the computed cropping size
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

  /////////////////////////////////////////
 //           SET THE VALUES            //
/////////////////////////////////////////

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


void ScriptWriter::setInterpolType(std::string Type)
{
	m_InterpolType = Type;
}

void ScriptWriter::setInterpolOption(std::string Option)
{
	m_InterpolOption = Option;
}

void ScriptWriter::setTensInterpol(std::string TensInterpol)
{
	m_TensInterpol = TensInterpol;
}

void ScriptWriter::setInterpolLogOption(std::string InterpolLogOption)
{
	m_InterpolLogOption = InterpolLogOption;
}

void ScriptWriter::setTensorTfm(std::string TensTfm)
{
	m_TensTfm = TensTfm;
}

void ScriptWriter::setSoftPath(std::vector < std::string > SoftPath) // 1=ImageMath, 2=ResampleDTIlogEuclidean, 3=CropDTI, 4=dtiprocess, 5=BRAINSFit, 6=GreedyAtlas, 7=dtiaverage, 8=DTI-Reg, 9=unu
{
	m_SoftPath.clear();
	for (unsigned int i=0;i<SoftPath.size();i++) m_SoftPath.push_back( SoftPath[i] );
}

void ScriptWriter::setDTIRegOptions(std::vector < std::string > DTIRegOptions)
{
	m_DTIRegOptions.clear();
	for (unsigned int i=0;i<DTIRegOptions.size();i++) m_DTIRegOptions.push_back( DTIRegOptions[i] );
/*
	RegMethod
	ANTS
		ARegType
		TfmStep
		Iter
		SimMet
		SimParam
		GSigma
		SmoothOff

	BRAINS
		BRegType
		TfmMode
		Sigma
		NbPyrLev
		PyrLevIt
*/
}

void ScriptWriter::setDTIRegExtraPath(std::string DTIRegExtraPath)
{
	m_DTIRegExtraPath=DTIRegExtraPath;
}

void ScriptWriter::setBFAffineTfmMode(std::string BFAffineTfmMode)
{
	m_BFAffineTfmMode=BFAffineTfmMode;
}

void ScriptWriter::setGridProcess(bool useGridProcess)
{
	m_useGridProcess = useGridProcess;
}

void ScriptWriter::setGridCommand(std::string GridCommand)
{
	m_GridCommand = GridCommand;
}

