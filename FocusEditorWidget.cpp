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
#include <cmath>
#include "ui_exportDialog.h"
#include "easeInOut.h"

namespace RackFocusFixer
{
using namespace std;

FocusEditorWidget::FocusEditorWidget():
    frameIndex(0),
    bFramesHaveAlpha(false),
    refocusKeyCount(10),
    refocusKeySelected(0),
    refocusSetState(RSS_NONE),
    bPaused(false),
    bShowLine(true),
    editMode(0)
{
    setAttribute(Qt::WA_OpaquePaintEvent);
    exporter = new Ui_ExportDialog();
    exporter->setupUi(exportDialog = new QDialog());
    connect(exporter->exportButton, SIGNAL(clicked()), this, SLOT(exportVideo()));
    connect(exporter->cancelButton, SIGNAL(clicked()), exportDialog, SLOT(hide()));
    setCursor(Qt::CrossCursor);
    setMouseTracking(true);
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
    QTextStream out(&file);
    out << (quint32) refocusKeyCount << "\n";
    out << refocusLineStart.x() << " " << refocusLineStart.y() << "\n";
    out << refocusLineEnd.x() << " " << refocusLineEnd.y() << "\n";
    for (unsigned i=0; i<refocusKeyCount; i++)
    {
        out << refocusKeys[i] << " ";
    }
    out << "\n";
    for (unsigned i=0; i<refocusKeyCount; i++)
    {
        out << refocusPoints[i].x() << " ";
        out << refocusPoints[i].y() << " ";
    }
    out << "\n";
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
    QTextStream in(&file);
    //QDataStream in(&file);
    QString text;
    in >> text;
    refocusKeyCount = text.toInt();
    in >> text;
    refocusLineStart.setX(text.toFloat());
    in >> text;
    refocusLineStart.setY(text.toFloat());
    in >> text;
    refocusLineEnd.setX(text.toFloat());
    in >> text;
    refocusLineEnd.setY(text.toFloat());
    refocusKeys.resize(refocusKeyCount);
    refocusPoints.resize(refocusKeyCount);
    for (unsigned i=0; i<refocusKeyCount; i++)
    {
        in >> text;
        refocusKeys[i] = text.toInt();
    }
    if (in.atEnd())
        refocusPoints = generateRefocusPoints();
    else
    {
        for (unsigned i=0; i<refocusKeyCount; i++)
        {
            float x=-1,y=-1;
            in >> text;
            x = text.toFloat();
            in >> text;
            y = text.toFloat();
            refocusPoints[i].setX(x);
            refocusPoints[i].setY(y);
        }
    }
    refocusSetState = RSS_COMPLETE;
    sortRefocusKeys();
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

void FocusEditorWidget::deleteRefocusKey()
{
    refocusKeys.erase(refocusKeys.begin() + refocusKeySelected);
    refocusPoints.erase(refocusPoints.begin() + refocusKeySelected);
    if (refocusKeySelected >= refocusKeys.size()) refocusKeySelected = refocusKeys.size()-1;
    refocusKeyCount--;
    update();
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
        if (bShowLine)
        {
            painter.drawLine(refocusLineStart, refocusLineEnd);
            for (unsigned i = 0; i < refocusKeyCount; ++i)
            {
                painter.drawLine(refocusLineStart, refocusLineEnd);
                for (unsigned i = 0; i < refocusPoints.size(); ++i)
                {
                    const QPointF keyPos(refocusPoints[i]);
                    /*
                    const float factor(float(i)/float(refocusKeyCount-1));
                    const QPointF keyPos(
                                QPointF(refocusLineStart) +
                                QPointF(refocusLineEnd - refocusLineStart) * factor
                                );
                                */
                    painter.setBrush(Qt::white);
                    if (i == refocusKeySelected)
                    {
                        painter.setBrush(Qt::NoBrush);
                        painter.drawEllipse(keyPos, 5, 5);
                    }
                    else
                        painter.drawEllipse(keyPos, 2, 2);
                }
            }
        }
        else
        {
            const QPointF keyPos(refocusPoints[refocusKeySelected]);
            /*
            const QPointF keyPos(
                        QPointF(refocusLineStart) +
                        QPointF(refocusLineEnd - refocusLineStart) * (float(refocusKeySelected)/float(refocusKeyCount-1))
                        );
                        */
            painter.setBrush(Qt::NoBrush);
            painter.setPen(QPen(Qt::white,2));
            painter.drawEllipse(keyPos, 20, 20);
            painter.setPen(QPen(Qt::white, 1, Qt::DotLine));
            painter.drawLine(keyPos.x()-20, keyPos.y(), keyPos.x()-5, keyPos.y());
            painter.drawLine(keyPos.x()+5, keyPos.y(), keyPos.x()+20, keyPos.y());
            painter.drawLine(keyPos.x(), keyPos.y()-20, keyPos.x(), keyPos.y()-5);
            painter.drawLine(keyPos.x(), keyPos.y()+5, keyPos.x(), keyPos.y()+20);

            if (editMode!=1)
            {
                // we add the zoom
                int zoomX(keyPos.x()-32);
                int zoomY(keyPos.y()-32);
                const int zoomW(64);
                const int zoomH(64);
                zoomX = max(0, zoomX);
                zoomY = max(0, zoomY);
                if(zoomX + zoomW > frames[frameIndex].width()) zoomX = frames[frameIndex].width() - zoomW - 1;
                if(zoomY + zoomH > frames[frameIndex].height()) zoomY = frames[frameIndex].height() - zoomH - 1;
                const int bigZoomW(zoomW*4);
                const int bigZoomH(zoomH*4);
                int bigX(0);
                int bigY(height()-bigZoomH-1);
                if(keyPos.x() < bigZoomW*1.5) bigX = width() - bigZoomW - 1;

                Frame zoomPix = frames[frameIndex].copy(zoomX, zoomY, zoomW, zoomH);
                painter.drawPixmap(bigX, bigY, bigZoomW, bigZoomH, zoomPix.scaled(bigZoomW, bigZoomH));
                painter.drawLine(bigX + bigZoomW/2+10, bigY + bigZoomH/2,
                                 bigX + bigZoomW/2+40, bigY + bigZoomH/2);
                painter.drawLine(bigX + bigZoomW/2-10, bigY + bigZoomH/2,
                                 bigX + bigZoomW/2-40, bigY + bigZoomH/2);
                painter.drawLine(bigX + bigZoomW/2, bigY + bigZoomH/2+10,
                                 bigX + bigZoomW/2, bigY + bigZoomH/2+40);
                painter.drawLine(bigX + bigZoomW/2, bigY + bigZoomH/2-10,
                                 bigX + bigZoomW/2, bigY + bigZoomH/2-40);
            }
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
                painter.drawEllipse(keyPos, 3, 3);
                painter.setPen(QPen(Qt::white, 0.5, Qt::DotLine));
                painter.drawLine(keyPos-QPointF(20,0),keyPos+QPointF(20,0));
                painter.drawLine(keyPos-QPointF(0,20),keyPos+QPointF(0,20));
            }
            else
            {
                painter.drawEllipse(keyPos, 2, 2);
            }
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

    if (editMode==1)
    {
        QPointF closestPoint = getClosestPointOnLine(mousePos);
        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(Qt::white, 1, Qt::DotLine));
        painter.drawLine(mousePos, closestPoint);
        painter.drawEllipse(mousePos, 5, 5);
        painter.drawEllipse(closestPoint, 5, 5);

        // we add the zoom
        int zoomX(closestPoint.x()-32);
        int zoomY(closestPoint.y()-32);
        const int zoomW(64);
        const int zoomH(64);
        zoomX = max(0, zoomX);
        zoomY = max(0, zoomY);
        if(zoomX + zoomW > frames[frameIndex].width()) zoomX = frames[frameIndex].width() - zoomW - 1;
        if(zoomY + zoomH > frames[frameIndex].height()) zoomY = frames[frameIndex].height() - zoomH - 1;
        const int bigZoomW(zoomW*4);
        const int bigZoomH(zoomH*4);
        int bigX(0);
        int bigY(height()-bigZoomH-1);
        if(closestPoint.x() < bigZoomW*1.5) bigX = width() - bigZoomW - 1;

        Frame zoomPix = frames[frameIndex].copy(zoomX, zoomY, zoomW, zoomH);
        painter.drawPixmap(bigX, bigY, bigZoomW, bigZoomH, zoomPix.scaled(bigZoomW, bigZoomH));
        painter.drawLine(bigX + bigZoomW/2+10, bigY + bigZoomH/2,
                         bigX + bigZoomW/2+40, bigY + bigZoomH/2);
        painter.drawLine(bigX + bigZoomW/2-10, bigY + bigZoomH/2,
                         bigX + bigZoomW/2-40, bigY + bigZoomH/2);
        painter.drawLine(bigX + bigZoomW/2, bigY + bigZoomH/2+10,
                         bigX + bigZoomW/2, bigY + bigZoomH/2+40);
        painter.drawLine(bigX + bigZoomW/2, bigY + bigZoomH/2-10,
                         bigX + bigZoomW/2, bigY + bigZoomH/2-40);
    }

    // just testing the ease-in ease-out functions
    /*
    QStringList methodList;
    methodList << "Linear" << "Squareroot" << "Tanh" << "Sinusoidal";
    methodList << "Quadratic" << "Cubic" << "Quartic" << "Quintic";
    methodList << "Exponential" << "Circular";
    QStringList easeList;
    easeList << "In" << "Out" << "InOut";
    painter.setPen(QPen(Qt::white, 2));
    painter.setBrush(Qt::white);
    //for (int easeType=0; easeType < 3; easeType++)
    {
        int easeType(1);
        for (int method=0; method < 10; method++)
        {
            QPointF start(50,100+30*method);
            //QPointF start(50,100+30*((method-6) + easeType*3));
            QPointF end(start + QPointF(900,0));
            QPointF oldPoint;
            QString methodString(methodList.at(method) + "-" + easeList.at(easeType));
            painter.drawText(start - QPointF(0,7), methodString);
            int steps = 40;
            for (int i=0; i <= steps; i++)
            {
                float percentage(float(i)/float(steps));
                switch (easeType)
                {
                    case 0: percentage = EaseInOut::easeIn(percentage, method); break;
                    case 1: percentage = EaseInOut::easeOut(percentage, method); break;
                    case 2: percentage = EaseInOut::easeInOut(percentage, method); break;
                }
                QPointF point(start + (end-start)*percentage);
                if (i) painter.drawLine(oldPoint, point);
                oldPoint = point;
                painter.drawEllipse(point, 4, 4);
            }
        }
    }
    */
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
    case Qt::Key_Delete:
    case Qt::Key_Backspace: resetRefocusKeyFrame(); break;
    case Qt::Key_X: deleteRefocusKey(); break;
    case Qt::Key_S:
    case Qt::Key_Save: saveRefocusKeys(); break;
    case Qt::Key_L: loadRefocusKeys(); break;
    case Qt::Key_E: showExportDialog(); break;
    case Qt::Key_H: toggleLine(); break;
    case Qt::Key_R: resizeRefocusKeys(); break;
    default: break;
    }
}

