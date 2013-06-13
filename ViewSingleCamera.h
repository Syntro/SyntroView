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

#ifndef VIEWSINGLECAMERA_H
#define VIEWSINGLECAMERA_H

#include <QDialog>
#include <qlabel.h>

#include "ui_ViewSingleCamera.h"
#include "SyntroLib.h"
#include "ImageWindow.h"

class ViewSingleCamera : public QDialog
{
	Q_OBJECT

public:
	ViewSingleCamera(QWidget *parent, QSettings *settings, QString sourceName);

	void setSourceName(QString sourceName);
	void newImage(SYNTRO_RECORD_VIDEO *videoRecord);
	void newImage(VideoFrame frame);

signals:
	void closed();

protected:
	void timerEvent(QTimerEvent *event);
	void closeEvent(QCloseEvent *event);

private:
	void displayImage(VideoFrame *vidFrame);
	void saveWindowState();
	void restoreWindowState();

	Ui::ViewSingleCamera ui;

	QSettings *m_settings;
	int m_displayTimer;
	int m_timeoutTimer;
	QMutex m_frameQMutex;
	QQueue<VideoFrame> m_frameQ;
	qint64 m_lastFrame;
	QLabel *m_cameraView;
};

#endif // VIEWSINGLECAMERA_H
