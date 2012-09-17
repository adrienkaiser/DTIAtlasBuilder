/*Qt classes*/
#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>
#include <QFile>
#include <QTextStream>
#include <QCloseEvent>
#include <QSignalMapper>
#include <QDialog>
#include <QLabel>
#include <QComboBox>
#include <QStackedWidget>
#include <QLineEdit>

#include <QDebug>

/*std classes*/
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <cstdlib> // for getenv()
#include <signal.h> // for kill()
//#include <sys/wait.h> // for waitpid(): see function "void GUI::LaunchScriptRunner()" at the end

/*itk classes*/
#include "itkImage.h"
#include "itkImageFileReader.h"
#include <itksys/SystemTools.hxx> // for FindProgram() and GetFilenamePath()

#include "GUI.h"
#include "ScriptWriter.h"

  /////////////////////////////////////////
 //            CONSTRUCTOR              //
/////////////////////////////////////////
	
GUI::GUI(std::string ParamFile, std::string ConfigFile, std::string CSVFile, bool overwrite, bool noGUI, bool Quiet) : QMainWindow()
{
/*	if(!m_Quiet) std::cout<<"Command Line parameter file :"<<ParamFile<<std::endl;
	if(!m_Quiet) std::cout<<"Command Line configuration file :"<<ConfigFile<<std::endl;
	if(!m_Quiet) std::cout<<"Command Line dataset file :"<<CSVFile<<std::endl;
*/
	setupUi(this);

	m_ErrorDetectedInConstructor=false;

/* Script writing object */
	m_scriptwriter = new ScriptWriter; // delete in "void GUI::Exit()"

/* Variables */
	m_ParamFileHeader = QString("DTIAtlasBuilderParameterFileVersion");
	m_CSVseparator = QString(",");
	m_ParamSaved=1;
	m_lastCasePath="";
	m_noGUI=noGUI;
	m_Quiet=Quiet;
	if( overwrite ) OverwritecheckBox->setChecked(true);

/* Initialize the options */
	InitOptions();

	if(!m_noGUI)
	{
/* Objects connections */
	QObject::connect(ComputepushButton, SIGNAL(clicked()), this, SLOT(Compute()));
	QObject::connect(BrowseCSVPushButton, SIGNAL(clicked()), this, SLOT(ReadCSVSlot()));
	QObject::connect(SaveCSVPushButton, SIGNAL(clicked()), this, SLOT(SaveCSVDatasetBrowse()));
	QObject::connect(BrowseOutputPushButton, SIGNAL(clicked()), this, SLOT(OpenOutputBrowseWindow()));
	QObject::connect(TemplateBrowsePushButton, SIGNAL(clicked()), this, SLOT(OpenTemplateBrowseWindow()));
	QObject::connect(AddPushButton, SIGNAL(clicked()), this, SLOT(OpenAddCaseBrowseWindow()));
	QObject::connect(RemovePushButton, SIGNAL(clicked()), this, SLOT(RemoveSelectedCases()));
	RemovePushButton->setEnabled(false);
	ComputepushButton->setEnabled(false);

	QObject::connect(actionLoad_parameters, SIGNAL(triggered()), this, SLOT(LoadParametersSlot()));
	QObject::connect(actionSave_parameters, SIGNAL(triggered()), this, SLOT(SaveParametersSlot()));
	QObject::connect(actionExit, SIGNAL(triggered()), this, SLOT(Exit()));
	QObject::connect(actionLoad_Software_Configuration, SIGNAL(triggered()), this, SLOT(LoadConfigSlot()));
	QObject::connect(actionSave_Software_Configuration, SIGNAL(triggered()), this, SLOT(SaveConfig()));
	QObject::connect(actionRead_Me, SIGNAL(triggered()), this, SLOT(ReadMe()));

	QObject::connect(InterpolTypeComboBox, SIGNAL(currentIndexChanged (int)), this, SLOT(InterpolTypeComboBoxChanged(int)));
	QObject::connect(TensInterpolComboBox, SIGNAL(currentIndexChanged (int)), this, SLOT(TensorInterpolComboBoxChanged(int)));
	QObject::connect(RegMethodcomboBox, SIGNAL(currentIndexChanged (int)), this, SLOT(RegMethodComboBoxChanged(int)));

	QObject::connect(DefaultButton, SIGNAL(clicked()), this, SLOT(ConfigDefault()));
	QObject::connect(AWPath, SIGNAL(editingFinished()), this, SLOT(testAW())); // test the version of AtlasWerks automatically when the text is changed manually ( not by a setText() )
	QObject::connect(DTIRegPath, SIGNAL(editingFinished()), this, SLOT(testDTIReg())); // test the version of DTI-Reg automatically when the text is changed manually ( not by a setText() )

	QObject::connect(AffineQCButton, SIGNAL(clicked()), this, SLOT(DisplayAffineQC()));
	QObject::connect(DeformQCButton, SIGNAL(clicked()), this, SLOT(DisplayDeformQC()));
	QObject::connect(ResampQCButton, SIGNAL(clicked()), this, SLOT(DisplayResampQC()));

/* Browse software path Buttons */
	QSignalMapper *SoftButtonMapper = new QSignalMapper();
	QObject::connect(SoftButtonMapper, SIGNAL(mapped(int)), this, SLOT( BrowseSoft(int) ));

	QObject::connect(ImagemathButton, SIGNAL(clicked()), SoftButtonMapper, SLOT(map()));
	SoftButtonMapper->setMapping(ImagemathButton,1);
	QObject::connect(ResampButton, SIGNAL(clicked()), SoftButtonMapper, SLOT(map()));
	SoftButtonMapper->setMapping(ResampButton,2);
	QObject::connect(CropDTIButton, SIGNAL(clicked()), SoftButtonMapper, SLOT(map()));
	SoftButtonMapper->setMapping(CropDTIButton,3);
	QObject::connect(dtiprocButton, SIGNAL(clicked()), SoftButtonMapper, SLOT(map()));
	SoftButtonMapper->setMapping(dtiprocButton,4);
	QObject::connect(BRAINSFitButton, SIGNAL(clicked()), SoftButtonMapper, SLOT(map()));
	SoftButtonMapper->setMapping(BRAINSFitButton,5);
	QObject::connect(AWButton, SIGNAL(clicked()), SoftButtonMapper, SLOT(map()));
	SoftButtonMapper->setMapping(AWButton,6);
	QObject::connect(dtiavgButton, SIGNAL(clicked()), SoftButtonMapper, SLOT(map()));
	SoftButtonMapper->setMapping(dtiavgButton,7);
	QObject::connect(DTIRegButton, SIGNAL(clicked()), SoftButtonMapper, SLOT(map()));
	SoftButtonMapper->setMapping(DTIRegButton,8);
	QObject::connect(unuButton, SIGNAL(clicked()), SoftButtonMapper, SLOT(map()));
	SoftButtonMapper->setMapping(unuButton,9);
	QObject::connect(MriWatcherButton, SIGNAL(clicked()), SoftButtonMapper, SLOT(map()));
	SoftButtonMapper->setMapping(MriWatcherButton,10);

/* Reset software path Buttons */
	QSignalMapper *ResetSoftButtonMapper = new QSignalMapper();
	QObject::connect(ResetSoftButtonMapper, SIGNAL(mapped(int)), this, SLOT( ResetSoft(int) ));

	QObject::connect(ImagemathResetButton, SIGNAL(clicked()), ResetSoftButtonMapper, SLOT(map()));
	ResetSoftButtonMapper->setMapping(ImagemathResetButton,1);
	QObject::connect(ResampResetButton, SIGNAL(clicked()), ResetSoftButtonMapper, SLOT(map()));
	ResetSoftButtonMapper->setMapping(ResampResetButton,2);
	QObject::connect(CropDTIResetButton, SIGNAL(clicked()), ResetSoftButtonMapper, SLOT(map()));
	ResetSoftButtonMapper->setMapping(CropDTIResetButton,3);
	QObject::connect(dtiprocResetButton, SIGNAL(clicked()), ResetSoftButtonMapper, SLOT(map()));
	ResetSoftButtonMapper->setMapping(dtiprocResetButton,4);
	QObject::connect(BRAINSFitResetButton, SIGNAL(clicked()), ResetSoftButtonMapper, SLOT(map()));
	ResetSoftButtonMapper->setMapping(BRAINSFitResetButton,5);
	QObject::connect(AWResetButton, SIGNAL(clicked()), ResetSoftButtonMapper, SLOT(map()));
	ResetSoftButtonMapper->setMapping(AWResetButton,6);
	QObject::connect(dtiavgResetButton, SIGNAL(clicked()), ResetSoftButtonMapper, SLOT(map()));
	ResetSoftButtonMapper->setMapping(dtiavgResetButton,7);
	QObject::connect(DTIRegResetButton, SIGNAL(clicked()), ResetSoftButtonMapper, SLOT(map()));
	ResetSoftButtonMapper->setMapping(DTIRegResetButton,8);
	QObject::connect(unuResetButton, SIGNAL(clicked()), ResetSoftButtonMapper, SLOT(map()));
	ResetSoftButtonMapper->setMapping(unuResetButton,9);
	QObject::connect(MriWatcherResetButton, SIGNAL(clicked()), ResetSoftButtonMapper, SLOT(map()));
	ResetSoftButtonMapper->setMapping(MriWatcherResetButton,10);

/* When any value changes, the value of m_ParamSaved is set to 0 */
	QObject::connect(TemplateLineEdit, SIGNAL(textChanged(QString)), this, SLOT(WidgetHasChangedParamNoSaved()));
	QObject::connect(OutputFolderLineEdit, SIGNAL(textChanged(QString)), this, SLOT(WidgetHasChangedParamNoSaved()));
	QObject::connect(SafetyMargincheckBox, SIGNAL(stateChanged(int)), this, SLOT(WidgetHasChangedParamNoSaved()));
	QObject::connect(BFAffineTfmModecomboBox, SIGNAL(currentIndexChanged (int)), this, SLOT(WidgetHasChangedParamNoSaved()));
	QObject::connect(OverwritecheckBox, SIGNAL(stateChanged(int)), this, SLOT(WidgetHasChangedParamNoSaved()));
	QObject::connect(SL4checkBox, SIGNAL(stateChanged(int)), this, SLOT(WidgetHasChangedParamNoSaved()));
	QObject::connect(SL2checkBox, SIGNAL(stateChanged(int)), this, SLOT(WidgetHasChangedParamNoSaved()));
	QObject::connect(SL1checkBox, SIGNAL(stateChanged(int)), this, SLOT(WidgetHasChangedParamNoSaved()));
	QObject::connect(NbLoopsSpinBox, SIGNAL(valueChanged(int)), this, SLOT(WidgetHasChangedParamNoSaved()));
	QObject::connect(SL4spinBox, SIGNAL(valueChanged(int)), this, SLOT(WidgetHasChangedParamNoSaved()));
	QObject::connect(nbIter4SpinBox, SIGNAL(valueChanged(int)), this, SLOT(WidgetHasChangedParamNoSaved()));
	QObject::connect(alpha4DoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(WidgetHasChangedParamNoSaved()));
	QObject::connect(beta4DoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(WidgetHasChangedParamNoSaved()));
	QObject::connect(gamma4DoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(WidgetHasChangedParamNoSaved()));
	QObject::connect(maxPerturbation4DoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(WidgetHasChangedParamNoSaved()));
	QObject::connect(SL2spinBox, SIGNAL(valueChanged(int)), this, SLOT(WidgetHasChangedParamNoSaved()));
	QObject::connect(nbIter2SpinBox, SIGNAL(valueChanged(int)), this, SLOT(WidgetHasChangedParamNoSaved()));
	QObject::connect(alpha2DoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(WidgetHasChangedParamNoSaved()));
	QObject::connect(beta2DoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(WidgetHasChangedParamNoSaved()));
	QObject::connect(gamma2DoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(WidgetHasChangedParamNoSaved()));
	QObject::connect(maxPerturbation2DoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(WidgetHasChangedParamNoSaved()));
	QObject::connect(SL1spinBox, SIGNAL(valueChanged(int)), this, SLOT(WidgetHasChangedParamNoSaved()));
	QObject::connect(nbIter1SpinBox, SIGNAL(valueChanged(int)), this, SLOT(WidgetHasChangedParamNoSaved()));
	QObject::connect(alpha1DoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(WidgetHasChangedParamNoSaved()));
	QObject::connect(beta1DoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(WidgetHasChangedParamNoSaved()));
	QObject::connect(gamma1DoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(WidgetHasChangedParamNoSaved()));
	QObject::connect(maxPerturbation1DoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(WidgetHasChangedParamNoSaved()));
	QObject::connect(TensTfmComboBox, SIGNAL(currentIndexChanged (int)), this, SLOT(WidgetHasChangedParamNoSaved()));
	QObject::connect(averageStatMethodComboBox, SIGNAL(currentIndexChanged (int)), this, SLOT(WidgetHasChangedParamNoSaved()));
	QObject::connect(InterpolTypeComboBox, SIGNAL(currentIndexChanged (int)), this, SLOT(WidgetHasChangedParamNoSaved()));
	QObject::connect(TensInterpolComboBox, SIGNAL(currentIndexChanged (int)), this, SLOT(WidgetHasChangedParamNoSaved()));
	QObject::connect(RegMethodcomboBox, SIGNAL(currentIndexChanged (int)), this, SLOT(WidgetHasChangedParamNoSaved()));
	QObject::connect(m_windowComboBox, SIGNAL(currentIndexChanged (int)), this, SLOT(WidgetHasChangedParamNoSaved()));
	QObject::connect(m_BSplineComboBox, SIGNAL(currentIndexChanged (int)), this, SLOT(WidgetHasChangedParamNoSaved()));
	QObject::connect(m_nologComboBox, SIGNAL(currentIndexChanged (int)), this, SLOT(WidgetHasChangedParamNoSaved()));
	QObject::connect(m_BRegTypeComboBox, SIGNAL(currentIndexChanged (int)), this, SLOT(WidgetHasChangedParamNoSaved()));
	QObject::connect(m_TfmModeComboBox, SIGNAL(currentIndexChanged (int)), this, SLOT(WidgetHasChangedParamNoSaved()));
	QObject::connect(m_SigmaDble, SIGNAL(valueChanged(double)), this, SLOT(WidgetHasChangedParamNoSaved()));
	QObject::connect(m_NbPyrLevSpin, SIGNAL(valueChanged(int)), this, SLOT(WidgetHasChangedParamNoSaved()));
	QObject::connect(m_PyrLevItLine, SIGNAL(textChanged(QString)), this, SLOT(WidgetHasChangedParamNoSaved()));
	QObject::connect(m_ARegTypeComboBox, SIGNAL(currentIndexChanged (int)), this, SLOT(WidgetHasChangedParamNoSaved()));
	QObject::connect(m_TfmStepLine, SIGNAL(textChanged(QString)), this, SLOT(WidgetHasChangedParamNoSaved()));
	QObject::connect(m_IterLine, SIGNAL(textChanged(QString)), this, SLOT(WidgetHasChangedParamNoSaved()));
	QObject::connect(m_SimMetComboBox, SIGNAL(currentIndexChanged (int)), this, SLOT(WidgetHasChangedParamNoSaved()));
	QObject::connect(m_SimParamDble, SIGNAL(valueChanged(double)), this, SLOT(WidgetHasChangedParamNoSaved()));
	QObject::connect(m_GSigmaDble, SIGNAL(valueChanged(double)), this, SLOT(WidgetHasChangedParamNoSaved()));
	QObject::connect(m_SmoothOffCheck, SIGNAL(stateChanged(int)), this, SLOT(WidgetHasChangedParamNoSaved()));

	} //if(!m_noGUI)

	m_FromConstructor=1; // do not test AW path if 'Default' called from constructor -> test at the end of constructor

/* SET the soft config */
// look for the programs with the itk function
	ConfigDefault();

// Look for the config file in the executable directory
	std::string executable = itksys::SystemTools::FindProgram("DTIAtlasBuilder");
	std::string path = itksys::SystemTools::GetFilenamePath(executable); // get the path WITHOUT '/'
	std::string SoftConfigPath= path + "/DTIAtlasBuilderSoftConfig.txt";
	if( access( SoftConfigPath.c_str() , F_OK) == 0 ) if( LoadConfig(QString( SoftConfigPath.c_str() )) == -1 ) m_ErrorDetectedInConstructor=true;

// Look for the config file in the current directory
	std::string CurrentPath = itksys::SystemTools::GetRealPath( itksys::SystemTools::GetCurrentWorkingDirectory().c_str() ); //GetRealPath() to remove symlinks
	SoftConfigPath = CurrentPath + "/DTIAtlasBuilderSoftConfig.txt";
	if( access( SoftConfigPath.c_str() , F_OK) == 0 ) if( LoadConfig(QString( SoftConfigPath.c_str() )) == -1 ) m_ErrorDetectedInConstructor=true;

// Look for the config file in the env variable
	const char * value = getenv("DTIAtlasBuilderSoftPath");
	if (value!=NULL) 
	{
		if(!m_Quiet) printf ("| Environment variable read. The config file is \'%s\'\n",value);
		if( LoadConfig(QString(value)) == -1 ) m_ErrorDetectedInConstructor=true; // replace the paths by the paths given in the config file
	}
	else if(!m_Quiet) std::cout<<"| No environment variable found"<<std::endl;

/* Look for the parameter file in the current directory */
	std::string ParamPath = CurrentPath + "/DTIAtlasBuilderParameters.txt";
	if( access( ParamPath.c_str() , F_OK) == 0 ) if( LoadParameters(QString( ParamPath.c_str() )) == -1 ) m_ErrorDetectedInConstructor=true;

/* Load Parameters from Command Line => cmd line arguments a taking into account at last and change the parameters at last because they have priority */
	if( !ParamFile.empty() )
	{
		if( LoadParameters(QString(ParamFile.c_str())) == -1 ) m_ErrorDetectedInConstructor=true;
	}
	else if(m_noGUI) // no parameters and nogui => not possible
	{
		if(!m_Quiet) std::cout<<"| Please give a parameter file"<<std::endl; // command line display
		m_ErrorDetectedInConstructor=true;
	}

	if( !CSVFile.empty() ) if( ReadCSV( QString(CSVFile.c_str())) == -1 ) m_ErrorDetectedInConstructor=true;
	if( !ConfigFile.empty() ) if( LoadConfig( QString(ConfigFile.c_str())) == -1 ) m_ErrorDetectedInConstructor=true;

	m_FromConstructor=0;

	//NOW that all the files have been loaded => test if all the paths are here
	bool AWFound=true;
	bool DTIRegFound=true;
	std::string notFound;

	if(ImagemathPath->text().isEmpty()) notFound = notFound + "> ImageMath\n";
	if(ResampPath->text().isEmpty()) notFound = notFound + "> ResampleDTIlogEuclidean\n";
	if(CropDTIPath->text().isEmpty()) notFound = notFound + "> CropDTI\n";
	if(dtiprocPath->text().isEmpty()) notFound = notFound + "> dtiprocess\n";
	if(BRAINSFitPath->text().isEmpty()) notFound = notFound + "> BRAINSFit\n";
	if(AWPath->text().isEmpty())
	{
		notFound = notFound + "> AtlasWerks\n";
		AWFound=false; // so it will not test the version
	}
	if(dtiavgPath->text().isEmpty()) notFound = notFound + "> dtiaverage\n";
	if(DTIRegPath->text().isEmpty())
	{
		notFound = notFound + "> DTI-Reg\n";
		DTIRegFound=false; // so it will not test the version
	}
	if(unuPath->text().isEmpty()) notFound = notFound + "> unu\n";
	if(MriWatcherPath->text().isEmpty()) notFound = notFound + "> MriWatcher\n";

	if( !notFound.empty() )
	{
		if(!m_noGUI) 
		{
			std::string text = "The following programs have not been found.\nPlease enter the path manually or open a configuration file:\n" + notFound;
			QMessageBox::warning(this, "Program missing", QString(text.c_str()) );
		}
		else if(!m_Quiet) std::cout<<"| The following programs have not been found. Please give a configuration file or modify it or enter the path manually in the GUI:\n"<< notFound <<std::endl;
	}

	if(AWFound) testAW(); // test the version of AtlasWerks only if found
	if(DTIRegFound) testDTIReg(); // test the version of DTI-Reg only if found
}

  /////////////////////////////////////////
 //           INITIALIZATIONS           //
