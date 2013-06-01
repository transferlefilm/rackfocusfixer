// kate: replace-tabs off; tab-width 4; indent-width 4; tab-indents true; indent-mode normal
// vim: ts=4:sw=4:noexpandtab

#include "FocusEditorWidget.h"
#include <QPaintEvent>
#include <QPainter>
#include <QFileDialog>
#include <QDebug>
#include <QProgressDialog>
#include <QEvent>
#include <QCoreApplication>

namespace RackFocusFixer
{
	FocusEditorWidget::FocusEditorWidget():
        frameIndex(0), bFramesHaveAlpha(false), bPaused(false)
	{
		setAttribute(Qt::WA_OpaquePaintEvent);
	}

    void FocusEditorWidget::loadFramesFolder()
    {
        QString filename = QFileDialog::getOpenFileName(0, "Open first frame file", "", "Images (*.png *.jpg)");
        QString ext = filename.right(3);
        QString prefix = filename.left(filename.lastIndexOf("."));
        int digits = 0;
        while (prefix.at(prefix.length()-1).isDigit())
        {
            digits++;
            prefix.chop(1);
        }
        loadFrames(prefix, digits, ext);
    }

    int FocusEditorWidget::frameCount() const
    {
        return frames.size();
    }
	
    void FocusEditorWidget::loadFrames(const QString& prefix, const int digits, const QString& ext)
	{
		unsigned counter(0);
        unsigned filesCount(0);
        const unsigned digitCount(digits);

        // we first check how many images we are going to load
        while (true)
        {
            // load file
            const QString fileName(
                QString("%1%2.%3").arg(prefix).arg(counter++, digitCount, 10, QChar('0')).arg(ext)
            );
            bool bExists = QFile::exists(fileName);

            // stop if load failed
            if(!bExists && counter > 1)
                break;
            else if (!bExists) continue; // in case frames begin with 00001
            ++filesCount;
        }

        QProgressDialog progress("Loading Frames...", "Stop Loading", 0, filesCount, this);
        progress.setWindowModality(Qt::WindowModal);
        progress.show();

        counter = 0;
        bool first(true);
        unsigned lastPercentageLoad(0);
        while (true)
		{
			// load file
			const QString fileName(
                QString("%1%2.%3").arg(prefix).arg(counter++, digitCount, 10, QChar('0')).arg(ext)
			);
			QPixmap pixmap(fileName);
			// stop if load failed
            if (pixmap.isNull() && counter > 1)
				break;
            else if (pixmap.isNull())
                continue;
            pixmap = pixmap.scaledToWidth(1024,Qt::SmoothTransformation);
			// resize widget according to first frame
            if (first)
			{
				resize(pixmap.size());
				first = false;
			}
			// store in frame list
			frames.push_back(pixmap);
			bFramesHaveAlpha = bFramesHaveAlpha || pixmap.hasAlpha();
			const unsigned percentageLoad((100*frames.size())/filesCount);
			if (percentageLoad != lastPercentageLoad);
			{
				progress.setValue(counter);
				QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
				if (progress.wasCanceled())
					break;
				lastPercentageLoad = percentageLoad;
			}
		}
        qDebug() << frames.size() << "frames loaded";
	}
	
	void FocusEditorWidget::nextFrame()
	{
		if (!frames.size())
			return;
		frameIndex = (frameIndex + 1) % frames.size();
		update();
	}
	
	void FocusEditorWidget::prevFrame()
	{
		if (!frames.size())
			return;
		frameIndex = (frameIndex + frames.size() - 1) % frames.size();
		update();
	}
	
	void FocusEditorWidget::nextFrameBlock()
	{
		if (!frames.size())
			return;
		const int frameSkip(frames.size() / 10);
		frameIndex = (frameIndex + frameSkip) % frames.size();
		update();
	}
    
    void FocusEditorWidget::prevFrameBlock()
    {
		if (!frames.size())
			return;
		const int frameSkip(frames.size() / 10);
		frameIndex = (frameIndex + frames.size() - frameSkip) % frames.size();
		update();
    }
	
	void FocusEditorWidget::paintEvent(QPaintEvent * event)
	{
		if (frames.empty())
			return;
        const float percentage = frameIndex / (float) frames.size();
        const int timelineHeight = 50;
        const int w = width();
        const int h = height();

        QPainter painter(this);
        QFont font = painter.font();
        font.setPointSize(12);
        painter.setFont(font);
        const QFontMetrics fontMetrics(font);
        
        // if alpha, draw background
        if (bFramesHaveAlpha)
        {
			int c(0);
			for (int y = 0; y < h+31; ++y)
				for (int x = 0; x < w+31; ++x, ++c)
					painter.fillRect(QRect(x*32, y*32, 32, 32), (c % 2) == 0 ? Qt::black : Qt::white);
        }

        // draw the frame pixmap
        painter.drawPixmap(0, 0, frames[frameIndex]);

        // draw the timeline
        painter.setRenderHint(QPainter::Antialiasing);
        painter.fillRect(QRect(0, 0, w, timelineHeight), QColor(0,0,0,128));
        painter.setPen(Qt::white);
        painter.drawLine(0, timelineHeight, w, timelineHeight);
        painter.drawLine(0, 0, w, 0);
        painter.fillRect(QRect(w*percentage, 0, 2, timelineHeight), Qt::white);
        const int textHeight(fontMetrics.height());
        const QString text(QString("%1").arg(frameIndex));
        const int textWidth(fontMetrics.width(text));
        if (w*percentage < textWidth+3)
            painter.drawText(w*percentage+3, timelineHeight-textHeight-2, 100, textHeight, Qt::AlignLeft, text);
        else
            painter.drawText(w*percentage-101, timelineHeight-textHeight-2, 100, textHeight, Qt::AlignRight, text);
    }

    void FocusEditorWidget::timerEvent(QTimerEvent *)
    {
        if (!bPaused)
        {
            nextFrame();
        }
    }

    void FocusEditorWidget::keyPressEvent(QKeyEvent *event)
    {
        switch (event->key())
        {
			case Qt::Key_Space: bPaused = !bPaused; break;
			case Qt::Key_Right: event->modifiers() & Qt::ShiftModifier ? nextFrameBlock() : nextFrame(); break;
			case Qt::Key_Left: event->modifiers() & Qt::ShiftModifier ? prevFrameBlock() : prevFrame(); break;
			default: break;
		}
    }

    void FocusEditorWidget::mouseMoveEvent(QMouseEvent *event)
    {

    }

    void FocusEditorWidget::mouseDoubleClickEvent(QMouseEvent *event)
    {

    }

    void FocusEditorWidget::mousePressEvent(QMouseEvent *event)
    {

    }

    void FocusEditorWidget::mouseReleaseEvent(QMouseEvent *event)
    {

    }

} // RackFocusFixer
