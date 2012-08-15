#ifndef DEF_GUI
#define DEF_GUI

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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // only for the function sleep()
#include <vector>

#include "ui_GUIwindow.h"
#include "ScriptWriter.h"

class GUI : public QMainWindow, public Ui::MainWindow 
{
	Q_OBJECT

	public:
		GUI(std::string ParamFile, std::string ConfigFile, std::string CSVFile); //constructor
		int LaunchScriptWriter(); //returns -1 if failed, otherwise 0
		void LaunchScriptRunner();
		void SaveCSVDataset();
		void SaveCSVResults(int Crop, int nbLoops);
		void GenerateXMLForAW();
		void LoadParameters(QString paramFile);
		void ReadCSV(QString CSVfile);

	public slots:
		void Compute();

//////////Layout Elements
		void OpenAddCaseBrowseWindow();
		void RemoveSelectedCases();
//		void AddAWScaleLevel();
//		void RemoveAWScaleLevel(int nb);
		void OpenOutputBrowseWindow();
 		void OpenTemplateBrowseWindow();
		void OpenRunningCompleteWindow();
		void OpenRunningFailWindow();
		void ReadCSVSlot();
		void SaveCSVDatasetBrowse();
		void LoadParametersSlot();
		void SaveParameters();
		void Config();
		void ConfigOK();
		void ConfigCancel();
		void ReadMe();
		void BrowseSoft(int); //in the soft dialog window
		void InterpolTypeComboBoxChanged(int);

	signals:
		void runningcomplete(); //sent when the running is complete
		void runningfail(); //sent when the running has failed

	protected :
		void closeEvent(QCloseEvent* event);

	private:
		int m_ParamSaved; // 0 if the last parameters have not been saved, 1 if the last have been saved
		QString m_ParamFileHeader; // to check if the file to read is really a parameter file
		QString m_CSVseparator;
		ScriptWriter* m_scriptwriter; ////contains the writing pipeline
//		int m_nbAWSL; // number of AtlasWerks Scale Levels
//		int m_indexAWSL; // index of the current last SL in the QForm
//		std::vector<int> m_QFormIndexs; //contains the index numbers in the QForm for the corresponding nb of the remove Scale Level Button (change because of removing)
//		std::vector<int>::iterator m_VecIt;

  ////////////////////////////////////////////
 //             LAYOUT ELEMENTS            //
////////////////////////////////////////////

		std::vector < QLineEdit* > m_CasesQ; // index begin at 0
		std::vector < std::string > m_CasesPath; // index begin at 0

////////////////////PARAMETERS
//////////Text		
		QString m_CSVPath;
		QString m_OutputPath;
		QString m_TemplatePath;
		QString m_lastCasePath;

		QComboBox *m_windowComboBox;
		QComboBox *m_BSplineComboBox;
		QStackedWidget *m_optionStackLayout;
//		QSignalMapper * m_SLRmButtonMapper;

//////////Config Dialog Window
		QDialog * m_dlg;
		QLineEdit *m_ImagemathPath;
		QLineEdit *m_ResampPath;
		QLineEdit *m_CropDTIPath;
		QLineEdit *m_dtiprocPath;
		QLineEdit *m_BRAINSFitPath;
		QLineEdit *m_AWPath;
		QLineEdit *m_dtiavgPath;
		QSignalMapper *m_SoftButtonMapper; // to avoid multiple browse functions

};
#endif
