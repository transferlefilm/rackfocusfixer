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
#include <QInputDialog>
#include <stdint.h>
#include "ui_exportDialog.h"

namespace RackFocusFixer
{
using namespace std;

FocusEditorWidget::FocusEditorWidget():
    frameIndex(0),
    bFramesHaveAlpha(false),
    refocusKeyCount(10),
    refocusKeySelected(0),
    refocusSetState(RSS_NONE),
    bPaused(false)
{
    setAttribute(Qt::WA_OpaquePaintEvent);
    exporter = new Ui_ExportDialog();
    exporter->setupUi(exportDialog = new QDialog());
    connect(exporter->exportButton, SIGNAL(clicked()), this, SLOT(exportVideo()));
}

void FocusEditorWidget::loadFramesFolder()
{
    QString filename = QFileDialog::getOpenFileName(0, tr("Open first frame file"), "", tr("Images (*.png *.jpg)"));
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
    this->prefix = prefix;

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

    QProgressDialog progress(tr("Loading Frames..."), tr("Stop Loading"), 0, filesCount, this);
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
        frameNames.push_back(fileName);
        bFramesHaveAlpha = bFramesHaveAlpha || pixmap.hasAlpha();
        const unsigned percentageLoad((100*frames.size())/filesCount);

        if (percentageLoad != lastPercentageLoad) // this f*ks up on MAC for some reason (does not accept input anymore)
        {
            progress.setValue(counter);
            lastPercentageLoad = percentageLoad;
            qApp->processEvents(QEventLoop::AllEvents);
            if (progress.wasCanceled())
                break;
        }
    }
    setWindowTitle(QString(tr("Rack Focus Fixer - %1 frames loaded").arg(frames.size())));
}

void FocusEditorWidget::saveRefocusKeys()
{
    QString filename(prefix+".keys");
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly))
    {
        qDebug() << "unable to open keys file for writing";
        return;
    }
    QDataStream out(&file);
    out << (quint32) refocusKeyCount;
    out << refocusLineStart;
    out << refocusLineEnd;
    for (unsigned i=0; i<refocusKeyCount; i++)
    {
        out << refocusKeys[i];
    }
}

void FocusEditorWidget::loadRefocusKeys()
{
    QString filename(prefix+".keys");
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "unable to open keys file for reading";
        return;
    }
    QDataStream in(&file);
    in >> refocusKeyCount;
    in >> refocusLineStart;
    in >> refocusLineEnd;
    refocusKeys.resize(refocusKeyCount);
    for (unsigned i=0; i<refocusKeyCount; i++)
    {
        in >> refocusKeys[i];
    }
    refocusSetState = RSS_COMPLETE;
    update();
}

