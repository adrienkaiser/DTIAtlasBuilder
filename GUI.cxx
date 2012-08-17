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

#include "GUI.h"
#include "ScriptWriter.h"

  /////////////////////////////////////////
 //            CONSTRUCTOR              //
/////////////////////////////////////////

GUI::GUI(std::string ParamFile, std::string ConfigFile, std::string CSVFile) : QMainWindow()
{
/*	std::cout<<"Command Line parameter file :"<<ParamFile<<std::endl;
	std::cout<<"Command Line configuration file :"<<ConfigFile<<std::endl;
	std::cout<<"Command Line dataset file :"<<CSVFile<<std::endl;
*/

	setupUi(this);

/* Script writing object */
	m_scriptwriter = new ScriptWriter; // delete in "void GUI::OpenRunningCompleteWindow()"

/* Objects connections */
	QObject::connect(ComputepushButton, SIGNAL(clicked()), this, SLOT(Compute()));
	QObject::connect(BrowseCSVPushButton, SIGNAL(clicked()), this, SLOT(ReadCSVSlot()));
	QObject::connect(SaveCSVPushButton, SIGNAL(clicked()), this, SLOT(SaveCSVDatasetBrowse()));
	QObject::connect(BrowseOutputPushButton, SIGNAL(clicked()), this, SLOT(OpenOutputBrowseWindow()));
	QObject::connect(TemplateBrowsePushButton, SIGNAL(clicked()), this, SLOT(OpenTemplateBrowseWindow()));
	QObject::connect(AddPushButton, SIGNAL(clicked()), this, SLOT(OpenAddCaseBrowseWindow()));
	QObject::connect(RemovePushButton, SIGNAL(clicked()), this, SLOT(RemoveSelectedCases()));
	RemovePushButton->setEnabled(false);

	QObject::connect(actionLoad_parameters, SIGNAL(triggered()), this, SLOT(LoadParametersSlot()));
	QObject::connect(actionSave_parameters, SIGNAL(triggered()), this, SLOT(SaveParameters()));
	QObject::connect(actionExit, SIGNAL(triggered()), qApp, SLOT(quit()));
	QObject::connect(actionLoad_Software_Configuration, SIGNAL(triggered()), this, SLOT(LoadConfigSlot()));
	QObject::connect(actionSave_Software_Configuration, SIGNAL(triggered()), this, SLOT(SaveConfig()));
	QObject::connect(actionRead_Me, SIGNAL(triggered()), this, SLOT(ReadMe()));

	QObject::connect(InterpolTypeComboBox, SIGNAL(currentIndexChanged (int)), this, SLOT(InterpolTypeComboBoxChanged(int)));
	QObject::connect(TensInterpolComboBox, SIGNAL(currentIndexChanged (int)), this, SLOT(TensorInterpolComboBoxChanged(int)));

	QObject::connect(this, SIGNAL(runningcomplete()), this, SLOT(OpenRunningCompleteWindow()));
	QObject::connect(this, SIGNAL(runningfail()), this, SLOT(OpenRunningFailWindow()));

/* When any value changed, the value of m_ParamSaved is set to 0 */
	QObject::connect(TemplateLineEdit, SIGNAL(textChanged(QString)), this, SLOT(WidgetHasChangedParamNoSaved()));
	QObject::connect(OutputFolderLineEdit, SIGNAL(textChanged(QString)), this, SLOT(WidgetHasChangedParamNoSaved()));
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

/* Variables */
	m_ParamFileHeader = QString("DTIAtlasBuilderParameterFileVersion");
	m_CSVseparator = QString(",");
	m_ParamSaved=0;
	m_lastCasePath="";

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

	QObject::connect(DefaultButton, SIGNAL(clicked()), this, SLOT(ConfigDefault()));
	QObject::connect(setenvButton, SIGNAL(clicked()), this, SLOT(ConfigEnv()));

/* Init options for Final AtlasBuilding param : Interpolation algo */
	m_optionStackLayout = new QStackedWidget;
	optionHLayout->addWidget(m_optionStackLayout);

	QHBoxLayout *emptyLayout= new QHBoxLayout;
	QWidget *emptyWidget= new QWidget;
	emptyWidget->setLayout(emptyLayout);
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

	QHBoxLayout *logEmptyLayout= new QHBoxLayout;
	QWidget *logEmptyWidget= new QWidget;
	logEmptyWidget->setLayout(logEmptyLayout);
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

/* SET the soft config from an env variable or look in the PATH */
/*
	if there is an env variable
		then copy the paths from this variable SoftEnvSet();
//////////// POP-UP window : "The following softwares weren't found: please give the path to these."
	else ConfigDefault(); (look for the programs with the itk function)
*/
	ConfigDefault();

/* Load Parameters from Command Line */
	if( !ParamFile.empty() ) LoadParameters( QString(ParamFile.c_str()) );
	if( !CSVFile.empty() ) ReadCSV( QString(CSVFile.c_str()) );
	if( !ConfigFile.empty() ) LoadConfig( QString(ConfigFile.c_str()) );
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
		if ( CaseListWidget->count()>0 ) RemovePushButton->setEnabled(true);
		m_ParamSaved=0;
		SelectCasesLabel->setText( QString("") );
		m_lastCasePath = CaseListBrowse.last();
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
		if ( CaseListWidget->count()==0 ) RemovePushButton->setEnabled(false);
		m_ParamSaved=0;
		SelectCasesLabel->setText( QString("") );
	}
}

  /////////////////////////////////////////
 //               OUTPUT                //
/////////////////////////////////////////

void GUI::OpenOutputBrowseWindow() /*SLOT*/
{
	QString OutputBrowse=QFileDialog::getExistingDirectory(this);
	OutputFolderLineEdit->setText(OutputBrowse);
}

  /////////////////////////////////////////
 //              TEMPLATE               //
