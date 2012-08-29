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
#include <ctime> // for clock()

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
		
/*CONSTRUCTOR*/	GUI(std::string ParamFile, std::string ConfigFile, std::string CSVFile, bool overwrite, bool noGUI, bool Quiet); //constructor

/*INIT*/	void InitOptions();

/*DATASET*/	void ReadCSV(QString CSVfile);
		void SaveCSVDataset();
		void SaveCSVResults(int Crop, int nbLoops);

/*PARAMETERS*/	void LoadParameters(QString paramFile);

/*XML FILE*/	void GenerateXMLForAW();

/*SOFT CONFIG*/	void LoadConfig(QString ConfigFile);

/*CHECK IMAGE*/	int checkImage(std::string Image); // returns 1 if not an image, 2 if not a dti, otherwise 0

/*MAIN FUNCT*/	int LaunchScriptWriter(); // returns -1 if failed, otherwise 0
		void LaunchScriptRunner();

	public slots:

/*CASES*/	void OpenAddCaseBrowseWindow();
		void RemoveSelectedCases();

/*OUTPUT*/	void OpenOutputBrowseWindow();

/*TEMPLATE*/	void OpenTemplateBrowseWindow();

/*RUNNING*/	void OpenRunningCompleteWindow();
		void RunningCompleteWindowClosed();
		void OpenRunningFailWindow();
		void DisplayAffineQC();
		void DisplayDeformQC();
		void DisplayResampQC();
		void OpenFolder();
		void Close();
		void Exit();

/*DATASET*/	void ReadCSVSlot();
		void SaveCSVDatasetBrowse();

/*PARAMETERS*/	void LoadParametersSlot();
		void SaveParameters();

/*SOFT CONFIG*/	void LoadConfigSlot();
		void SaveConfig();
		void ConfigDefault();
		void BrowseSoft(int); //in the soft dialog window
		void ResetSoft(int);
		void testAW();

/*READ ME*/	void ReadMe();

/*RESAMP PARAM*/void InterpolTypeComboBoxChanged(int);
		void TensorInterpolComboBoxChanged(int);

/*DTI-REG*/	void RegMethodComboBoxChanged(int);
		void SimMetChanged(int);
		void ANTSRegTypeChanged(int);

/*WIDGETCHANGE*/void WidgetHasChangedParamNoSaved();

/*MAIN FUNCT*/	void Compute();

	signals:
		void runningcomplete(); //sent when the running is complete
		void runningfail(); //sent when the running has failed

	protected :

/*CLOSE WINDOW*/void closeEvent(QCloseEvent* event);

	private:

		bool m_noGUI;
		bool m_Quiet;

/* RUNNING */	QDialog *m_Rundlg; // dialog window that appears when running is done

/*CASES*/	std::vector < QLineEdit* > m_CasesQ; // index begin at 0
		std::vector < std::string > m_CasesPath; // index begin at 0

/*DATASET*/	QString m_CSVseparator;
		QString m_CSVPath;
		QString m_OutputPath;
		QString m_TemplatePath;
		QString m_lastCasePath;

/*PARAMETERS*/	int m_ParamSaved; // 0 if the last parameters have not been saved, 1 if the last have been saved
		QString m_ParamFileHeader; // to check if the file to read is really a parameter file

/*SOFT CONFIG*/	int m_FromConstructor; // do not test AW path if 'Default' called from constructor

/*RESAMP PARAM*/QComboBox *m_windowComboBox;
		QComboBox *m_BSplineComboBox;
		QStackedWidget *m_optionStackLayout;
		QComboBox *m_nologComboBox;
		QStackedWidget *m_logOptionStackLayout;

/*DTI-REG*/	QStackedWidget *m_DTIRegOptionStackLayout;
		/*BRAINS*/
		QComboBox *m_BRegTypeComboBox;
		QComboBox *m_TfmModeComboBox;
		QDoubleSpinBox *m_SigmaDble;
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

};
#endif
