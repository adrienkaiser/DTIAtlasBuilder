#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>
#include <QFile>
#include <QTextStream>
#include <QCloseEvent>
#include <QSignalMapper>
#include <QDialog>
#include <QLabel>

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
//	QObject::connect(AddSLPushButton, SIGNAL(clicked()), this, SLOT(AddAWScaleLevel()));

	QObject::connect(this, SIGNAL(runningcomplete()), this, SLOT(OpenRunningCompleteWindow()));
	QObject::connect(this, SIGNAL(runningfail()), this, SLOT(OpenRunningFailWindow()));

	QObject::connect(actionLoad_parameters, SIGNAL(triggered()), this, SLOT(LoadParameters()));
	QObject::connect(actionSave_parameters, SIGNAL(triggered()), this, SLOT(SaveParameters()));
	QObject::connect(actionExit, SIGNAL(triggered()), qApp, SLOT(quit()));
	QObject::connect(actionConfigure_Softwares, SIGNAL(triggered()), this, SLOT(Config()));
	QObject::connect(actionRead_Me, SIGNAL(triggered()), this, SLOT(ReadMe()));

/* Buttons */
	RemovePushButton->setEnabled(false);

/* Variables */
	m_ParamFileHeader = QString("DTIAtlasBuilderParameterFileVersion");
	m_CSVseparator = QString(",");
	m_ParamSaved=0;
//	m_nbAWSL=0;
//	m_indexAWSL=-1;

/* Initalizating Scale Levels */
//	AddAWScaleLevel();

/* Remove Scale Level Buttons */
//	m_SLRmButtonMapper = new QSignalMapper();
//	QObject::connect(m_SLRmButtonMapper, SIGNAL(mapped(int)), this, SLOT( RemoveAWScaleLevel(int) ));

/* Browse softwares path Buttons */
	m_SoftButtonMapper = new QSignalMapper();
	QObject::connect(m_SoftButtonMapper, SIGNAL(mapped(int)), this, SLOT( BrowseSoft(int) ));
}

