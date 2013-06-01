// kate: replace-tabs off; tab-width 4; indent-width 4; tab-indents true; indent-mode normal
// vim: ts=4:sw=4:noexpandtab

#include "FocusEditorWidget.h"
#include <QApplication>
#include <iostream>

using namespace RackFocusFixer;
using namespace std;

int main(int argc, char *argv[])
{
    /*if (argc < 2)
	{
		cerr << "Not enough arguments, usage: " << argv[0] << " FRAMES_PREFIX" << endl;
		return -1;
    }*/
	QApplication app(argc, argv);
	FocusEditorWidget editor;
    if (argc >= 2) editor.loadFrames(QString(argv[1]));
    else editor.loadFramesFolder();
    if(!editor.frameCount())
    {
        cerr << "No frames selected!" << endl;
        return -1;
    }
	editor.show();
    editor.startTimer(40); // 25 images per second
	return app.exec();
}
