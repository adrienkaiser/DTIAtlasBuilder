#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>
#include <QFile>
#include <QTextStream>
#include <QCloseEvent>


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // for the function sleep() and get the access setup
#include <vector>

#include "GUI.h"
#include "ScriptWriter.h"

GUI::GUI() : QMainWindow() // constructor
{
	setupUi(this);

/* Script writing object */
	m_scriptwriter = new ScriptWriter; // delete in "void GUI::OpenRunningCompleteWindow()"

/* Objects connections */
	QObject::connect(ComputepushButton, SIGNAL(clicked()), this, SLOT(Compute()));
	QObject::connect(BrowseCSVPushButton, SIGNAL(clicked()), this, SLOT(ReadCSV()));
	QObject::connect(SaveCSVPushButton, SIGNAL(clicked()), this, SLOT(SaveCSVDatasetBrowse()));
	QObject::connect(BrowseOutputPushButton, SIGNAL(clicked()), this, SLOT(OpenOutputBrowseWindow()));
	QObject::connect(TemplateBrowsePushButton, SIGNAL(clicked()), this, SLOT(OpenTemplateBrowseWindow()));
	QObject::connect(AddPushButton, SIGNAL(clicked()), this, SLOT(OpenAddCaseBrowseWindow()));
	QObject::connect(RemovePushButton, SIGNAL(clicked()), this, SLOT(RemoveSelectedCases()));

	QObject::connect(this, SIGNAL(runningcomplete()), this, SLOT(OpenRunningCompleteWindow()));
	QObject::connect(this, SIGNAL(runningfail()), this, SLOT(OpenRunningFailWindow()));

	QObject::connect(actionLoad_parameters, SIGNAL(triggered()), this, SLOT(LoadParameters()));
	QObject::connect(actionSave_parameters, SIGNAL(triggered()), this, SLOT(SaveParameters()));
	QObject::connect(actionExit, SIGNAL(triggered()), qApp, SLOT(quit()));

/* Buttons */
	RemovePushButton->setEnabled(false);

/* Variables */
	m_ParamFileHeader = QString("DTIAtlasBuilderParameterFileVersion");
	m_CSVseparator = QString(",");
	m_ParamSaved=0;
}

void GUI::OpenAddCaseBrowseWindow()
{
	QStringList CaseListBrowse=QFileDialog::getOpenFileNames(this, "Open Cases", QString(), ".nrrd Images (*.nrrd)");
	CaseListWidget->addItems(CaseListBrowse);
	if ( CaseListWidget->count()>0 ) RemovePushButton->setEnabled(true);
	m_ParamSaved=0;
	SelectCasesLabel->setText( QString("Select Cases :") );
}

void GUI::RemoveSelectedCases()
{
	int NbOfSelectedItems = (CaseListWidget->selectedItems()).size();
	int ItemRow;
	while(NbOfSelectedItems>0)
	{
		ItemRow = CaseListWidget->row( CaseListWidget->selectedItems().at(0) );
		CaseListWidget->takeItem(ItemRow);
		delete CaseListWidget->selectedItems().at(0);
		NbOfSelectedItems--;
	}
	if ( CaseListWidget->count()==0 ) RemovePushButton->setEnabled(false);
	m_ParamSaved=0;
	SelectCasesLabel->setText( QString("Select Cases :") );
}

void GUI::OpenOutputBrowseWindow()
{
	QString OutputBrowse=QFileDialog::getExistingDirectory(this);
	OutputFolderLineEdit->setText(OutputBrowse);
}

void GUI::OpenTemplateBrowseWindow()
{
	QString TemplateBrowse=QFileDialog::getOpenFileName(this, "Open Atlas Template", QString(), ".nrrd Image (*.nrrd)");
	TemplateLineEdit->setText(TemplateBrowse);
}