void GUI::OpenAddCaseBrowseWindow()
{
	QStringList CaseListBrowse=QFileDialog::getOpenFileNames(this, "Open Cases", QString(), ".nrrd Images (*.nrrd)");
	CaseListWidget->addItems(CaseListBrowse);
	if ( CaseListWidget->count()>0 ) RemovePushButton->setEnabled(true);
	m_ParamSaved=0;
	SelectCasesLabel->setText( QString("") );
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
	SelectCasesLabel->setText( QString("") );
}
/*
void GUI::AddAWScaleLevel()
{
	m_nbAWSL++;
	m_indexAWSL++;

	QSpinBox * SLSpinBox=new QSpinBox;
	SLFormLayout->addRow("Scale Level : ",SLSpinBox);

	QSpinBox * IterSpinBox = new QSpinBox;
	IterFormLayout->addRow("Nb of iterations : ",IterSpinBox);

	QPushButton * RemSLButton = new QPushButton ("-", this);
	QObject::connect(RemSLButton, SIGNAL(clicked()), m_SLRmButtonMapper, SLOT(map()));
	m_SLRmButtonMapper->setMapping(RemSLButton, m_nbAWSL);
	m_QFormIndexs.push_back(m_indexAWSL); // the nbAWSL is unique and the corresponding QForm index is in the vector : it can change if SL are removed.
	ButtonFormLayout->addRow(RemSLButton);
}

void GUI::RemoveAWScaleLevel(int buttonNb)
{
	std::cout<<"Button removed : "<<buttonNb<<" | QForm row removed : "<<m_QFormIndexs[buttonNb-1]<<std::endl<<"Button nb    = 1";
	for(int i=1;i< (int) m_QFormIndexs.size();i++) std::cout<<"  "<<i+1;
	std::cout<<""<<std::endl<<"m_QFormIndexs=["<<m_QFormIndexs[0];
	for(int i=1;i<(int) m_QFormIndexs.size();i++) std::cout<<", "<<m_QFormIndexs[i];
	std::cout<<"]"<<std::endl;
/////////removing the items
	delete SLFormLayout->takeAt( m_QFormIndexs[buttonNb-1] );
	delete IterFormLayout->takeAt( m_QFormIndexs[buttonNb-1] );
	delete ButtonFormLayout->takeAt( m_QFormIndexs[buttonNb-1] );

////////modify the vector :
	m_indexAWSL--;
	m_VecIt=m_QFormIndexs.begin();
	m_QFormIndexs.insert(m_VecIt+buttonNb-1,m_QFormIndexs[buttonNb-1]);
	// => insert a value at the place of the button so all the values get pushed on the right of the vector.
}
*/
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
			program = "rm -r " + m_OutputPath.toStdString() + "/DTIAtlas/Affine_Registration " + m_OutputPath.toStdString() + "/DTIAtlas/NonLinear_Registration";
			//std::cout<<"| Removing command line : $ "<<program<<std::endl;
			std::cout<<"| Removing folders : \'"<<m_OutputPath.toStdString() + "/DTIAtlas/Affine_Registration\' and \'" + m_OutputPath.toStdString() + "/DTIAtlas/NonLinear_Registration\'"<<std::endl;
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
	
		SelectCasesLabel->setText( QString("Current CSV file : ") + CSVBrowse );
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

	QString CSVBrowseName = QFileDialog::getSaveFileName(this, tr("Save Dataset"),"./DTIAtlasBuilderDataSet.csv",tr("CSV File (*.csv)"));

	QString m_CSVseparator = QString(",");

	QFile file(CSVBrowseName);

	if ( file.open( IO_WriteOnly | IO_Translate ) )
	{
		std::cout<<"| Generating Dataset csv file..."<<std::endl; // command line display

		QTextStream stream( &file );
		stream << QString("#") << m_CSVseparator << QString("Original DTI Image") << endl;
		for(int i=0; i < CaseListWidget->count() ;i++) stream << i+1 << m_CSVseparator << CaseListWidget->item(i)->text() << endl;
		std::cout<<"| Dataset csv file generated"<<std::endl; // command line display
	
		SelectCasesLabel->setText( QString("Current CSV file : ") + CSVBrowseName );
		QMessageBox::information(this, "Saving succesful", "Dataset has been succesfully saved at" + CSVBrowseName);		
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
		
		SelectCasesLabel->setText( QString("Current CSV file : ") + csvPath );
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
		stream << m_CSVseparator << QString("FA from original") << m_CSVseparator << QString("Affine registered FA") << m_CSVseparator << QString("Affine transform") << m_CSVseparator << QString("Affine registered DTI") << m_CSVseparator << QString("Final FA for AtlasWerks") << m_CSVseparator << QString("Deformed Image") << m_CSVseparator << QString("Deformation field") << m_CSVseparator << QString("Deformation Inverse field") << m_CSVseparator << QString("Final DTI") << endl;

		for(int i=0; i < CaseListWidget->count() ;i++) // for all cases
		{
			stream << i+1 << m_CSVseparator << CaseListWidget->item(i)->text();
			if(Crop==1) stream << m_CSVseparator << m_OutputPath + QString("/DTIAtlas/Affine_Registration/Case") << i+1 << QString("_croppedDTI.nrrd");
			stream << m_CSVseparator << m_OutputPath + QString("/DTIAtlas/Affine_Registration/Case") << i+1 << QString("_FA.nrrd");
			stream << m_CSVseparator << m_OutputPath + QString("/DTIAtlas/Affine_Registration/Case") << i+1 << QString(" _Loop ") << nbLoops << QString("_LinearTrans_FA.nrrd"); // only the last processing (last loop) is remembered
			stream << m_CSVseparator << m_OutputPath + QString("/DTIAtlas/Affine_Registration/Case") << i+1 << QString(" _Loop ") << nbLoops << QString("_LinearTrans.txt");
			stream << m_CSVseparator << m_OutputPath + QString("/DTIAtlas/Affine_Registration/Case") << i+1 << QString(" _Loop ") << nbLoops << QString("_LinearTrans_DTI.nrrd");
			stream << m_CSVseparator << m_OutputPath + QString("/DTIAtlas/Affine_Registration/Case") << i+1 << QString(" _Loop ") << nbLoops << QString("_FinalFA.nrrd");
			stream << m_CSVseparator << m_OutputPath + QString("/DTIAtlas/NonLinear_Registration/Case") << i+1 << QString("_NonLinearTrans_FA.mhd");
			stream << m_CSVseparator << m_OutputPath + QString("/DTIAtlas/NonLinear_Registration/Case") << i+1 << QString("_DeformationField.mhd");
			stream << m_CSVseparator << m_OutputPath + QString("/DTIAtlas/NonLinear_Registration/Case") << i+1 << QString("_InverseDeformationField.mhd");
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
		stream << "Scale Level 4= " << nbIter4SpinBox->value() << "," << alpha4DoubleSpinBox->value() << "," << beta4DoubleSpinBox->value() << "," << gamma4DoubleSpinBox->value() << "," << maxPerturbation4DoubleSpinBox->value() << endl;
		stream << "Scale Level 2= " << nbIter2SpinBox->value() << "," << alpha2DoubleSpinBox->value() << "," << beta2DoubleSpinBox->value() << "," << gamma2DoubleSpinBox->value() << "," << maxPerturbation2DoubleSpinBox->value() << endl;
		stream << "Scale Level 1= " << nbIter1SpinBox->value() << "," << alpha1DoubleSpinBox->value() << "," << beta1DoubleSpinBox->value() << "," << gamma1DoubleSpinBox->value() << "," << maxPerturbation1DoubleSpinBox->value() << endl;
		stream << "Resampling Interpolation Algorithm=" << InterpolTypeComboBox->currentText() << endl;
		stream << "DTI Average Statistics Method=" << averageStatMethodComboBox->currentText() << endl;

		std::cout<<"| Parameters file generated"<<std::endl; // command line display

		QFile filecsv(CSVFileName);
		if ( filecsv.open( IO_WriteOnly | IO_Translate ) )
		{
			std::cout<<"| Generating Dataset csv file..."<<std::endl; // command line display

			QTextStream streamcsv( &filecsv );
			streamcsv << QString("#") << m_CSVseparator << QString("Original DTI Image") << endl;
			for(int i=0; i < CaseListWidget->count() ;i++) streamcsv << i+1 << m_CSVseparator << CaseListWidget->item(i)->text() << endl;
			std::cout<<"| Dataset csv file generated"<<std::endl; // command line display
		
			SelectCasesLabel->setText( QString("Current CSV file : ") + CSVFileName );
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
			SelectCasesLabel->setText( QString("Current CSV file : ") + CSVpath );
		} 
		else 
		{
			SelectCasesLabel->setText( QString("Could not open CSV File"));
			qDebug( "Could not open csv file");
		}
		if ( CaseListWidget->count()>0 ) RemovePushButton->setEnabled(true);

/////////////////Scale Levels
		line = stream.readLine();
		list = line.split("=");
		if(!list.at(0).contains(QString("Scale Level 4")))
		{
			QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
			return;
		}
		else
		{
			QStringList nbrs= list.at(1).split(",");
			if( nbrs.size()!=5 )
			{
				QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
				return;
			}
			if( (nbrs.at(1).toDouble()==0) || (nbrs.at(2).toDouble()==0) || (nbrs.at(3).toDouble()==0) || (nbrs.at(4).toDouble()==0))
			{
				QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
				return;
			}
			nbIter4SpinBox->setValue(nbrs.at(0).toInt());
			alpha4DoubleSpinBox->setValue(nbrs.at(1).toDouble());
			beta4DoubleSpinBox->setValue(nbrs.at(2).toDouble());
			gamma4DoubleSpinBox->setValue(nbrs.at(3).toDouble());
			maxPerturbation4DoubleSpinBox->setValue(nbrs.at(4).toDouble());
		}

		line = stream.readLine();
		list = line.split("=");
		if(!list.at(0).contains(QString("Scale Level 2")))
		{
			QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
			return;
		}
		else
		{
			QStringList nbrs= list.at(1).split(",");
			if( nbrs.size()!=5 )
			{
				QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
				return;
			}
			if( (nbrs.at(1).toDouble()==0) || (nbrs.at(2).toDouble()==0) || (nbrs.at(3).toDouble()==0) || (nbrs.at(4).toDouble()==0))
			{
				QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
				return;
			}
			nbIter2SpinBox->setValue(nbrs.at(0).toInt());
			alpha2DoubleSpinBox->setValue(nbrs.at(1).toDouble());
			beta2DoubleSpinBox->setValue(nbrs.at(2).toDouble());
			gamma2DoubleSpinBox->setValue(nbrs.at(3).toDouble());
			maxPerturbation2DoubleSpinBox->setValue(nbrs.at(4).toDouble());
		}

		line = stream.readLine();
		list = line.split("=");
		if(!list.at(0).contains(QString("Scale Level 1")))
		{
			QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
			return;
		}
		else
		{
			QStringList nbrs= list.at(1).split(",");
			if( nbrs.size()!=5 )
			{
				QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
				return;
			}
			if( (nbrs.at(1).toDouble()==0) || (nbrs.at(2).toDouble()==0) || (nbrs.at(3).toDouble()==0) || (nbrs.at(4).toDouble()==0))
			{
				QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
				return;
			}
			nbIter1SpinBox->setValue(nbrs.at(0).toInt());
			alpha1DoubleSpinBox->setValue(nbrs.at(1).toDouble());
			beta1DoubleSpinBox->setValue(nbrs.at(2).toDouble());
			gamma1DoubleSpinBox->setValue(nbrs.at(3).toDouble());
			maxPerturbation1DoubleSpinBox->setValue(nbrs.at(4).toDouble());
		}

		line = stream.readLine();
		list = line.split("=");
		if(!list.at(0).contains(QString("Resampling Interpolation Algorithm")))
		{
			QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
			return;
		}
		if( list.at(1).contains(QString("Linear")) ) InterpolTypeComboBox->setCurrentIndex(0);
		else if( list.at(1).contains(QString("Nearest Neighborhoor")) ) InterpolTypeComboBox->setCurrentIndex(1);
		else if( list.at(1).contains(QString("Windowed Sinc")) ) InterpolTypeComboBox->setCurrentIndex(2);
		else if( list.at(1).contains(QString("BSpline")) ) InterpolTypeComboBox->setCurrentIndex(3);
		else
		{
			QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
			return;
		}

		line = stream.readLine();
		list = line.split("=");
		if(!list.at(0).contains(QString("DTI Average Statistics Method")))
		{
			QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
			return;
		}
		if( list.at(1).contains(QString("PGA")) ) averageStatMethodComboBox->setCurrentIndex(0);
		else if( list.at(1).contains(QString("Euclidean")) ) averageStatMethodComboBox->setCurrentIndex(1);
		else if( list.at(1).contains(QString("Log Euclidean")) ) averageStatMethodComboBox->setCurrentIndex(2);
		else
		{
			QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
			return;
		}

	} 
	else if ( !ParamBrowse.isEmpty() ) qDebug( "Could not open file");

	m_ParamSaved=1;
}

