// Qt includes
#include <QTimer>

// DTIAtlasBuildder includes
#include "Code/GUI.h"

int main(int argc, char * argv[] )
{
  QApplication app(argc, argv); // create a QApplication

  GUI TestGUI("","","",false,false,argv[0]); // create a GUI object : GUI(ParamFile,ConfigFile,CSVFile,Overwrite,noGUI,argv[0])

  TestGUI.show(); // open the window

  QTimer::singleShot(2000, qApp, SLOT(quit())); // close the window after 2000ms = 2s

  return app.exec();
}