/////////////////////////////////////////

void GUI::InitOptions()
{
/* Init options for Final AtlasBuilding param : Interpolation algo */
	m_optionStackLayout = new QStackedWidget;
	optionHLayout->addWidget(m_optionStackLayout);

	QWidget *emptyWidget= new QWidget;
	m_optionStackLayout->addWidget(emptyWidget);

	QLabel *windowLabel = new QLabel("Type:", this);
	m_windowComboBox = new QComboBox(this);
	m_windowComboBox->addItem("Hamming");
	m_windowComboBox->addItem("Cosine");
	m_windowComboBox->addItem("Welch");
	m_windowComboBox->addItem("Lanczos");
	m_windowComboBox->addItem("Blackman");
	m_windowComboBox->setCurrentIndex(1);
	QHBoxLayout *windowLayout= new QHBoxLayout;
	windowLayout->addWidget(windowLabel);
	windowLayout->addWidget(m_windowComboBox);
	QWidget *windowWidget= new QWidget;
	windowWidget->setLayout(windowLayout);
	m_optionStackLayout->addWidget(windowWidget);

	QLabel *BSplineLabel = new QLabel("Order:", this);
	m_BSplineComboBox = new QComboBox(this);
	m_BSplineComboBox->addItem("0");
	m_BSplineComboBox->addItem("1");
	m_BSplineComboBox->addItem("2");
	m_BSplineComboBox->addItem("3");
	m_BSplineComboBox->addItem("4");
	m_BSplineComboBox->addItem("5");
	m_BSplineComboBox->setCurrentIndex(3);
	QHBoxLayout *BSplineLayout= new QHBoxLayout;
	BSplineLayout->addWidget(BSplineLabel);
	BSplineLayout->addWidget(m_BSplineComboBox);
	QWidget *BSplineWidget= new QWidget;
	BSplineWidget->setLayout(BSplineLayout);
	m_optionStackLayout->addWidget(BSplineWidget);

	m_optionStackLayout->setCurrentIndex(0);

/* Init options for Final AtlasBuilding param : Tensor Interpolation */
	m_logOptionStackLayout = new QStackedWidget;
	logOptionHLayout->addWidget(m_logOptionStackLayout);

	QWidget *logEmptyWidget= new QWidget;
	m_logOptionStackLayout->addWidget(logEmptyWidget);

	QLabel *nologLabel = new QLabel("Correction:", this);
	m_nologComboBox = new QComboBox(this);
	m_nologComboBox->addItem("Zero");
	m_nologComboBox->addItem("None");
	m_nologComboBox->addItem("Absolute Value");
	m_nologComboBox->addItem("Nearest");
	m_nologComboBox->setCurrentIndex(1);
	QHBoxLayout *nologLayout= new QHBoxLayout;
	nologLayout->addWidget(nologLabel);
	nologLayout->addWidget(m_nologComboBox);
	QWidget *nologWidget= new QWidget;
	nologWidget->setLayout(nologLayout);
	m_logOptionStackLayout->addWidget(nologWidget);

	m_logOptionStackLayout->setCurrentIndex(0);

/* Init options for DTI-Reg: Reg Method */
	m_DTIRegOptionStackLayout = new QStackedWidget;
	DTIRegOptionsVLayout->addWidget(m_DTIRegOptionStackLayout);

	/*BRAINS*/
	QHBoxLayout *BRAINSHLayout = new QHBoxLayout;
	QVBoxLayout *BRAINSLabelVLayout = new QVBoxLayout;
	BRAINSHLayout->addLayout(BRAINSLabelVLayout);
	QVBoxLayout *BRAINSWidgetVLayout = new QVBoxLayout;
	BRAINSHLayout->addLayout(BRAINSWidgetVLayout);

	QLabel *BRegTypeLabel = new QLabel("Registration Type:", this);
	BRAINSLabelVLayout->addWidget(BRegTypeLabel);
	m_BRegTypeComboBox = new QComboBox(this);
	m_BRegTypeComboBox->addItem("None");
	m_BRegTypeComboBox->addItem("Rigid");
	m_BRegTypeComboBox->addItem("BSpline");
	m_BRegTypeComboBox->addItem("Diffeomorphic");
	m_BRegTypeComboBox->addItem("Demons");
	m_BRegTypeComboBox->addItem("LogDemons");
	m_BRegTypeComboBox->addItem("SymmetricLogDemons");
	m_BRegTypeComboBox->setCurrentIndex(5);
	BRAINSWidgetVLayout->addWidget(m_BRegTypeComboBox);

	QLabel *TfmModeLabel = new QLabel("Transform Mode:", this);
	BRAINSLabelVLayout->addWidget(TfmModeLabel);
	m_TfmModeComboBox = new QComboBox(this);
	m_TfmModeComboBox->addItem("Off");
	m_TfmModeComboBox->addItem("useMomentsAlign");
	m_TfmModeComboBox->addItem("useCenterOfHeadAlign");
	m_TfmModeComboBox->addItem("useGeometryAlign");
	m_TfmModeComboBox->addItem("Use computed affine transform");
	m_TfmModeComboBox->setCurrentIndex(4);
	BRAINSWidgetVLayout->addWidget(m_TfmModeComboBox);

	QLabel *SigmaLabel = new QLabel("Smooth Deformation Field Sigma:", this);
	BRAINSLabelVLayout->addWidget(SigmaLabel);
	m_SigmaDble = new QDoubleSpinBox(this);
	m_SigmaDble->setValue(1);
	BRAINSWidgetVLayout->addWidget(m_SigmaDble);

	QLabel *NbPyrLevLabel = new QLabel("Number Of Pyramid Levels:", this);
	BRAINSLabelVLayout->addWidget(NbPyrLevLabel);
	m_NbPyrLevSpin = new QSpinBox(this);
	m_NbPyrLevSpin->setValue(5);
	BRAINSWidgetVLayout->addWidget(m_NbPyrLevSpin);

	QLabel *PyrLevItLabel = new QLabel("Number Of Iterations for the Pyramid Levels:", this);
	BRAINSLabelVLayout->addWidget(PyrLevItLabel);
	m_PyrLevItLine = new QLineEdit(this);
	m_PyrLevItLine->setText("300,50,30,20,15");
	BRAINSWidgetVLayout->addWidget(m_PyrLevItLine);

	QWidget *BRAINSWidget = new QWidget;
	BRAINSWidget->setLayout(BRAINSHLayout);
	m_DTIRegOptionStackLayout->addWidget(BRAINSWidget);

	/*ANTS*/
	QHBoxLayout *ANTSHLayout = new QHBoxLayout;
	QVBoxLayout *ANTSLabelVLayout = new QVBoxLayout;
	ANTSHLayout->addLayout(ANTSLabelVLayout);
	QVBoxLayout *ANTSWidgetVLayout = new QVBoxLayout;
	ANTSHLayout->addLayout(ANTSWidgetVLayout);

	QLabel *ARegTypeLabel = new QLabel("Registration Type:", this);
	ANTSLabelVLayout->addWidget(ARegTypeLabel);
	m_ARegTypeComboBox = new QComboBox(this);
	m_ARegTypeComboBox->addItem("None");
	m_ARegTypeComboBox->addItem("Rigid");
	m_ARegTypeComboBox->addItem("Elast");
	m_ARegTypeComboBox->addItem("Exp");
	m_ARegTypeComboBox->addItem("GreedyExp");
	m_ARegTypeComboBox->addItem("GreedyDiffeo (SyN)");
	m_ARegTypeComboBox->addItem("SpatioTempDiffeo (SyN)");
	m_ARegTypeComboBox->setCurrentIndex(5);
	QObject::connect(m_ARegTypeComboBox, SIGNAL(currentIndexChanged (int)), this, SLOT(ANTSRegTypeChanged( int )));
	ANTSWidgetVLayout->addWidget(m_ARegTypeComboBox);

	QLabel *TfmStepLabel = new QLabel("Transformation Step:", this);
	ANTSLabelVLayout->addWidget(TfmStepLabel);
	m_TfmStepLine = new QLineEdit(this);
	m_TfmStepLine->setText("0.25");
	ANTSWidgetVLayout->addWidget(m_TfmStepLine);

	QLabel *IterLabel = new QLabel("Iterations:", this);
	ANTSLabelVLayout->addWidget(IterLabel);
	m_IterLine = new QLineEdit(this);
	m_IterLine->setText("100x50x25");
	ANTSWidgetVLayout->addWidget(m_IterLine);

	QLabel *SimMetLabel = new QLabel("Similarity Metric:", this);
	ANTSLabelVLayout->addWidget(SimMetLabel);
	m_SimMetComboBox = new QComboBox(this);
	m_SimMetComboBox->addItem("CC");
	m_SimMetComboBox->addItem("MI");
	m_SimMetComboBox->addItem("MSQ");
	m_SimMetComboBox->setCurrentIndex(0);
	QObject::connect(m_SimMetComboBox, SIGNAL(currentIndexChanged (int)), this, SLOT(SimMetChanged( int )));
	ANTSWidgetVLayout->addWidget(m_SimMetComboBox);

	m_SimParamLabel = new QLabel("Region Radius:", this);
	ANTSLabelVLayout->addWidget(m_SimParamLabel);
	m_SimParamDble = new QDoubleSpinBox(this);
	m_SimParamDble->setValue(2);
	ANTSWidgetVLayout->addWidget(m_SimParamDble);

	QLabel *GSigmaLabel = new QLabel("Gaussian Sigma:", this);
	ANTSLabelVLayout->addWidget(GSigmaLabel);
	m_GSigmaDble = new QDoubleSpinBox(this);
	m_GSigmaDble->setValue(3);
	ANTSWidgetVLayout->addWidget(m_GSigmaDble);

	m_SmoothOffCheck = new QCheckBox("Gaussian Smoothing Off", this);
	m_SmoothOffCheck->setChecked(false);
	m_SmoothOffCheck->setLayoutDirection(Qt::RightToLeft);
	ANTSLabelVLayout->addWidget(m_SmoothOffCheck);
	QLabel *emptyL = new QLabel("", this);
	ANTSWidgetVLayout->addWidget(emptyL);

	QWidget *ANTSWidget = new QWidget;
	ANTSWidget->setLayout(ANTSHLayout);
	m_DTIRegOptionStackLayout->addWidget(ANTSWidget);


	m_DTIRegOptionStackLayout->setCurrentIndex(0); //BRAINS is the default
}

  /////////////////////////////////////////
 //                CASES                //
/////////////////////////////////////////

void GUI::OpenAddCaseBrowseWindow() /*SLOT*/
{
	QStringList CaseListBrowse=QFileDialog::getOpenFileNames(this, "Open Cases", m_lastCasePath, "NERD Images (*.nrrd *.nhdr *.*)");
	CaseListWidget->addItems(CaseListBrowse);
	if(!CaseListBrowse.isEmpty())
	{
		if ( CaseListWidget->count()>0 )
		{
			RemovePushButton->setEnabled(true);
			ComputepushButton->setEnabled(true);
		}
		m_ParamSaved=0;
		SelectCasesLabel->setText( QString("") );
		m_lastCasePath = CaseListBrowse.last();

		CheckCasesIndex();
	}
}

void GUI::RemoveSelectedCases() /*SLOT*/
{
	int NbOfSelectedItems = (CaseListWidget->selectedItems()).size();
	int NB=NbOfSelectedItems;
	int ItemRow;
	while(NbOfSelectedItems>0)
	{
		ItemRow = CaseListWidget->row( CaseListWidget->selectedItems().at(0) );
		CaseListWidget->takeItem(ItemRow);
		delete CaseListWidget->selectedItems().at(0);
		NbOfSelectedItems--;
	}
	if( NB>0 ) // only if some items were removed
	{
		if ( CaseListWidget->count()==0 )		
		{
			RemovePushButton->setEnabled(false);
			ComputepushButton->setEnabled(false);
		}
		m_ParamSaved=0;
		SelectCasesLabel->setText( QString("") );

		CheckCasesIndex();
	}
}

void GUI::CheckCasesIndex() /* Change ids at the begining of the lines */
{
	std::string text;

	for(int i=0; i < CaseListWidget->count() ;i++)
	{
		std::ostringstream outi;
		outi << i+1;
		std::string i_str = outi.str();

		if( CaseListWidget->item(i)->text().contains(": ") ) text = i_str + ": " + CaseListWidget->item(i)->text().toStdString().substr( CaseListWidget->item(i)->text().split(":").at(0).size()+2 ); // from pos to the end
		else text = i_str + ": " + CaseListWidget->item(i)->text().toStdString();

		CaseListWidget->item(i)->setText( QString( text.c_str() ) );
	}
}

  /////////////////////////////////////////
 //               OUTPUT                //
/////////////////////////////////////////

void GUI::OpenOutputBrowseWindow() /*SLOT*/
{
	QString OutputBrowse=QFileDialog::getExistingDirectory(this);
	if(!OutputBrowse.isEmpty())
	{
		OutputFolderLineEdit->setText(OutputBrowse);
	}
	
}

  /////////////////////////////////////////
 //              TEMPLATE               //
/////////////////////////////////////////

void GUI::OpenTemplateBrowseWindow() /*SLOT*/
{
	QString TemplateBrowse=QFileDialog::getOpenFileName(this, "Open Atlas Template", QString(), "NERD Image (*.nrrd *.nhdr *.*)");
	if(!TemplateBrowse.isEmpty())
	{
		TemplateLineEdit->setText(TemplateBrowse);
	}
}

  /////////////////////////////////////////
 //           QUALITY CONTROL           //
/////////////////////////////////////////

void GUI::DisplayAffineQC() /*SLOT*/ 

{
	std::ostringstream out;
	out << NbLoopsSpinBox->value();
	std::string nbLoops_str = out.str();

	std::string program = MriWatcherPath->text().toStdString() + " --viewAll";
	std::string path;
	for(int i=1; i <= CaseListWidget->count() ;i++) 
	{
		std::ostringstream outi;
		outi << i;
		std::string outi_str = outi.str();
		path = OutputFolderLineEdit->text().toStdString() + "/DTIAtlas/1_Affine_Registration/Loop" + nbLoops_str + "/Case" + outi_str + "_Loop" + nbLoops_str + "_FinalFA.nrrd";
		if( access(path.c_str(), F_OK) == 0 ) program = program + " " + path;
	}
	std::ostringstream out1;
	out1 << NbLoopsSpinBox->value()-1;
	std::string nbLoops1_str = out1.str();
	path = OutputFolderLineEdit->text().toStdString() + "/DTIAtlas/1_Affine_Registration/Loop" + nbLoops1_str + "/Loop" + nbLoops1_str + "_FAAverage.nrrd";
	if( access(path.c_str(), F_OK) == 0 ) program = program + " " + path;

	if(!m_Quiet) std::cout<<"| $ " << program << std::endl;

	int pid=fork(); // cloning the process : returns the son's pid in the father and 0 in the son

	if(pid==0) // we are in the son
	{
		system( program.c_str() );

		kill(getpid(),SIGKILL); // the son kills himself
	}
}

