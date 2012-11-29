#include <iostream>
#include <string>

#include <QApplication>

#include "GUI.h"

#include "DTIAtlasBuilderCLP.h" //generated when ccmake

GUI *OtherGUI;

void ProgressBarSigHandler(int sig, siginfo_t *info, void *extra) // signaled by python script each time a step is done
{
	int Progress = info->si_value.sival_int;

	OtherGUI->ProgressBar(Progress);
}

void SignalHandler(int sig, siginfo_t *info, void *extra) // signaled by son when done
{
	int ExitCode = info->si_value.sival_int;

	if(ExitCode==0) OtherGUI->RunningCompleted();
	else OtherGUI->RunningFailed();
}

int main(int argc, char* argv[])
{
	PARSE_ARGS; //thanks to this line, we can use the variables entered in command line as variables of the program
	//string ParamFile, string ConfigFile, string CSVFile, bool Overwrite, bool noGUI

	QApplication app(argc, argv);

	GUI AtlasGUI(ParamFile,ConfigFile,CSVFile,Overwrite,noGUI);

	OtherGUI=&AtlasGUI;

/* Signals */
	struct sigaction action1;
	action1.sa_flags = SA_SIGINFO; 
	action1.sa_sigaction = &SignalHandler;
	sigaction(SIGUSR1, &action1, NULL);

	struct sigaction action2;
	action2.sa_flags = SA_SIGINFO; 
	action2.sa_sigaction = &ProgressBarSigHandler;
	sigaction(SIGUSR2, &action2, NULL);

/* Launch App */
	if(noGUI) AtlasGUI.Compute();
	else
	{
		AtlasGUI.show();
		return app.exec();
	}
}

