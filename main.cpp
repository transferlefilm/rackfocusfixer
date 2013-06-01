// kate: replace-tabs off; tab-width 4; indent-width 4; tab-indents true; indent-mode normal
// vim: ts=4:sw=4:noexpandtab

#include "FocusEditorWidget.h"
#include <QApplication>
#include <iostream>

using namespace RackFocusFixer;
using namespace std;

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		cerr << "Not enough arguments, usage: " << argv[0] << " FRAMES_PREFIX" << endl;
		return -1;
	}
	QApplication app(argc, argv);
	FocusEditorWidget editor;
	editor.loadFrames(QString(argv[1]));
	editor.show();
	return app.exec();
}