void GUI::DisplayDeformQC() /*SLOT*/
{
	std::string program = MriWatcherPath->text().toStdString() + " --viewAll";
	std::string path;
	for(int i=1; i <= CaseListWidget->count() ;i++) 
	{
		std::ostringstream outi;
		outi << i;
		std::string outi_str = outi.str();
		path = OutputFolderLineEdit->text().toStdString() + "/DTIAtlas/3_AW_Atlas/Case" + outi_str + "_AWFA.nrrd";
		if( access(path.c_str(), F_OK) == 0 ) program = program + " " + path;
	}
	path = OutputFolderLineEdit->text().toStdString() + "/DTIAtlas/3_AW_Atlas/AWAtlasFA.nrrd";
	if( access(path.c_str(), F_OK) == 0 ) program = program + " " + path;

	if(!m_Quiet) std::cout<<"| $ " << program << std::endl;

	int pid=fork(); // cloning the process : returns the son's pid in the father and 0 in the son

	if(pid==0) // we are in the son
	{
		system( program.c_str() );

		kill(getpid(),SIGKILL); // the son kills himself
	}
}

void GUI::DisplayResampQC() /*SLOT*/
{
	std::string program = MriWatcherPath->text().toStdString() + " --viewAll";
	std::string path;
	for(int i=1; i <= CaseListWidget->count() ;i++) 
	{
		std::ostringstream outi;
		outi << i;
		std::string outi_str = outi.str();
		path = OutputFolderLineEdit->text().toStdString() + "/DTIAtlas/4_Final_Resampling/Second_Resampling/Case" + outi_str + "_FinalDeformedFA.nrrd";
		if( access(path.c_str(), F_OK) == 0 ) program = program + " " + path;
	}
	path = OutputFolderLineEdit->text().toStdString() + "/DTIAtlas/4_Final_Resampling/FinalAtlasFA.nrrd";
	if( access(path.c_str(), F_OK) == 0 ) program = program + " " + path;

	if(!m_Quiet) std::cout<<"| $ " << program << std::endl;

	int pid=fork(); // cloning the process : returns the son's pid in the father and 0 in the son

	if(pid==0) // we are in the son
	{
		system( program.c_str() );

		kill(getpid(),SIGKILL); // the son kills himself
	}
}

  /////////////////////////////////////////
 //                EXIT                 //
/////////////////////////////////////////

void GUI::Exit() /*SLOT*/
{
	if(!m_Quiet) std::cout<<"| End of the program"<<std::endl; // command line display
	delete m_scriptwriter;
	qApp->quit(); //end of Application: close the main window
}

void GUI::closeEvent(QCloseEvent* event)
{
	while(m_ParamSaved==0)
	{
		int ret = QMessageBox::question(this,"Quit","Last parameters have not been saved.\nDo you want to save the last parameters ?",QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes);
		if (ret == QMessageBox::Yes) SaveParametersSlot();
		else if (ret == QMessageBox::No) break;
		else if (ret == QMessageBox::Cancel) 
		{
			event->ignore();
			return;
		}
	}
	delete m_scriptwriter;
	event->accept();
}

  /////////////////////////////////////////
 //               DATASET               //
/////////////////////////////////////////

void GUI::ReadCSVSlot() /*SLOT*/
{	
	QString CSVBrowse=QFileDialog::getOpenFileName(this, "Open CSV File", QString(), ".csv Files (*.csv)");

	if(!CSVBrowse.isEmpty())
	{
		ReadCSV(CSVBrowse);
	}
}

int GUI::ReadCSV(QString CSVfile)
{	
	if(!CSVfile.isEmpty())
	{

		if( access(CSVfile.toStdString().c_str(), F_OK) == 0 ) // Test if the csv file exists => unistd::access() returns 0 if F(file)_OK
		{

			QFile file(CSVfile);

			if (file.open(QFile::ReadOnly))
			{
				if(!m_Quiet) std::cout<<"| Loading csv file \'"<< CSVfile.toStdString() <<"\'..."; // command line display

				QTextStream stream(&file);
				while(!stream.atEnd()) //read all the lines
				{
					QString line = stream.readLine();
					QStringList list = line.split(m_CSVseparator);
					if( list.at(0) != "id" )  CaseListWidget->addItem( list.at(1) ); //display in the Widget so that some can be removed
				}
				CheckCasesIndex();
	
				SelectCasesLabel->setText( QString("Current CSV file : ") + CSVfile );
				m_ParamSaved=0;
				if(!m_Quiet) std::cout<<"DONE"<<std::endl; // command line display
			} 
			else
			{
				SelectCasesLabel->setText( QString("Could not open CSV File"));
				if(!m_Quiet) qDebug( "Could not open csv file");
				return -1;
			}

			if ( CaseListWidget->count()>0 )
			{
				RemovePushButton->setEnabled(true);
				ComputepushButton->setEnabled(true);
			}
		}
		else if(!m_Quiet) std::cout<<"| The given CSV file does not exist"<<std::endl; // command line display
	}

	return 0;
}

void GUI::SaveCSVDatasetBrowse() /*SLOT*/
{	
	if(CaseListWidget->count()==0)
	{
		QMessageBox::critical(this, "No Dataset", "No Dataset");
		return;
	}

	QString CSVBrowseName = QFileDialog::getSaveFileName(this, tr("Save Dataset"),"./DTIAtlasBuilderDataSet.csv",tr("CSV File (*.csv)"));

	if(!CSVBrowseName.isEmpty())
	{

	QFile file(CSVBrowseName);

	if ( file.open( IO_WriteOnly | IO_Translate ) )
	{
		if(!m_Quiet) std::cout<<"| Generating Dataset csv file..."; // command line display

		QTextStream stream( &file );
		stream << QString("id") << m_CSVseparator << QString("Original DTI Image") << endl;
		for(int i=0; i < CaseListWidget->count() ;i++) stream << i+1 << m_CSVseparator << CaseListWidget->item(i)->text().remove(0, CaseListWidget->item(i)->text().split(":").at(0).size()+2 ) << endl;
		if(!m_Quiet) std::cout<<"DONE"<<std::endl; // command line display
	
		SelectCasesLabel->setText( QString("Current CSV file : ") + CSVBrowseName );
		QMessageBox::information(this, "Saving succesful", "Dataset has been succesfully saved at" + CSVBrowseName);		
	}
	else qDebug( "Could not create file");

	}
}

void GUI::SaveCSVResults(int Crop, int nbLoops) // Crop = 0 if no cropping , 1 if cropping needed
{	

	QString csvPath;
	csvPath = m_OutputPath + QString("/DTIAtlas/DTIAtlasBuilderResults.csv");
	QFile file(csvPath);

	if ( file.open( IO_WriteOnly | IO_Translate ) )
	{
		if(!m_Quiet) std::cout<<"| Generating Results csv file..."; // command line display

		QTextStream stream( &file );

		stream << QString("id") << m_CSVseparator << QString("Original DTI Image");
		if(Crop==1) stream << m_CSVseparator << QString("Cropped DTI");
		stream << m_CSVseparator << QString("FA from original") << m_CSVseparator << QString("Affine transform") << m_CSVseparator << QString("Affine registered DTI") << m_CSVseparator << QString("Affine Registered FA") << m_CSVseparator << QString("AW Deformed FA") << m_CSVseparator << QString("AW Deformation field to Affine space") << m_CSVseparator << QString("AW Inverse Deformation field to Affine space") << m_CSVseparator << QString("AW DTI") << m_CSVseparator << QString("AW Deformation field to Original space") << m_CSVseparator << QString("DTI-Reg Final DTI") << endl;

		for(int i=0; i < CaseListWidget->count() ;i++) // for all cases
		{
			stream << i+1 << m_CSVseparator << CaseListWidget->item(i)->text().remove(0, CaseListWidget->item(i)->text().split(":").at(0).size()+2 ); // Original DTI Image
			if(Crop==1) stream << m_CSVseparator << m_OutputPath + QString("/DTIAtlas/1_Affine_Registration/Case") << i+1 << QString("_croppedDTI.nrrd"); // Cropped DTI
			stream << m_CSVseparator << m_OutputPath + QString("/DTIAtlas/1_Affine_Registration/Case") << i+1 << QString("_FA.nrrd"); // FA from original
			stream << m_CSVseparator << m_OutputPath + QString("/DTIAtlas/1_Affine_Registration/Loop") << nbLoops << QString("/Case") << i+1 << QString("_Loop ") << nbLoops << QString("_LinearTrans.txt"); // Affine transform
			stream << m_CSVseparator << m_OutputPath + QString("/DTIAtlas/1_Affine_Registration/Loop") << nbLoops << QString("/Case") << i+1 << QString("_Loop ") << nbLoops << QString("_LinearTrans_DTI.nrrd"); // Affine registered DTI
			stream << m_CSVseparator << m_OutputPath + QString("/DTIAtlas/1_Affine_Registration/Loop") << nbLoops << QString("/Case") << i+1 << QString("_Loop ") << nbLoops << QString("_FinalFA.nrrd"); // Affine Registered FA
			stream << m_CSVseparator << m_OutputPath + QString("/DTIAtlas/2_NonLinear_Registration_AW/Case") << i+1 << QString("_NonLinearTrans_FA.mhd"); // AW Deformed FA
			stream << m_CSVseparator << m_OutputPath + QString("/DTIAtlas/2_NonLinear_Registration_AW/Case") << i+1 << QString("_DeformationField.mhd"); // AW Deformation field to Affine space
			stream << m_CSVseparator << m_OutputPath + QString("/DTIAtlas/2_NonLinear_Registration_AW/Case") << i+1 << QString("_InverseDeformationField.mhd"); // AW Inverse Deformation field to Affine space
			stream << m_CSVseparator << m_OutputPath + QString("/DTIAtlas/3_AW_Atlas/Case") << i+1 << QString("_AWDTI.nrrd"); // AW DTI
			stream << m_CSVseparator << m_OutputPath + QString("/DTIAtlas/4_Final_Resampling/Second_Resampling/Case") << i+1 << QString("_GlobalDeformationField.nrrd"); // AW Deformation field to Original space
			stream << m_CSVseparator << m_OutputPath + QString("/DTIAtlas/4_Final_Resampling/Second_Resampling/Case") << i+1 << QString("_FinalDeformedDTI.nrrd"); // DTI-Reg Final DTI
			stream << endl;
		}

		if(!m_Quiet) std::cout<<"DONE"<<std::endl; // command line display
		
	}
	else qDebug( "Could not create file");
}

  /////////////////////////////////////////
 //             PARAMETERS              //
/////////////////////////////////////////

void GUI::SaveParametersSlot() /*SLOT*/
{
	if(CaseListWidget->count()==0)
	{
		QMessageBox::critical(this, "No Dataset", "No Dataset");
		return;
	}

	QString ParamBrowseName=QFileDialog::getSaveFileName(this, tr("Save Parameter File"),"./DTIAtlasBuilderParameters.txt",tr("Text File (*.txt)"));

	if(!ParamBrowseName.isEmpty())
	{
		QString CSVFileName = ParamBrowseName.split(".").at(0) + QString(".csv"); // [Name].txt => [Name].csv
		SaveParameters(ParamBrowseName,CSVFileName);

		QMessageBox::information(this, "Saving succesful", "Parameters have been succesfully saved at" + ParamBrowseName);
		m_ParamSaved=1;
	}
}

void GUI::SaveParameters(QString ParamBrowseName,QString CSVFileName)
{
	if(!ParamBrowseName.isEmpty())
	{

	QFile file(ParamBrowseName);
	if ( file.open( IO_WriteOnly | IO_Translate ) )
	{
		if(!m_Quiet) std::cout<<"| Saving Parameters file..."; // command line display

		QTextStream stream( &file );

		stream << m_ParamFileHeader << "=1" << endl;
		stream << "Output Folder=" << OutputFolderLineEdit->text() << endl;
		stream << "Atlas Template=" << TemplateLineEdit->text() << endl;
		if(SafetyMargincheckBox->isChecked()) stream << "SafetyMargin=true" << endl;
		else  stream << "SafetyMargin=false" << endl;
		stream << "Loops for the registration=" << NbLoopsSpinBox->value() << endl;
		stream << "BRAINSFit Affine Tfm Mode=" << BFAffineTfmModecomboBox->currentText() << endl;
		if(OverwritecheckBox->isChecked()) stream << "Overwrite=true" << endl;
		else  stream << "Overwrite=false" << endl;

		int SL4,SL2,SL1;
		if(SL4checkBox->isChecked()) SL4=1;
		else SL4=0;
		stream << "Scale Level =" << SL4 << ","  << SL4spinBox->value() << ","  << nbIter4SpinBox->value() << "," << alpha4DoubleSpinBox->value() << "," << beta4DoubleSpinBox->value() << "," << gamma4DoubleSpinBox->value() << "," << maxPerturbation4DoubleSpinBox->value() << endl;
		if(SL2checkBox->isChecked()) SL2=1;
		else SL2=0;
		stream << "Scale Level =" << SL2 << ","  << SL2spinBox->value() << "," << nbIter2SpinBox->value() << "," << alpha2DoubleSpinBox->value() << "," << beta2DoubleSpinBox->value() << "," << gamma2DoubleSpinBox->value() << "," << maxPerturbation2DoubleSpinBox->value() << endl;
		if(SL1checkBox->isChecked()) SL1=1;
		else SL1=0;
		stream << "Scale Level =" << SL1 << ","  << SL1spinBox->value() << "," << nbIter1SpinBox->value() << "," << alpha1DoubleSpinBox->value() << "," << beta1DoubleSpinBox->value() << "," << gamma1DoubleSpinBox->value() << "," << maxPerturbation1DoubleSpinBox->value() << endl;

		stream << "Resampling Interpolation Algorithm=" << InterpolTypeComboBox->currentText() ;
		if( InterpolTypeComboBox->currentText()==QString("Windowed Sinc") ) stream << "=" << m_windowComboBox->currentText() << endl;
		else if( InterpolTypeComboBox->currentText()==QString("BSpline") ) stream << "=" << m_BSplineComboBox->currentText() << endl;
		else stream << endl;

		stream << "Tensor interpolation=" << TensInterpolComboBox->currentText() ;
		if( TensInterpolComboBox->currentText()==QString("Non Log Euclidean") ) stream << "=" << m_nologComboBox->currentText() << endl;
		else stream << endl;

		stream << "Tensor transformation=" << TensTfmComboBox->currentText()<< endl;
		stream << "DTI Average Statistics Method=" << averageStatMethodComboBox->currentText() << endl;

		stream << "DTIRegMethod=" << RegMethodcomboBox->currentText() ;
		if( RegMethodcomboBox->currentText()==QString("ANTS") ) 
		{
			stream << "=" << m_ARegTypeComboBox->currentText() << ";" << m_TfmStepLine->text() << ";" << m_IterLine->text() << ";" << m_SimMetComboBox->currentText() << ";" << m_SimParamDble->value() << ";" << m_GSigmaDble->value() ;
			if(m_SmoothOffCheck->isChecked()) stream << ";1" << endl;
			else stream << ";0" << endl;
		}
		else stream << "=" << m_BRegTypeComboBox->currentText() << ";" << m_TfmModeComboBox->currentText() << ";" << m_SigmaDble->value() << ";" << m_NbPyrLevSpin->value() << ";" << m_PyrLevItLine->text() << endl;

		stream << "CSV Dataset File=" << CSVFileName << endl;

		if(!m_Quiet) std::cout<<"DONE"<<std::endl; // command line display

		QFile filecsv(CSVFileName);
		if ( filecsv.open( IO_WriteOnly | IO_Translate ) )
		{
			if(!m_Quiet) std::cout<<"| Generating Dataset csv file..."; // command line display

			QTextStream streamcsv( &filecsv );
			streamcsv << QString("id") << m_CSVseparator << QString("Original DTI Image") << endl;
			for(int i=0; i < CaseListWidget->count() ;i++) streamcsv << i+1 << m_CSVseparator << CaseListWidget->item(i)->text().remove(0, CaseListWidget->item(i)->text().split(":").at(0).size()+2 ) << endl;
			if(!m_Quiet) std::cout<<"DONE"<<std::endl; // command line display
		
			SelectCasesLabel->setText( QString("Current CSV file : ") + CSVFileName );
		}
		else 
		{
			if(!m_Quiet) std::cout<<"FAILED"<<std::endl; // command line display
			qDebug( "Could not create csv file");
		}

	}
	else qDebug( "Could not create parameter file");

	}
}

void GUI::LoadParametersSlot() /*SLOT*/
{
	QString ParamBrowse=QFileDialog::getOpenFileName(this, "Open Parameter File", QString(), ".txt Files (*.txt)");

	if(!ParamBrowse.isEmpty())
	{
		LoadParameters(ParamBrowse);
	}
}