void GUI::GenerateXMLForAW()
{	
	if( access((m_OutputPath.toStdString() + "/DTIAtlas/NonLinear_Registration").c_str(), F_OK) != 0 ) ///////Test if the main folder does not exists => unistd::access() returns 0 if F(file)_OK
	{
		std::cout<<"| Creating Non Linear Registration directory..."<<std::endl; // command line display
		QProcess * mkdirMainProcess = new QProcess;
		std::string program = "mkdir " + m_OutputPath.toStdString() + "/DTIAtlas/NonLinear_Registration"; //// Creates the directory
		mkdirMainProcess->execute( program.c_str() );
	}

	QString	xmlFileName = m_OutputPath + QString("/DTIAtlas/NonLinear_Registration/AtlasWerksParameters.xml");
	QFile file(xmlFileName);
	if ( file.open( IO_WriteOnly | IO_Translate ) )
	{
		std::cout<<"| Saving XML file for AtlasWerks..."<<std::endl; // command line display
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
					stream <<"\t\t\t<Filename val=\"" << m_OutputPath << "/DTIAtlas/Affine_Registration/Case" << i_str.c_str() << "_Loop" << nbLoops_str.c_str() << "_NormFA.nrrd\" />"<< endl;
					stream <<"\t\t\t<Transform val=\"" << m_OutputPath << "/DTIAtlas/Affine_Registration/Case" << i_str.c_str() << "_Loop" << nbLoops_str.c_str() << "_LinearTrans_float.txt\" />"<< endl;
					stream <<"\t\t\t<ItkTransform val=\"1\" />"<< endl;
				stream <<"\t\t</WeightedImage>"<< endl;
			}
			stream <<"\t</WeightedImageSet>"<< endl;

			stream <<"\t<GreedyAtlasScaleLevel>"<< endl;
				stream <<"\t\t<ScaleLevel>"<< endl;
					stream <<"\t\t\t<!--factor by which to downsample images-->"<< endl;
					stream <<"\t\t\t<DownsampleFactor val=\"4\" />"<< endl;
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

			stream <<"\t<GreedyAtlasScaleLevel>"<< endl;
				stream <<"\t\t<ScaleLevel>"<< endl;
					stream <<"\t\t\t<!--factor by which to downsample images-->"<< endl;
					stream <<"\t\t\t<DownsampleFactor val=\"2\" />"<< endl;
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

			stream <<"\t<GreedyAtlasScaleLevel>"<< endl;
				stream <<"\t\t<ScaleLevel>"<< endl;
					stream <<"\t\t\t<!--factor by which to downsample images-->"<< endl;
					stream <<"\t\t\t<DownsampleFactor val=\"1\" />"<< endl;
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

			stream <<"\t<!--number of threads to use, 0=one per processor (only for CPU computation)-->"<< endl;
			stream <<"\t<nThreads val=\"4\" />"<< endl;
			stream <<"\t<OutputImageNamePrefix val=\"" << m_OutputPath << "/DTIAtlas/NonLinear_Registration/AverageImage_\" />"<< endl;
			stream <<"\t<OutputDeformedImageNamePrefix val=\"" << m_OutputPath << "/DTIAtlas/NonLinear_Registration/DeformedImage_\" />"<< endl;
			stream <<"\t<OutputHFieldImageNamePrefix val=\"" << m_OutputPath << "/DTIAtlas/NonLinear_Registration/DeformationField_\" />"<< endl;
			stream <<"\t<OutputInvHFieldImageNamePrefix val=\"" << m_OutputPath << "/DTIAtlas/NonLinear_Registration/InverseDeformationField_\" />"<< endl;
			stream <<"\t<OutputFileType val=\"mhd\" />"<< endl;

		stream <<"</ParameterFile>"<< endl;
	}
	else qDebug( "Could not create xml file");
}

