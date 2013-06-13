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

#ifndef SYNTROVIEW_H
#define SYNTROVIEW_H

#define	PRODUCT_TYPE "SyntroView"

#include <QtGui>
#include "ui_SyntroView.h"
#include "SyntroLib.h"
#include "DisplayStats.h"
#include "ViewClient.h"
#include "ImageWindow.h"
#include "ViewSingleCamera.h"
#include "SyntroServer.h"

class SyntroView : public QMainWindow
{
	Q_OBJECT

public:
	SyntroView(QSettings *settings, QWidget *parent = 0);

public slots:
	void onStats();
	void onAbout();
	void onBasicSetup();
	void onSelectStreams();
	void newImage(int slot, SYNTRO_RECORD_VIDEO *videoRecord);
	void onShowName();
	void onShowDate();
	void onShowTime();
	void onTextColor();
	void imageMousePress(int id);
	void imageDoubleClick(int id);
	void singleCameraClosed();
	void newStreams();
	void newWindowLayout();

signals:
	void deleteAllServices();
	void deleteStreams();
	void addStreams();

protected:
	void closeEvent(QCloseEvent *event);
	void timerEvent(QTimerEvent *event);

private:
	void layoutGrid(QStringList sourceList);
	void deleteGrid();
	void initStatusBar();
	void initMenus();
	void saveWindowState();
	void restoreWindowState();

	Ui::CSyntroViewClass ui;

	SyntroServer *m_server;
	ViewClient *m_client;
	QSettings *m_settings;
	QGridLayout *m_grid;
	QList<ImageWindow *> m_windowList;
	DisplayStats *m_displayStats;
	QLabel *m_controlStatus;
	int m_statusTimer;
	bool m_showName;
	bool m_showDate;
	bool m_showTime;
	QColor m_textColor;
	int m_singleCameraId;
	ViewSingleCamera *m_singleCamera;

	int m_enableServicesTimer;
};

#endif // SYNTROVIEW_H