/////////////////////////////////////////

void GUI::OpenTemplateBrowseWindow() /*SLOT*/
{
	QString TemplateBrowse=QFileDialog::getOpenFileName(this, "Open Atlas Template", QString(), "NERD Image (*.nrrd *.nhdr *.*)");
	TemplateLineEdit->setText(TemplateBrowse);
}

  /////////////////////////////////////////
 //          RUNNING FINISHED           //
/////////////////////////////////////////

void GUI::OpenRunningCompleteWindow() /*SLOT*/
{
	std::string program;
	int ExitCode=0;

/* Remove temp files ? */
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

/* open containing Folder ? */
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

void GUI::OpenRunningFailWindow() /*SLOT*/
{
	QMessageBox::critical(this, "Running Fail", "Running Failed...\nClick OK to exit");
	delete m_scriptwriter;
 	qApp->quit(); //end of Application: close the main window
}

  /////////////////////////////////////////
 //            CLOSE WINDOW             //
/////////////////////////////////////////

void GUI::closeEvent(QCloseEvent* event)
{
	while(m_ParamSaved==0)
	{
		int ret = QMessageBox::question(this,"Quit","Last parameters have not been saved.\nDo you want to save the last parameters ?",QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes);
		if (ret == QMessageBox::Yes) SaveParameters();
		else if (ret == QMessageBox::No) break;
		else if (ret == QMessageBox::Cancel) 
		{
			event->ignore();
			return;
		}
	}
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
	else std::cout<<"| No file given"<<std::endl;
}

void GUI::ReadCSV(QString CSVfile)
{	
	if(!CSVfile.isEmpty())
	{

	QFile file(CSVfile);

	if (file.open(QFile::ReadOnly))
	{
		std::cout<<"| Loading csv file..."; // command line display

		QTextStream stream(&file);
		while(!stream.atEnd()) //read all the lines
		{
			QString line = stream.readLine();
			QStringList list = line.split(m_CSVseparator);
			if( list.at(0).at(0).toAscii() != '#' )  CaseListWidget->addItem( list.at(1) ); //display in the Widget so that some can be removed
		}
	
		SelectCasesLabel->setText( QString("Current CSV file : ") + CSVfile );
		m_ParamSaved=0;
		std::cout<<"DONE"<<std::endl; // command line display
	} 
	else
	{
		SelectCasesLabel->setText( QString("Could not open CSV File"));
		qDebug( "Could not open csv file");
	}

	if ( CaseListWidget->count()>0 ) RemovePushButton->setEnabled(true);

	}
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

	QString m_CSVseparator = QString(",");

	QFile file(CSVBrowseName);

	if ( file.open( IO_WriteOnly | IO_Translate ) )
	{
		std::cout<<"| Generating Dataset csv file..."; // command line display

		QTextStream stream( &file );
		stream << QString("#") << m_CSVseparator << QString("Original DTI Image") << endl;
		for(int i=0; i < CaseListWidget->count() ;i++) stream << i+1 << m_CSVseparator << CaseListWidget->item(i)->text() << endl;
		std::cout<<"DONE"<<std::endl; // command line display
	
		SelectCasesLabel->setText( QString("Current CSV file : ") + CSVBrowseName );
		QMessageBox::information(this, "Saving succesful", "Dataset has been succesfully saved at" + CSVBrowseName);		
	}
	else qDebug( "Could not create file");

	}
	else std::cout<<"| No file given"<<std::endl;
}

void GUI::SaveCSVDataset()
{	
	QString m_CSVseparator = QString(",");

	QString csvPath;
	csvPath = m_OutputPath + QString("/DTIAtlas/DTIAtlasBuilderDataSet.csv");
	QFile file(csvPath);

	if ( file.open( IO_WriteOnly | IO_Translate ) )
	{
		std::cout<<"| Generating Dataset csv file..."; // command line display

		QTextStream stream( &file );
		stream << QString("#") << m_CSVseparator << QString("Original DTI Image") << endl;
		for(int i=0; i < CaseListWidget->count() ;i++) stream << i+1 << m_CSVseparator << CaseListWidget->item(i)->text() << endl;
		std::cout<<"DONE"<<std::endl; // command line display
		
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
		std::cout<<"| Generating Results csv file..."; // command line display

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

		std::cout<<"DONE"<<std::endl; // command line display
		
	}
	else qDebug( "Could not create file");
}

  /////////////////////////////////////////
 //             PARAMETERS              //
/////////////////////////////////////////