void GUI::Config()
{
//////////GUI window creation
	m_dlg = new QDialog(this);
	m_dlg->setWindowTitle ("Configure Softwares");

	QLabel *ImagemathLabel = new QLabel("ImageMath :", this);
	m_ImagemathPath = new QLineEdit;
	QPushButton *ImagemathButton = new QPushButton ("Browse", this);
	QObject::connect(ImagemathButton, SIGNAL(clicked()), m_SoftButtonMapper, SLOT(map()));
	m_SoftButtonMapper->setMapping(ImagemathButton,1);
	QHBoxLayout *ImagemathLayout = new QHBoxLayout;
	ImagemathLayout->addWidget(ImagemathLabel);
	ImagemathLayout->addWidget(m_ImagemathPath);
	ImagemathLayout->addWidget(ImagemathButton);

	QLabel *ResampLabel = new QLabel("ResampleDTIlogEuclidean :", this);
	m_ResampPath = new QLineEdit;
	QPushButton *ResampButton = new QPushButton ("Browse", this);
	QObject::connect(ResampButton, SIGNAL(clicked()), m_SoftButtonMapper, SLOT(map()));
	m_SoftButtonMapper->setMapping(ResampButton,2);
	QHBoxLayout *ResampLayout = new QHBoxLayout;
	ResampLayout->addWidget(ResampLabel);
	ResampLayout->addWidget(m_ResampPath);
	ResampLayout->addWidget(ResampButton);

	QLabel *CropDTILabel = new QLabel("CropDTI :", this);
	m_CropDTIPath = new QLineEdit;
	QPushButton *CropDTIButton = new QPushButton ("Browse", this);
	QObject::connect(CropDTIButton, SIGNAL(clicked()), m_SoftButtonMapper, SLOT(map()));
	m_SoftButtonMapper->setMapping(CropDTIButton,3);
	QHBoxLayout *CropDTILayout = new QHBoxLayout;
	CropDTILayout->addWidget(CropDTILabel);
	CropDTILayout->addWidget(m_CropDTIPath);
	CropDTILayout->addWidget(CropDTIButton);

	QLabel *dtiprocLabel = new QLabel("dtiprocess :", this);
	m_dtiprocPath = new QLineEdit;
	QPushButton *dtiprocButton = new QPushButton ("Browse", this);
	QObject::connect(dtiprocButton, SIGNAL(clicked()), m_SoftButtonMapper, SLOT(map()));
	m_SoftButtonMapper->setMapping(dtiprocButton,4);
	QHBoxLayout *dtiprocLayout = new QHBoxLayout;
	dtiprocLayout->addWidget(dtiprocLabel);
	dtiprocLayout->addWidget(m_dtiprocPath);
	dtiprocLayout->addWidget(dtiprocButton);

	QLabel *BRAINSFitLabel = new QLabel("BRAINSFit :", this);
	m_BRAINSFitPath = new QLineEdit;
	QPushButton *BRAINSFitButton = new QPushButton ("Browse", this);
	QObject::connect(BRAINSFitButton, SIGNAL(clicked()), m_SoftButtonMapper, SLOT(map()));
	m_SoftButtonMapper->setMapping(BRAINSFitButton,5);
	QHBoxLayout *BRAINSFitLayout = new QHBoxLayout;
	BRAINSFitLayout->addWidget(BRAINSFitLabel);
	BRAINSFitLayout->addWidget(m_BRAINSFitPath);
	BRAINSFitLayout->addWidget(BRAINSFitButton);

	QLabel *AWLabel = new QLabel("AtlasWerks :", this);
	m_AWPath = new QLineEdit;
	QPushButton *AWButton = new QPushButton ("Browse", this);
	QObject::connect(AWButton, SIGNAL(clicked()), m_SoftButtonMapper, SLOT(map()));
	m_SoftButtonMapper->setMapping(AWButton,6);
	QHBoxLayout *AWLayout = new QHBoxLayout;
	AWLayout->addWidget(AWLabel);
	AWLayout->addWidget(m_AWPath);
	AWLayout->addWidget(AWButton);

	QLabel *dtiavgLabel = new QLabel("dtiaverage :", this);
	m_dtiavgPath = new QLineEdit;
	QPushButton *dtiavgButton = new QPushButton ("Browse", this);
	QObject::connect(dtiavgButton, SIGNAL(clicked()), m_SoftButtonMapper, SLOT(map()));
	m_SoftButtonMapper->setMapping(dtiavgButton,7);
	QHBoxLayout *dtiavgLayout = new QHBoxLayout;
	dtiavgLayout->addWidget(dtiavgLabel);
	dtiavgLayout->addWidget(m_dtiavgPath);
	dtiavgLayout->addWidget(dtiavgButton);

	QPushButton *OKButton = new QPushButton ("OK", this);
	QObject::connect(OKButton, SIGNAL(clicked()), this, SLOT(ConfigOK()));
	QPushButton *CancelButton = new QPushButton ("Cancel", this);
	QObject::connect(CancelButton, SIGNAL(clicked()), this, SLOT(ConfigCancel()));
	QHBoxLayout *ButtonsLayout = new QHBoxLayout();
	ButtonsLayout->addWidget(OKButton);
	ButtonsLayout->addWidget(CancelButton);

	QLabel *InfoLabel = new QLabel ("Give the path to the following softwares.\nIf no path given, the program will look in the path environnement variable.", this);
	QVBoxLayout *VLayout = new QVBoxLayout();
	VLayout->addWidget(InfoLabel);
	VLayout->addLayout(ImagemathLayout);
	VLayout->addLayout(ResampLayout);
	VLayout->addLayout(CropDTILayout);
	VLayout->addLayout(dtiprocLayout);
	VLayout->addLayout(BRAINSFitLayout);
	VLayout->addLayout(AWLayout);
	VLayout->addLayout(dtiavgLayout);
	VLayout->addLayout(ButtonsLayout);

	m_dlg->setLayout(VLayout);

//////////getting the values
	if( access("DTIAtlasBuilderSoftConfig.txt", F_OK) == 0 ) ///////Test if the config file already exists => unistd::access() returns 0 if F(file)_OK
	{
		std::cout<<"| Config file already exists, getting the paths..."<<std::endl; // command line display

		///get the values from file
		QFile file("DTIAtlasBuilderSoftConfig.txt");
		if (file.open(QFile::ReadOnly))
		{
			QTextStream stream(&file);

			QString line = stream.readLine();
			QStringList list = line.split("=");
			if(!list.at(0).contains(QString("ImageMath")))
			{
				QMessageBox::critical(this, "Corrupt File", "This config file is corrupted");
				return;
			}
			m_ImagemathPath->setText(list.at(1));

			line = stream.readLine();
			list = line.split("=");
			if(!list.at(0).contains(QString("ResampleDTIlogEuclidean")))
			{
				QMessageBox::critical(this, "Corrupt File", "The existing config file is corrupted");
				return;
			}
			m_ResampPath->setText(list.at(1));

			line = stream.readLine();
			list = line.split("=");
			if(!list.at(0).contains(QString("CropDTI")))
			{
				QMessageBox::critical(this, "Corrupt File", "This config file is corrupted");
				return;
			}
			m_CropDTIPath->setText(list.at(1));

			line = stream.readLine();
			list = line.split("=");
			if(!list.at(0).contains(QString("dtiprocess")))
			{
				QMessageBox::critical(this, "Corrupt File", "This config file is corrupted");
				return;
			}
			m_dtiprocPath->setText(list.at(1));

			line = stream.readLine();
			list = line.split("=");
			if(!list.at(0).contains(QString("BRAINSFit")))
			{
				QMessageBox::critical(this, "Corrupt File", "This config file is corrupted");
				return;
			}
			m_BRAINSFitPath->setText(list.at(1));

			line = stream.readLine();
			list = line.split("=");
			if(!list.at(0).contains(QString("AtlasWerks")))
			{
				QMessageBox::critical(this, "Corrupt File", "This config file is corrupted");
				return;
			}
			m_AWPath->setText(list.at(1));

			line = stream.readLine();
			list = line.split("=");
			if(!list.at(0).contains(QString("dtiaverage")))
			{
				QMessageBox::critical(this, "Corrupt File", "This config file is corrupted");
				return;
			}
			m_dtiavgPath->setText(list.at(1));
		} 
		else qDebug( "Could not open file");
	}

	m_dlg->setVisible(!m_dlg->isVisible()); // display the window
}