void FocusEditorWidget::keyReleaseEvent(QKeyEvent *event)
{
    if(event->modifiers() != Qt::ControlModifier) editMode = 0;
}

void FocusEditorWidget::mouseMoveEvent(QMouseEvent *event)
{    
    mousePos = event->posF();
    editMode = 0;
    if (event->buttons() == Qt::LeftButton && event->y() < timelineHeight)
    {
        const float percentage = float(event->x()) / float(width());
        frameIndex = max(min(int(float(frames.size())*percentage), frames.size()-1), 0);
    }
    else if (event->buttons() == Qt::LeftButton && event->modifiers() == Qt::ShiftModifier && refocusSetState == RSS_COMPLETE)
    {
        // we look for the closest point and move the line
        if ((refocusLineStart-event->pos()).manhattanLength() < (refocusLineEnd-event->pos()).manhattanLength())
           refocusLineStart = event->pos();
        else
            refocusLineEnd = event->pos();
    }
    else if (event->modifiers() == Qt::ControlModifier)
    {
        editMode = 1;
    }
    update();
}

void FocusEditorWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    editMode = 0;
    mousePos = event->posF();
    update();
}

void FocusEditorWidget::mousePressEvent(QMouseEvent *event)
{
    mousePos = event->posF();
    if (event->y() < timelineHeight)
    {
        const float percentage = float(event->x()) / float(width());
        frameIndex = max(min(int(float(frames.size())*percentage), frames.size()-1), 0);
        update();
    }
    else
    {
        if (event->modifiers() == Qt::ShiftModifier)
        {
            editMode = 0;
            switch (refocusSetState)
            {
            case RSS_COMPLETE:
                // we look for the closest point and move the line
                if ((refocusLineStart-event->pos()).manhattanLength() < (refocusLineEnd-event->pos()).manhattanLength())
                   refocusLineStart = event->pos();
                else
                    refocusLineEnd = event->pos();
                update();
                break;
            case RSS_NONE:
                refocusLineStart = event->pos();
                refocusSetState = RSS_START;
                update();
                break;
            case RSS_START:
                refocusLineEnd = event->pos();
                refocusSetState = RSS_COMPLETE;
                resizeRefocusKeys();
                break;
            default: break;
            };
        }
        else if(event->modifiers() == Qt::ControlModifier)
        {
            editMode = 1;
            update();
        }
    }
}