int GUI::LoadParameters(QString paramFile)
{
	if( access(paramFile.toStdString().c_str(), F_OK) == 0 ) // Test if the config file exists => unistd::access() returns 0 if F(file)_OK
	{

	QFile file(paramFile);

	if (file.open(QFile::ReadOnly))
	{
		QTextStream stream(&file);

		QString line = stream.readLine();
		QStringList list = line.split("=");
		if( ! list.at(0).contains(m_ParamFileHeader) )
		{
			if(!m_noGUI) QMessageBox::critical(this, "No parameter file", "This file is not a parameter file\nfor this program");
			else if(!m_Quiet) std::cout<<"| This file is not a parameter file for this program"<<std::endl;
			return -1;
		}

		if(!m_Quiet) std::cout<<"| Loading Parameters file \'"<< paramFile.toStdString() <<"\'..."; // command line display

/* Other Parameters */
		line = stream.readLine();
		list = line.split("=");
		if(!list.at(0).contains(QString("Output Folder")))
		{
			if(!m_noGUI) 
			{
				QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
				if(!m_Quiet) std::cout<<"FAILED"<<std::endl; // command line display
			}
			else if(!m_Quiet) std::cout<<"FAILED"<<std::endl<<"| This parameter file is corrupted"<<std::endl;
			return -1;
		}
		OutputFolderLineEdit->setText(list.at(1));

		line = stream.readLine();
		list = line.split("=");
		if(!list.at(0).contains(QString("Atlas Template")))
		{
			if(!m_noGUI) 
			{
				QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
				if(!m_Quiet) std::cout<<"FAILED"<<std::endl; // command line display
			}
			else if(!m_Quiet) std::cout<<"FAILED"<<std::endl<<"| This parameter file is corrupted"<<std::endl;
			return -1;
		}
		TemplateLineEdit->setText(list.at(1));

		line = stream.readLine();
		list = line.split("=");
		if(!list.at(0).contains(QString("SafetyMargin")))
		{
			if(!m_noGUI) 
			{
				QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
				if(!m_Quiet) std::cout<<"FAILED"<<std::endl; // command line display
			}
			else if(!m_Quiet) std::cout<<"FAILED"<<std::endl<<"| This parameter file is corrupted"<<std::endl;
			return -1;
		}
		if( list.at(1) == QString("true") ) SafetyMargincheckBox->setChecked(true);

		line = stream.readLine();
		list = line.split("=");
		if(!list.at(0).contains(QString("Loops for the registration")))
		{
			if(!m_noGUI) 
			{
				QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
				if(!m_Quiet) std::cout<<"FAILED"<<std::endl; // command line display
			}
			else if(!m_Quiet) std::cout<<"FAILED"<<std::endl<<"| This parameter file is corrupted"<<std::endl;
			return -1;
		}
		NbLoopsSpinBox->setValue( list.at(1).toInt() );

		line = stream.readLine();
		list = line.split("=");
		if(!list.at(0).contains(QString("BRAINSFit Affine Tfm Mode")))
		{
			if(!m_noGUI) 
			{
				QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
				if(!m_Quiet) std::cout<<"FAILED"<<std::endl; // command line display
			}
			else if(!m_Quiet) std::cout<<"FAILED"<<std::endl<<"| This parameter file is corrupted"<<std::endl;
			return -1;
		}
		if( list.at(1).contains(QString("Off")) ) BFAffineTfmModecomboBox->setCurrentIndex(0);
		else if( list.at(1).contains(QString("useMomentsAlign")) ) BFAffineTfmModecomboBox->setCurrentIndex(1);
		else if( list.at(1).contains(QString("useCenterOfHeadAlign")) ) BFAffineTfmModecomboBox->setCurrentIndex(2);
		else if( list.at(1).contains(QString("useGeometryAlign")) ) BFAffineTfmModecomboBox->setCurrentIndex(3);

		line = stream.readLine();
		list = line.split("=");
		if(!list.at(0).contains(QString("Overwrite")))
		{
			if(!m_noGUI) 
			{
				QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
				if(!m_Quiet) std::cout<<"FAILED"<<std::endl; // command line display
			}
			else if(!m_Quiet) std::cout<<"FAILED"<<std::endl<<"| This parameter file is corrupted"<<std::endl;
			return -1;
		}
		if( list.at(1) == QString("true") ) OverwritecheckBox->setChecked(true);

/* Scale Levels */
		line = stream.readLine();
		list = line.split("=");
		if(!list.at(0).contains(QString("Scale Level")))
		{
			if(!m_noGUI) 
			{
				QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
				if(!m_Quiet) std::cout<<"FAILED"<<std::endl; // command line display
			}
			else if(!m_Quiet) std::cout<<"FAILED"<<std::endl<<"| This parameter file is corrupted"<<std::endl;
			return -1;
		}
		else
		{
			QStringList nbrs= list.at(1).split(",");
			if( nbrs.size()!=7 )
			{
				if(!m_noGUI) 
				{
					QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
					if(!m_Quiet) std::cout<<"FAILED"<<std::endl; // command line display
				}
				else if(!m_Quiet) std::cout<<"FAILED"<<std::endl<<"| This parameter file is corrupted"<<std::endl;
				return -1;
			}
			if(nbrs.at(0).toInt()==1)
			{
				if( (nbrs.at(1).toInt()==0) || (nbrs.at(2).toInt()==0) || (nbrs.at(3).toDouble()==0) || (nbrs.at(4).toDouble()==0) || (nbrs.at(5).toDouble()==0) || (nbrs.at(6).toDouble()==0))
				{
					if(!m_noGUI) 
					{
						QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
						if(!m_Quiet) std::cout<<"FAILED"<<std::endl; // command line display
					}
					else if(!m_Quiet) std::cout<<"FAILED"<<std::endl<<"| This parameter file is corrupted"<<std::endl;
					return -1;
				}
				SL4checkBox->setChecked(true);
				SL4spinBox->setValue(nbrs.at(1).toInt());
				nbIter4SpinBox->setValue(nbrs.at(2).toInt());
				alpha4DoubleSpinBox->setValue(nbrs.at(3).toDouble());
				beta4DoubleSpinBox->setValue(nbrs.at(4).toDouble());
				gamma4DoubleSpinBox->setValue(nbrs.at(5).toDouble());
				maxPerturbation4DoubleSpinBox->setValue(nbrs.at(6).toDouble());
			}
		}

		line = stream.readLine();
		list = line.split("=");
		if(!list.at(0).contains(QString("Scale Level")))
		{
			if(!m_noGUI) 
			{
				QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
				if(!m_Quiet) std::cout<<"FAILED"<<std::endl; // command line display
			}
			else if(!m_Quiet) std::cout<<"FAILED"<<std::endl<<"| This parameter file is corrupted"<<std::endl;
			return -1;
		}
		else
		{
			QStringList nbrs= list.at(1).split(",");
			if( nbrs.size()!=7 )
			{
				if(!m_noGUI) 
				{
					QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
					if(!m_Quiet) std::cout<<"FAILED"<<std::endl; // command line display
				}
				else if(!m_Quiet) std::cout<<"FAILED"<<std::endl<<"| This parameter file is corrupted"<<std::endl;
				return -1;
			}
			if(nbrs.at(0).toInt()==1)
			{
				if( (nbrs.at(1).toInt()==0) || (nbrs.at(2).toInt()==0) || (nbrs.at(3).toDouble()==0) || (nbrs.at(4).toDouble()==0) || (nbrs.at(5).toDouble()==0) || (nbrs.at(6).toDouble()==0))
				{
					if(!m_noGUI) 
					{
						QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
						if(!m_Quiet) std::cout<<"FAILED"<<std::endl; // command line display
					}
					else if(!m_Quiet) std::cout<<"FAILED"<<std::endl<<"| This parameter file is corrupted"<<std::endl;
					return -1;
				}
				SL2checkBox->setChecked(true);
				SL2spinBox->setValue(nbrs.at(1).toInt());
				nbIter2SpinBox->setValue(nbrs.at(2).toInt());
				alpha2DoubleSpinBox->setValue(nbrs.at(3).toDouble());
				beta2DoubleSpinBox->setValue(nbrs.at(4).toDouble());
				gamma2DoubleSpinBox->setValue(nbrs.at(5).toDouble());
				maxPerturbation2DoubleSpinBox->setValue(nbrs.at(6).toDouble());
			}
		}

		line = stream.readLine();
		list = line.split("=");
		if(!list.at(0).contains(QString("Scale Level")))
		{
			if(!m_noGUI) 
			{
				QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
				if(!m_Quiet) std::cout<<"FAILED"<<std::endl; // command line display
			}
			else if(!m_Quiet) std::cout<<"FAILED"<<std::endl<<"| This parameter file is corrupted"<<std::endl;
			return -1;
		}
		else
		{
			QStringList nbrs= list.at(1).split(",");
			if( nbrs.size()!=7 )
			{
				if(!m_noGUI) 
				{
					QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
					if(!m_Quiet) std::cout<<"FAILED"<<std::endl; // command line display
				}
				else if(!m_Quiet) std::cout<<"FAILED"<<std::endl<<"| This parameter file is corrupted"<<std::endl;
				return -1;
			}
			if(nbrs.at(0).toInt()==1)
			{
				if( (nbrs.at(1).toInt()==0) || (nbrs.at(2).toInt()==0) || (nbrs.at(3).toDouble()==0) || (nbrs.at(4).toDouble()==0) || (nbrs.at(5).toDouble()==0) || (nbrs.at(6).toDouble()==0))
				{
					if(!m_noGUI) 
					{
						QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
						if(!m_Quiet) std::cout<<"FAILED"<<std::endl; // command line display
					}
					else if(!m_Quiet) std::cout<<"FAILED"<<std::endl<<"| This parameter file is corrupted"<<std::endl;
					return -1;
				}
				SL1checkBox->setChecked(true);
				SL1spinBox->setValue(nbrs.at(1).toInt());
				nbIter1SpinBox->setValue(nbrs.at(2).toInt());
				alpha1DoubleSpinBox->setValue(nbrs.at(3).toDouble());
				beta1DoubleSpinBox->setValue(nbrs.at(4).toDouble());
				gamma1DoubleSpinBox->setValue(nbrs.at(5).toDouble());
				maxPerturbation1DoubleSpinBox->setValue(nbrs.at(6).toDouble());
			}
		}

/* Final Atlas Building parameters */
		line = stream.readLine();
		list = line.split("=");
		if(!list.at(0).contains(QString("Resampling Interpolation Algorithm")))
		{
			if(!m_noGUI) 
			{
				QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
				if(!m_Quiet) std::cout<<"FAILED"<<std::endl; // command line display
			}
			else if(!m_Quiet) std::cout<<"FAILED"<<std::endl<<"| This parameter file is corrupted"<<std::endl;
			return -1;
		}
		if( list.at(1).contains(QString("Linear")) ) InterpolTypeComboBox->setCurrentIndex(0);
		else if( list.at(1).contains(QString("Nearest Neighborhoor")) ) InterpolTypeComboBox->setCurrentIndex(1);
		else if( list.at(1).contains(QString("Windowed Sinc")) ) 
		{ 
			InterpolTypeComboBox->setCurrentIndex(2);
			if( list.at(2).contains(QString("Hamming")) ) m_windowComboBox->setCurrentIndex(0);
			else if( list.at(2).contains(QString("Cosine")) ) m_windowComboBox->setCurrentIndex(1);
			else if( list.at(2).contains(QString("Welch")) ) m_windowComboBox->setCurrentIndex(2);
			else if( list.at(2).contains(QString("Lanczos")) ) m_windowComboBox->setCurrentIndex(3);
			else if( list.at(2).contains(QString("Blackman")) ) m_windowComboBox->setCurrentIndex(4);
			else
			{
				if(!m_noGUI) 
				{
					QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
					if(!m_Quiet) std::cout<<"FAILED"<<std::endl; // command line display
				}
				else if(!m_Quiet) std::cout<<"FAILED"<<std::endl<<"| This parameter file is corrupted"<<std::endl;
				return -1;
			}
		}
		else if( list.at(1).contains(QString("BSpline")) ) 
		{ 
			InterpolTypeComboBox->setCurrentIndex(3);
			if( list.at(2).toInt()>=0 && list.at(2).toInt()<=5 ) m_BSplineComboBox->setCurrentIndex( list.at(2).toInt()-1 );
			else
			{
			if(!m_noGUI) 
			{
				QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
				if(!m_Quiet) std::cout<<"FAILED"<<std::endl; // command line display
			}
			else if(!m_Quiet) std::cout<<"FAILED"<<std::endl<<"| This parameter file is corrupted"<<std::endl;
				return -1;
			}
		}
		else
		{
			if(!m_noGUI) 
			{
				QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
				if(!m_Quiet) std::cout<<"FAILED"<<std::endl; // command line display
			}
			else if(!m_Quiet) std::cout<<"FAILED"<<std::endl<<"| This parameter file is corrupted"<<std::endl;
			return -1;
		}

		line = stream.readLine();
		list = line.split("=");
		if(!list.at(0).contains(QString("Tensor interpolation")))
		{
			if(!m_noGUI) 
			{
				QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
				if(!m_Quiet) std::cout<<"FAILED"<<std::endl; // command line display
			}
			else if(!m_Quiet) std::cout<<"FAILED"<<std::endl<<"| This parameter file is corrupted"<<std::endl;
			return -1;
		}
		if( list.at(1).contains(QString("Non Log Euclidean")) )
		{ 
			TensInterpolComboBox->setCurrentIndex(1);
			if( list.at(2).contains(QString("Zero")) ) m_nologComboBox->setCurrentIndex(0);
			else if( list.at(2).contains(QString("None")) ) m_nologComboBox->setCurrentIndex(1);
			else if( list.at(2).contains(QString("Absolute Value")) ) m_nologComboBox->setCurrentIndex(2);
			else if( list.at(2).contains(QString("Nearest")) ) m_nologComboBox->setCurrentIndex(3);
			else
			{
				if(!m_noGUI) 
				{
					QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
					if(!m_Quiet) std::cout<<"FAILED"<<std::endl; // command line display
				}
				else if(!m_Quiet) std::cout<<"FAILED"<<std::endl<<"| This parameter file is corrupted"<<std::endl;
				return -1;
			}
		}
		else if( list.at(1).contains(QString("Log Euclidean")) ) TensInterpolComboBox->setCurrentIndex(0);
		else
		{
			if(!m_noGUI) 
			{
				QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
				if(!m_Quiet) std::cout<<"FAILED"<<std::endl; // command line display
			}
			else if(!m_Quiet) std::cout<<"FAILED"<<std::endl<<"| This parameter file is corrupted"<<std::endl;
			return -1;
		}

		line = stream.readLine();
		list = line.split("=");
		if(!list.at(0).contains(QString("Tensor transformation")))
		{
			if(!m_noGUI) 
			{
				QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
				if(!m_Quiet) std::cout<<"FAILED"<<std::endl; // command line display
			}
			else if(!m_Quiet) std::cout<<"FAILED"<<std::endl<<"| This parameter file is corrupted"<<std::endl;
			return -1;
		}
		if( list.at(1).contains(QString("Preservation of the Principal Direction (PPD)")) ) TensTfmComboBox->setCurrentIndex(0);
		else if( list.at(1).contains(QString("Finite Strain (FS)")) ) TensTfmComboBox->setCurrentIndex(1);
		else
		{
			if(!m_noGUI) 
			{
				QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
				if(!m_Quiet) std::cout<<"FAILED"<<std::endl; // command line display
			}
			else if(!m_Quiet) std::cout<<"FAILED"<<std::endl<<"| This parameter file is corrupted"<<std::endl;
			return -1;
		}

		line = stream.readLine();
		list = line.split("=");
		if(!list.at(0).contains(QString("DTI Average Statistics Method")))
		{
			if(!m_noGUI) 
			{
				QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
				if(!m_Quiet) std::cout<<"FAILED"<<std::endl; // command line display
			}
			else if(!m_Quiet) std::cout<<"FAILED"<<std::endl<<"| This parameter file is corrupted"<<std::endl;
			return -1;
		}
		if( list.at(1).contains(QString("Principal Geodesic Analysis (PGA)")) ) averageStatMethodComboBox->setCurrentIndex(0);
		else if( list.at(1).contains(QString("Log Euclidean")) ) averageStatMethodComboBox->setCurrentIndex(2);
		else if( list.at(1).contains(QString("Euclidean")) ) averageStatMethodComboBox->setCurrentIndex(1);
		else
		{
			if(!m_noGUI) 
			{
				QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
				if(!m_Quiet) std::cout<<"FAILED"<<std::endl; // command line display
			}
			else if(!m_Quiet) std::cout<<"FAILED"<<std::endl<<"| This parameter file is corrupted"<<std::endl;
			return -1;
		}

/* Final Resampling parameters */
		line = stream.readLine();
		list = line.split("=");
		if(!list.at(0).contains(QString("DTIRegMethod")))
		{
			if(!m_noGUI) 
			{
				QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
				if(!m_Quiet) std::cout<<"FAILED"<<std::endl; // command line display
			}
			else if(!m_Quiet) std::cout<<"FAILED"<<std::endl<<"| This parameter file is corrupted"<<std::endl;
			return -1;
		}
		if( list.at(1).contains(QString("BRAINS")) )
		{
			InterpolTypeComboBox->setCurrentIndex(0);
			QStringList param= list.at(2).split(";");
			if( param.size()!=5 )
			{
				if(!m_noGUI) 
				{
					QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
					if(!m_Quiet) std::cout<<"FAILED"<<std::endl; // command line display
				}
				else if(!m_Quiet) std::cout<<"FAILED"<<std::endl<<"| This parameter file is corrupted"<<std::endl;
				return -1;
			}

			if( param.at(0).contains(QString("None")) ) m_BRegTypeComboBox->setCurrentIndex(0);
			else if( param.at(0).contains(QString("Rigid")) ) m_BRegTypeComboBox->setCurrentIndex(1);
			else if( param.at(0).contains(QString("BSpline")) ) m_BRegTypeComboBox->setCurrentIndex(2);
			else if( param.at(0).contains(QString("Diffeomorphic")) ) m_BRegTypeComboBox->setCurrentIndex(3);
			else if( param.at(0).contains(QString("SymmetricLogDemons")) ) m_BRegTypeComboBox->setCurrentIndex(6);
			else if( param.at(0).contains(QString("LogDemons")) ) m_BRegTypeComboBox->setCurrentIndex(5);
			else if( param.at(0).contains(QString("Demons")) ) m_BRegTypeComboBox->setCurrentIndex(4);
			else
			{
				if(!m_noGUI) 
				{
					QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
					if(!m_Quiet) std::cout<<"FAILED"<<std::endl; // command line display
				}
				else if(!m_Quiet) std::cout<<"FAILED"<<std::endl<<"| This parameter file is corrupted"<<std::endl;
				return -1;
			}

			if( param.at(1).contains(QString("Off")) ) m_TfmModeComboBox->setCurrentIndex(0);
			else if( param.at(1).contains(QString("useMomentsAlign")) ) m_TfmModeComboBox->setCurrentIndex(1);
			else if( param.at(1).contains(QString("useCenterOfHeadAlign")) ) m_TfmModeComboBox->setCurrentIndex(2);
			else if( param.at(1).contains(QString("useGeometryAlign")) ) m_TfmModeComboBox->setCurrentIndex(3);
			else if( param.at(1).contains(QString("Use computed affine transform")) ) m_TfmModeComboBox->setCurrentIndex(4);
			else
			{
				if(!m_noGUI) 
				{
					QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
					if(!m_Quiet) std::cout<<"FAILED"<<std::endl; // command line display
				}
				else if(!m_Quiet) std::cout<<"FAILED"<<std::endl<<"| This parameter file is corrupted"<<std::endl;
				return -1;
			}

			m_SigmaDble->setValue( param.at(2).toDouble() );
			m_NbPyrLevSpin->setValue( param.at(3).toInt() );
			m_PyrLevItLine->setText( param.at(4) );
		}
		else if( list.at(1).contains(QString("ANTS")) )
		{
			RegMethodcomboBox->setCurrentIndex(1);
			QStringList param= list.at(2).split(";");
			if( param.size()!=7 )
			{
				if(!m_noGUI) 
				{
					QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
					if(!m_Quiet) std::cout<<"FAILED"<<std::endl; // command line display
				}
				else if(!m_Quiet) std::cout<<"FAILED"<<std::endl<<"| This parameter file is corrupted"<<std::endl;
				return -1;
			}

			if( param.at(0).contains(QString("None")) ) m_ARegTypeComboBox->setCurrentIndex(0);
			else if( param.at(0).contains(QString("Rigid")) ) m_ARegTypeComboBox->setCurrentIndex(1);
			else if( param.at(0).contains(QString("Elast")) ) m_ARegTypeComboBox->setCurrentIndex(2);
			else if( param.at(0).contains(QString("Exp")) ) m_ARegTypeComboBox->setCurrentIndex(3);
			else if( param.at(0).contains(QString("GreedyExp")) ) m_ARegTypeComboBox->setCurrentIndex(4);
			else if( param.at(0).contains(QString("GreedyDiffeo (SyN)")) ) m_ARegTypeComboBox->setCurrentIndex(5);
			else if( param.at(0).contains(QString("SpatioTempDiffeo (SyN)")) ) m_ARegTypeComboBox->setCurrentIndex(6);
			else
			{
				if(!m_noGUI) 
				{
					QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
					if(!m_Quiet) std::cout<<"FAILED"<<std::endl; // command line display
				}
				else if(!m_Quiet) std::cout<<"FAILED"<<std::endl<<"| This parameter file is corrupted"<<std::endl;
				return -1;
			}

			m_TfmStepLine->setText( param.at(1) );
			m_IterLine->setText( param.at(2) );

			if( param.at(3).contains(QString("CC")) ) m_SimMetComboBox->setCurrentIndex(0);
			else if( param.at(3).contains(QString("MI")) ) m_SimMetComboBox->setCurrentIndex(1);
			else if( param.at(3).contains(QString("MSQ")) ) m_SimMetComboBox->setCurrentIndex(2);
			else
			{
				if(!m_noGUI) 
				{
					QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
					if(!m_Quiet) std::cout<<"FAILED"<<std::endl; // command line display
				}
				else if(!m_Quiet) std::cout<<"FAILED"<<std::endl<<"| This parameter file is corrupted"<<std::endl;
				return -1;
			}

			m_SimParamDble->setValue( param.at(4).toDouble() );
			m_GSigmaDble->setValue( param.at(5).toDouble() );
			if ( param.at(6).toInt()==1 ) m_SmoothOffCheck->setChecked(true);
		}
		else
		{
			if(!m_noGUI) 
			{
				QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
				if(!m_Quiet) std::cout<<"FAILED"<<std::endl; // command line display
			}
			else if(!m_Quiet) std::cout<<"FAILED"<<std::endl<<"| This parameter file is corrupted"<<std::endl;
			return -1;
		}

		if(!m_Quiet) std::cout<<"DONE"<<std::endl; // command line display

/* Opening CSV File */
		line = stream.readLine();
		list = line.split("=");
		if(!list.at(0).contains(QString("CSV Dataset File")))
		{
			if(!m_noGUI) 
			{
				QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
				if(!m_Quiet) std::cout<<"FAILED"<<std::endl; // command line display
			}
			else if(!m_Quiet) std::cout<<"FAILED"<<std::endl<<"| This parameter file is corrupted"<<std::endl;
			return -1;
		}
		QString CSVpath = list.at(1);
		CaseListWidget->clear();
		ReadCSV(CSVpath);

		m_ParamSaved=1;
	} 
	else if ( !paramFile.isEmpty() ) qDebug( "Could not open file");

	}
	else if(!m_Quiet) std::cout<<"| The given parameter file does not exist"<<std::endl; // command line display

	return 0;
}

  /////////////////////////////////////////
 //              XML FILE               //
