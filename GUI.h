#ifndef DEF_GUI
#define DEF_GUI

#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>
#include <QFile>
#include <QTextStream>
#include <QCloseEvent>
#include <QSignalMapper>

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
		GUI(); //constructor

	public slots:
		void Compute();
		int LaunchScriptWriter(); //returns -1 if failed, otherwise 0
		void LaunchScriptRunner();

//////////Layout Elements
		void OpenAddCaseBrowseWindow();
		void RemoveSelectedCases();
//		void AddAWScaleLevel();
//		void RemoveAWScaleLevel(int nb);
		void OpenOutputBrowseWindow();
 		void OpenTemplateBrowseWindow();
		void OpenRunningCompleteWindow();
		void OpenRunningFailWindow();
		void ReadCSV();
		void SaveCSVDatasetBrowse();
		void SaveCSVDataset();
		void SaveCSVResults(int Crop, int nbLoops);
		void LoadParameters();
		void SaveParameters();
		void GenerateXMLForAW();

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
//		QSignalMapper * m_SLRmButtonMapper;

};
#endif