void GUI::OpenRunningCompleteWindow()
{
	std::string program;
	int ExitCode=0;

//////////Remove temp files ?
 	int answer = QMessageBox::question(this, "Running Complete", "Running completed !\nDo you want to remove the temporary files ?", QMessageBox::Yes | QMessageBox::No);
	if (answer == QMessageBox::Yes)
	{
		int answer2 = QMessageBox::question(this, "Remove temp files", "Are you sure you want to remove the temporary files ?", QMessageBox::Yes | QMessageBox::No);
		if (answer2 == QMessageBox::Yes)
		{
			QProcess * RemoveProcess = new QProcess;
			program = "rm -r " + m_OutputPath.toStdString() + "/DTIAtlas/Affine_Registration " + m_OutputPath.toStdString() + "/DTIAtlas/Deform_Registration";
			//std::cout<<"| Removing command line : $ "<<program<<std::endl;
			std::cout<<"| Removing folders : \'"<<m_OutputPath.toStdString() + "/DTIAtlas/Affine_Registration\' and \'" + m_OutputPath.toStdString() + "/DTIAtlas/Deform_Registration\'"<<std::endl;
			ExitCode = RemoveProcess->execute( program.c_str() );

			if(ExitCode==0) QMessageBox::information(this, "Removing succesful", "Temporary files removed succesfully !");
		}
	}

//////////open containing Folder ?
 	answer = QMessageBox::question(this, "Running Complete", "Click Open to open the Atlas containing folder and exit\nClick Close to exit", QMessageBox::Open | QMessageBox::Close);
 
	if (answer == QMessageBox::Open)
	{
		QProcess * OpenProcess = new QProcess;
		program = "nautilus " + m_OutputPath.toStdString() + "/DTIAtlas";
		ExitCode = OpenProcess->execute( program.c_str() );
	}

	delete m_scriptwriter;
 	qApp->quit(); //end of Application: close the main window
}

void GUI::OpenRunningFailWindow()
{
	QMessageBox::critical(this, "Running Fail", "Running Failed...\nClick OK to exit");
	delete m_scriptwriter;
 	qApp->quit(); //end of Application: close the main window
}

void GUI::closeEvent(QCloseEvent* event)
{
	if(m_ParamSaved==0)
	{
		int ret = QMessageBox::question(this,"Quit","Last parameters have not been saved.\nDo you want to save the last parameters ?",QMessageBox::Yes | QMessageBox::No);
		if (ret == QMessageBox::Yes) SaveParameters();
//		else event->ignore();
	}
	event->accept();
}

void GUI::ReadCSV()
{	
	QString CSVBrowse=QFileDialog::getOpenFileName(this, "Open CSV File", QString(), ".csv Files (*.csv)");

	QFile file(CSVBrowse);

	if (file.open(QFile::ReadOnly))
	{
		std::cout<<"| Loading csv file..."<<std::endl; // command line display

		QTextStream stream(&file);
		while(!stream.atEnd()) //read all the lines
		{
			QString line = stream.readLine();
			QStringList list = line.split(m_CSVseparator);
			if( list.at(0).at(0).toAscii() != '#' )  CaseListWidget->addItem( list.at(1) ); //display in the Widget so that some can be removed
		}
	
		SelectCasesLabel->setText( QString("Select Cases : current CSV file : ") + CSVBrowse );
		m_ParamSaved=0;
	} 
	else qDebug( "Could not open csv file");

	if ( CaseListWidget->count()>0 ) RemovePushButton->setEnabled(true);
}