/////////////////////////////////////////

void GUI::GenerateXMLForAW()
{	
	if( access((m_OutputPath.toStdString() + "/DTIAtlas/2_NonLinear_Registration_AW").c_str(), F_OK) != 0 ) // Test if the main folder does not exists => unistd::access() returns 0 if F(file)_OK
	{
		if(!m_Quiet) std::cout<<"| Creating Non Linear Registration directory..."<<std::endl; // command line display
		QProcess * mkdirMainProcess = new QProcess;
		std::string program = "mkdir " + m_OutputPath.toStdString() + "/DTIAtlas/2_NonLinear_Registration_AW"; //// Creates the directory
		if(!m_Quiet) std::cout<<"| $ " << program << std::endl;
		mkdirMainProcess->execute( program.c_str() );
	}

	QString	xmlFileName = m_OutputPath + QString("/DTIAtlas/2_NonLinear_Registration_AW/AtlasWerksParameters.xml");
	QFile file(xmlFileName);
	if ( file.open( IO_WriteOnly | IO_Translate ) )
	{
		if(!m_Quiet) std::cout<<"| Saving XML file for AtlasWerks..."<<std::endl; // command line display
		QTextStream stream( &file );

		stream <<"<!--top-level node-->"<< endl;
		stream <<"<ParameterFile>"<< endl;
			stream <<"\t<ScaleImageWeights val=\"true\"/>"<< endl;
			stream <<"\t<WeightedImageSet>"<< endl;
				stream <<"\t\t<InputImageFormatString>"<< endl;
					stream <<"\t\t\t<FormatString val=\"\" />"<< endl;
					stream <<"\t\t\t<Base val=\"0\" />"<< endl;
					std::ostringstream out;
					out << m_CasesPath.size();
					std::string nbcases_str = out.str();
					stream <<"\t\t\t<NumFiles val=\"" << nbcases_str.c_str() << "\" />"<< endl;
					stream <<"\t\t\t<Weight val=\"1\" />"<< endl;
				stream <<"\t\t</InputImageFormatString>"<< endl;
			std::ostringstream out1;
			out1 << NbLoopsSpinBox->value();
			std::string nbLoops_str = out1.str();
			for (unsigned int i=0;i<m_CasesPath.size();i++)
			{
				std::ostringstream outi;
				outi << i+1;
				std::string i_str = outi.str();
				stream <<"\t\t<WeightedImage>"<< endl;
					stream <<"\t\t\t<Filename val=\"" << m_OutputPath << "/DTIAtlas/1_Affine_Registration/Loop" << nbLoops_str.c_str() << "/Case" << i_str.c_str() << "_Loop" << nbLoops_str.c_str() << "_FinalFA.nrrd\" />"<< endl;
					stream <<"\t\t\t<ItkTransform val=\"1\" />"<< endl;
				stream <<"\t\t</WeightedImage>"<< endl;
			}
			stream <<"\t</WeightedImageSet>"<< endl;

/* Scale Levels */
/* Aditya :--scaleLevel=4 --numberOfIterations=150 --alpha=1 --beta=1 --gamma=0.0001 --maxPerturbation=0.001 --scaleLevel=2 --numberOfIterations=120 --alpha=1 --beta=1 --gamma=0.001 --maxPerturbation=0.01 --scaleLevel=1 --numberOfIterations=100 --alpha=0.1 --beta=0.1 --gamma=0.01 --maxPerturbation=0.1 */
/* Default Parameters for AtlasWerks:
Scale                 : 1
alpha                 : 0.01
beta                  : 0.01
gamma                 : 0.001
Max. Pert.            : 0.5
Num. Iterations       : 50
*/

			if(SL4checkBox->isChecked())
			{
			stream <<"\t<GreedyAtlasScaleLevel>"<< endl;
				stream <<"\t\t<ScaleLevel>"<< endl;
					stream <<"\t\t\t<!--factor by which to downsample images-->"<< endl;
					std::ostringstream outSL46;
					outSL46 << SL4spinBox->value();
					std::string SL46_str = outSL46.str();
					stream <<"\t\t\t<DownsampleFactor val=\"" << SL46_str.c_str() << "\" />"<< endl;
				stream <<"\t\t</ScaleLevel>"<< endl;
				stream <<"\t\t<!--Scale factor on the maximum velocity in a given deformation for computing delta-->"<< endl;
				std::ostringstream outSL4;
				outSL4 << maxPerturbation4DoubleSpinBox->value();
				std::string SL4_str = outSL4.str();
				stream <<"\t\t<MaxPert val=\"" << SL4_str.c_str() << "\" />"<< endl;
				std::ostringstream outSL42;
				outSL42 << nbIter4SpinBox->value();
				std::string SL42_str = outSL42.str();
				stream <<"\t\t<NIterations val=\"" << SL42_str.c_str() << "\" />"<< endl;
				stream <<"\t\t<DiffOper>"<< endl;
					std::ostringstream outSL43;
					outSL43 << alpha4DoubleSpinBox->value();
					std::string SL43_str = outSL43.str();
					stream <<"\t\t\t<Alpha val=\"" << SL43_str.c_str() << "\" />"<< endl;
					std::ostringstream outSL44;
					outSL44 << beta4DoubleSpinBox->value();
					std::string SL44_str = outSL44.str();
					stream <<"\t\t\t<Beta val=\"" << SL44_str.c_str() << "\" />"<< endl;
					std::ostringstream outSL45;
					outSL45 << gamma4DoubleSpinBox->value();
					std::string SL45_str = outSL45.str();
					stream <<"\t\t\t<Gamma val=\"" << SL45_str.c_str() << "\" />"<< endl;
				stream <<"\t\t</DiffOper>"<< endl;
			stream <<"\t</GreedyAtlasScaleLevel>"<< endl;
			}

			if(SL2checkBox->isChecked())
			{
			stream <<"\t<GreedyAtlasScaleLevel>"<< endl;
				stream <<"\t\t<ScaleLevel>"<< endl;
					stream <<"\t\t\t<!--factor by which to downsample images-->"<< endl;
					std::ostringstream outSL26;
					outSL26 << SL2spinBox->value();
					std::string SL26_str = outSL26.str();
					stream <<"\t\t\t<DownsampleFactor val=\"" << SL26_str.c_str() << "\" />"<< endl;
				stream <<"\t\t</ScaleLevel>"<< endl;
				stream <<"\t\t<!--Scale factor on the maximum velocity in a given deformation for computing delta-->"<< endl;
				std::ostringstream outSL2;
				outSL2 << maxPerturbation2DoubleSpinBox->value();
				std::string SL2_str = outSL2.str();
				stream <<"\t\t<MaxPert val=\"" << SL2_str.c_str() << "\" />"<< endl;
				std::ostringstream outSL22;
				outSL22 << nbIter2SpinBox->value();
				std::string SL22_str = outSL22.str();
				stream <<"\t\t<NIterations val=\"" << SL22_str.c_str() << "\" />"<< endl;
				stream <<"\t\t<DiffOper>"<< endl;
					std::ostringstream outSL23;
					outSL23 << alpha2DoubleSpinBox->value();
					std::string SL23_str = outSL23.str();
					stream <<"\t\t\t<Alpha val=\"" << SL23_str.c_str() << "\" />"<< endl;
					std::ostringstream outSL24;
					outSL24 << beta2DoubleSpinBox->value();
					std::string SL24_str = outSL24.str();
					stream <<"\t\t\t<Beta val=\"" << SL24_str.c_str() << "\" />"<< endl;
					std::ostringstream outSL25;
					outSL25 << gamma2DoubleSpinBox->value();
					std::string SL25_str = outSL25.str();
					stream <<"\t\t\t<Gamma val=\"" << SL25_str.c_str() << "\" />"<< endl;
				stream <<"\t\t</DiffOper>"<< endl;
			stream <<"\t</GreedyAtlasScaleLevel>"<< endl;
			}

			if(SL1checkBox->isChecked())
			{
			stream <<"\t<GreedyAtlasScaleLevel>"<< endl;
				stream <<"\t\t<ScaleLevel>"<< endl;
					stream <<"\t\t\t<!--factor by which to downsample images-->"<< endl;
					std::ostringstream outSL16;
					outSL16 << SL1spinBox->value();
					std::string SL16_str = outSL16.str();
					stream <<"\t\t\t<DownsampleFactor val=\"" << SL16_str.c_str() << "\" />"<< endl;
				stream <<"\t\t</ScaleLevel>"<< endl;
				stream <<"\t\t<!--Scale factor on the maximum velocity in a given deformation for computing delta-->"<< endl;
				std::ostringstream outSL1;
				outSL1 << maxPerturbation1DoubleSpinBox->value();
				std::string SL1_str = outSL1.str();
				stream <<"\t\t<MaxPert val=\"" << SL1_str.c_str() << "\" />"<< endl;
				std::ostringstream outSL12;
				outSL12 << nbIter1SpinBox->value();
				std::string SL12_str = outSL12.str();
				stream <<"\t\t<NIterations val=\"" << SL12_str.c_str() << "\" />"<< endl;
				stream <<"\t\t<DiffOper>"<< endl;
					std::ostringstream outSL13;
					outSL13 << alpha1DoubleSpinBox->value();
					std::string SL13_str = outSL13.str();
					stream <<"\t\t\t<Alpha val=\"" << SL13_str.c_str() << "\" />"<< endl;
					std::ostringstream outSL14;
					outSL14 << beta1DoubleSpinBox->value();
					std::string SL14_str = outSL14.str();
					stream <<"\t\t\t<Beta val=\"" << SL14_str.c_str() << "\" />"<< endl;
					std::ostringstream outSL15;
					outSL15 << gamma1DoubleSpinBox->value();
					std::string SL15_str = outSL15.str();
					stream <<"\t\t\t<Gamma val=\"" << SL15_str.c_str() << "\" />"<< endl;
				stream <<"\t\t</DiffOper>"<< endl;
			stream <<"\t</GreedyAtlasScaleLevel>"<< endl;
			}

			stream <<"\t<!--number of threads to use, 0=one per processor (only for CPU computation)-->"<< endl;
			stream <<"\t<nThreads val=\"4\" />"<< endl;
			stream <<"\t<OutputImageNamePrefix val=\"" << m_OutputPath << "/DTIAtlas/2_NonLinear_Registration_AW/AverageImage\" />"<< endl;
			stream <<"\t<OutputDeformedImageNamePrefix val=\"" << m_OutputPath << "/DTIAtlas/2_NonLinear_Registration_AW/DeformedImage_\" />"<< endl;
			stream <<"\t<OutputHFieldImageNamePrefix val=\"" << m_OutputPath << "/DTIAtlas/2_NonLinear_Registration_AW/DeformationField_\" />"<< endl;
			stream <<"\t<OutputInvHFieldImageNamePrefix val=\"" << m_OutputPath << "/DTIAtlas/2_NonLinear_Registration_AW/InverseDeformationField_\" />"<< endl;
			stream <<"\t<OutputFileType val=\"mhd\" />"<< endl;

		stream <<"</ParameterFile>"<< endl;
	}
	else qDebug( "Could not create xml file");
}

  /////////////////////////////////////////
 //         SOFT CONFIGURATION          //
/////////////////////////////////////////

void GUI::LoadConfigSlot() /*SLOT*/
{
	QString ConfigBrowse=QFileDialog::getOpenFileName(this, "Open Configuration File", QString(), ".txt Files (*.txt)");

	if(!ConfigBrowse.isEmpty())
	{
		LoadConfig(ConfigBrowse);
	}
}