void FocusEditorWidget::mouseReleaseEvent(QMouseEvent *event)
{
    mousePos = event->posF();
    if (editMode==1) // we use the mouse to add a point in the line at the closest position, and set the frame as the current one
    {
        const QPointF closestPoint = getClosestPointOnLine(mousePos);
        const QPointF segment(closestPoint - refocusLineStart);
        const QPointF lineVector(refocusLineStart-refocusLineEnd);
        const QPointF sumVector(lineVector + segment);
        const float sumLength(sqrtf(sumVector.x()*sumVector.x() + sumVector.y()*sumVector.y()));
        const float segmentLength(sqrtf(segment.x()*segment.x() + segment.y()*segment.y()));
        const float lineLength(sqrtf(lineVector.x()*lineVector.x() + lineVector.y()*lineVector.y()));
        const float ratio = segmentLength / lineLength;
        const float indexRatio(ratio*refocusKeyCount);
        const int index = int(indexRatio);
        qDebug() << "index" << index << "indexRatio" << indexRatio << "ratio" << ratio
                 << "distance" << indexRatio - index << "sumlength" << sumLength << lineLength;
        if (indexRatio-index > 1e-3 && sumLength < lineLength && index < refocusKeyCount)
        {
            refocusKeys.insert(refocusKeys.begin() + index, frameIndex);
            refocusPoints.insert(refocusPoints.begin() + index, closestPoint);
            refocusKeyCount++;
            refocusKeySelected = index;
            sortRefocusKeys();
            // we resort the points
            vector< pair<float,int> > distances;
            for (int i=0; i< refocusPoints.size(); i++)
            {
                const QPointF segment(refocusPoints[i]-refocusLineStart);
                const float segmentLength(sqrtf(segment.x()*segment.x() + segment.y()*segment.y()));
                const float percentage = segmentLength / lineLength;
                distances.push_back(make_pair(percentage,i));
            }
            sort(distances.begin(), distances.end());
            RefocusPoints newPoints(refocusPoints.size());
            RefocusKeys newKeys(refocusKeys.size());
            int newSelectedIndex = refocusKeySelected;
            for (int i=0; i< refocusPoints.size(); i++)
            {
                newPoints[i] = refocusPoints[distances[i].second];
                newKeys[i] = refocusKeys[distances[i].second];
                if (distances[i].second == refocusKeySelected) newSelectedIndex = i;
            }
            refocusPoints = newPoints;
            refocusKeys = newKeys;
            refocusKeySelected = newSelectedIndex;
        }
        update();
    }
}