void GUI::ConfigOK()
{
////////// checking if the given files are executable
	if(!m_ImagemathPath->text().isEmpty() && access(m_ImagemathPath->text().toStdString().c_str(), X_OK)!=0 )
	{
		std::string text = "The file \'" + m_ImagemathPath->text().toStdString() + "\' is not executable";
		QMessageBox::critical(this, "Non executable File", QString(text.c_str()) );
		return;
	}
	if(!m_ResampPath->text().isEmpty() && access(m_ResampPath->text().toStdString().c_str(), X_OK) != 0 )
	{
		std::string text = "The file \'" + m_ResampPath->text().toStdString() + "\' is not executable";
		QMessageBox::critical(this, "Non executable File", QString(text.c_str()) );
		return;
	}
	if(!m_CropDTIPath->text().isEmpty() && access(m_CropDTIPath->text().toStdString().c_str(), X_OK) != 0 )
	{
		std::string text = "The file \'" + m_CropDTIPath->text().toStdString() + "\' is not executable";
		QMessageBox::critical(this, "Non executable File", QString(text.c_str()) );
		return;
	}
	if(!m_dtiprocPath->text().isEmpty() && access(m_dtiprocPath->text().toStdString().c_str(), X_OK) != 0 )
	{
		std::string text = "The file \'" + m_dtiprocPath->text().toStdString() + "\' is not executable";
		QMessageBox::critical(this, "Non executable File", QString(text.c_str()) );
		return;
	}
	if(!m_BRAINSFitPath->text().isEmpty() && access(m_BRAINSFitPath->text().toStdString().c_str(), X_OK) != 0 )
	{
		std::string text = "The file \'" + m_BRAINSFitPath->text().toStdString() + "\' is not executable";
		QMessageBox::critical(this, "Non executable File", QString(text.c_str()) );
		return;
	}
	if(!m_AWPath->text().isEmpty() && access(m_AWPath->text().toStdString().c_str(), X_OK) != 0 )
	{
		std::string text = "The file \'" + m_AWPath->text().toStdString() + "\' is not executable";
		QMessageBox::critical(this, "Non executable File", QString(text.c_str()) );
		return;
	}
	if(!m_dtiavgPath->text().isEmpty() && access(m_dtiavgPath->text().toStdString().c_str(), X_OK) != 0 )
	{
		std::string text = "The file \'" + m_dtiavgPath->text().toStdString() + "\' is not executable";
		QMessageBox::critical(this, "Non executable File", QString(text.c_str()) );
		return;
	}

//////////getting the values and generating the config file
	QFile file("DTIAtlasBuilderSoftConfig.txt");
	if ( file.open( IO_WriteOnly | IO_Translate ) )
	{
		std::cout<<"| Generating config file..."<<std::endl; // command line display

		QTextStream stream( &file );

		stream << "ImageMath=" << m_ImagemathPath->text() << endl;
		stream << "ResampleDTIlogEuclidean=" << m_ResampPath->text() << endl;
		stream << "CropDTI=" << m_CropDTIPath->text() << endl;
		stream << "dtiprocess=" << m_dtiprocPath->text() << endl;
		stream << "BRAINSFit=" << m_BRAINSFitPath->text() << endl;
		stream << "AtlasWerks=" << m_AWPath->text() << endl;
		stream << "dtiaverage=" << m_dtiavgPath->text() << endl;

		std::cout<<"| Config file generated"<<std::endl; // command line display
	}
	else qDebug( "Could not create config file");

//////////closing the dialog window
	m_dlg->setVisible(!m_dlg->isVisible());
}