void GUI::SaveCSVDatasetBrowse()
{	
	if(CaseListWidget->count()==0)
	{
		QMessageBox::critical(this, "No Dataset", "No Dataset");
		return;
	}

	QString CSVBrowsePath=QFileDialog::getExistingDirectory(this);

	QString m_CSVseparator = QString(",");

	QString csvPath;
	csvPath = CSVBrowsePath + QString("/DTIAtlasBuilderDataSet.csv");
	QFile file(csvPath);

	if ( file.open( IO_WriteOnly | IO_Translate ) )
	{
		std::cout<<"| Generating Dataset csv file..."<<std::endl; // command line display

		QTextStream stream( &file );
		stream << QString("#") << m_CSVseparator << QString("Original DTI Image") << endl;
		for(int i=0; i < CaseListWidget->count() ;i++) stream << i+1 << m_CSVseparator << CaseListWidget->item(i)->text() << endl;
		std::cout<<"| Dataset csv file generated"<<std::endl; // command line display
	
		SelectCasesLabel->setText( QString("Select Cases : current CSV file : ") + csvPath );
		QMessageBox::information(this, "Saving succesful", "Dataset has been succesfully saved at" + CSVBrowsePath + "/DTIAtlasBuilderDataSet.csv");		
	}
	else qDebug( "Could not create file");

}

void GUI::SaveCSVDataset()
{	
	QString m_CSVseparator = QString(",");

	QString csvPath;
	csvPath = m_OutputPath + QString("/DTIAtlas/DTIAtlasBuilderDataSet.csv");
	QFile file(csvPath);

	if ( file.open( IO_WriteOnly | IO_Translate ) )
	{
		std::cout<<"| Generating Dataset csv file..."<<std::endl; // command line display

		QTextStream stream( &file );
		stream << QString("#") << m_CSVseparator << QString("Original DTI Image") << endl;
		for(int i=0; i < CaseListWidget->count() ;i++) stream << i+1 << m_CSVseparator << CaseListWidget->item(i)->text() << endl;
		std::cout<<"| Dataset csv file generated"<<std::endl; // command line display
		
		SelectCasesLabel->setText( QString("Select Cases : current CSV file : ") + csvPath );
	}
	else qDebug( "Could not create file");
}

void GUI::SaveCSVResults(int Crop, int nbLoops) // Crop = 0 if no cropping , 1 if cropping needed
{	
	QString m_CSVseparator = QString(",");

	QString csvPath;
	csvPath = m_OutputPath + QString("/DTIAtlas/DTIAtlasBuilderResults.csv");
	QFile file(csvPath);

	if ( file.open( IO_WriteOnly | IO_Translate ) )
	{
		std::cout<<"| Generating Results csv file..."<<std::endl; // command line display

		QTextStream stream( &file );

		stream << QString("#") << m_CSVseparator << QString("Original DTI Image");
		if(Crop==1) stream << m_CSVseparator << QString("Cropped DTI");
		stream << m_CSVseparator << QString("FA from original") << m_CSVseparator << QString("Affine registered FA") << m_CSVseparator << QString("Affine transform") << m_CSVseparator << QString("Affine registered DTI") << m_CSVseparator << QString("Final FA for AtlasWerks") << m_CSVseparator << QString("Deformed Image") << m_CSVseparator << QString("Deformation field") << m_CSVseparator << QString("Final DTI") << endl;

		for(int i=0; i < CaseListWidget->count() ;i++) // for all cases
		{
			stream << i+1 << m_CSVseparator << CaseListWidget->item(i)->text();
			if(Crop==1) stream << m_CSVseparator << m_OutputPath + QString("/DTIAtlas/Affine_Registration/Case") << i+1 << QString("_croppedDTI.nrrd");
			stream << m_CSVseparator << m_OutputPath + QString("/DTIAtlas/Affine_Registration/Case") << i+1 << QString("_FA.nrrd");
			stream << m_CSVseparator << m_OutputPath + QString("/DTIAtlas/Affine_Registration/Case") << i+1 << QString(" _Loop ") << nbLoops << QString("_LinearTrans_FA.nrrd"); // only the last processing (last loop) is remembered
			stream << m_CSVseparator << m_OutputPath + QString("/DTIAtlas/Affine_Registration/Case") << i+1 << QString(" _Loop ") << nbLoops << QString("_LinearTrans.tfm");
			stream << m_CSVseparator << m_OutputPath + QString("/DTIAtlas/Affine_Registration/Case") << i+1 << QString(" _Loop ") << nbLoops << QString("_LinearTrans_DTI.nrrd");
			stream << m_CSVseparator << m_OutputPath + QString("/DTIAtlas/Affine_Registration/Case") << i+1 << QString(" _Loop ") << nbLoops << QString("_FinalFA.nrrd");
			if(i<10) 
			{
				stream << m_CSVseparator << m_OutputPath + QString("/DTIAtlas/Deform_Registration/DeformedImage_0_000") << i << QString(".mhd");
				stream << m_CSVseparator << m_OutputPath + QString("/DTIAtlas/Deform_Registration/DeformationField_000") << i << QString(".mhd");
			}
			if( (i>10) && (i<100) )
			{
				stream << m_CSVseparator << m_OutputPath + QString("/DTIAtlas/Deform_Registration/DeformedImage_0_00") << i << QString(".mhd");
				stream << m_CSVseparator << m_OutputPath + QString("/DTIAtlas/Deform_Registration/DeformationField_00") << i << QString(".mhd");
			}
			if( (i>100) && (i<1000) )
			{
				stream << m_CSVseparator << m_OutputPath + QString("/DTIAtlas/Deform_Registration/DeformedImage_0_0") << i << QString(".mhd");
				stream << m_CSVseparator << m_OutputPath + QString("/DTIAtlas/Deform_Registration/DeformationField_0") << i << QString(".mhd");
			}
			if(i>1000)
			{
				stream << m_CSVseparator << m_OutputPath + QString("/DTIAtlas/Deform_Registration/DeformedImage_0_") << i << QString(".mhd");
				stream << m_CSVseparator << m_OutputPath + QString("/DTIAtlas/Deform_Registration/DeformationField_") << i << QString(".mhd");
			}
			stream << m_CSVseparator << m_OutputPath + QString("/DTIAtlas/Final_Atlas/Case") << i+1 << QString(" _FinalDTI.nrrd");
			stream << endl;
		}

		std::cout<<"| Results csv file generated"<<std::endl; // command line display
		
	}
	else qDebug( "Could not create file");
}