void GUI::SaveParameters() /*SLOT*/
{
	if(CaseListWidget->count()==0)
	{
		QMessageBox::critical(this, "No Dataset", "No Dataset");
		return;
	}

	QString ParamBrowseName=QFileDialog::getSaveFileName(this, tr("Save Parameter File"),"./DTIAtlasBuilderParameters.txt",tr("Text File (*.txt)"));
	QString CSVFileName = ParamBrowseName.split(".").at(0) + QString(".csv");

	if(!ParamBrowseName.isEmpty())
	{

	QFile file(ParamBrowseName);
	if ( file.open( IO_WriteOnly | IO_Translate ) )
	{
		std::cout<<"| Saving Parameters file..."; // command line display

		QTextStream stream( &file );

		stream << m_ParamFileHeader << "=1" << endl;
		stream << "Output Folder=" << OutputFolderLineEdit->text() << endl;
		stream << "Atlas Template=" << TemplateLineEdit->text() << endl;
		stream << "Loops for the registration=" << NbLoopsSpinBox->value() << endl;
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
		stream << "CSV Dataset File=" << CSVFileName << endl;

		std::cout<<"DONE"<<std::endl; // command line display

		QFile filecsv(CSVFileName);
		if ( filecsv.open( IO_WriteOnly | IO_Translate ) )
		{
			std::cout<<"| Generating Dataset csv file..."; // command line display

			QTextStream streamcsv( &filecsv );
			streamcsv << QString("#") << m_CSVseparator << QString("Original DTI Image") << endl;
			for(int i=0; i < CaseListWidget->count() ;i++) streamcsv << i+1 << m_CSVseparator << CaseListWidget->item(i)->text() << endl;
			std::cout<<"DONE"<<std::endl; // command line display
		
			SelectCasesLabel->setText( QString("Current CSV file : ") + CSVFileName );
		}
		else 
		{
			std::cout<<"FAILED"<<std::endl; // command line display
			qDebug( "Could not create csv file");
		}

		QMessageBox::information(this, "Saving succesful", "Parameters have been succesfully saved at" + ParamBrowseName);
		m_ParamSaved=1;
	}
	else qDebug( "Could not create parameter file");

	}
	else std::cout<<"| No file given"<<std::endl;
}

void GUI::LoadParametersSlot() /*SLOT*/
{
	QString ParamBrowse=QFileDialog::getOpenFileName(this, "Open Parameter File", QString(), ".txt Files (*.txt)");

	if(!ParamBrowse.isEmpty())
	{
		LoadParameters(ParamBrowse);
	}
	else std::cout<<"| No file given"<<std::endl;
}

