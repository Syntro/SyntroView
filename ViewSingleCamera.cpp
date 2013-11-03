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

#include "ViewSingleCamera.h"

#define	SPACESVIEW_CAMERA_DEADTIME		(10 * SYNTRO_CLOCKS_PER_SEC)

ViewSingleCamera::ViewSingleCamera(QWidget *parent, QString sourceName)
	: QDialog(parent, Qt::WindowCloseButtonHint | Qt::WindowTitleHint)
{
	ui.setupUi(this);

	m_lastFrame = SyntroClock();

	restoreWindowState();
	setWindowTitle(sourceName);

	m_displayTimer = startTimer(20);
	m_timeoutTimer = startTimer(SPACESVIEW_CAMERA_DEADTIME);
}

void ViewSingleCamera::setSourceName(QString sourceName)
{
	setWindowTitle(sourceName);
}

void ViewSingleCamera::closeEvent(QCloseEvent *)
{
	saveWindowState();
	emit closed();
}

void ViewSingleCamera::newImage(SYNTRO_RECORD_VIDEO *videoRecord)
{
	m_lastFrame = SyntroClock();

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

void ViewSingleCamera::newImage(VideoFrame frame)
{
	m_lastFrame = SyntroClock();

	if (m_frameQMutex.tryLock()) {

		m_frameQ.enqueue(frame);
		m_frameQMutex.unlock();
	}
}

void ViewSingleCamera::timerEvent(QTimerEvent *event)
{
	if (event->timerId() == m_displayTimer) {
		VideoFrame vidFrame;

		m_frameQMutex.lock();

		if (!m_frameQ.empty()) {
			vidFrame = m_frameQ.dequeue();
			m_frameQMutex.unlock();
			displayImage(&vidFrame);
		} else {
			m_frameQMutex.unlock();
		}
	}
	else if (event->timerId() == m_timeoutTimer) {
		if (SyntroUtils::syntroTimerExpired(SyntroClock(), m_lastFrame, SPACESVIEW_CAMERA_DEADTIME)) {
			ui.cameraView->setText("No Image");
		}
	}
}

void ViewSingleCamera::displayImage(VideoFrame *vidFrame)
{
	if (vidFrame->m_image.length() != 0) {
		QImage img;
		img.loadFromData(vidFrame->m_image, "JPEG");
		ui.cameraView->setPixmap(QPixmap::fromImage(img.scaled(size(), Qt::KeepAspectRatio)));
		repaint();
	}
}

void ViewSingleCamera::saveWindowState()
{
	QSettings *settings = SyntroUtils::getSettings();
	
	settings->beginGroup("SingleCameraView");
	settings->setValue("Geometry", saveGeometry());
	settings->endGroup();
	
	delete settings;
}

void ViewSingleCamera::restoreWindowState()
{
	QSettings *settings = SyntroUtils::getSettings();
	
	settings->beginGroup("SingleCameraView");
		restoreGeometry(settings->value("Geometry").toByteArray());
	settings->endGroup();
	
	delete settings;
}
