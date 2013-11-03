//
//  Copyright (c) 2012, 2013 Pansenti, LLC.
//	
//  This file is part of Syntro
//
//  Syntro is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  Syntro is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with Syntro.  If not, see <http://www.gnu.org/licenses/>.
//

#include <QtGui>
#include "ImageWindow.h"

#define	SPACESVIEW_CAMERA_DEADTIME		(10 * SYNTRO_CLOCKS_PER_SEC)


ImageWindow::ImageWindow(int id, QString sourceName, bool showName, bool showDate, 
						bool showTime, QColor textColor, QWidget *parent)
	: QLabel(parent)
{
	m_id = id;
	m_sourceName = sourceName;
	m_showName = showName;
	m_showDate = showDate;
	m_showTime = showTime;
	m_textColor = textColor;

	m_selected = false;
	m_idle = true;

	m_lastFrame = SyntroClock();
	m_displayDate = QDate::currentDate();
	m_displayTime = QTime::currentTime();

	setAlignment(Qt::AlignCenter);

	setMinimumWidth(120);
	setMinimumHeight(90);

	setMaximumWidth(640);
	setMaximumHeight(480);

	m_displayTimer = startTimer(20);
	m_timeoutTimer = startTimer(SPACESVIEW_CAMERA_DEADTIME);
}


ImageWindow::~ImageWindow()
{
	killTimer(m_displayTimer);
	killTimer(m_timeoutTimer);
}

QString ImageWindow::sourceName()
{
	return m_sourceName;
}

void ImageWindow::setShowName(bool enable)
{
	m_showName = enable;
	update();
}

void ImageWindow::setShowDate(bool enable)
{
	m_showDate = enable;
	update();
}

void ImageWindow::setShowTime(bool enable)
{
	m_showTime = enable;
	update();
}

void ImageWindow::setTextColor(QColor color)
{
	m_textColor = color;
	update();
}

void ImageWindow::mousePressEvent(QMouseEvent *)
{
	emit imageMousePress(m_id);
}

void ImageWindow::mouseDoubleClickEvent(QMouseEvent *)
{
	emit imageDoubleClick(m_id);
}

bool ImageWindow::selected()
{
	return m_selected;
}

void ImageWindow::setSelected(bool select)
{
	m_selected = select;
	update();
}

void ImageWindow::newImage(SYNTRO_RECORD_VIDEO *videoRecord)
{
	m_lastFrame = SyntroClock();
	m_idle = false;

	if (m_frameQMutex.tryLock()) {

		if (m_frameQ.empty()) {
			VideoFrame frame;

			frame.m_timestamp = SyntroUtils::convertUC8ToInt64(videoRecord->recordHeader.timestamp);
			int size = SyntroUtils::convertUC4ToInt(videoRecord->size);
			frame.m_image = QByteArray(reinterpret_cast<const char *>(videoRecord + 1), size);

			m_frameQ.enqueue(frame);
		}

		m_frameQMutex.unlock();
	}
}

void ImageWindow::timerEvent(QTimerEvent *event)
{
	if (event->timerId() == m_displayTimer) {
		

		m_frameQMutex.lock();

		if (!m_frameQ.empty()) {
			VideoFrame videoFrame = m_frameQ.dequeue();
			m_frameQMutex.unlock();
			displayImage(&videoFrame);
		} else {
			m_frameQMutex.unlock();
		}
	}
	else if (event->timerId() == m_timeoutTimer) {
		if (SyntroUtils::syntroTimerExpired(SyntroClock(), m_lastFrame, SPACESVIEW_CAMERA_DEADTIME)) {
			m_idle = true;
			update();
		}
	}
}

void ImageWindow::displayImage(VideoFrame *vidFrame)
{
	if (vidFrame->m_image.length() != 0) {
		m_currentImage.loadFromData(vidFrame->m_image, "JPEG");
		setPixmap(QPixmap::fromImage(m_currentImage.scaled(size(), Qt::KeepAspectRatio)));
		m_videoFrame = *vidFrame;
	}

	QDateTime dt = QDateTime::fromMSecsSinceEpoch(vidFrame->m_timestamp);
	m_displayDate = dt.date();
	m_displayTime = dt.time();
	repaint();
}

void ImageWindow::paintEvent(QPaintEvent *event)
 {
	QString timestamp;
	QLabel::paintEvent(event);
    QString timeFormat("hh:mm:ss:zzz");
    QString dateFormat("ddd MMMM d yyyy");

	QPainter painter(this);

	QRect dr = drawingRect();

	if (m_idle)
		painter.fillRect(dr, Qt::Dense2Pattern);
	
	if (m_selected) {
		QPen pen(Qt::green, 3);
		painter.setPen(pen);
		painter.drawRect(dr);
	}

	if (!m_showDate && !m_showTime && !m_showName)
		return;

	painter.setPen(m_textColor);

	int fontHeight = dr.height() / 20;

	if (fontHeight < 8)
		fontHeight = 8;
	else if (fontHeight > 12)
		fontHeight = 12;

	painter.setFont(QFont("Arial", fontHeight));

	if (m_showName)
		painter.drawText(dr.left() + 4, dr.top() + fontHeight + 2, m_sourceName);

	if (m_showTime || m_showDate) {
		if (dr.width() < 160) {
			// only room for one, choose time over date
			if (m_showDate && m_showTime)
                timestamp = m_displayTime.toString(timeFormat);
			else if (m_showDate)
                timestamp = m_displayDate.toString(dateFormat);
			else
                timestamp = m_displayTime.toString(timeFormat);
		}
		else if (!m_showDate) {
            timestamp = m_displayTime.toString(timeFormat);
		}
		else if (!m_showTime) {
            timestamp = m_displayDate.toString(dateFormat);
		}
		else {
            timestamp = m_displayDate.toString(dateFormat) + " " + m_displayTime.toString(timeFormat);
		}

		painter.drawText(dr.left() + 4, dr.bottom() - 2, timestamp);
	}
}

// Assumes horizontal and vertical center alignment
QRect ImageWindow::drawingRect()
{
	QRect dr = rect();

	const QPixmap *pm = pixmap();

	if (pm) {
		QRect pmRect = pm->rect();

		int x = (dr.width() - pmRect.width()) / 2;
		int y = (dr.height() - pmRect.height()) / 2;

		dr.adjust(x, y, -x, -y);
	}

	return dr;
}