void FocusEditorWidget::showExportDialog()
{
    exportDialog->show();
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

void FocusEditorWidget::nextRefocusKey()
{
    refocusKeySelected = (refocusKeySelected + 1) % refocusKeyCount;
    update();
}

void FocusEditorWidget::prevRefocusKey()
{
    refocusKeySelected = (refocusKeySelected + refocusKeyCount - 1) % refocusKeyCount;
    update();
}

void FocusEditorWidget::setRefocusKeyFrame()
{
    // we check that we're not trying to add an index prior to the previous ones
    for (int i=refocusKeySelected-1; i>=0; i--)
    {
        if(refocusKeys[i] == -1) continue;
        if (refocusKeys[i] > frameIndex)
        {
            qDebug() << "refusing to add refocus key " << refocusKeySelected << " at frame" << frameIndex;
            return;
        }
    }
    refocusKeys[refocusKeySelected] = frameIndex;
    update();
}

void FocusEditorWidget::resetRefocusKeyFrame()
{
    refocusKeys[refocusKeySelected] = -1;
}

void FocusEditorWidget::paintEvent(QPaintEvent * event)
{
    if (frames.empty())
        return;
    const float percentage = float(frameIndex) / float(frames.size()-1);
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
    painter.fillRect(QRect(percentage >= 1 ? w-1 : w*percentage, 0, 2, timelineHeight), Qt::white);
    const int textHeight(fontMetrics.height());
    // scope for local variables for text
    {
		const QString text(QString("%1").arg(frameIndex));
		const int textWidth(fontMetrics.width(text));
		if (w*percentage < textWidth*2+3)
			painter.drawText(w*percentage+3, timelineHeight-textHeight-2, 100, textHeight, Qt::AlignLeft, text);
		else
			painter.drawText(w*percentage-101, timelineHeight-textHeight-2, 100, textHeight, Qt::AlignRight, text);
		painter.setPen(Qt::gray);
		painter.drawText(1, timelineHeight-textHeight-2, 100, textHeight, Qt::AlignLeft, QString("0"));
		painter.drawText(w-100, timelineHeight-textHeight-2, 100, textHeight, Qt::AlignRight, QString("%1").arg(frames.size()));
		painter.setPen(Qt::white);
	}

    // draw the refocus line and points
    if (refocusSetState == RSS_COMPLETE)
    {
        // we draw it on the image itself
        painter.drawLine(refocusLineStart, refocusLineEnd);
        for (unsigned i = 0; i < refocusKeyCount; ++i)
        {
			const float factor(float(i)/float(refocusKeyCount-1));
            const QPointF keyPos(
                        QPointF(refocusLineStart) +
                        QPointF(refocusLineEnd - refocusLineStart) * factor
                        );
            painter.setBrush(Qt::white);
            if (i == refocusKeySelected)
            {
                painter.drawEllipse(keyPos, 5, 5);
                painter.setBrush(Qt::black);
                painter.drawEllipse(keyPos, 4, 4);
            }
            else
                painter.drawEllipse(keyPos, 4, 4);
        }

        // and on the timeline
        QPointF previousKeyPos;
        RefocusKeys interpolatedKeys = getFullKeypointList();

        for (unsigned i=0; i < refocusKeys.size(); i++)
        {
            const int keyFrame(interpolatedKeys[i]);
            const double percentage(float(keyFrame)/float(frames.size()-1));
            const QPointF keyPos(percentage >= 1 ? w-1 : percentage*w, timelineHeight/2);
            const QString text(QString("%1").arg(i+1));
			const int textWidth(fontMetrics.width(text));
            painter.setBrush(Qt::NoBrush);
            painter.setPen(refocusKeys[i]==-1 ? QPen(Qt::white, 1, Qt::DotLine) : QPen(Qt::white));
            if (i == 0)
				painter.drawText(keyPos.x(), keyPos.y()-6-textHeight, textWidth, textHeight, Qt::AlignLeft, text);
			else if (i == refocusKeys.size()-1)
				painter.drawText(keyPos.x()-textWidth, keyPos.y()-6-textHeight, textWidth, textHeight, Qt::AlignRight, text);
			else
				painter.drawText(keyPos.x()-textWidth/2, keyPos.y()-6-textHeight, textWidth, textHeight, Qt::AlignHCenter, text);
            if (i == refocusKeySelected)
            {
                painter.setBrush(Qt::white);
                painter.drawEllipse(keyPos, 5, 5);
                painter.setPen(QPen(Qt::white, 0.5, Qt::DotLine));
                painter.drawLine(keyPos-QPointF(20,0),keyPos+QPointF(20,0));
                painter.drawLine(keyPos-QPointF(0,20),keyPos+QPointF(0,20));
            }
            else
                painter.drawEllipse(keyPos, 4, 4);
            if (i)
            {
                painter.setPen(QPen(Qt::white, 1, Qt::DotLine));
                painter.drawLine(keyPos, previousKeyPos);
            }
            previousKeyPos = keyPos;
        }
    }
    else if (refocusSetState == RSS_START)
    {
        painter.drawEllipse(refocusLineStart, 4, 4);
    }
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
    case Qt::Key_Escape: qApp->quit(); break;
    case Qt::Key_Space: bPaused = !bPaused; break;
    case Qt::Key_Right: event->modifiers() & Qt::ShiftModifier ? nextFrameBlock() : nextFrame(); break;
    case Qt::Key_Left: event->modifiers() & Qt::ShiftModifier ? prevFrameBlock() : prevFrame(); break;
    case Qt::Key_Up: nextRefocusKey(); break;
    case Qt::Key_Down: prevRefocusKey(); break;
    case Qt::Key_Enter:
    case Qt::Key_Return: setRefocusKeyFrame(); break;
    case Qt::Key_Backspace:
    case Qt::Key_Delete: resetRefocusKeyFrame(); break;
    case Qt::Key_S:
    case Qt::Key_Save: saveRefocusKeys(); break;
    case Qt::Key_L: loadRefocusKeys(); break;
    case Qt::Key_E: showExportDialog(); break;
    default: break;
    }
}

void FocusEditorWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() == Qt::LeftButton && event->y() < timelineHeight)
    {
        const float percentage = float(event->x()) / float(width());
        frameIndex = max(min(int(float(frames.size())*percentage), frames.size()-1), 0);
        update();
    }

}

void FocusEditorWidget::mouseDoubleClickEvent(QMouseEvent *event)
{

}

void FocusEditorWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->y() < timelineHeight)
    {
        const float percentage = float(event->x()) / float(width());
        frameIndex = max(min(int(float(frames.size())*percentage), frames.size()-1), 0);
        update();
    }
    else
    {
        switch (refocusSetState)
        {
        case RSS_NONE:
        case RSS_COMPLETE:
            refocusLineStart = event->pos();
            refocusSetState = RSS_START;
            update();
            break;
        case RSS_START:
            refocusLineEnd = event->pos();
            refocusSetState = RSS_COMPLETE;
            refocusKeyCount = QInputDialog::getInt(
                        this,
                        tr("Input keypoint count"),
                        tr("Choose the number of refocus keypoints"),
                        refocusKeyCount, 2, 1000
                        );
            refocusKeySelected = std::min(refocusKeySelected, refocusKeyCount-1);
            refocusKeys = RefocusKeys(refocusKeyCount,-1);
            update();
            break;
        default: break;
        };
    }
}

void FocusEditorWidget::mouseReleaseEvent(QMouseEvent *event)
{

}

