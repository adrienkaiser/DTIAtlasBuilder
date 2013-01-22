#ifndef DEF_GUI
#define DEF_GUI

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

/*std classes*/
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <cstdlib> // for getenv()
#include <signal.h> // for the signal types

/*itk classes*/
#include "itkImage.h"
#include "itkImageFileReader.h"
#include <itksys/SystemTools.hxx> // for FindProgram()

#include "ui_GUIwindow.h"

#include "ScriptWriter.h"

class GUI : public QMainWindow, public Ui::MainWindow 
{
	Q_OBJECT

	public:
		
/*CONSTRUCTOR*/	GUI(std::string ParamFile, std::string ConfigFile, std::string CSVFile, bool overwrite, bool noGUI, std::string commandRan); //constructor

/*INIT*/	void InitOptions();

/*CASES*/	void CheckCasesIndex();

/* QC */	void DisableQC(); // <=> disable QC buttons
/*DATASET*/	int ReadCSV(QString CSVfile); // returns -1 if fails, otherwise 0
		void SaveCSVResults(int Crop, int nbLoops);

/*PARAMETERS*/	int LoadParameters(QString paramFile); // returns -1 if fails, otherwise 0
		void SaveParameters(QString ParamBrowseName,QString CSVFileName);

/*XML FILE*/	void GenerateXMLForAW();
		void GenerateXMLForGA();

/*SOFT CONFIG*/	int LoadConfig(QString ConfigFile); // returns -1 if fails, otherwise 0

/*CHECK IMAGE*/	int checkImage(std::string Image); // returns 1 if not an image, 2 if not a dti, otherwise 0

/*MAIN FUNCT*/	int LaunchScriptWriter(); // returns -1 if failed, otherwise 0
		int LaunchScriptRunner();

		void RunningCompleted();
		void RunningFailed();

/*PROGRESSBAR*/	void ProgressBar(int Progress);

	public slots:

/*CASES*/	void OpenAddCaseBrowseWindow();
		void RemoveSelectedCases();

/*OUTPUT*/	void OpenOutputBrowseWindow();

/*DTIREG*/	void OpenDTIRegExtraPathBrowseWindow();

/*TEMPLATE*/	void OpenTemplateBrowseWindow();

/*QC*/		void DisplayAffineQC();
		void DisplayDeformQC();
		void DisplayResampQC();
		
/*EXIT*/	void ExitProgram();

/*DATASET*/	void ReadCSVSlot();
		void SaveCSVDatasetBrowse();

/*PARAMETERS*/	void LoadParametersSlot();
		void SaveParametersSlot();

/*SOFT CONFIG*/	void LoadConfigSlot();
		void SaveConfig();
		void ConfigDefault();
		void BrowseSoft(int); //in the soft dialog window
		void ResetSoft(int);
		int testGA(); // returns 0 if version ok, -1 if bad version
		int testDTIReg(); // returns 0 if version ok, -1 if bad version

/*READ ME*/	void ReadMe();

/*RESAMP PARAM*/void InterpolTypeComboBoxChanged(int);
		void TensorInterpolComboBoxChanged(int);

/*DTI-REG*/	void RegMethodComboBoxChanged(int);
		void SimMetChanged(int);
		void ANTSRegTypeChanged(int);

/*WIDGETCHANGE*/void WidgetHasChangedParamNoSaved();

/*MAIN FUNCT*/	int Compute();

	signals: // none

	protected :

/*EXIT*/	void closeEvent(QCloseEvent* event);

	private:

		bool m_noGUI;
		bool m_ErrorDetectedInConstructor; // useful in --nogui mode to exit the program without trying the compute function
		bool m_ScriptRunning;
		std::vector< std::string > m_FindProgramDTIABExecDirVec;

/*CASES*/	std::vector < QLineEdit* > m_CasesQ; // index begin at 0
		std::vector < std::string > m_CasesPath; // index begin at 0

/*DATASET*/	QString m_CSVseparator;
		QString m_CSVPath;
		QString m_OutputPath;
		QString m_TemplatePath;
		QString m_lastCasePath;

/*PARAMETERS*/	int m_ParamSaved; // 0 if the last parameters have not been saved, 1 if the last have been saved
		QString m_ParamFileHeader; // to check if the file to read is really a parameter file

/*SOFT CONFIG*/	int m_FromConstructor; // do not test GA path if 'Default' called from constructor

/*RESAMP PARAM*/QComboBox *m_windowComboBox;
		QComboBox *m_BSplineComboBox;
		QStackedWidget *m_optionStackLayout;
		QComboBox *m_nologComboBox;
		QStackedWidget *m_logOptionStackLayout;

/*DTI-REG*/	QStackedWidget *m_DTIRegOptionStackLayout;
		/*BRAINS*/
		QComboBox *m_BRegTypeComboBox;
		QComboBox *m_TfmModeComboBox;
		QSpinBox *m_NbPyrLevSpin;
		QLineEdit *m_PyrLevItLine;
		/*ANTS*/
		QComboBox *m_ARegTypeComboBox;
		QLineEdit *m_TfmStepLine;
		QLineEdit *m_IterLine;
		QComboBox *m_SimMetComboBox;
		QDoubleSpinBox *m_SimParamDble;
		QLabel *m_SimParamLabel;
		QDoubleSpinBox *m_GSigmaDble;
		QCheckBox *m_SmoothOffCheck;

/*MAIN FUNCT*/	ScriptWriter* m_scriptwriter; ////contains the writing pipeline
		int m_NeedToBeCropped;

};
#endif