void GUI::SaveParameters()
{
	if(CaseListWidget->count()==0)
	{
		QMessageBox::critical(this, "No Dataset", "No Dataset");
		return;
	}

	QString ParamBrowseName=QFileDialog::getSaveFileName(this, tr("Save Parameter File"),"./DTIAtlasBuilderParameters.txt",tr("Text File (*.txt)"));
	QString CSVFileName = ParamBrowseName.split(".").at(0) + QString(".csv");
	QFile file(ParamBrowseName);
	if ( file.open( IO_WriteOnly | IO_Translate ) )
	{
		std::cout<<"| Saving Parameters file..."<<std::endl; // command line display

		QTextStream stream( &file );

		stream << m_ParamFileHeader << "=1" << endl;
		stream << "Output Folder=" << OutputFolderLineEdit->text() << endl;
		stream << "Atlas Template=" << TemplateLineEdit->text() << endl;
		stream << "Loops for the registration=" << NbLoopsSpinBox->value() << endl;
		if(OverwritecheckBox->isChecked()) stream << "Overwrite=true" << endl;
		else  stream << "Overwrite=false" << endl;
		stream << "CSV Dataset File=" << CSVFileName << endl;

		std::cout<<"| Parameters file generated"<<std::endl; // command line display

		QFile filecsv(CSVFileName);
		if ( filecsv.open( IO_WriteOnly | IO_Translate ) )
		{
			std::cout<<"| Generating Dataset csv file..."<<std::endl; // command line display

			QTextStream streamcsv( &filecsv );
			streamcsv << QString("#") << m_CSVseparator << QString("Original DTI Image") << endl;
			for(int i=0; i < CaseListWidget->count() ;i++) streamcsv << i+1 << m_CSVseparator << CaseListWidget->item(i)->text() << endl;
			std::cout<<"| Dataset csv file generated"<<std::endl; // command line display
		
			SelectCasesLabel->setText( QString("Select Cases : current CSV file : ") + CSVFileName );
		}
		else qDebug( "Could not create csv file");

		QMessageBox::information(this, "Saving succesful", "Parameters have been succesfully saved at" + ParamBrowseName);
		m_ParamSaved=1;
	}
	else qDebug( "Could not create parameter file");
}