void GUI::LoadParameters(QString paramFile)
{
	if( access(paramFile.toStdString().c_str(), F_OK) == 0 ) // Test if the config file already exists => unistd::access() returns 0 if F(file)_OK
	{

	QFile file(paramFile);

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

		std::cout<<"| Loading Parameters file..."; // command line display

/* Other Parameters */
		line = stream.readLine();
		list = line.split("=");
		if(!list.at(0).contains(QString("Output Folder")))
		{
			QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
			std::cout<<"FAILED"<<std::endl; // command line display
			return;
		}
		OutputFolderLineEdit->setText(list.at(1));


		line = stream.readLine();
		list = line.split("=");
		if(!list.at(0).contains(QString("Atlas Template")))
		{
			QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
			std::cout<<"FAILED"<<std::endl; // command line display
			return;
		}
		TemplateLineEdit->setText(list.at(1));

		line = stream.readLine();
		list = line.split("=");
		if(!list.at(0).contains(QString("Loops for the registration")))
		{
			QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
			std::cout<<"FAILED"<<std::endl; // command line display
			return;
		}
		NbLoopsSpinBox->setValue( list.at(1).toInt() );

		line = stream.readLine();
		list = line.split("=");
		if(!list.at(0).contains(QString("Overwrite")))
		{
			QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
			std::cout<<"FAILED"<<std::endl; // command line display
			return;
		}
		if( list.at(1) == QString("true") ) OverwritecheckBox->setChecked(true);

/* Scale Levels */
		line = stream.readLine();
		list = line.split("=");
		if(!list.at(0).contains(QString("Scale Level")))
		{
			QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
			std::cout<<"FAILED"<<std::endl; // command line display
			return;
		}
		else
		{
			QStringList nbrs= list.at(1).split(",");
			if( nbrs.size()!=7 )
			{
				QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
				std::cout<<"FAILED"<<std::endl; // command line display
				return;
			}
			if(nbrs.at(0).toInt()==1)
			{
				if( (nbrs.at(1).toInt()==0) || (nbrs.at(2).toInt()==0) || (nbrs.at(3).toDouble()==0) || (nbrs.at(4).toDouble()==0) || (nbrs.at(5).toDouble()==0) || (nbrs.at(6).toDouble()==0))
				{
					QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
					std::cout<<"FAILED"<<std::endl; // command line display
					return;
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
			QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
			std::cout<<"FAILED"<<std::endl; // command line display
			return;
		}
		else
		{
			QStringList nbrs= list.at(1).split(",");
			if( nbrs.size()!=7 )
			{
				QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
				std::cout<<"FAILED"<<std::endl; // command line display
				return;
			}
			if(nbrs.at(0).toInt()==1)
			{
				if( (nbrs.at(1).toInt()==0) || (nbrs.at(2).toInt()==0) || (nbrs.at(3).toDouble()==0) || (nbrs.at(4).toDouble()==0) || (nbrs.at(5).toDouble()==0) || (nbrs.at(6).toDouble()==0))
				{
					QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
					std::cout<<"FAILED"<<std::endl; // command line display
					return;
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
			QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
			std::cout<<"FAILED"<<std::endl; // command line display
			return;
		}
		else
		{
			QStringList nbrs= list.at(1).split(",");
			if( nbrs.size()!=7 )
			{
				QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
				std::cout<<"FAILED"<<std::endl; // command line display
				return;
			}
			if(nbrs.at(0).toInt()==1)
			{
				if( (nbrs.at(1).toInt()==0) || (nbrs.at(2).toInt()==0) || (nbrs.at(3).toDouble()==0) || (nbrs.at(4).toDouble()==0) || (nbrs.at(5).toDouble()==0) || (nbrs.at(6).toDouble()==0))
				{
					QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
					std::cout<<"FAILED"<<std::endl; // command line display
					return;
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
			QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
			std::cout<<"FAILED"<<std::endl; // command line display
			return;
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
				QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
				std::cout<<"FAILED"<<std::endl; // command line display
				return;
			}
		}
		else if( list.at(1).contains(QString("BSpline")) ) 
		{ 
			InterpolTypeComboBox->setCurrentIndex(3);
			if( list.at(2).toInt()>=0 && list.at(2).toInt()<=5 ) m_BSplineComboBox->setCurrentIndex( list.at(2).toInt()-1 );
			else
			{
				QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
				std::cout<<"FAILED"<<std::endl; // command line display
				return;
			}
		}
		else
		{
			QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
			std::cout<<"FAILED"<<std::endl; // command line display
			return;
		}

		line = stream.readLine();
		list = line.split("=");
		if(!list.at(0).contains(QString("Tensor interpolation")))
		{
			QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
			std::cout<<"FAILED"<<std::endl; // command line display
			return;
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
				QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
				std::cout<<"FAILED"<<std::endl; // command line display
				return;
			}
		}
		else if( list.at(1).contains(QString("Log Euclidean")) ) TensInterpolComboBox->setCurrentIndex(0);
		else
		{
			QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
			std::cout<<"FAILED"<<std::endl; // command line display
			return;
		}

		line = stream.readLine();
		list = line.split("=");
		if(!list.at(0).contains(QString("Tensor transformation")))
		{
			QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
			std::cout<<"FAILED"<<std::endl; // command line display
			return;
		}
		if( list.at(1).contains(QString("PPD")) ) TensTfmComboBox->setCurrentIndex(0);
		else if( list.at(1).contains(QString("FS")) ) TensTfmComboBox->setCurrentIndex(1);
		else
		{
			QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
			std::cout<<"FAILED"<<std::endl; // command line display
			return;
		}

		line = stream.readLine();
		list = line.split("=");
		if(!list.at(0).contains(QString("DTI Average Statistics Method")))
		{
			QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
			std::cout<<"FAILED"<<std::endl; // command line display
			return;
		}
		if( list.at(1).contains(QString("PGA")) ) averageStatMethodComboBox->setCurrentIndex(0);
		else if( list.at(1).contains(QString("Log Euclidean")) ) averageStatMethodComboBox->setCurrentIndex(2);
		else if( list.at(1).contains(QString("Euclidean")) ) averageStatMethodComboBox->setCurrentIndex(1);
		else
		{
			QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
			std::cout<<"FAILED"<<std::endl; // command line display
			return;
		}

		std::cout<<"DONE"<<std::endl; // command line display

/* Opening CSV File */
		line = stream.readLine();
		list = line.split("=");
		if(!list.at(0).contains(QString("CSV Dataset File")))
		{
			QMessageBox::critical(this, "Corrupt File", "This parameter file is corrupted");
			std::cout<<"FAILED"<<std::endl; // command line display
			return;
		}
		QString CSVpath = list.at(1);
		CaseListWidget->clear();
		ReadCSV(CSVpath);

		m_ParamSaved=1;

	} 
	else if ( !paramFile.isEmpty() ) qDebug( "Could not open file");

	}
}

  /////////////////////////////////////////
 //              XML FILE               //
/////////////////////////////////////////

void GUI::GenerateXMLForAW()
{	
	if( access((m_OutputPath.toStdString() + "/DTIAtlas/NonLinear_Registration").c_str(), F_OK) != 0 ) // Test if the main folder does not exists => unistd::access() returns 0 if F(file)_OK
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
					stream <<"\t\t\t<Filename val=\"" << m_OutputPath << "/DTIAtlas/Affine_Registration/Case" << i_str.c_str() << "_Loop" << nbLoops_str.c_str() << "_FinalFA.nrrd\" />"<< endl;
					stream <<"\t\t\t<ItkTransform val=\"1\" />"<< endl;
				stream <<"\t\t</WeightedImage>"<< endl;
			}
			stream <<"\t</WeightedImageSet>"<< endl;

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
			stream <<"\t<OutputImageNamePrefix val=\"" << m_OutputPath << "/DTIAtlas/NonLinear_Registration/AverageImage_\" />"<< endl;
			stream <<"\t<OutputDeformedImageNamePrefix val=\"" << m_OutputPath << "/DTIAtlas/NonLinear_Registration/DeformedImage_\" />"<< endl;
			stream <<"\t<OutputHFieldImageNamePrefix val=\"" << m_OutputPath << "/DTIAtlas/NonLinear_Registration/DeformationField_\" />"<< endl;
			stream <<"\t<OutputInvHFieldImageNamePrefix val=\"" << m_OutputPath << "/DTIAtlas/NonLinear_Registration/InverseDeformationField_\" />"<< endl;
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
	else std::cout<<"| No file given"<<std::endl;
}

void GUI::LoadConfig(QString configFile)
{
/* getting the values */
	if( access(configFile.toStdString().c_str(), F_OK) == 0 ) // Test if the config file already exists => unistd::access() returns 0 if F(file)_OK
	{
		std::cout<<"| Loading Configuration file..."; // command line display

		///get the values from file
		QFile file(configFile);
		if (file.open(QFile::ReadOnly))
		{
			QTextStream stream(&file);

			QString line = stream.readLine();
			QStringList list = line.split("=");
			if(!list.at(0).contains(QString("ImageMath")))
			{
				QMessageBox::critical(this, "Corrupt File", "This config file is corrupted");
				std::cout<<"FAILED"<<std::endl; // command line display
				return;
			}
			if(!list.at(1).isEmpty()) ImagemathPath->setText(list.at(1));

			line = stream.readLine();
			list = line.split("=");
			if(!list.at(0).contains(QString("ResampleDTIlogEuclidean")))
			{
				QMessageBox::critical(this, "Corrupt File", "The existing config file is corrupted");
				std::cout<<"FAILED"<<std::endl; // command line display
				return;
			}
			if(!list.at(1).isEmpty()) ResampPath->setText(list.at(1));

			line = stream.readLine();
			list = line.split("=");
			if(!list.at(0).contains(QString("CropDTI")))
			{
				QMessageBox::critical(this, "Corrupt File", "This config file is corrupted");
				std::cout<<"FAILED"<<std::endl; // command line display
				return;
			}
			if(!list.at(1).isEmpty()) CropDTIPath->setText(list.at(1));

			line = stream.readLine();
			list = line.split("=");
			if(!list.at(0).contains(QString("dtiprocess")))
			{
				QMessageBox::critical(this, "Corrupt File", "This config file is corrupted");
				std::cout<<"FAILED"<<std::endl; // command line display
				return;
			}
			if(!list.at(1).isEmpty()) dtiprocPath->setText(list.at(1));

			line = stream.readLine();
			list = line.split("=");
			if(!list.at(0).contains(QString("BRAINSFit")))
			{
				QMessageBox::critical(this, "Corrupt File", "This config file is corrupted");
				std::cout<<"FAILED"<<std::endl; // command line display
				return;
			}
			if(!list.at(1).isEmpty()) BRAINSFitPath->setText(list.at(1));

			line = stream.readLine();
			list = line.split("=");
			if(!list.at(0).contains(QString("AtlasWerks")))
			{
				QMessageBox::critical(this, "Corrupt File", "This config file is corrupted");
				std::cout<<"FAILED"<<std::endl; // command line display
				return;
			}
			if(!list.at(1).isEmpty()) AWPath->setText(list.at(1));

			line = stream.readLine();
			list = line.split("=");
			if(!list.at(0).contains(QString("dtiaverage")))
			{
				QMessageBox::critical(this, "Corrupt File", "This config file is corrupted");
				std::cout<<"FAILED"<<std::endl; // command line display
				return;
			}
			if(!list.at(1).isEmpty()) dtiavgPath->setText(list.at(1));

			std::cout<<"DONE"<<std::endl; // command line display
		} 
		else qDebug( "Could not open file");
	}
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
			std::cout<<"| Generating config file..."; // command line display

			QTextStream stream( &file );

			stream << "ImageMath=" << ImagemathPath->text() << endl;
			stream << "ResampleDTIlogEuclidean=" << ResampPath->text() << endl;
			stream << "CropDTI=" << CropDTIPath->text() << endl;
			stream << "dtiprocess=" << dtiprocPath->text() << endl;
			stream << "BRAINSFit=" << BRAINSFitPath->text() << endl;
			stream << "AtlasWerks=" << AWPath->text() << endl;
			stream << "dtiaverage=" << dtiavgPath->text() << endl;

			std::cout<<"DONE"<<std::endl; // command line display
		}
		else 
		{
			std::cout<<"FAILED"<<std::endl; // command line display
			qDebug( "Could not create config file");
		}
	}
	else std::cout<<"| No file given"<<std::endl;
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

	std::cout<<"| Searching the softwares..."; // command line display

	std::string program;
	std::string notFound;

	program = itksys::SystemTools::FindProgram("ImageMath");
	if(program.empty()) notFound = notFound + "> ImageMath\n";
	else ImagemathPath->setText(QString(program.c_str()));

	program = itksys::SystemTools::FindProgram("ResampleDTIlogEuclidean");
	if(program.empty()) notFound = notFound + "> ResampleDTIlogEuclidean\n";
	else ResampPath->setText(QString(program.c_str()));

	program = itksys::SystemTools::FindProgram("CropDTI");
	if(program.empty()) notFound = notFound + "> CropDTI\n";
	else CropDTIPath->setText(QString(program.c_str()));

	program = itksys::SystemTools::FindProgram("dtiprocess");
	if(program.empty()) notFound = notFound + "> dtiprocess\n";
	else dtiprocPath->setText(QString(program.c_str()));

	program = itksys::SystemTools::FindProgram("BRAINSFit");
	if(program.empty()) notFound = notFound + "> BRAINSFit\n";
	else BRAINSFitPath->setText(QString(program.c_str()));

	program = itksys::SystemTools::FindProgram("AtlasWerks");
	if(program.empty()) notFound = notFound + "> AtlasWerks\n";
	else AWPath->setText(QString(program.c_str()));

	program = itksys::SystemTools::FindProgram("dtiaverage");
	if(program.empty()) notFound = notFound + "> dtiaverage\n";
	else dtiavgPath->setText(QString(program.c_str()));

	std::cout<<"DONE"<<std::endl; // command line display

	if( !notFound.empty() )
	{
		std::string text = "The following programs are missing.\nPlease enter the path manually:\n" + notFound;
		QMessageBox::warning(this, "Program missing", QString(text.c_str()) ); // POP-UP window : "The following softwares weren't found: please give the path to these."
	}
}

