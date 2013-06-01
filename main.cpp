// kate: replace-tabs off; tab-width 4; indent-width 4; tab-indents true; indent-mode normal
// vim: ts=4:sw=4:noexpandtab

#include "FocusEditorWidget.h"
#include <QApplication>

using namespace RackFocusFixer;

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	FocusEditorWidget editor;
	editor.show();
	return app.exec();
}