void FocusEditorWidget::wheelEvent(QWheelEvent *event)
{
    if (event->delta()>0) frameIndex = (frameIndex+1)%frames.size();
    else if (event->delta()<0) frameIndex = (frameIndex-1 + frames.size())%frames.size();
    else return;
    update();
}

void FocusEditorWidget::resizeRefocusKeys()
{
	refocusKeyCount = QInputDialog::getInt(
				this,
				tr("Input keypoint count"),
				tr("Choose the number of refocus keypoints"),
				refocusKeyCount, 2, 1000
				);
	refocusKeySelected = std::min(refocusKeySelected, refocusKeyCount-1);
	refocusKeys.resize(refocusKeyCount,-1);
    refocusPoints = generateRefocusPoints();
	update();
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

QPointF FocusEditorWidget::getClosestPointOnLine(const QPointF& point) const
{
    QPointF lineVector(refocusLineEnd-refocusLineStart);
    const float lineLength(sqrtf(lineVector.x()*lineVector.x() + lineVector.y()*lineVector.y()));
    lineVector.setX(lineVector.x()/lineLength);
    lineVector.setY(lineVector.y()/lineLength);
    const QPointF pointVector(point-refocusLineStart);
    const float ratio(lineVector.x()*pointVector.x() + lineVector.y()*pointVector.y());
    return lineVector*ratio + refocusLineStart;
}

RefocusPoints FocusEditorWidget::generateRefocusPoints()
{
    RefocusPoints points(refocusKeyCount);
    for (unsigned i = 0; i < refocusKeyCount; ++i)
    {
        const float factor(float(i)/float(refocusKeyCount-1));
        const QPointF keyPos(
                    QPointF(refocusLineStart) +
                    QPointF(refocusLineEnd - refocusLineStart) * factor
                    );
        points[i] = keyPos;
    }
    return points;
}

void FocusEditorWidget::sortRefocusKeys()
{
    // we resort the points
    const QPointF lineVector(refocusLineStart-refocusLineEnd);
    const float lineLength(sqrtf(lineVector.x()*lineVector.x() + lineVector.y()*lineVector.y()));

    vector< pair<float,int> > distances;
    for (int i=0; i< refocusPoints.size(); i++)
    {
        const QPointF segment(refocusPoints[i]-refocusLineStart);
        const float segmentLength(sqrtf(segment.x()*segment.x() + segment.y()*segment.y()));
        const float percentage = segmentLength / lineLength;
        distances.push_back(make_pair(percentage,i));
    }
    sort(distances.begin(), distances.end());

    RefocusPoints newPoints(refocusPoints.size());
    RefocusKeys newKeys(refocusKeys.size());
    int newSelectedIndex = refocusKeySelected;
    for (int i=0; i< refocusPoints.size(); i++)
    {
        newPoints[i] = refocusPoints[distances[i].second];
        newKeys[i] = refocusKeys[distances[i].second];
        if (distances[i].second == refocusKeySelected) newSelectedIndex = i;
    }
    refocusPoints = newPoints;
    refocusKeys = newKeys;
    refocusKeySelected = newSelectedIndex;

}

unsigned FocusEditorWidget::getBestDuration() const
{
    //TODO: compute the frames size that requires the least interpolations
    return frames.size();
}

FrameList FocusEditorWidget::getLinearFrames(const int duration, const RefocusKeys& keys, const int distanceType) const
{
    FrameList list(duration==-1 ? getBestDuration() : duration);

    vector< float > keyPercentages;
    const QPointF lineVector(refocusLineStart-refocusLineEnd);
    const float lineLength(sqrtf(lineVector.x()*lineVector.x() + lineVector.y()*lineVector.y()));
    for (int j=0; j< refocusPoints.size(); j++)
    {
        const QPointF segment(refocusPoints[j]-refocusLineStart);
        const float segmentLength(sqrtf(segment.x()*segment.x() + segment.y()*segment.y()));
        const float keyPercentage = segmentLength / lineLength;
        keyPercentages.push_back(keyPercentage);
        qDebug() << "percentage" << j << keyPercentage;
    }

    for (unsigned i=0; i<list.size(); i++)
    {
        float percentage = float(i) / float(list.size()-1);
        percentage = EaseInOut::easeIn(percentage, distanceType);
        int keypointPre = 0;
        while(keypointPre < keyPercentages.size() && percentage >= keyPercentages[keypointPre]) keypointPre++;
        keypointPre--;

        if (percentage - float(keyPercentages[keypointPre]) < 1e-5) // we are on a keypoint, or sufficiently close
        {
            list[i] = keys[keypointPre];
            qDebug() << "i" << i << "percentage" << percentage << "keypointPre" << keypointPre << "frame" << list[i];
        }
        else // we interpolate between the two closest keypoints
        {
            int framePre = keys[keypointPre];
            int framePost = keys[keypointPre+1];
            float remainder = (percentage - keyPercentages[keypointPre])/(keyPercentages[keypointPre+1]-keyPercentages[keypointPre]);
            list[i] = framePre + (framePost-framePre)*remainder;
            qDebug() << "i" << i << "percentage" << percentage << "keypointPre" << keypointPre << "keyframe" << framePre << "remainder" << remainder << "frame" << list[i];
        }
    }
    //for (unsigned i=0; i<list.size(); i++) list[i] = i*frames.size()/list.size();
    return list;
}

FrameList FocusEditorWidget::getRampFrames(const int easeMethod, const int duration, const RefocusKeys& keys, const int distanceType) const
{
    // easeMethod: 0: ease-in + ease-out, 1: ease-in, 2: ease-out
    //TODO: compute the ease-in ease-out
    return getLinearFrames(duration, keys, distanceType);
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

void FocusEditorWidget::toggleLine()
{
    bShowLine = !bShowLine;
    update();
}

void FocusEditorWidget::exportVideo()
{
    bPaused = true;
    exportDialog->hide();
    const bool bBestDuration(exporter->durationCheck->isChecked());
    const int duration(bBestDuration ? -1 : exporter->durationSpin->value());
    int distanceType(exporter->distanceCombo->currentIndex());
    const int rampMethod(exporter->rampCombo->currentIndex());

    switch(distanceType)
    {
    case 0: distanceType = EaseInOut::Linear; break;
    case 1: distanceType = EaseInOut::Exponential; break;
    case 2: distanceType = EaseInOut::Squareroot; break;
    case 3: distanceType = EaseInOut::Tanh; break;
    }

    FrameList frameList;
    switch(rampMethod)
    {
    case 0: frameList = getLinearFrames(duration, getFullKeypointList(), distanceType); break;
    case 1:
    case 2:
    case 3:
        frameList = getRampFrames(rampMethod-1, duration, getFullKeypointList(), distanceType); break;
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