int GUI::LoadConfig(QString configFile) // returns -1 if fails, otherwise 0
{
	if( access(configFile.toStdString().c_str(), F_OK) == 0 ) // Test if the config file exists => unistd::access() returns 0 if F(file)_OK
	{
		if(!m_Quiet) std::cout<<"| Loading Configuration file \'"<< configFile.toStdString() <<"\'..."; // command line display

		std::string notFound;

		///get the values from file
		QFile file(configFile);
		if (file.open(QFile::ReadOnly))
		{
			QTextStream stream(&file);

			QString line = stream.readLine();
			QStringList list = line.split("=");
			if(!list.at(0).contains(QString("ImageMath")))
			{
				if(!m_noGUI) 
				{
					QMessageBox::critical(this, "Corrupt File", "This config file is corrupted");
					if(!m_Quiet) std::cout<<"FAILED"<<std::endl; // command line display
				}
				else if(!m_Quiet) std::cout<<"FAILED"<<std::endl<<"| This config file is corrupted"<<std::endl;
				return -1;
			}
			if(!list.at(1).isEmpty()) ImagemathPath->setText(list.at(1));
			else if(ImagemathPath->text().isEmpty()) notFound = notFound + "> ImageMath\n";	

			line = stream.readLine();
			list = line.split("=");
			if(!list.at(0).contains(QString("ResampleDTIlogEuclidean")))
			{
				if(!m_noGUI) 
				{
					QMessageBox::critical(this, "Corrupt File", "This config file is corrupted");
					if(!m_Quiet) std::cout<<"FAILED"<<std::endl; // command line display
				}
				else if(!m_Quiet) std::cout<<"FAILED"<<std::endl<<"| This config file is corrupted"<<std::endl;
				return -1;
			}
			if(!list.at(1).isEmpty()) ResampPath->setText(list.at(1));
			else if(ResampPath->text().isEmpty()) notFound = notFound + "> ResampleDTIlogEuclidean\n";

			line = stream.readLine();
			list = line.split("=");
			if(!list.at(0).contains(QString("CropDTI")))
			{
				if(!m_noGUI) 
				{
					QMessageBox::critical(this, "Corrupt File", "This config file is corrupted");
					if(!m_Quiet) std::cout<<"FAILED"<<std::endl; // command line display
				}
				else if(!m_Quiet) std::cout<<"FAILED"<<std::endl<<"| This config file is corrupted"<<std::endl;
				return -1;
			}
			if(!list.at(1).isEmpty()) CropDTIPath->setText(list.at(1));
			else if(CropDTIPath->text().isEmpty()) notFound = notFound + "> CropDTI\n";

			line = stream.readLine();
			list = line.split("=");
			if(!list.at(0).contains(QString("dtiprocess")))
			{
				if(!m_noGUI) 
				{
					QMessageBox::critical(this, "Corrupt File", "This config file is corrupted");
					if(!m_Quiet) std::cout<<"FAILED"<<std::endl; // command line display
				}
				else if(!m_Quiet) std::cout<<"FAILED"<<std::endl<<"| This config file is corrupted"<<std::endl;
				return -1;
			}
			if(!list.at(1).isEmpty()) dtiprocPath->setText(list.at(1));
			else if(dtiprocPath->text().isEmpty()) notFound = notFound + "> dtiprocess\n";

			line = stream.readLine();
			list = line.split("=");
			if(!list.at(0).contains(QString("BRAINSFit")))
			{
				if(!m_noGUI) 
				{
					QMessageBox::critical(this, "Corrupt File", "This config file is corrupted");
					if(!m_Quiet) std::cout<<"FAILED"<<std::endl; // command line display
				}
				else if(!m_Quiet) std::cout<<"FAILED"<<std::endl<<"| This config file is corrupted"<<std::endl;
				return -1;
			}
			if(!list.at(1).isEmpty()) BRAINSFitPath->setText(list.at(1));
			else if(BRAINSFitPath->text().isEmpty()) notFound = notFound + "> BRAINSFit\n";


			line = stream.readLine();
			list = line.split("=");
			if(!list.at(0).contains(QString("AtlasWerks")))
			{
				if(!m_noGUI) 
				{
					QMessageBox::critical(this, "Corrupt File", "This config file is corrupted");
					if(!m_Quiet) std::cout<<"FAILED"<<std::endl; // command line display
				}
				else if(!m_Quiet) std::cout<<"FAILED"<<std::endl<<"| This config file is corrupted"<<std::endl;
				return -1;
			}
			bool AWToTest=false;
			if( !list.at(1).isEmpty() )
			{
				AWToTest=true; // call testAW after the display of "DONE"
				AWPath->setText(list.at(1));
			}
			else if(AWPath->text().isEmpty()) notFound = notFound + "> AtlasWerks\n";

			line = stream.readLine();
			list = line.split("=");
			if(!list.at(0).contains(QString("dtiaverage")))
			{
				if(!m_noGUI) 
				{
					QMessageBox::critical(this, "Corrupt File", "This config file is corrupted");
					if(!m_Quiet) std::cout<<"FAILED"<<std::endl; // command line display
				}
				else if(!m_Quiet) std::cout<<"FAILED"<<std::endl<<"| This config file is corrupted"<<std::endl;
				return -1;
			}
			if(!list.at(1).isEmpty()) dtiavgPath->setText(list.at(1));
			else if(dtiavgPath->text().isEmpty()) notFound = notFound + "> dtiaverage\n";

			line = stream.readLine();
			list = line.split("=");
			if(!list.at(0).contains(QString("DTI-Reg")))
			{
				if(!m_noGUI) 
				{
					QMessageBox::critical(this, "Corrupt File", "This config file is corrupted");
					if(!m_Quiet) std::cout<<"FAILED"<<std::endl; // command line display
				}
				else if(!m_Quiet) std::cout<<"FAILED"<<std::endl<<"| This config file is corrupted"<<std::endl;
				return -1;
			}
			bool DTIRegToTest=false;
			if( !list.at(1).isEmpty() )
			{
				DTIRegToTest=true; // call testDTIReg after the display of "DONE"
				DTIRegPath->setText(list.at(1));
			}
			else if(DTIRegPath->text().isEmpty()) notFound = notFound + "> DTI-Reg\n";

			line = stream.readLine();
			list = line.split("=");
			if(!list.at(0).contains(QString("unu")))
			{
				if(!m_noGUI) 
				{
					QMessageBox::critical(this, "Corrupt File", "This config file is corrupted");
					if(!m_Quiet) std::cout<<"FAILED"<<std::endl; // command line display
				}
				else if(!m_Quiet) std::cout<<"FAILED"<<std::endl<<"| This config file is corrupted"<<std::endl;
				return -1;
			}
			if(!list.at(1).isEmpty()) unuPath->setText(list.at(1));
			else if(unuPath->text().isEmpty()) notFound = notFound + "> unu\n";

			line = stream.readLine();
			list = line.split("=");
			if(!list.at(0).contains(QString("MriWatcher")))
			{
				if(!m_noGUI) 
				{
					QMessageBox::critical(this, "Corrupt File", "This config file is corrupted");
					if(!m_Quiet) std::cout<<"FAILED"<<std::endl; // command line display
				}
				else if(!m_Quiet) std::cout<<"FAILED"<<std::endl<<"| This config file is corrupted"<<std::endl;
				return -1;
			}
			if(!list.at(1).isEmpty()) MriWatcherPath->setText(list.at(1));
			else if(MriWatcherPath->text().isEmpty()) notFound = notFound + "> MriWatcher\n";

			if(!m_Quiet) std::cout<<"DONE"<<std::endl; // command line display

			if(m_FromConstructor!=1) // do not test when from constructor -> test at the end of it
			{
				if( !notFound.empty() )
				{
					if(!m_noGUI) 
					{
						std::string text = "The following programs are missing.\nPlease enter the path manually:\n" + notFound;
						QMessageBox::warning(this, "Program missing", QString(text.c_str()) );
					}
					else
					{
						if(!m_Quiet) std::cout<<"| The following programs have not been found. Please give a configuration file or modify it or enter the path manually in the GUI:\n"<< notFound <<std::endl;
						return -1;
					}
				}

				if(AWToTest) testAW();  // do not test AW path if 'LoadConfig' called from constructor -> test at the end of constructor
				if(DTIRegToTest) testDTIReg();  // do not test DTIReg path if 'LoadConfig' called from constructor -> test at the end of constructor
			}

		} 
		else qDebug( "Could not open file");
	}
	else if(!m_Quiet) std::cout<<"| The given configuration file does not exist"<<std::endl; // command line display

	return 0;
}

void GUI::SaveConfig() /*SLOT*/
{
	QString ConfigBrowseName=QFileDialog::getSaveFileName(this, tr("Save Configuration File"),"./DTIAtlasBuilderSoftConfiguration.txt",tr("Text File (*.txt)"));

	if(!ConfigBrowseName.isEmpty())
	{
/* getting the values and generating the config file */

		QFile file(ConfigBrowseName);
		if ( file.open( IO_WriteOnly | IO_Translate ) )
		{
			if(!m_Quiet) std::cout<<"| Generating config file..."; // command line display

			QTextStream stream( &file );

			stream << "ImageMath=" << ImagemathPath->text() << endl;
			stream << "ResampleDTIlogEuclidean=" << ResampPath->text() << endl;
			stream << "CropDTI=" << CropDTIPath->text() << endl;
			stream << "dtiprocess=" << dtiprocPath->text() << endl;
			stream << "BRAINSFit=" << BRAINSFitPath->text() << endl;
			stream << "AtlasWerks=" << AWPath->text() << endl;
			stream << "dtiaverage=" << dtiavgPath->text() << endl;
			stream << "DTI-Reg=" << DTIRegPath->text() << endl;
			stream << "unu=" << unuPath->text() << endl;
			stream << "MriWatcher=" << unuPath->text() << endl;

			if(!m_Quiet) std::cout<<"DONE"<<std::endl; // command line display
		}
		else 
		{
			if(!m_Quiet) std::cout<<"FAILED"<<std::endl; // command line display
			qDebug( "Could not create config file");
		}
	}
}

void GUI::ConfigDefault() /*SLOT*/
{
/* /tools/ITK/ITKv4.1.0/ITKv4.1.0_linux64/include/ITK-4.1/itksys/SystemTools.hxx */
/**
   * Find an executable in the system PATH, with optional extra paths
*/
/*  static kwsys_stl::string FindProgram(
    const char* name,
    const kwsys_stl::vector<kwsys_stl::string>& path = 
    kwsys_stl::vector<kwsys_stl::string>(),
    bool no_system_path = false);
*/

	if(!m_Quiet) std::cout<<"| Searching the softwares..."; // command line display

	std::string program;
	std::string notFound;

	program = itksys::SystemTools::FindProgram("ImageMath");
	if(program.empty()) { if(ImagemathPath->text().isEmpty()) notFound = notFound + "> ImageMath\n"; }
	else ImagemathPath->setText(QString(program.c_str()));

	program = itksys::SystemTools::FindProgram("ResampleDTIlogEuclidean");
	if(program.empty()) { if(ResampPath->text().isEmpty()) notFound = notFound + "> ResampleDTIlogEuclidean\n"; }
	else ResampPath->setText(QString(program.c_str()));

	program = itksys::SystemTools::FindProgram("CropDTI");
	if(program.empty()) { if(CropDTIPath->text().isEmpty()) notFound = notFound + "> CropDTI\n"; }
	else CropDTIPath->setText(QString(program.c_str()));

	program = itksys::SystemTools::FindProgram("dtiprocess");
	if(program.empty()) { if(dtiprocPath->text().isEmpty()) notFound = notFound + "> dtiprocess\n"; }
	else dtiprocPath->setText(QString(program.c_str()));

	program = itksys::SystemTools::FindProgram("BRAINSFit");
	if(program.empty()) { if(BRAINSFitPath->text().isEmpty()) notFound = notFound + "> BRAINSFit\n"; }
	else BRAINSFitPath->setText(QString(program.c_str()));

	bool AWToTest=false;
	program = itksys::SystemTools::FindProgram("AtlasWerks");
	if(program.empty()) { if(AWPath->text().isEmpty()) notFound = notFound + "> AtlasWerks\n"; }
	else
	{
		AWToTest=true; // call testAW after the display of "DONE"
		AWPath->setText(QString(program.c_str()));	
	}

	program = itksys::SystemTools::FindProgram("dtiaverage");
	if(program.empty()) { if(dtiavgPath->text().isEmpty()) notFound = notFound + "> dtiaverage\n"; }
	else dtiavgPath->setText(QString(program.c_str()));

	bool DTIRegToTest=false;
	program = itksys::SystemTools::FindProgram("DTI-Reg_1.1.2");
	if(program.empty()) program = itksys::SystemTools::FindProgram("DTI-Reg"); // if 1.1.2 not found, look for "DTI-Reg"
	if(program.empty()) { if(DTIRegPath->text().isEmpty()) notFound = notFound + "> DTI-Reg\n"; }
	else
	{
		DTIRegToTest=true; // call testDTIReg after the display of "DONE"
		DTIRegPath->setText(QString(program.c_str()));	
	}

	program = itksys::SystemTools::FindProgram("unu");
	if(program.empty()) { if(unuPath->text().isEmpty()) notFound = notFound + "> unu\n"; }
	else unuPath->setText(QString(program.c_str()));

	program = itksys::SystemTools::FindProgram("MriWatcher");
	if(program.empty()) { if(MriWatcherPath->text().isEmpty()) notFound = notFound + "> MriWatcher\n"; }
	else MriWatcherPath->setText(QString(program.c_str()));

	if(!m_Quiet) std::cout<<"DONE"<<std::endl; // command line display

	if(m_FromConstructor!=1) // do not test when from constructor -> test at the end of it
	{
		if( !notFound.empty() )
		{
			if(!m_noGUI) 
			{
				std::string text = "The following programs have not been found.\nPlease enter the path manually or open a configuration file:\n" + notFound;
				QMessageBox::warning(this, "Program missing", QString(text.c_str()) );
			}
			else if(!m_Quiet) std::cout<<"| The following programs have not been found. Please give a configuration file or modify it or enter the path manually in the GUI:\n"<< notFound <<std::endl;
		}

		if(AWToTest) testAW();  // do not test AW path if 'Default' called from constructor -> test at the end of constructor
		if(DTIRegToTest) testDTIReg();  // do not test DTIReg path if 'Default' called from constructor -> test at the end of constructor
	}
}

void GUI::BrowseSoft(int soft)  /*SLOT*/ //softwares: 1=ImageMath, 2=ResampleDTIlogEuclidean, 3=CropDTI, 4=dtiprocess, 5=BRAINSFit, 6=AtlasWerks, 7=dtiaverage, 8=DTI-Reg, 9=unu, 10=MriWatcher
{
	QString SoftBrowse = QFileDialog::getOpenFileName(this, "Open Software", QString(), "Executable Files (*)");

	if(!SoftBrowse.isEmpty())
	{
		switch (soft)
		{
		case 1: ImagemathPath->setText(SoftBrowse);
			break;
		case 2: ResampPath->setText(SoftBrowse);
			break;
		case 3: CropDTIPath->setText(SoftBrowse);
			break;
		case 4: dtiprocPath->setText(SoftBrowse);
			break;
		case 5: BRAINSFitPath->setText(SoftBrowse);
			break;
		case 6: {
			AWPath->setText(SoftBrowse);
			testAW();
			}
			break;
		case 7: dtiavgPath->setText(SoftBrowse);
			break;
		case 8: {
			DTIRegPath->setText(SoftBrowse);
			testDTIReg();
			}
			break;
		case 9: unuPath->setText(SoftBrowse);
			break;
		case 10: MriWatcherPath->setText(SoftBrowse);
			break;
		}
	}
}

void GUI::ResetSoft(int softindex) /*SLOT*/ //softwares: 1=ImageMath, 2=ResampleDTIlogEuclidean, 3=CropDTI, 4=dtiprocess, 5=BRAINSFit, 6=AtlasWerks, 7=dtiaverage, 8=DTI-Reg, 9=unu, 10=MriWatcher
{
	std::string soft;

	switch (softindex)
	{
	case 1: soft="ImageMath";
		break;
	case 2: soft="ResampleDTIlogEuclidean";
		break;
	case 3: soft="CropDTI";
		break;
	case 4: soft="dtiprocess";
		break;
	case 5:	soft="BRAINSFit";
		break;
	case 6: soft="AtlasWerks";
		break;
	case 7: soft="dtiaverage";
		break;
	case 8: soft="DTI-Reg_1.1.2";
		break;
	case 9: soft="unu";
		break;
	case 10: soft="MriWatcher";
		break;
	}

	if(!m_Quiet) std::cout<<"| Searching the software \'"<< soft <<"\'..."; // command line display

	std::string program = itksys::SystemTools::FindProgram(soft.c_str());

	if(program.empty() && soft=="DTI-Reg_1.1.2") program = itksys::SystemTools::FindProgram("DTI-Reg"); // if 1.1.2 not found, look for "DTI-Reg"


	bool AWToTest=false;
	bool DTIRegToTest=false;
	if(program.empty()) 
	{
		std::string text = "The program \'" + soft + "\' is missing.\nPlease enter the path manually.\n";
		QMessageBox::warning(this, "Program missing", QString(text.c_str()) );
	}
	else 
	{
		if(softindex==1) ImagemathPath->setText(QString(program.c_str()));
		else if(softindex==2) ResampPath->setText(QString(program.c_str()));
		else if(softindex==3) CropDTIPath->setText(QString(program.c_str()));
		else if(softindex==4) dtiprocPath->setText(QString(program.c_str()));
		else if(softindex==5) BRAINSFitPath->setText(QString(program.c_str()));
		else if(softindex==6)
		{
			AWPath->setText(QString(program.c_str()));
			AWToTest=true; // call testAW after the display of "DONE"
		}
		else if(softindex==7) dtiavgPath->setText(QString(program.c_str()));
		else if(softindex==8)
		{
			DTIRegPath->setText(QString(program.c_str()));
			DTIRegToTest=true; // call testDTIReg after the display of "DONE"
		}
		else if(softindex==9) unuPath->setText(QString(program.c_str()));
		else if(softindex==10) MriWatcherPath->setText(QString(program.c_str()));
	}

	if(!m_Quiet) std::cout<<"DONE"<<std::endl; // command line display

	if(m_FromConstructor!=1) // do not test paths if 'Default' called from constructor -> test at the end of constructor
	{
		if(AWToTest) testAW();
		if(DTIRegToTest) testDTIReg();
	}
}