void GUI::ConfigCancel()
{	
//////////closing the dialog window
	m_dlg->setVisible(!m_dlg->isVisible());
}

void GUI::ReadMe() /////to improve !!
{
/*	QProcess * Process = new QProcess;
	std::string program = "gedit /home/akaiser/Desktop/Projects/DTIAtlasBuilderGUI_07-12/DTIABGUIFinal_07-18-12/src/README.md";
	Process->execute( program.c_str() );*/

	QDialog *dlg = new QDialog(this);
	dlg->setWindowTitle ("Read Me");

	std::string info = "DTIAtlasBuilder\n===============\n\nA tool to create an atlas from several DTI images\n\nThese Softwares need to bee installed before executing the tool :\n= ImageMath\n= ResampleDTIlogEuclidean\n= CropDTI\n= dtiprocess\n= BRAINSFit\n= AtlasWerks\n= dtiaverage\n";
	QLabel *InfoLabel = new QLabel (info.c_str(), this);
	QVBoxLayout *VLayout = new QVBoxLayout();
	VLayout->addWidget(InfoLabel);

	dlg->setLayout(VLayout);

	dlg->setVisible(!dlg->isVisible()); // display the window
}

void GUI::BrowseSoft(int soft) //soft: 1=ImageMath, 2=ResampleDTIlogEuclidean, 3=CropDTI, 4=dtiprocess, 5=BRAINSFit, 6=AtlasWerks, 7=dtiaverage
{
	QString SoftBrowse = QFileDialog::getOpenFileName(this, "Open Software", QString(), "Executable Files (*)");

	switch (soft)
	{
	case 1: m_ImagemathPath->setText(SoftBrowse);
		break;
	case 2: m_ResampPath->setText(SoftBrowse);
		break;
	case 3: m_CropDTIPath->setText(SoftBrowse);
		break;
	case 4: m_dtiprocPath->setText(SoftBrowse);
		break;
	case 5: m_BRAINSFitPath->setText(SoftBrowse);
		break;
	case 6: m_AWPath->setText(SoftBrowse);
		break;
	case 7: m_dtiavgPath->setText(SoftBrowse);
		break;
	}
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
		m_scriptwriter->setRegType(0);
		m_scriptwriter->setTemplatePath(TemplateLineEdit->text().toStdString());
	}

	else m_scriptwriter->setRegType(1); // default
	
	m_scriptwriter->setInterpolType(InterpolTypeComboBox->currentText().toStdString());
	m_scriptwriter->setAverageStatMethod(averageStatMethodComboBox->currentText().toStdString());

	if(OverwritecheckBox->isChecked()) m_scriptwriter->setOverwrite(1);
	else  m_scriptwriter->setOverwrite(0);

	m_scriptwriter->setnbLoops(NbLoopsSpinBox->value());// QLineEdit.value() is an int