void GUI::LoadParameters()
{
	QString ParamBrowse=QFileDialog::getOpenFileName(this, "Open Parameter File", QString(), ".txt Files (*.txt)");

	QFile file(ParamBrowse);

	if (file.open(QFile::ReadOnly))
	{
		QTextStream stream(&file);

		QString line = stream.readLine();
		QStringList list = line.split("=");
		if( ! list.at(0).contains(m_ParamFileHeader) )
		{
			QMessageBox::critical(this, "No parameter file", "This file is not a parameter file\nfor this program");
			return;
		}

		std::cout<<"| Loading Parameters file..."<<std::endl; // command line display

///////// Other Parameters
		line = stream.readLine();
		list = line.split("=");
		if(!list.at(0).contains(QString("Output Folder")))
		{
			QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
			return;
		}
		OutputFolderLineEdit->setText(list.at(1));


		line = stream.readLine();
		list = line.split("=");
		if(!list.at(0).contains(QString("Atlas Template")))
		{
			QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
			return;
		}
		TemplateLineEdit->setText(list.at(1));

		line = stream.readLine();
		list = line.split("=");
		if(!list.at(0).contains(QString("Loops for the registration")))
		{
			QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
			return;
		}
		NbLoopsSpinBox->setValue( list.at(1).toInt() );

		line = stream.readLine();
		list = line.split("=");
		if(!list.at(0).contains(QString("Overwrite")))
		{
			QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
			return;
		}
		if( list.at(1) == QString("true") ) OverwritecheckBox->setChecked(true);

/////////////Opening CSV File
		line = stream.readLine();
		list = line.split("=");
		if(!list.at(0).contains(QString("CSV Dataset File")))
		{
			QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
			return;
		}
		QString CSVpath = list.at(1);
		CaseListWidget->clear();
		QFile CSVfile(CSVpath);
		if (CSVfile.open(QFile::ReadOnly))
		{
			std::cout<<"| Loading csv file..."<<std::endl; // command line display

			QTextStream CSVstream(&CSVfile);
			while(!CSVstream.atEnd()) //read all the lines
			{
				QString CSVline = CSVstream.readLine();
				QStringList CSVlist = CSVline.split(m_CSVseparator);
				if( CSVlist.at(0).at(0).toAscii() != '#' )  CaseListWidget->addItem( CSVlist.at(1) ); //display in the Widget so that some can be removed
			}
			SelectCasesLabel->setText( QString("Select Cases : current CSV file : ") + CSVpath );
		} 
		else qDebug( "Could not open csv file");
		if ( CaseListWidget->count()>0 ) RemovePushButton->setEnabled(true);

	} 
	else qDebug( "Could not open file");

	m_ParamSaved=1;
}

void GUI::Compute()
{
	
	if(CaseListWidget->count()==0) QMessageBox::critical(this, "No Cases", "Please give at least one case");
	else // OK Case
	{
	
	if(OutputFolderLineEdit->text().isEmpty()) QMessageBox::critical(this, "No Output Folder", "Please give an output folder");
	else // OK Output
	{

		if(LaunchScriptWriter()==-1) 
		{
			std::cout<<"| Clearing previous cases in vectors..."<<std::endl; // command line display
			m_CasesPath.clear();
			m_scriptwriter->clearCasesPath();
			return;
		}
		LaunchScriptRunner();

	} // else of if(OutputFolderLineEdit->text().isEmpty())

	} // else of if[Case]
}

