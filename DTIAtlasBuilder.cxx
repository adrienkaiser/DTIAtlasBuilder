#include <iostream>
#include <string>

#include <QApplication>

#include "GUI.h"

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);

	GUI AtlasGUI; //type QWidget

	AtlasGUI.show();

	return app.exec();
}