//////////Scale Levels :--scaleLevel=4 --numberOfIterations=150 --alpha=1 --beta=1 --gamma=0.0001 --maxPerturbation=0.001 --scaleLevel=2 --numberOfIterations=120 --alpha=1 --beta=1 --gamma=0.001 --maxPerturbation=0.01 --scaleLevel=1 --numberOfIterations=100 --alpha=0.1 --beta=0.1 --gamma=0.01 --maxPerturbation=0.1
/* Default Parameters for AtlasWerks:
Scale                 : 1
alpha                 : 0.01
beta                  : 0.01
gamma                 : 0.001
Max. Pert.            : 0.5
Num. Iterations       : 50
*/
	std::vector< std::vector< double > > T2;
	std::vector < double > T;
	T.push_back(4); // scale level
	T.push_back(nbIter4SpinBox->value()); //nb iterations
	T.push_back(alpha4DoubleSpinBox->value()); //alpha
	T.push_back(beta4DoubleSpinBox->value()); //beta
	T.push_back(gamma4DoubleSpinBox->value()); //gamma
	T.push_back(maxPerturbation4DoubleSpinBox->value()); //max perturbation
	T2.push_back(T);
	T.clear();

	T.push_back(2); // scale level
	T.push_back(nbIter2SpinBox->value()); //nb iterations
	T.push_back(alpha2DoubleSpinBox->value()); //alpha
	T.push_back(beta2DoubleSpinBox->value()); //beta
	T.push_back(gamma2DoubleSpinBox->value()); //gamma
	T.push_back(maxPerturbation2DoubleSpinBox->value()); //max perturbation
	T2.push_back(T);
	T.clear();

	T.push_back(1); // scale level
	T.push_back(nbIter1SpinBox->value()); //nb iterations
	T.push_back(alpha1DoubleSpinBox->value()); //alpha
	T.push_back(beta1DoubleSpinBox->value()); //beta
	T.push_back(gamma1DoubleSpinBox->value()); //gamma
	T.push_back(maxPerturbation1DoubleSpinBox->value()); //max perturbation
	T2.push_back(T);
	T.clear();

	m_scriptwriter->setScaleLevels(T2);
	T2.clear();

