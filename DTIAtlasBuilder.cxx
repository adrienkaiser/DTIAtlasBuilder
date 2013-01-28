#include <iostream>
#include <string>

#include <QApplication>

#include "GUI.h"

#include "DTIAtlasBuilderCLP.h" //generated when ccmake
/*
GUI *OtherGUI; // for the progress bar

void ProgressBarSigHandler(int sig, siginfo_t *info, void *extra) // signaled by python script each time a step is done
{
	int Progress = info->si_value.sival_int;

	OtherGUI->ProgressBar(Progress);
}

void ExecOKSignalHandler(int sig, siginfo_t *info, void *extra) // signaled by son when done
{
	OtherGUI->RunningCompleted();
}

void ExecFailedSignalHandler(int sig, siginfo_t *info, void *extra) // signaled by son when done
{
	OtherGUI->RunningFailed();
}
*/
int main(int argc, char* argv[])
{
	PARSE_ARGS; //thanks to this line, we can use the variables entered in command line as variables of the program
	//string ParamFile, string ConfigFile, string CSVFile, bool Overwrite, bool noGUI

	QApplication app(argc, argv);

	GUI AtlasGUI(ParamFile,ConfigFile,CSVFile,Overwrite,noGUI,argv[0]); // argv[0] is the command that the user has ran -> to search the config file in the same directory

//	OtherGUI=&AtlasGUI;

/* Signals */
/*	struct sigaction action1;
	action1.sa_flags = SA_SIGINFO; 
	action1.sa_sigaction = &ExecOKSignalHandler;
	sigaction(SIGUSR1, &action1, NULL);

	struct sigaction action2;
	action2.sa_flags = SA_SIGINFO; 
	action2.sa_sigaction = &ExecFailedSignalHandler;
	sigaction(SIGUSR2, &action2, NULL);
*/
/* Launch App */
	if(noGUI) return AtlasGUI.Compute();
	else
	{
		AtlasGUI.show();
		return app.exec();
	}

	return -1;
}

