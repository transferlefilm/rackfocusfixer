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
		
	protected:
		void nextFrame();
        void prevFrame();
        void nextFrameBlock();
        void prevFrameBlock();
        
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
		
		Frames frames;
		unsigned frameIndex;
		bool bFramesHaveAlpha;
        bool bPaused;
	};
} // RackFocusFixer

#endif // __FOCUS_EDITOR_WIDGET_H