////////// Software paths
	std::vector < std::string > SoftPath;

	if( access("DTIAtlasBuilderSoftConfig.txt", F_OK) == 0 ) ///////Test if the config file already exists => unistd::access() returns 0 if F(file)_OK
	{
		std::cout<<"| Config file exists, getting the paths..."<<std::endl; // command line display

		///get the values from file
		QFile file("DTIAtlasBuilderSoftConfig.txt");
		if (file.open(QFile::ReadOnly))
		{
			QTextStream stream(&file);

			QString line = stream.readLine();
			QStringList list = line.split("=");
			if(!list.at(0).contains(QString("ImageMath"))) QMessageBox::critical(this, "Corrupt File", "Config file corrupted: it will not be read");
			else
			{
				SoftPath.push_back(list.at(1).toStdString());

				line = stream.readLine();
				list = line.split("=");
				if(!list.at(0).contains(QString("ResampleDTIlogEuclidean"))) QMessageBox::critical(this, "Corrupt File", "Config file corrupted: it will not be read");
				else
				{
					SoftPath.push_back(list.at(1).toStdString());

					line = stream.readLine();
					list = line.split("=");
					if(!list.at(0).contains(QString("CropDTI"))) QMessageBox::critical(this, "Corrupt File", "Config file corrupted: it will not be read");
					else
					{
						SoftPath.push_back(list.at(1).toStdString());

						line = stream.readLine();
						list = line.split("=");
						if(!list.at(0).contains(QString("dtiprocess"))) QMessageBox::critical(this, "Corrupt File", "Config file corrupted: it will not be read");
						else
						{
							SoftPath.push_back(list.at(1).toStdString());

							line = stream.readLine();
							list = line.split("=");
							if(!list.at(0).contains(QString("BRAINSFit"))) QMessageBox::critical(this, "Corrupt File", "Config file corrupted: it will not be read");
							else
							{
								SoftPath.push_back(list.at(1).toStdString());

								line = stream.readLine();
								list = line.split("=");
								if(!list.at(0).contains(QString("AtlasWerks"))) QMessageBox::critical(this, "Corrupt File", "Config file corrupted: it will not be read");
								else
								{
									SoftPath.push_back(list.at(1).toStdString());

									line = stream.readLine();
									list = line.split("=");
									if(!list.at(0).contains(QString("dtiaverage"))) QMessageBox::critical(this, "Corrupt File", "Config file corrupted: it will not be read");
									else
									{
										SoftPath.push_back(list.at(1).toStdString());
									}

								}
							}
						}
					}
				}
			}
		} 
		else 
		{
			qDebug( "Could not open file");
			SoftPath.push_back("");
			SoftPath.push_back("");
			SoftPath.push_back("");
			SoftPath.push_back("");
			SoftPath.push_back("");
			SoftPath.push_back("");
			SoftPath.push_back("");
		}
	}
	else // if no config file -> empty strings
	{
		SoftPath.push_back("");
		SoftPath.push_back("");
		SoftPath.push_back("");
		SoftPath.push_back("");
		SoftPath.push_back("");
		SoftPath.push_back("");
		SoftPath.push_back("");
	}

	m_scriptwriter->setSoftPath(SoftPath);
	SoftPath.clear();

//////////Launch writing
	if(m_scriptwriter->CheckVoxelSize()==1) 
	{
		QMessageBox::critical(this, "Different Voxel Sizes", "Error: The voxel size of the images\nare not the same,\nplease change dataset"); // returns 0 if voxel size OK , otherwise 1		
		return -1;
	}
	if(m_scriptwriter->CheckSpaceDir()==1) 
	{
		int answer = QMessageBox::question(this, "Different Space Directions", "Warning: The space directions of the images are not the same\nDo you want to execute the script so they will be resampled ?", QMessageBox::Yes | QMessageBox::No);
		if (answer == QMessageBox::No) return -1;
	}
	int Crop=m_scriptwriter->setCroppingSize(); // returns 0 if no cropping , 1 if cropping needed
	if(Crop==1) QMessageBox::warning(this, "Cropping", "Warning: The images do not have the same size, \nso some of them will be cropped");

	m_scriptwriter->WriteScript(); // Master Function

////////// XML file for AtlasWerks
	GenerateXMLForAW();

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