int GUI::testAW() /*SLOT*/ // returns 0 if version ok, -1 if bad version
{
	QProcess * Process = new QProcess;
	std::string program;
	program = AWPath->text().toStdString() + " --version";

	if(!m_Quiet) std::cout<<"| Testing the version of AtlasWerks...";

	Process->start( program.c_str() ); // try to find the version => returns nothing if not the right version

	Process->waitForReadyRead();
	QByteArray BA =  Process->readAllStandardOutput();

	if(!m_Quiet) std::cout<<"DONE"<<std::endl;

	if(BA.size()==0) // if notinh displayed in std output, '--version' does not exists so it is not the right version
	{
		if(!m_noGUI) 
		{
			std::string text = "The version of AtlasWerks \'" + AWPath->text().toStdString() + "\' is not the right one.\nPlease give a version supporting a XML file (--paramFile).\n";
			QMessageBox::warning(this, "Wrong version", QString(text.c_str()) );
		}
		else if(!m_Quiet) std::cout<<"| The version of AtlasWerks \'" << AWPath->text().toStdString() << "\' is not the right one. Please give a version supporting a XML file (--paramFile)."<<std::endl;

		return -1;
	}

	return 0;
}

int GUI::testDTIReg() /*SLOT*/ // returns 0 if version ok, -1 if bad version
{
	QProcess * Process = new QProcess;
	std::string program;
	program = DTIRegPath->text().toStdString() + " --version";

	if(!m_Quiet) std::cout<<"| Testing the version of DTI-Reg...";

	Process->start( program.c_str() ); // try to find the version => returns nothing if not the right version

	Process->waitForReadyRead();
	QByteArray BA =  Process->readAllStandardOutput();

	QString text = QString( BA.data() );

	if(!m_Quiet) std::cout<<"DONE"<<std::endl;

	if( BA.size()==0 || text.contains("1.0") || ( text.contains("1.1") && !text.contains("1.1.") ) || text.contains("1.1.1") ) //old version -> NOK
	{
		if(!m_noGUI) 
		{
			std::string text = "The version of DTI-Reg \'" + DTIRegPath->text().toStdString() + "\' is not the right one.\nPlease give a version older than 1.1.2.\n";
			QMessageBox::warning(this, "Wrong version", QString(text.c_str()) );
		}
		else if(!m_Quiet) std::cout<<"| The version of DTI-Reg \'" << DTIRegPath->text().toStdString() << "\' is not the right one. Please give a version older than 1.1.2."<<std::endl;

		return -1;
	}

	return 0;
}

  /////////////////////////////////////////
 //               READ ME               //
/////////////////////////////////////////

void GUI::ReadMe()  /*SLOT*/ /////to UPDATE
{
/*	QProcess * Process = new QProcess;
	std::string program = "gedit /home/akaiser/Desktop/Projects/DTIAtlasBuilderGUI_07-12/DTIABGUIFinal_07-18-12/src/README.md";
	if(!m_Quiet) std::cout<<"| $ " << program << std::endl;
	Process->execute( program.c_str() );
*/
	QDialog *dlg = new QDialog(this);
	dlg->setWindowTitle ("Read Me");

	std::string info = "DTIAtlasBuilder\n===============\n\nA tool to create an atlas from several DTI images\n\nThese Softwares need to be installed before executing the tool :\n- ImageMath\n- ResampleDTIlogEuclidean\n- CropDTI\n- dtiprocess\n- BRAINSFit\n- AtlasWerks\n- dtiaverage\n- DTI-Reg\n- unu\n- MriWatcher\n";
	QLabel *InfoLabel = new QLabel (info.c_str(), this);
	QVBoxLayout *VLayout = new QVBoxLayout();
	VLayout->addWidget(InfoLabel);

	dlg->setLayout(VLayout);

	dlg->setVisible(!dlg->isVisible()); // display the window
}

  /////////////////////////////////////////
 //         FINAL RESAMP PARAM          //
/////////////////////////////////////////

void GUI::InterpolTypeComboBoxChanged(int index)  /*SLOT*/ // index: 0=Linear, 1=Nearest Neighborhoor, 2=Windowed Sinc, 3=BSpline
{
	switch (index)
	{
	case 0:	m_optionStackLayout->setCurrentIndex(0);
		break;
	case 1:	m_optionStackLayout->setCurrentIndex(0);
		break;
	case 2:	m_optionStackLayout->setCurrentIndex(1);
		break;
	case 3:	m_optionStackLayout->setCurrentIndex(2);
		break;
	}
}

void GUI::TensorInterpolComboBoxChanged(int index) /*SLOT*/ // 0= log, 1= nolog
{
	switch (index)
	{
	case 0:	m_logOptionStackLayout->setCurrentIndex(0);
		break;
	case 1:	m_logOptionStackLayout->setCurrentIndex(1);
		break;
	}
}

  /////////////////////////////////////////
 //               DTI-REG               //
/////////////////////////////////////////

void GUI::RegMethodComboBoxChanged(int index) /*SLOT*/
{
	switch (index)
	{
	case 0:	m_DTIRegOptionStackLayout->setCurrentIndex(0);
		break;
	case 1:	m_DTIRegOptionStackLayout->setCurrentIndex(1);
		break;
	}
}

void GUI::SimMetChanged(int index) /*SLOT*/
{
	switch (index)
	{
	case 0:	{
		m_SimParamDble->setValue(2); //CC
		m_SimParamLabel->setText(QString("Region Radius:"));
		}
		break;
	case 1:	{
		m_SimParamDble->setValue(32); //MI
		m_SimParamLabel->setText(QString("Number of bins:"));
		}
		break;
	case 2: m_SimParamLabel->setText(QString("Similarity Parameter:")); //MSQ
		break;
	}
}

void GUI::ANTSRegTypeChanged(int index) /*SLOT*/
{
	switch (index)
	{
	case 2:	m_TfmStepLine->setText("0.25"); //GreedyDiffeo
		break;
	case 3:	m_TfmStepLine->setText("0.25,5,0.01"); //SpatioTempDiffeo
		break;
	}
}

  /////////////////////////////////////////
 //         WIDGET CHANGE SLOT          //
/////////////////////////////////////////

void GUI::WidgetHasChangedParamNoSaved() /*SLOT*/ //called when any widget is changed
{
	m_ParamSaved=0;
}

  /////////////////////////////////////////
 //           CHECK IMAGE OK            //
/////////////////////////////////////////

int GUI::checkImage(std::string Image) // returns 1 if not an image, 2 if not a dti, otherwise 0
{
	typedef itk::Image < double , 4 > ImageType; //itk type for image
	typedef itk::ImageFileReader <ImageType> ReaderType; //itk reader class to open an image
	ReaderType::Pointer reader=ReaderType::New();
	ImageType::RegionType region;

	reader->SetFileName( Image );

	try{
		reader->UpdateOutputInformation();
	}
	catch(itk::ExceptionObject & err)
	{
		return 1; // file is not an image
	}

	itk::ImageIOBase::IOPixelType pixel  = reader->GetImageIO()->GetPixelType() ;
	if( pixel == itk::ImageIOBase::SYMMETRICSECONDRANKTENSOR || pixel == itk::ImageIOBase::DIFFUSIONTENSOR3D || pixel == itk::ImageIOBase::VECTOR ) return 0; // test if DTI
	return 2;
}

  /////////////////////////////////////////
 //           MAIN FUNCTIONS            //
/////////////////////////////////////////

void GUI::Compute() /*SLOT*/
{
	if( m_ErrorDetectedInConstructor && m_noGUI ) Exit();
	else
	{

	if(CaseListWidget->count()==0)
	{
		if(!m_noGUI) QMessageBox::critical(this, "No Cases", "Please give at least one case");
		else if(!m_Quiet) std::cout<<"| No Cases: Please give at least one case"<<std::endl;
	}
	else // OK Case
	{
	
	if(OutputFolderLineEdit->text().isEmpty())
	{
		if(!m_noGUI) QMessageBox::critical(this, "No Output Folder", "Please give an output folder");
		else if(!m_Quiet) std::cout<<"| No Output Folder: Please give an output folder"<<std::endl;
	}
	else // OK Output
	{

		if(LaunchScriptWriter()==-1) 
		{
			if(!m_Quiet) std::cout<<"| Clearing previous cases in vectors..."<<std::endl; // command line display
			m_CasesPath.clear();
			m_scriptwriter->clearCasesPath();
			if(m_noGUI) Exit(); // no possibility to change because no GUI so QUIT
			return;
		}
		if(!m_Quiet) std::cout<<"| Clearing previous cases in vectors..."<<std::endl; // command line display
		m_CasesPath.clear();
		m_scriptwriter->clearCasesPath();
		LaunchScriptRunner();

	} // else of if(OutputFolderLineEdit->text().isEmpty())

	} // else of if[Case]

	if(m_noGUI) Exit(); // Only 1 compute in nogui mode

	} // else of if( m_ErrorDetectedInConstructor && m_noGUI )
}

