#include <iostream>
#include <string>

#include <QApplication>

#include "GUI.h"

#include "DTIAtlasBuilderCLP.h" //generated when ccmake

int main(int argc, char* argv[])
{
	PARSE_ARGS; //thanks to this line, we can use the variables entered in command line as variables of the program
	//string ParamFile, string ConfigFile, string CSVFile, bool Overwrite, bool noGUI, bool Quiet

	QApplication app(argc, argv);

	GUI AtlasGUI(ParamFile,ConfigFile,CSVFile,Overwrite,noGUI,Quiet);

	if(noGUI) AtlasGUI.Compute();
	else AtlasGUI.show();

	return app.exec();
}

