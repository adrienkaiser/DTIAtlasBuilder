#ifndef DEF_SCRIPTWRITER
#define DEF_SCRIPTWRITER

#include "itkImage.h"
#include "itkImageFileReader.h"

#include <iostream>
#include <string>
#include <vector>
#include <math.h>// for the absolute value

class ScriptWriter
{
	public:
		void WriteScript();
		void PreprocessCase();
		void PreprocessTemplate();
		void AtlasBuilding();
		void MainScript();

		int setCroppingSize(); // returns 0 if no cropping , 1 if cropping needed
		int CheckVoxelSize(); // returns 0 if voxel size OK , otherwise 1
		int CheckSpaceDir(); // returns 0 if space dir OK , otherwise 1
		std::string getScript_Preprocess();
		std::string getScript_AtlasBuilding();
		std::string getScript_Main();
		void setCasesPath(std::vector < std::string > CasesPath);
		void clearCasesPath();
		void setOutputPath(std::string OutputPath);
		void setRegType(int RegType);
		void setnbLoops(int nbLoops);
		void setTemplatePath(std::string FAAtlasrefPath);
		void setOverwrite(int Overwrite);
		void setScaleLevels(std::vector< std::vector< double > > AtlasWerksScaleLevels);
		void setInterpolType(std::string Type);
		void setAverageStatMethod(std::string Method);

	private:
		std::vector < std::string > m_CasesPath;
		std::string m_OutputPath;
		int m_RegType; //=0 for using template and =1 for using existing first case
		int m_nbLoops;
		std::string m_nbLoops_str;
		std::string m_TemplatePath;
		int m_CropSize [3]; //x,y,z
		int m_NeedToBeCropped; //=0 if not and =1 if need to be cropped
		int m_NeedToBeResampled; //=0 if not and =1 if need to be resampled
		int m_Overwrite; //=0 if not and =1 if overwrite on existing files
		std::vector< std::vector<double> > m_AtlasWerksScaleLevels; // contains X 2-int tables : [Scale,NbIter]
		std::string m_InterpolType;
		std::string m_AverageStatMethod;
	
		std::string m_Script_Preprocess; //contains the whole Script for Pre processing
		std::string m_Script_AtlasBuilding; //contains the whole Script for Atlas Building
		std::string m_Script_Main;//contains the whole Main Scrit
};
#endif