int GUI::LaunchScriptWriter()
{
//////////Variables for the QProcesses
	int ExitCode=0;
	std::string program;

//////////Setting the values
	for(int i=0; i < CaseListWidget->count() ;i++) 
	{
		if( access(CaseListWidget->item(i)->text().toStdString().c_str(), F_OK) != 0 ) ///////Test if the case files exist => unistd::access() returns 0 if F(file)_OK
		{
			std::string text = "This file does not exist :\n" + CaseListWidget->item(i)->text().toStdString();
			QMessageBox::critical(this, "Case does not exist", QString(text.c_str()) );
			return -1;
		}
		m_CasesPath.push_back( CaseListWidget->item(i)->text().toStdString() );
	}
	m_scriptwriter->setCasesPath(m_CasesPath); //m_CasesPath is a vector

	m_OutputPath=OutputFolderLineEdit->text();
	if( access(m_OutputPath.toStdString().c_str(), F_OK) != 0 ) ///////Test if the folder exists => unistd::access() returns 0 if F(file)_OK
	{
		QMessageBox::critical(this, "No Output Folder", "The output folder does not exist");
		return -1;
	}
	if( access(m_OutputPath.toStdString().c_str(), W_OK) != 0 ) ///////Test if the program can write in the output folder => unistd::access() returns 0 if W(write)_OK
	{
		QMessageBox::critical(this, "Output Folder Unwritable", "Please give an output folder authorized in reading");
		return -1;
	}
	m_scriptwriter->setOutputPath(m_OutputPath.toStdString());

	if( access((m_OutputPath.toStdString() + "/DTIAtlas").c_str(), F_OK) != 0 ) ///////Test if the main folder does not exists => unistd::access() returns 0 if F(file)_OK
	{
		std::cout<<"| Creating Main directory..."<<std::endl; // command line display
		QProcess * mkdirMainProcess = new QProcess;
		program = "mkdir " + m_OutputPath.toStdString() + "/DTIAtlas"; //// Creates the directory
		ExitCode = mkdirMainProcess->execute( program.c_str() );
	}

	if( access((m_OutputPath.toStdString() + "/DTIAtlas/Script").c_str(), F_OK) != 0 ) ///////Test if the script folder does not exists => unistd::access() returns 0 if F(file)_OK
	{
		std::cout<<"| Creating Script directory..."<<std::endl; // command line display
		QProcess * mkdirScriptProcess = new QProcess;
		program = "mkdir " + m_OutputPath.toStdString() + "/DTIAtlas/Script";
		ExitCode = mkdirScriptProcess->execute( program.c_str() );
	}

	if (!TemplateLineEdit->text().isEmpty()) 
	{
		if( access(TemplateLineEdit->text().toStdString().c_str(), F_OK) != 0 ) ///////Test if the case files exist => unistd::access() returns 0 if F(file)_OK
		{
			std::string text = "This template file does not exist :\n" + TemplateLineEdit->text().toStdString();
			QMessageBox::critical(this, "Template does not exist", QString(text.c_str()) );
			return -1;
		}
///////////////////////////////
/////// TO DO : test if FA image and not DTI
/////////////////////////////		
		m_scriptwriter->setRegType(0);
		m_scriptwriter->setTemplatePath(TemplateLineEdit->text().toStdString());
	}

	else m_scriptwriter->setRegType(1); // default

	if(OverwritecheckBox->isChecked()) m_scriptwriter->setOverwrite(1);
	else  m_scriptwriter->setOverwrite(0);

	m_scriptwriter->setnbLoops(NbLoopsSpinBox->value());// QLineEdit.value() is an int

//////////Launch writing
	if(m_scriptwriter->CheckVoxelSize()==1) 
	{
		QMessageBox::critical(this, "Voxel Size", "Error: The voxel size of the images\nare not the same,\nplease change dataset"); // returns 0 if voxel size OK , otherwise 1		
		return -1;
	}
	int Crop=m_scriptwriter->setCroppingSize(); // returns 0 if no cropping , 1 if cropping needed
	if(Crop==1) QMessageBox::warning(this, "Cropping", "Warning: The images do not have the same size, \nso some of them will be cropped");
	m_scriptwriter->WriteScript(); // Master Function

//////////Save CSV
	SaveCSVDataset();
	SaveCSVResults(Crop,NbLoopsSpinBox->value());

//////////Generate Preprocess script file
	std::cout<<"| Generating Pre processing script file..."<<std::endl; // command line display

	QString ScriptPath;
	ScriptPath = m_OutputPath + QString("/DTIAtlas/Script/DTIAtlasBuilder_Preprocess.script");
	QFile filePreP(ScriptPath);

	if ( filePreP.open( IO_WriteOnly | IO_Translate ) )
	{
		//file.setPermissions(QFile::ExeOwner); //make the file executable for the owner
		QTextStream stream( &filePreP );
		stream << QString((m_scriptwriter->getScript_Preprocess()).c_str()) << endl;
		std::cout<<"| Pre process script file generated"<<std::endl; // command line display
	}
	else qDebug( "Could not create file");

//////////Generate Atlas Building script file
	std::cout<<"| Generating Atlas Building script file..."<<std::endl; // command line display

	ScriptPath = m_OutputPath + QString("/DTIAtlas/Script/DTIAtlasBuilder_AtlasBuilding.script");
	QFile fileAtlas(ScriptPath);

	if ( fileAtlas.open( IO_WriteOnly | IO_Translate ) )
	{
		//file.setPermissions(QFile::ExeOwner); //make the file executable for the owner
		QTextStream stream( &fileAtlas );
		stream << QString((m_scriptwriter->getScript_AtlasBuilding()).c_str()) << endl;
		std::cout<<"| Atlas Building script file generated"<<std::endl; // command line display
	}
	else qDebug( "Could not create file");

//////////Generate Main script file
	std::cout<<"| Generating Main script file..."<<std::endl; // command line display

	ScriptPath = m_OutputPath + QString("/DTIAtlas/Script/DTIAtlasBuilder_Main.script");
	QFile fileMain(ScriptPath);

	if ( fileMain.open( IO_WriteOnly | IO_Translate ) )
	{
		//file.setPermissions(QFile::ExeOwner); //make the file executable for the owner
		QTextStream stream( &fileMain );
		stream << QString((m_scriptwriter->getScript_Main()).c_str()) << endl;
		std::cout<<"| Main script file generated"<<std::endl; // command line display
	}
	else qDebug( "Could not create file");

//////////Give the right to user to execute the scripts
	QProcess * chmodProcess = new QProcess;
	program = "chmod u+x " + m_OutputPath.toStdString() + "/DTIAtlas/Script/DTIAtlasBuilder_Preprocess.script " + m_OutputPath.toStdString() + "/DTIAtlas/Script/DTIAtlasBuilder_AtlasBuilding.script " + m_OutputPath.toStdString() + "/DTIAtlas/Script/DTIAtlasBuilder_Main.script"; // 'chmod u+x = user+execute'
	ExitCode = chmodProcess->execute( program.c_str() );
	
	return 0;
}

void GUI::LaunchScriptRunner()
{
	std::cout<<"| Script Running..."<<std::endl; // command line display

//////////Running the Script:
	QProcess * ScriptProcess = new QProcess;

	std::string program;
	program = m_OutputPath.toStdString() + "/DTIAtlas/Script/DTIAtlasBuilder_Main.script";

	int ExitCode=0;
	ExitCode = ScriptProcess->execute( program.c_str() );

//////////When we are here the running is finished : emit signal to display the "Running Completed" Window:
	if(ExitCode==0)
	{
		std::cout<<"| Running Completed !"<<std::endl; // command line display
		emit runningcomplete();
	}

	else
	{
		std::cout<<"| Running Failed..."<<std::endl; // command line display
		emit runningfail();
	}
}

