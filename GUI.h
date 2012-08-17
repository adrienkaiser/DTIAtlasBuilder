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
#include <unistd.h> // to get the access setup
#include <vector>

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
		
/*CONSTRUCTOR*/	GUI(std::string ParamFile, std::string ConfigFile, std::string CSVFile); //constructor

/*DATASET*/	void ReadCSV(QString CSVfile);
		void SaveCSVDataset();
		void SaveCSVResults(int Crop, int nbLoops);

/*PARAMETERS*/	void LoadParameters(QString paramFile);

/*XML FILE*/	void GenerateXMLForAW();

/*SOFT CONFIG*/	void LoadConfig(QString ConfigFile);
		void SoftEnvSet(); // set the software paths from the env variable if it exists

/*CHECK IMAGE*/	int checkImage(std::string Image); // returns 1 if not an image, 2 if not a dti, otherwise 0

/*MAIN FUNCT*/	int LaunchScriptWriter(); // returns -1 if failed, otherwise 0
		void LaunchScriptRunner();

	public slots:

/*CASES*/	void OpenAddCaseBrowseWindow();
		void RemoveSelectedCases();

/*OUTPUT*/	void OpenOutputBrowseWindow();

/*TEMPLATE*/	void OpenTemplateBrowseWindow();

/*RUNNING*/	void OpenRunningCompleteWindow();
		void OpenRunningFailWindow();

/*DATASET*/	void ReadCSVSlot();
		void SaveCSVDatasetBrowse();

/*PARAMETERS*/	void LoadParametersSlot();
		void SaveParameters();

/*SOFT CONFIG*/	void LoadConfigSlot();
		void SaveConfig();
		void ConfigDefault();
		void ConfigEnv();
		void BrowseSoft(int); //in the soft dialog window

/*READ ME*/	void ReadMe();

/*RESAMP PARAM*/void InterpolTypeComboBoxChanged(int);
		void TensorInterpolComboBoxChanged(int);

/*WIDGETCHANGE*/void WidgetHasChangedParamNoSaved();

/*MAIN FUNCT*/	void Compute();

	signals:
		void runningcomplete(); //sent when the running is complete
		void runningfail(); //sent when the running has failed

	protected :

/*CLOSE WINDOW*/void closeEvent(QCloseEvent* event);

	private:

/*CASES*/	std::vector < QLineEdit* > m_CasesQ; // index begin at 0
		std::vector < std::string > m_CasesPath; // index begin at 0

/*DATASET*/	QString m_CSVseparator;
		QString m_CSVPath;
		QString m_OutputPath;
		QString m_TemplatePath;
		QString m_lastCasePath;

/*PARAMETERS*/	int m_ParamSaved; // 0 if the last parameters have not been saved, 1 if the last have been saved
		QString m_ParamFileHeader; // to check if the file to read is really a parameter file

/*RESAMP PARAM*/QComboBox *m_windowComboBox;
		QComboBox *m_BSplineComboBox;
		QStackedWidget *m_optionStackLayout;
		QComboBox *m_nologComboBox;
		QStackedWidget *m_logOptionStackLayout;

/*MAIN FUNCT*/	ScriptWriter* m_scriptwriter; ////contains the writing pipeline

};
#endif
