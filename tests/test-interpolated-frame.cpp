#include <QImage>
#include <QPainter>
#include <QDebug>
#include <iostream>
#include <stdint.h>

using namespace std;

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		cerr << "Usage: " << argv[0] << " IMG0 IMG1" << endl;
		return 1;
	}
	// load two frames
	QImage firstFrame(QImage(argv[1]).convertToFormat(QImage::Format_ARGB32));
	QImage secondFrame(QImage(argv[2]).convertToFormat(QImage::Format_ARGB32));
	unsigned i(0);
	for (float dt=0.1; dt<=0.9; dt += 0.1)
	{
		// interpolation
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
		firstFrame.save(QString("out-%1.png").arg(i++));
	}
	return 0;
}