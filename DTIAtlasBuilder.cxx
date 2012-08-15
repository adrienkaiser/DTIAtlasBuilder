#include <iostream>
#include <string>

#include <QApplication>

#include "GUI.h"

#include "DTIAtlasBuilderCLP.h" //generated when ccmake

int main(int argc, char* argv[])
{
	PARSE_ARGS; //thanks to this line, we can use the variables entered in command line as variables of the program
	//ParamFile, ConfigFile, CSVFile

	QApplication app(argc, argv);

	GUI AtlasGUI(ParamFile,ConfigFile,CSVFile); //type QWidget

	AtlasGUI.show();

	return app.exec();
}

