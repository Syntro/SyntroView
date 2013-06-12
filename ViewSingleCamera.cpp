//
//  Copyright (c) 2012 Pansenti, LLC.
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

#define	SPACESVIEW_CAMERA_DEADTIME		(7 * SYNTRO_CLOCKS_PER_SEC)

ViewSingleCamera::ViewSingleCamera(QWidget *parent, QSettings *settings, QString sourceName)
	: QDialog(parent, Qt::WindowCloseButtonHint | Qt::WindowTitleHint), m_settings(settings)
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
			QByteArray frame = QByteArray(reinterpret_cast<const char *>(videoRecord + 1), 
				convertUC4ToInt(videoRecord->size));
			
			m_frameQ.enqueue(frame);
		}

		m_frameQMutex.unlock();
	}
}

void ViewSingleCamera::timerEvent(QTimerEvent *event)
{
	if (event->timerId() == m_displayTimer) {
		QByteArray frame;

		m_frameQMutex.lock();

		if (!m_frameQ.empty())
			frame = m_frameQ.dequeue();

		m_frameQMutex.unlock();

		if (!frame.isEmpty())
			displayImage(frame);
	}
	else if (event->timerId() == m_timeoutTimer) {
		if (syntroTimerExpired(SyntroClock(), m_lastFrame, SPACESVIEW_CAMERA_DEADTIME)) {
			ui.cameraView->setText("No Image");
		}
	}
}

void ViewSingleCamera::displayImage(QByteArray frame)
{
	QImage img;
	img.loadFromData(frame, "JPEG");
	ui.cameraView->setPixmap(QPixmap::fromImage(img.scaled(size(), Qt::KeepAspectRatio)));
}

void ViewSingleCamera::saveWindowState()
{
	if (m_settings) {
		m_settings->beginGroup("SingleCameraView");
		m_settings->setValue("Geometry", saveGeometry());
		m_settings->endGroup();
	}
}

void ViewSingleCamera::restoreWindowState()
{
	if (m_settings) {
		m_settings->beginGroup("SingleCameraView");
		restoreGeometry(m_settings->value("Geometry").toByteArray());
		m_settings->endGroup();
	}
}