void GUI::ConfigEnv() /*SLOT*/
{
//create an env variable with the paths given in ths lineEdits

	std::cout<<"| Setting the environment variable..."; // command line display
	std::cout<<"DONE"<<std::endl; // command line display
}

void GUI::BrowseSoft(int soft)  /*SLOT*/ //softwares: 1=ImageMath, 2=ResampleDTIlogEuclidean, 3=CropDTI, 4=dtiprocess, 5=BRAINSFit, 6=AtlasWerks, 7=dtiaverage
{
	QString SoftBrowse = QFileDialog::getOpenFileName(this, "Open Software", QString(), "Executable Files (*)");

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
	case 6: AWPath->setText(SoftBrowse);
		break;
	case 7: dtiavgPath->setText(SoftBrowse);
		break;
	}
}

void GUI::SoftEnvSet()
{
	// set the softwares with the env variable
}

  /////////////////////////////////////////
 //               READ ME               //
/////////////////////////////////////////

void GUI::ReadMe()  /*SLOT*/ /////to enhance !!
{
/*	QProcess * Process = new QProcess;
	std::string program = "gedit /home/akaiser/Desktop/Projects/DTIAtlasBuilderGUI_07-12/DTIABGUIFinal_07-18-12/src/README.md";
	Process->execute( program.c_str() );
*/
	QDialog *dlg = new QDialog(this);
	dlg->setWindowTitle ("Read Me");

	std::string info = "DTIAtlasBuilder\n===============\n\nA tool to create an atlas from several DTI images\n\nThese Softwares need to be installed before executing the tool :\n- ImageMath\n- ResampleDTIlogEuclidean\n- CropDTI\n- dtiprocess\n- BRAINSFit\n- AtlasWerks\n- dtiaverage\n";
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
//		std::cerr<<"ExceptionObject caught! : " << Image << std::endl;
//		std::cerr<<err<<std::endl;
		return 1;
	}
