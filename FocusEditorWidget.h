// kate: replace-tabs off; tab-width 4; indent-width 4; tab-indents true; indent-mode normal
// vim: ts=4:sw=4:noexpandtab

#ifndef __FOCUS_EDITOR_WIDGET_H
#define __FOCUS_EDITOR_WIDGET_H

#include <QWidget>
#include <QList>
#include <QPixmap>
#include <QString>

class QPaintEvent;

namespace RackFocusFixer
{
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

	protected:
		void nextFrame();
        void prevFrame();
        void nextFrameBlock();
        void prevFrameBlock();
        void nextRefocusKey();
        void prevRefocusKey();
        void setRefocusKeyFrame();
        void resetRefocusKeyFrame();
        
		void paintEvent(QPaintEvent * event);
        void timerEvent(QTimerEvent *event);
        void keyPressEvent(QKeyEvent *event);
        void mouseMoveEvent(QMouseEvent *event);
        void mouseDoubleClickEvent(QMouseEvent *event);
        void mousePressEvent(QMouseEvent *event);
        void mouseReleaseEvent(QMouseEvent *event);

	protected:
		typedef QPixmap Frame;
		typedef QList<Frame> Frames;
        typedef std::vector<int> RefocusKeys;
		
        QString prefix;
		Frames frames;
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
	};
} // RackFocusFixer

#endif // __FOCUS_EDITOR_WIDGET_H