int GUI::LaunchScriptWriter()
{
/* Variables for the QProcesses */
	int ExitCode=0;
	std::string program;

/* Checking and Setting the values */

/* Cases */
	for(int i=0; i < CaseListWidget->count() ;i++) 
	{
		std::string CurrentCase = CaseListWidget->item(i)->text().toStdString().substr( CaseListWidget->item(i)->text().split(":").at(0).size()+2 );
		if( access(CurrentCase.c_str(), F_OK) != 0 ) // Test if the case files exist => unistd::access() returns 0 if F(file)_OK
		{
			if(!m_noGUI)
			{
				std::string text = "This file does not exist :\n" + CurrentCase;
				QMessageBox::critical(this, "Case does not exist", QString(text.c_str()) );
			}
			else if(!m_Quiet) std::cout<<"| This file does not exist : " << CurrentCase <<std::endl;
			return -1;
		}
		int checkIm = checkImage(CurrentCase); // returns 1 if not an image, 2 if not a dti, otherwise 0
		if( checkIm == 1 ) // returns 1 if not an image, 2 if not a dti, otherwise 0
		{
			if(!m_noGUI)
			{
				std::string text = "This file is not an image :\n" + CurrentCase;
				QMessageBox::critical(this, "No image", QString(text.c_str()) );
			}
			else if(!m_Quiet) std::cout<<"| This file is not an image : " << CurrentCase <<std::endl;
			return -1;
		}
		if( checkIm == 2 ) // returns 1 if not an image, 2 if not a dti, otherwise 0
		{
			if(!m_noGUI)
			{
				std::string text = "This image is not a DTI :\n" + CurrentCase;
				QMessageBox::critical(this, "No DTI", QString(text.c_str()) );
			}
			else if(!m_Quiet) std::cout<<"| This image is not a DTI : " << CurrentCase <<std::endl;
			return -1;
		}
		m_CasesPath.push_back( CurrentCase );
	}
	m_scriptwriter->setCasesPath(m_CasesPath); // m_CasesPath is a vector

/* Output */
	m_OutputPath=OutputFolderLineEdit->text();
	if( access(m_OutputPath.toStdString().c_str(), F_OK) != 0 ) // Test if the folder exists => unistd::access() returns 0 if F(file)_OK
	{
		if(!m_noGUI) QMessageBox::critical(this, "No Output Folder", "The output folder does not exist");
		else if(!m_Quiet) std::cout<<"| The output folder does not exist"<<std::endl;
		return -1;
	}
	if( access(m_OutputPath.toStdString().c_str(), W_OK) != 0 ) // Test if the program can write in the output folder => unistd::access() returns 0 if W(write)_OK
	{
		if(!m_noGUI) QMessageBox::critical(this, "Output Folder Unwritable", "Please give an output folder authorized in reading");
		else if(!m_Quiet) std::cout<<"| Please give an output folder authorized in reading"<<std::endl;
		return -1;
	}
	m_scriptwriter->setOutputPath(m_OutputPath.toStdString());

	if( access((m_OutputPath.toStdString() + "/DTIAtlas").c_str(), F_OK) != 0 ) // Test if the main folder does not exists => unistd::access() returns 0 if F(file)_OK
	{
		if(!m_Quiet) std::cout<<"| Creating Main directory..."<<std::endl; // command line display
		QProcess * mkdirMainProcess = new QProcess;
		program = "mkdir " + m_OutputPath.toStdString() + "/DTIAtlas"; //  Creates the directory
		if(!m_Quiet) std::cout<<"| $ " << program << std::endl;
		ExitCode = mkdirMainProcess->execute( program.c_str() );
	}

	if( access((m_OutputPath.toStdString() + "/DTIAtlas/Script").c_str(), F_OK) != 0 ) // Test if the script folder does not exists => unistd::access() returns 0 if F(file)_OK
	{
		if(!m_Quiet) std::cout<<"| Creating Script directory..."<<std::endl; // command line display
		QProcess * mkdirScriptProcess = new QProcess;
		program = "mkdir " + m_OutputPath.toStdString() + "/DTIAtlas/Script";
		if(!m_Quiet) std::cout<<"| $ " << program << std::endl;
		ExitCode = mkdirScriptProcess->execute( program.c_str() );
	}

/* Template */
	if (!TemplateLineEdit->text().isEmpty()) 
	{
		if( access(TemplateLineEdit->text().toStdString().c_str(), F_OK) != 0 ) // Test if the case files exist => unistd::access() returns 0 if F(file)_OK
		{
			if(!m_noGUI)
			{
				std::string text = "This template file does not exist :\n" + TemplateLineEdit->text().toStdString();
				QMessageBox::critical(this, "Template does not exist", QString(text.c_str()) );
			}
			else if(!m_Quiet) std::cout<<"| This template file does not exist : " << TemplateLineEdit->text().toStdString() <<std::endl;
			return -1;
		}
		int checkImTemp = checkImage(TemplateLineEdit->text().toStdString()); // returns 1 if not an image, 2 if not a dti, otherwise 0
		if( checkImTemp == 1 ) // returns 1 if not an image, 2 if not a dti, otherwise 0
		{
			if(!m_noGUI)
			{
				std::string text = "This template file is not an image :\n" + TemplateLineEdit->text().toStdString();
				QMessageBox::critical(this, "No image", QString(text.c_str()) );
			}
			else if(!m_Quiet) std::cout<<"| This template file is not an image : " << TemplateLineEdit->text().toStdString() <<std::endl;
			return -1;
		}
		if( checkImTemp == 0 ) // returns 1 if not an image, 2 if not a dti, otherwise 0 //the template has to be a FA image !!
		{
			if(!m_noGUI)
			{
				std::string text = "This template image is not a FA :\n" + TemplateLineEdit->text().toStdString();
				QMessageBox::critical(this, "No FA", QString(text.c_str()) );
			}
			else if(!m_Quiet) std::cout<<"| This template image is not a FA : " << TemplateLineEdit->text().toStdString() <<std::endl;
			return -1;
		}
		m_scriptwriter->setRegType(0);
		m_scriptwriter->setTemplatePath(TemplateLineEdit->text().toStdString());
	}

	else m_scriptwriter->setRegType(1); // default
	
/* Resamp options */
	m_scriptwriter->setInterpolType(InterpolTypeComboBox->currentText().toStdString());
	if( InterpolTypeComboBox->currentText()==QString("Windowed Sinc") ) m_scriptwriter->setInterpolOption(m_windowComboBox->currentText().toStdString());
	else if( InterpolTypeComboBox->currentText()==QString("BSpline") ) m_scriptwriter->setInterpolOption(m_BSplineComboBox->currentText().toStdString());

	m_scriptwriter->setTensInterpol(TensInterpolComboBox->currentText().toStdString());
	if( TensInterpolComboBox->currentText()==QString("Non Log Euclidean") ) m_scriptwriter->setInterpolLogOption(m_nologComboBox->currentText().toStdString());

	m_scriptwriter->setTensorTfm(TensTfmComboBox->currentText().toStdString());

	m_scriptwriter->setAverageStatMethod(averageStatMethodComboBox->currentText().toStdString());

/* Final Resamp options */

	std::vector < std::string > DTIRegOptions;

	DTIRegOptions.push_back(RegMethodcomboBox->currentText().toStdString());

	if( RegMethodcomboBox->currentText()==QString("ANTS") ) 
	{
		DTIRegOptions.push_back(m_ARegTypeComboBox->currentText().toStdString());
		DTIRegOptions.push_back(m_TfmStepLine->text().toStdString());
		DTIRegOptions.push_back(m_IterLine->text().toStdString());
		DTIRegOptions.push_back(m_SimMetComboBox->currentText().toStdString());

		std::ostringstream out;
		out << m_SimParamDble->value();
		std::string SimParam_str = out.str();
		DTIRegOptions.push_back(SimParam_str);

		std::ostringstream out1;
		out1 << m_GSigmaDble->value();
		std::string GSigma_str = out1.str();
		DTIRegOptions.push_back(GSigma_str);

		if(m_SmoothOffCheck->isChecked()) DTIRegOptions.push_back("1");
		else DTIRegOptions.push_back("0");
	}
	else //BRAINS
	{
		DTIRegOptions.push_back(m_BRegTypeComboBox->currentText().toStdString());
		DTIRegOptions.push_back(m_TfmModeComboBox->currentText().toStdString());

		std::ostringstream out;
		out << m_SigmaDble->value();
		std::string Sigma_str = out.str();
		DTIRegOptions.push_back(Sigma_str);

		std::ostringstream out1;
		out1 << m_NbPyrLevSpin->value();
		std::string NbPyrLev_str = out1.str();
		DTIRegOptions.push_back(NbPyrLev_str);

		DTIRegOptions.push_back(m_PyrLevItLine->text().toStdString());
	}

	m_scriptwriter->setDTIRegOptions(DTIRegOptions);
	DTIRegOptions.clear();

/* Software paths */
/* Checking if all the programs have been given */
	if(ImagemathPath->text().isEmpty() || ResampPath->text().isEmpty() || CropDTIPath->text().isEmpty() || dtiprocPath->text().isEmpty() || BRAINSFitPath->text().isEmpty() || AWPath->text().isEmpty() || dtiavgPath->text().isEmpty() || DTIRegPath->text().isEmpty() || unuPath->text().isEmpty() || MriWatcherPath->text().isEmpty()) // if any path is missing => check in the config file and in the PATH
	{
		char * value = getenv("DTIAtlasBuilderSoftPath");
		if (value) LoadConfig( QString(value) ); // replace the paths by the paths given in the config file

		std::string programPath;
		std::string notFound;

		if(ImagemathPath->text().isEmpty())
		{
			programPath = itksys::SystemTools::FindProgram("ImageMath");
			if(programPath.empty()) notFound = notFound + "> ImageMath\n";
			else ImagemathPath->setText(QString(programPath.c_str()));
		}
		if(ResampPath->text().isEmpty())
		{
			programPath = itksys::SystemTools::FindProgram("ResampleDTIlogEuclidean");
			if(programPath.empty()) notFound = notFound + "> ResampleDTIlogEuclidean\n";
			else ResampPath->setText(QString(programPath.c_str()));
		}
		if(CropDTIPath->text().isEmpty())
		{
			programPath = itksys::SystemTools::FindProgram("CropDTI");
			if(programPath.empty()) notFound = notFound + "> CropDTI\n";
			else CropDTIPath->setText(QString(programPath.c_str()));
		}
		if(dtiprocPath->text().isEmpty())
		{
			programPath = itksys::SystemTools::FindProgram("dtiprocess");
			if(programPath.empty()) notFound = notFound + "> dtiprocess\n";
			else dtiprocPath->setText(QString(programPath.c_str()));
		}
		if(BRAINSFitPath->text().isEmpty())
		{
			programPath = itksys::SystemTools::FindProgram("BRAINSFit");
			if(programPath.empty()) notFound = notFound + "> BRAINSFit\n";
			else BRAINSFitPath->setText(QString(programPath.c_str()));
		}
		if(AWPath->text().isEmpty())
		{
			programPath = itksys::SystemTools::FindProgram("AtlasWerks");
			if(programPath.empty()) notFound = notFound + "> AtlasWerks\n";
			else AWPath->setText(QString(programPath.c_str()));
		}
		if(dtiavgPath->text().isEmpty())
		{
			programPath = itksys::SystemTools::FindProgram("dtiaverage");
			if(programPath.empty()) notFound = notFound + "> dtiaverage\n";
			else dtiavgPath->setText(QString(programPath.c_str()));
		}
		if(DTIRegPath->text().isEmpty())
		{
			programPath = itksys::SystemTools::FindProgram("DTI-Reg_1.1.2");
			if(programPath.empty()) programPath = itksys::SystemTools::FindProgram("DTI-Reg"); // if 1.1.2 not found, look for "DTI-Reg"
			if(programPath.empty()) notFound = notFound + "> DTI-Reg\n";
			else DTIRegPath->setText(QString(programPath.c_str()));
		}
		if(unuPath->text().isEmpty())
		{
			programPath = itksys::SystemTools::FindProgram("unu");
			if(programPath.empty()) notFound = notFound + "> unu\n";
			else unuPath->setText(QString(programPath.c_str()));
		}
		if(MriWatcherPath->text().isEmpty())
		{
			programPath = itksys::SystemTools::FindProgram("MriWatcher");
			if(programPath.empty()) notFound = notFound + "> MriWatcher\n";
			else MriWatcherPath->setText(QString(programPath.c_str()));
		}

		if( !notFound.empty() )
		{
			if(!m_noGUI)
			{
				std::string text = "The following programs are missing.\nPlease enter the path manually:\n" + notFound;
				QMessageBox::critical(this, "Program missing", QString(text.c_str()) );
			}
			else if(!m_Quiet) std::cout<<"| The following programs are missing. Please modify the configuration file or enter the path manually in the GUI:\n" << notFound <<std::endl;
			return -1;
		}
	}

	if(testAW()==-1) return -1;
	if(testDTIReg()==-1) return -1;

/* Checking if the given files are executable */
	if(access(ImagemathPath->text().toStdString().c_str(), X_OK)!=0 )
	{
		if(!m_noGUI)
		{
			std::string text = "The file \'" + ImagemathPath->text().toStdString() + "\' is not executable";
			QMessageBox::critical(this, "Non executable File", QString(text.c_str()) );
		}
		else if(!m_Quiet) std::cout<<"| The file \'" << ImagemathPath->text().toStdString() << "\' is not executable" << std::endl;
		return -1;
	}
	if(access(ResampPath->text().toStdString().c_str(), X_OK) != 0 )
	{
		if(!m_noGUI)
		{
			std::string text = "The file \'" + ResampPath->text().toStdString() + "\' is not executable";
			QMessageBox::critical(this, "Non executable File", QString(text.c_str()) );
		}
		else if(!m_Quiet) std::cout<<"| The file \'" << ImagemathPath->text().toStdString() << "\' is not executable" << std::endl;
		return -1;
	}
	if(access(CropDTIPath->text().toStdString().c_str(), X_OK) != 0 )
	{
		if(!m_noGUI)
		{
			std::string text = "The file \'" + CropDTIPath->text().toStdString() + "\' is not executable";
			QMessageBox::critical(this, "Non executable File", QString(text.c_str()) );
		}
		else if(!m_Quiet) std::cout<<"| The file \'" << ImagemathPath->text().toStdString() << "\' is not executable" << std::endl;
		return -1;
	}
	if(access(dtiprocPath->text().toStdString().c_str(), X_OK) != 0 )
	{
		if(!m_noGUI)
		{
			std::string text = "The file \'" + dtiprocPath->text().toStdString() + "\' is not executable";
			QMessageBox::critical(this, "Non executable File", QString(text.c_str()) );
		}
		else if(!m_Quiet) std::cout<<"| The file \'" << ImagemathPath->text().toStdString() << "\' is not executable" << std::endl;
		return -1;
	}
	if(access(BRAINSFitPath->text().toStdString().c_str(), X_OK) != 0 )
	{
		if(!m_noGUI)
		{
			std::string text = "The file \'" + BRAINSFitPath->text().toStdString() + "\' is not executable";
			QMessageBox::critical(this, "Non executable File", QString(text.c_str()) );
		}
		else if(!m_Quiet) std::cout<<"| The file \'" << ImagemathPath->text().toStdString() << "\' is not executable" << std::endl;
		return -1;
	}
	if(access(AWPath->text().toStdString().c_str(), X_OK) != 0 )
	{
		if(!m_noGUI)
		{
			std::string text = "The file \'" + AWPath->text().toStdString() + "\' is not executable";
			QMessageBox::critical(this, "Non executable File", QString(text.c_str()) );
		}
		else if(!m_Quiet) std::cout<<"| The file \'" << ImagemathPath->text().toStdString() << "\' is not executable" << std::endl;
		return -1;
	}
	if(access(dtiavgPath->text().toStdString().c_str(), X_OK) != 0 )
	{
		if(!m_noGUI)
		{
			std::string text = "The file \'" + dtiavgPath->text().toStdString() + "\' is not executable";
			QMessageBox::critical(this, "Non executable File", QString(text.c_str()) );
		}
		else if(!m_Quiet) std::cout<<"| The file \'" << ImagemathPath->text().toStdString() << "\' is not executable" << std::endl;
		return -1;
	}
	if(access(DTIRegPath->text().toStdString().c_str(), X_OK) != 0 )
	{
		if(!m_noGUI)
		{
			std::string text = "The file \'" + DTIRegPath->text().toStdString() + "\' is not executable";
			QMessageBox::critical(this, "Non executable File", QString(text.c_str()) );
		}
		else if(!m_Quiet) std::cout<<"| The file \'" << ImagemathPath->text().toStdString() << "\' is not executable" << std::endl;
		return -1;
	}
	if(access(unuPath->text().toStdString().c_str(), X_OK) != 0 )
	{
		if(!m_noGUI)
		{
			std::string text = "The file \'" + unuPath->text().toStdString() + "\' is not executable";
			QMessageBox::critical(this, "Non executable File", QString(text.c_str()) );
		}
		else if(!m_Quiet) std::cout<<"| The file \'" << unuPath->text().toStdString() << "\' is not executable" << std::endl;
		return -1;
	}
	if(access(MriWatcherPath->text().toStdString().c_str(), X_OK) != 0 )
	{
		if(!m_noGUI)
		{
			std::string text = "The file \'" + MriWatcherPath->text().toStdString() + "\' is not executable";
			QMessageBox::critical(this, "Non executable File", QString(text.c_str()) );
		}
		else if(!m_Quiet) std::cout<<"| The file \'" << MriWatcherPath->text().toStdString() << "\' is not executable" << std::endl;
		return -1;
	}

	std::vector < std::string > SoftPath;

	SoftPath.push_back(ImagemathPath->text().toStdString());
	SoftPath.push_back(ResampPath->text().toStdString());
	SoftPath.push_back(CropDTIPath->text().toStdString());
	SoftPath.push_back(dtiprocPath->text().toStdString());
	SoftPath.push_back(BRAINSFitPath->text().toStdString());
	SoftPath.push_back(AWPath->text().toStdString());
	SoftPath.push_back(dtiavgPath->text().toStdString());
	SoftPath.push_back(DTIRegPath->text().toStdString());
	SoftPath.push_back(unuPath->text().toStdString());

	m_scriptwriter->setSoftPath(SoftPath);
	SoftPath.clear();

/* Voxel Size and Cropping*/
	if(m_scriptwriter->CheckVoxelSize()==1) 
	{
		if(!m_noGUI) QMessageBox::critical(this, "Different Voxel Sizes", "Error: The voxel size of the images\nare not the same,\nplease change dataset"); // returns 0 if voxel size OK , otherwise 1
		else if(!m_Quiet) std::cout<<"| Error: The voxel size of the images are not the same, please change dataset" << std::endl;
		return -1;
	}
	int Crop=m_scriptwriter->setCroppingSize( SafetyMargincheckBox->isChecked() ); // returns 0 if no cropping , 1 if cropping needed
	if( Crop==1 && !SafetyMargincheckBox->isChecked() )
	{
		if(!m_noGUI) QMessageBox::warning(this, "Cropping", "Warning: The images do not have the same size, \nso some of them will be cropped");
		else if(!m_Quiet) std::cout<<"| Warning: The images do not have the same size, so some of them will be cropped" << std::endl;
	}

/* Other Options */
	if(OverwritecheckBox->isChecked()) m_scriptwriter->setOverwrite(1);
	else m_scriptwriter->setOverwrite(0);

	m_scriptwriter->setnbLoops(NbLoopsSpinBox->value());

	m_scriptwriter->setQuiet(m_Quiet);

	m_scriptwriter->setBFAffineTfmMode(BFAffineTfmModecomboBox->currentText().toStdString());
	
/* Launch writing */
	m_scriptwriter->WriteScript(); // Master Function

/* XML file for AtlasWerks */
	GenerateXMLForAW();

/* Save CSV and parameters */
	SaveCSVResults(Crop,NbLoopsSpinBox->value());
	SaveParameters(m_OutputPath + QString("/DTIAtlas/DTIAtlasBuilderParameters.txt"), m_OutputPath + QString("/DTIAtlas/DTIAtlasBuilderDataset.csv"));

/* Generate Preprocess script file */
	if(!m_Quiet) std::cout<<"| Generating Pre processing script file..."; // command line display

	QString ScriptPath;
	ScriptPath = m_OutputPath + QString("/DTIAtlas/Script/DTIAtlasBuilder_Preprocess.script");
	QFile filePreP(ScriptPath);

	if ( filePreP.open( IO_WriteOnly | IO_Translate ) )
	{
		//file.setPermissions(QFile::ExeOwner); //make the file executable for the owner
		QTextStream stream( &filePreP );
		stream << QString((m_scriptwriter->getScript_Preprocess()).c_str()) << endl;
		if(!m_Quiet) std::cout<<"DONE"<<std::endl; // command line display
	}
	else qDebug( "Could not create file");

/* Generate Atlas Building script file */
	if(!m_Quiet) std::cout<<"| Generating Atlas Building script file..."; // command line display

	ScriptPath = m_OutputPath + QString("/DTIAtlas/Script/DTIAtlasBuilder_AtlasBuilding.script");
	QFile fileAtlas(ScriptPath);

	if ( fileAtlas.open( IO_WriteOnly | IO_Translate ) )
	{
		//file.setPermissions(QFile::ExeOwner); //make the file executable for the owner
		QTextStream stream( &fileAtlas );
		stream << QString((m_scriptwriter->getScript_AtlasBuilding()).c_str()) << endl;
		if(!m_Quiet) std::cout<<"DONE"<<std::endl; // command line display
	}
	else qDebug( "Could not create file");

/* Generate Main script file */
	if(!m_Quiet) std::cout<<"| Generating Main script file..."; // command line display

	ScriptPath = m_OutputPath + QString("/DTIAtlas/Script/DTIAtlasBuilder_Main.script");
	QFile fileMain(ScriptPath);

	if ( fileMain.open( IO_WriteOnly | IO_Translate ) )
	{
		//file.setPermissions(QFile::ExeOwner); // make the file executable for the owner
		QTextStream stream( &fileMain );
		stream << QString((m_scriptwriter->getScript_Main()).c_str()) << endl;
		if(!m_Quiet) std::cout<<"DONE"<<std::endl; // command line display
	}
	else qDebug( "Could not create file");

/* Give the right to user to execute the scripts */
	QProcess * chmodProcess = new QProcess;
	program = "chmod u+x " + m_OutputPath.toStdString() + "/DTIAtlas/Script/DTIAtlasBuilder_Preprocess.script " + m_OutputPath.toStdString() + "/DTIAtlas/Script/DTIAtlasBuilder_AtlasBuilding.script " + m_OutputPath.toStdString() + "/DTIAtlas/Script/DTIAtlasBuilder_Main.script"; // 'chmod u+x = user+execute'
	if(!m_Quiet) std::cout<<"| $ " << program << std::endl;
	ExitCode = chmodProcess->execute( program.c_str() );
	
	return 0;
}

void GUI::LaunchScriptRunner()
{
	if(!m_Quiet) std::cout<<"| Script Running..."<<std::endl; // command line display

/* Running the Script: */
	std::string program;
	program = m_OutputPath.toStdString() + "/DTIAtlas/Script/DTIAtlasBuilder_Main.script";

	if(!m_Quiet) std::cout<<"| $ " << program << std::endl;

	QProcess * ScriptProcess = new QProcess;
	int ExitCode=0;

	ExitCode = ScriptProcess->execute( program.c_str() ); // stuck here during execution of the scripts

	if(ExitCode==0)
	{
		if(!m_Quiet) std::cout<<"| Running Completed !"<<std::endl; // command line display
		QMessageBox::information(this, "Running Completed", "Running Completed !");
	}

	else
	{
		if(!m_Quiet) std::cout<<"| Running Failed..."<<std::endl; // command line display
		QMessageBox::information(this, "Running Failed", "Running Failed...");
	}

/* If need to not freeze the main window : code below 
/!\ Think to uncomment at the top : #include <sys/wait.h>
-> wait() or waitpid() make the process wait for the end of a son => FREEZE !!
*/
/*
	int pid=fork(); // cloning the process : returns the son's pid in the father and 0 in the son
	if(pid==0) // we are in the son
	{
		execl( program.c_str() , program.c_str(), NULL); // we REPLACE the son process by our command while the father is still running normally
	}

	if(!m_noGUI) // if gui, need to NOT freeze the main window: a new son will wait the end of the son running the script and the father will continue to make the main window active
	{
		pid=fork(); // cloning the process : returns the son's pid in the father and 0 in the son
		if(pid==0) // we are in the son
		{
			int ExitCode;
			waitpid( pid, &ExitCode ,0); // waiting for the son to finish (the value of pid is the pid of the son if we are in the father) (the last 0 is the options => not used)

			if(ExitCode==0)
			if(!m_Quiet) std::cout<<"| Running Completed !"<<std::endl; // command line display
			else if(!m_Quiet) std::cout<<"| Running Failed..."<<std::endl; // command line display

			Exit(); // son ends
		}
	}
	else //IF no GUI just make the main process wait (no need to use the gui, so the main program can freeze)
	{
		int ExitCode;
		waitpid( pid, &ExitCode ,0); // waiting for the son to finish (the value of pid is the pid of the son if we are in the father) (the last 0 is the options => not used)

		if(ExitCode==0) if(!m_Quiet) std::cout<<"| Running Completed !"<<std::endl; // command line display
		else if(!m_Quiet) std::cout<<"| Running Failed..."<<std::endl; // command line display

		Exit();
	}
*/
}

