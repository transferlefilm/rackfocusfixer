// kate: replace-tabs off; tab-width 4; indent-width 4; tab-indents true; indent-mode normal
// vim: ts=4:sw=4:noexpandtab

#ifndef __FOCUS_EDITOR_WIDGET_H
#define __FOCUS_EDITOR_WIDGET_H

#include <QWidget>
#include <QList>
#include <QPixmap>
#include <QString>

class QPaintEvent;
class Ui_ExportDialog;

namespace RackFocusFixer
{
typedef QPixmap Frame;
typedef QList<Frame> Frames;
typedef std::vector<int> RefocusKeys;
typedef std::vector<float> FrameList;

	class FocusEditorWidget: public QWidget
	{
		Q_OBJECT
		
	public:
		FocusEditorWidget();
        void loadFrames(const QString& prefix, const int digits=4, const QString &ext="png");
        void loadFramesFolder();
        int frameCount() const;
        void saveRefocusKeys();
        void loadRefocusKeys();
        void showExportDialog();

	protected:
		void nextFrame();
        void prevFrame();
        void nextFrameBlock();
        void prevFrameBlock();
        void nextRefocusKey();
        void prevRefocusKey();
        void setRefocusKeyFrame();
        void resetRefocusKeyFrame();
        QImage getInterpolatedFrame(const float &frameApproximation) const;
        
		void paintEvent(QPaintEvent * event);
        void timerEvent(QTimerEvent *event);
        void keyPressEvent(QKeyEvent *event);
        void mouseMoveEvent(QMouseEvent *event);
        void mouseDoubleClickEvent(QMouseEvent *event);
        void mousePressEvent(QMouseEvent *event);
        void mouseReleaseEvent(QMouseEvent *event);

    private:
        RefocusKeys getFullKeypointList() const;
        unsigned getBestDuration() const;
        FrameList getLinearFrames(const int& duration, const RefocusKeys& keys) const;
        FrameList getRampFrames(const int& easeMethod, const int& duration, const RefocusKeys& keys) const;

    public slots:
        void exportVideo();

    private:
        static const unsigned timelineHeight = 50;

	protected:
		typedef QPixmap Frame;
		typedef QList<Frame> Frames;
		typedef QList<QString> FrameNames;
        typedef std::vector<int> RefocusKeys;
		
        QString prefix;
		Frames frames;
		FrameNames frameNames;
		unsigned frameIndex;
		bool bFramesHaveAlpha;
		
		QPoint refocusLineStart;
		QPoint refocusLineEnd;
		unsigned refocusKeyCount;
		unsigned refocusKeySelected;
		enum RefocusSetState
		{
			RSS_NONE,
			RSS_START,
			RSS_COMPLETE
		} refocusSetState;
        RefocusKeys refocusKeys;
		
        bool bPaused;
        Ui_ExportDialog *exporter;
        QDialog *exportDialog;
	};
} // RackFocusFixer

#endif // __FOCUS_EDITOR_WIDGET_H