//std::cout<< reader-> GetOutput()->ImageDimension <<std::endl;
	region = reader->GetOutput()->GetLargestPossibleRegion();
//std::cout << region.GetSize()[0] << "," << region.GetSize()[1] << "," << region.GetSize()[2] << "," << region.GetSize()[3] << std::endl;
	if( reader->GetOutput()->ImageDimension != 4 ) return 2;
	return 0;
}

  /////////////////////////////////////////
 //           MAIN FUNCTIONS            //
/////////////////////////////////////////

void GUI::Compute() /*SLOT*/
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

/* Checking and Setting the values */

/* Cases */
	for(int i=0; i < CaseListWidget->count() ;i++) 
	{
		if( access(CaseListWidget->item(i)->text().toStdString().c_str(), F_OK) != 0 ) // Test if the case files exist => unistd::access() returns 0 if F(file)_OK
		{
			std::string text = "This file does not exist :\n" + CaseListWidget->item(i)->text().toStdString();
			QMessageBox::critical(this, "Case does not exist", QString(text.c_str()) );
			return -1;
		}
		int checkIm = checkImage(CaseListWidget->item(i)->text().toStdString()); // returns 1 if not an image, 2 if not a dti, otherwise 0
		if( checkIm == 1 ) // returns 1 if not an image, 2 if not a dti, otherwise 0
		{
			std::string text = "This file is not an image :\n" + CaseListWidget->item(i)->text().toStdString();
			QMessageBox::critical(this, "No image", QString(text.c_str()) );
			return -1;
		}
		if( checkIm == 2 ) // returns 1 if not an image, 2 if not a dti, otherwise 0
		{
			std::string text = "This image is not a DTI :\n" + CaseListWidget->item(i)->text().toStdString();
			QMessageBox::critical(this, "No DTI", QString(text.c_str()) );
			return -1;
		}
		m_CasesPath.push_back( CaseListWidget->item(i)->text().toStdString() );
	}
	m_scriptwriter->setCasesPath(m_CasesPath); // m_CasesPath is a vector

/* Output */
	m_OutputPath=OutputFolderLineEdit->text();
	if( access(m_OutputPath.toStdString().c_str(), F_OK) != 0 ) // Test if the folder exists => unistd::access() returns 0 if F(file)_OK
	{
		QMessageBox::critical(this, "No Output Folder", "The output folder does not exist");
		return -1;
	}
	if( access(m_OutputPath.toStdString().c_str(), W_OK) != 0 ) // Test if the program can write in the output folder => unistd::access() returns 0 if W(write)_OK
	{
		QMessageBox::critical(this, "Output Folder Unwritable", "Please give an output folder authorized in reading");
		return -1;
	}
	m_scriptwriter->setOutputPath(m_OutputPath.toStdString());

	if( access((m_OutputPath.toStdString() + "/DTIAtlas").c_str(), F_OK) != 0 ) // Test if the main folder does not exists => unistd::access() returns 0 if F(file)_OK
	{
		std::cout<<"| Creating Main directory..."<<std::endl; // command line display
		QProcess * mkdirMainProcess = new QProcess;
		program = "mkdir " + m_OutputPath.toStdString() + "/DTIAtlas"; //  Creates the directory
		ExitCode = mkdirMainProcess->execute( program.c_str() );
	}

	if( access((m_OutputPath.toStdString() + "/DTIAtlas/Script").c_str(), F_OK) != 0 ) // Test if the script folder does not exists => unistd::access() returns 0 if F(file)_OK
	{
		std::cout<<"| Creating Script directory..."<<std::endl; // command line display
		QProcess * mkdirScriptProcess = new QProcess;
		program = "mkdir " + m_OutputPath.toStdString() + "/DTIAtlas/Script";
		ExitCode = mkdirScriptProcess->execute( program.c_str() );
	}

