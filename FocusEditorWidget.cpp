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
        frameIndex(0), bPaused(false)
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
            progress.setValue(counter);
            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
            if (progress.wasCanceled())
                break;
		}
        qDebug() << frames.size() << "frames loaded";
	}
	
	void FocusEditorWidget::paintEvent(QPaintEvent * event)
	{
		if (frames.empty())
			return;
        float percentage = frameIndex / (float) frames.size();
        const int timelineHeight = 50;
        const int w = width();
        const int h = height();

        QPainter painter(this);
        QFont font = painter.font();
        font.setPointSize(8);
        painter.setFont(font);

        // draw the frame pixmap
        painter.drawPixmap(0, 0, frames[frameIndex]);

        // draw the timeline
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(Qt::white);
        painter.drawLine(0, timelineHeight, w, timelineHeight);
        painter.drawLine(0, 0, w, 0);
        painter.drawLine(w*percentage, 0, w*percentage, timelineHeight);
        if (w*(1-percentage) < 110)
            painter.drawText(w*percentage+1, timelineHeight-10, 100, 10, Qt::AlignLeft, QString("%1").arg(frameIndex));
        else
            painter.drawText(w*percentage-101, timelineHeight-10, 100, 10, Qt::AlignRight, QString("%1").arg(frameIndex));
    }

    void FocusEditorWidget::timerEvent(QTimerEvent *)
    {
        if(!frames.size()) return;
        if (!bPaused)
        {
            frameIndex = (frameIndex + 1) % frames.size();
            repaint();
        }
    }

    void FocusEditorWidget::keyPressEvent(QKeyEvent *event)
    {
        if(event->key() == Qt::Key_Space) bPaused = !bPaused;
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
