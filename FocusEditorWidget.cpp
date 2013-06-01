// kate: replace-tabs off; tab-width 4; indent-width 4; tab-indents true; indent-mode normal
// vim: ts=4:sw=4:noexpandtab

#include "FocusEditorWidget.h"
#include <QPaintEvent>
#include <QPainter>

namespace RackFocusFixer
{
	FocusEditorWidget::FocusEditorWidget():
		frameIndex(0)
	{
		setAttribute(Qt::WA_OpaquePaintEvent);
	}
	
	void FocusEditorWidget::loadFrames(const QString& prefix)
	{
		bool first(true);
		unsigned counter(0);
		const unsigned digitCount(4);
		const QString ext("png");
		while (true)
		{
			// load file
			const QString fileName(
				QString("%1%2.%3").arg(prefix).arg(counter++, 4, 10, QChar('0')).arg(ext)
			);
			QPixmap pixmap(fileName);
			// stop if load failed
			if (pixmap.isNull())
				break;
			// resize widget according to first frame
			if (first)
			{
				resize(pixmap.size());
				first = false;
			}
			// store in frame list
			frames.push_back(pixmap);
		}
	}
	
	void FocusEditorWidget::paintEvent(QPaintEvent * event)
	{
		if (frames.empty())
			return;
		QPainter painter(this);
		painter.drawPixmap(0, 0, frames[frameIndex]);
	}
}; // RackFocusFixer