RefocusKeys FocusEditorWidget::getFullKeypointList() const
{
    RefocusKeys interpolatedKeys = refocusKeys;
    for (unsigned i=0; i < refocusKeys.size(); i++)
    {
        if (interpolatedKeys[i] != -1) continue;
        if(!i)
            interpolatedKeys[i] = 0;
        else
        {
            unsigned j=i+1;
            while(j<refocusKeys.size())
            {
                if(interpolatedKeys[j] != -1) break;
                j++;
            }
            if(j<refocusKeys.size())
                interpolatedKeys[i] = interpolatedKeys[i-1] + (interpolatedKeys[j]-interpolatedKeys[i-1])/(j-i+1);
            else
                interpolatedKeys[i] = interpolatedKeys[i-1] + (frames.size()-interpolatedKeys[i-1])/(refocusKeys.size()-i);
        }
    }
    return interpolatedKeys;
}

unsigned FocusEditorWidget::getBestDuration() const
{
    //TODO: compute the frames size that requires the least interpolations
    return frames.size();
}

FrameList FocusEditorWidget::getLinearFrames(const int duration, const RefocusKeys& keys) const
{
    FrameList list(duration==-1 ? getBestDuration() : duration);
    //TODO: compute the interpolation (for now it's completely bogus!)
    for (unsigned i=0; i<list.size(); i++)
    {
        float percentage = float(i) / float(list.size()-1);
        int keypointPre = int(percentage * float(keys.size()-1));
        if (percentage*float(keys.size()-1) - float(keypointPre) < 1e-8) // we are on a keypoint, or sufficiently close
        {
            list[i] = keys[keypointPre];
        }
        else // we interpolate between the two closest keypoints
        {
            int framePre = keys[keypointPre];
            int framePost = keys[keypointPre+1];
            float remainder = percentage*float(keys.size()-1) - float(keypointPre);
            list[i] = framePre + (framePost-framePre)*remainder;
            qDebug() << "i" << i << "keyframe" << framePre << "remainder" << remainder << "frame" << list[i];
        }
    }
    //for (unsigned i=0; i<list.size(); i++) list[i] = i*frames.size()/list.size();
    return list;
}

FrameList FocusEditorWidget::getRampFrames(const int easeMethod, const int duration, const RefocusKeys& keys) const
{
    // easeMethod: 0: ease-in + ease-out, 1: ease-in, 2: ease-out
    //TODO: compute the ease-in ease-out
    return getLinearFrames(duration, keys);
}

QImage FocusEditorWidget::getInterpolatedFrame(const float& frameApproximation) const
{
    const unsigned index = unsigned(frameApproximation);
    // handle corner cases
    if (index >= frameNames.size())
		return QImage(frameNames.last());
	if ((float(index) == frameApproximation) ||
		(index + 1 >= frameNames.size()))
		return QImage(frameNames[index]);
	// load two frames
	QImage firstFrame(QImage(frameNames[index]).convertToFormat(QImage::Format_ARGB32));
	QImage secondFrame(QImage(frameNames[index+1]).convertToFormat(QImage::Format_ARGB32));
	// interpolation
	const float dt(frameApproximation-float(index));
	const int alpha2(255.f*dt);
	// set alpha on image 2
	QPainter p2(&secondFrame);
	for (int y=0; y<secondFrame.height(); ++y)
	{
		uint32_t* ptr(reinterpret_cast<uint32_t*>(secondFrame.scanLine(y)));
		for (int x=0; x<secondFrame.width(); ++x)
		{
			uint32_t v(*ptr);
			*ptr++ = (v & 0xFFFFFF) | (alpha2 << 24);
		}
	}
	p2.end();
	// paint image 2 on image 1
	QPainter p1(&firstFrame);
	p1.drawImage(firstFrame.rect(), secondFrame, secondFrame.rect());
	p1.end();
    return firstFrame;
}

void FocusEditorWidget::exportVideo()
{
    const bool bBestDuration(exporter->durationCheck->isChecked());
    const int duration(bBestDuration ? -1 : exporter->durationSpin->value());
    const int distanceMethod(exporter->distanceCombo->currentIndex());
    const int rampMethod(exporter->rampCombo->currentIndex());
    FrameList frameList;
    switch(rampMethod)
    {
    case 0: frameList = getLinearFrames(duration, getFullKeypointList()); break;
    case 1:
    case 2:
    case 3:
        frameList = getRampFrames(rampMethod-1, duration, getFullKeypointList()); break;
    }

    QProgressDialog progress(tr("Exporting Output Frames..."), tr("Stop Exporting"), 0, frameList.size(), this);
    progress.setWindowModality(Qt::WindowModal);
    progress.show();
    int i(0);
    for (; i<frameList.size(); i++)
    {
        QImage image = getInterpolatedFrame(frameList[i]);
        QString fileName = QString("%1-fixed%2.jpg").arg(prefix).arg(i+1,5, 10, QChar('0'));
        image.save(fileName);

        progress.setValue(i);
        qApp->processEvents(QEventLoop::AllEvents);
        if (progress.wasCanceled())
            break;
    }
    qDebug() << "Exported" << i << "frames";
}

} // RackFocusFixer