/* Template */
	if (!TemplateLineEdit->text().isEmpty()) 
	{
		if( access(TemplateLineEdit->text().toStdString().c_str(), F_OK) != 0 ) // Test if the case files exist => unistd::access() returns 0 if F(file)_OK
		{
			std::string text = "This template file does not exist :\n" + TemplateLineEdit->text().toStdString();
			QMessageBox::critical(this, "Template does not exist", QString(text.c_str()) );
			return -1;
		}
		int checkImTemp = checkImage(TemplateLineEdit->text().toStdString()); // returns 1 if not an image, 2 if not a dti, otherwise 0
		if( checkImTemp == 1 ) // returns 1 if not an image, 2 if not a dti, otherwise 0
		{
			std::string text = "This file is not an image :\n" + TemplateLineEdit->text().toStdString();
			QMessageBox::critical(this, "No image", QString(text.c_str()) );
			return -1;
		}
		if( checkImTemp == 0 ) // returns 1 if not an image, 2 if not a dti, otherwise 0 //the template has to be a FA image !!
		{
			std::string text = "This image is not a FA :\n" + TemplateLineEdit->text().toStdString();
			QMessageBox::critical(this, "No FA", QString(text.c_str()) );
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

	if(OverwritecheckBox->isChecked()) m_scriptwriter->setOverwrite(1);
	else  m_scriptwriter->setOverwrite(0);

	m_scriptwriter->setnbLoops(NbLoopsSpinBox->value());// QLineEdit.value() is an int

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
/*	std::vector< std::vector< double > > T2;
	std::vector < double > T;

	if(SL4checkBox->isChecked())
	{
		T.push_back(SL4spinBox->value()); // scale level
		T.push_back(nbIter4SpinBox->value()); //nb iterations
		T.push_back(alpha4DoubleSpinBox->value()); //alpha
		T.push_back(beta4DoubleSpinBox->value()); //beta
		T.push_back(gamma4DoubleSpinBox->value()); //gamma
		T.push_back(maxPerturbation4DoubleSpinBox->value()); //max perturbation
		T2.push_back(T);
		T.clear();
	}

	if(SL2checkBox->isChecked())
	{
		T.push_back(SL2spinBox->value()); // scale level
		T.push_back(nbIter2SpinBox->value()); //nb iterations
		T.push_back(alpha2DoubleSpinBox->value()); //alpha
		T.push_back(beta2DoubleSpinBox->value()); //beta
		T.push_back(gamma2DoubleSpinBox->value()); //gamma
		T.push_back(maxPerturbation2DoubleSpinBox->value()); //max perturbation
		T2.push_back(T);
		T.clear();
	}

	if(SL1checkBox->isChecked())
	{
		T.push_back(SL1spinBox->value()); // scale level
		T.push_back(nbIter1SpinBox->value()); //nb iterations
		T.push_back(alpha1DoubleSpinBox->value()); //alpha
		T.push_back(beta1DoubleSpinBox->value()); //beta
		T.push_back(gamma1DoubleSpinBox->value()); //gamma
		T.push_back(maxPerturbation1DoubleSpinBox->value()); //max perturbation
		T2.push_back(T);
		T.clear();
	}

	m_scriptwriter->setScaleLevels(T2);
	T2.clear();
*/

/* Software paths */
	std::vector < std::string > SoftPath;

/* Checking if all the programs have been given */
	std::string programPath;
	std::string notFound;

	if(ImagemathPath->text().isEmpty()) // + Look in the env variable ?
	{
		programPath = itksys::SystemTools::FindProgram("ImageMath");
		if(programPath.empty()) notFound = notFound + "> ImageMath\n";
		else ImagemathPath->setText(QString(programPath.c_str()));
	}
	if(ResampPath->text().isEmpty())
	{
		programPath = itksys::SystemTools::FindProgram("ResampleDTIlogEuclidean");
		if(programPath.empty()) notFound = notFound + "> ResampleDTIlogEuclidean\n";
		else ImagemathPath->setText(QString(programPath.c_str()));
	}
	if(CropDTIPath->text().isEmpty())
	{
		programPath = itksys::SystemTools::FindProgram("CropDTI");
		if(programPath.empty()) notFound = notFound + "> CropDTI\n";
		else ImagemathPath->setText(QString(programPath.c_str()));
	}
	if(dtiprocPath->text().isEmpty())
	{
		programPath = itksys::SystemTools::FindProgram("dtiprocess");
		if(programPath.empty()) notFound = notFound + "> dtiprocess\n";
		else ImagemathPath->setText(QString(programPath.c_str()));
	}
	if(BRAINSFitPath->text().isEmpty())
	{
		programPath = itksys::SystemTools::FindProgram("BRAINSFit");
		if(programPath.empty()) notFound = notFound + "> BRAINSFit\n";
		else ImagemathPath->setText(QString(programPath.c_str()));
	}
	if(AWPath->text().isEmpty())
	{
		programPath = itksys::SystemTools::FindProgram("AtlasWerks");
		if(programPath.empty()) notFound = notFound + "> AtlasWerks\n";
		else ImagemathPath->setText(QString(programPath.c_str()));
	}
	if(dtiavgPath->text().isEmpty())
	{
		programPath = itksys::SystemTools::FindProgram("dtiaverage");
		if(programPath.empty()) notFound = notFound + "> dtiaverage\n";
		else ImagemathPath->setText(QString(programPath.c_str()));
	}

	if( !notFound.empty() )
	{
		std::string text = "The following programs are missing.\nPlease enter the path manually:\n" + notFound;
		QMessageBox::critical(this, "Program missing", QString(text.c_str()) ); // POP-UP window : "The following softwares weren't found: please give the path to these."
		return -1;
	}

/* Checking if the given files are executable */
	if(access(ImagemathPath->text().toStdString().c_str(), X_OK)!=0 )
	{
		std::string text = "The file \'" + ImagemathPath->text().toStdString() + "\' is not executable";
		QMessageBox::critical(this, "Non executable File", QString(text.c_str()) );
		return -1;
	}
	if(access(ResampPath->text().toStdString().c_str(), X_OK) != 0 )
	{
		std::string text = "The file \'" + ResampPath->text().toStdString() + "\' is not executable";
		QMessageBox::critical(this, "Non executable File", QString(text.c_str()) );
		return -1;
	}
	if(access(CropDTIPath->text().toStdString().c_str(), X_OK) != 0 )
	{
		std::string text = "The file \'" + CropDTIPath->text().toStdString() + "\' is not executable";
		QMessageBox::critical(this, "Non executable File", QString(text.c_str()) );
		return -1;
	}
	if(access(dtiprocPath->text().toStdString().c_str(), X_OK) != 0 )
	{
		std::string text = "The file \'" + dtiprocPath->text().toStdString() + "\' is not executable";
		QMessageBox::critical(this, "Non executable File", QString(text.c_str()) );
		return -1;
	}
	if(access(BRAINSFitPath->text().toStdString().c_str(), X_OK) != 0 )
	{
		std::string text = "The file \'" + BRAINSFitPath->text().toStdString() + "\' is not executable";
		QMessageBox::critical(this, "Non executable File", QString(text.c_str()) );
		return -1;
	}
	if(access(AWPath->text().toStdString().c_str(), X_OK) != 0 )
	{
		std::string text = "The file \'" + AWPath->text().toStdString() + "\' is not executable";
		QMessageBox::critical(this, "Non executable File", QString(text.c_str()) );
		return -1;
	}
	if(access(dtiavgPath->text().toStdString().c_str(), X_OK) != 0 )
	{
		std::string text = "The file \'" + dtiavgPath->text().toStdString() + "\' is not executable";
		QMessageBox::critical(this, "Non executable File", QString(text.c_str()) );
		return -1;
	}

	SoftPath.push_back(ImagemathPath->text().toStdString());
	SoftPath.push_back(ResampPath->text().toStdString());
	SoftPath.push_back(CropDTIPath->text().toStdString());
	SoftPath.push_back(dtiprocPath->text().toStdString());
	SoftPath.push_back(BRAINSFitPath->text().toStdString());
	SoftPath.push_back(AWPath->text().toStdString());
	SoftPath.push_back(dtiavgPath->text().toStdString());

	m_scriptwriter->setSoftPath(SoftPath);
	SoftPath.clear();

/* Voxel Size and Cropping*/
	if(m_scriptwriter->CheckVoxelSize()==1) 
	{
		QMessageBox::critical(this, "Different Voxel Sizes", "Error: The voxel size of the images\nare not the same,\nplease change dataset"); // returns 0 if voxel size OK , otherwise 1		
		return -1;
	}
	int Crop=m_scriptwriter->setCroppingSize(); // returns 0 if no cropping , 1 if cropping needed
	if(Crop==1) QMessageBox::warning(this, "Cropping", "Warning: The images do not have the same size, \nso some of them will be cropped");

/* Launch writing */
	m_scriptwriter->WriteScript(); // Master Function

/* XML file for AtlasWerks */
	GenerateXMLForAW();

/* Save CSV */
	SaveCSVDataset();
	SaveCSVResults(Crop,NbLoopsSpinBox->value());

/* Generate Preprocess script file */
	std::cout<<"| Generating Pre processing script file..."; // command line display

	QString ScriptPath;
	ScriptPath = m_OutputPath + QString("/DTIAtlas/Script/DTIAtlasBuilder_Preprocess.script");
	QFile filePreP(ScriptPath);

	if ( filePreP.open( IO_WriteOnly | IO_Translate ) )
	{
		//file.setPermissions(QFile::ExeOwner); //make the file executable for the owner
		QTextStream stream( &filePreP );
		stream << QString((m_scriptwriter->getScript_Preprocess()).c_str()) << endl;
		std::cout<<"DONE"<<std::endl; // command line display
	}
	else qDebug( "Could not create file");

/* Generate Atlas Building script file */
	std::cout<<"| Generating Atlas Building script file..."; // command line display

	ScriptPath = m_OutputPath + QString("/DTIAtlas/Script/DTIAtlasBuilder_AtlasBuilding.script");
	QFile fileAtlas(ScriptPath);

	if ( fileAtlas.open( IO_WriteOnly | IO_Translate ) )
	{
		//file.setPermissions(QFile::ExeOwner); //make the file executable for the owner
		QTextStream stream( &fileAtlas );
		stream << QString((m_scriptwriter->getScript_AtlasBuilding()).c_str()) << endl;
		std::cout<<"DONE"<<std::endl; // command line display
	}
	else qDebug( "Could not create file");

/* Generate Main script file */
	std::cout<<"| Generating Main script file..."; // command line display

	ScriptPath = m_OutputPath + QString("/DTIAtlas/Script/DTIAtlasBuilder_Main.script");
	QFile fileMain(ScriptPath);

	if ( fileMain.open( IO_WriteOnly | IO_Translate ) )
	{
		//file.setPermissions(QFile::ExeOwner); // make the file executable for the owner
		QTextStream stream( &fileMain );
		stream << QString((m_scriptwriter->getScript_Main()).c_str()) << endl;
		std::cout<<"DONE"<<std::endl; // command line display
	}
	else qDebug( "Could not create file");

/* Give the right to user to execute the scripts */
	QProcess * chmodProcess = new QProcess;
	program = "chmod u+x " + m_OutputPath.toStdString() + "/DTIAtlas/Script/DTIAtlasBuilder_Preprocess.script " + m_OutputPath.toStdString() + "/DTIAtlas/Script/DTIAtlasBuilder_AtlasBuilding.script " + m_OutputPath.toStdString() + "/DTIAtlas/Script/DTIAtlasBuilder_Main.script"; // 'chmod u+x = user+execute'
	ExitCode = chmodProcess->execute( program.c_str() );
	
	return 0;
}

void GUI::LaunchScriptRunner()
{
	std::cout<<"| Script Running..."<<std::endl; // command line display

/* Running the Script: */
	QProcess * ScriptProcess = new QProcess;

	std::string program;
	program = m_OutputPath.toStdString() + "/DTIAtlas/Script/DTIAtlasBuilder_Main.script";

	int ExitCode=0;
	ExitCode = ScriptProcess->execute( program.c_str() );

/* When we are here the running is finished : emit signal to display the "Running Completed" Window: */
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

