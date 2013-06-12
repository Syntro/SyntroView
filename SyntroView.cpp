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

#include <QColorDialog>
#include "SyntroView.h"
#include "SyntroAboutDlg.h"
#include "BasicSetupDlg.h"
#include "SelectStreamsDlg.h"

#define GRID_SPACING 3

SyntroView::SyntroView(QSettings *settings, QWidget *parent)
	: QMainWindow(parent), m_settings(settings)
{
	ui.setupUi(this);

	m_singleCameraId = -1;
	m_singleCamera = NULL;

	m_displayStats = new DisplayStats(this, true, false, m_settings);

	syntroAppInit(m_settings);

	if (m_settings->value(SYNTRO_PARAMS_LOCALCONTROL).toBool()) {
		m_server = new SyntroServer(m_settings);
		m_server->resumeThread();
	} else {
		m_server = NULL;
	}

	m_client = new ViewClient(this, m_settings);

	connect(ui.actionExit, SIGNAL(triggered()), this, SLOT(close()));

	connect(m_client, SIGNAL(setServiceName(int, QString)), m_displayStats, SLOT(setServiceName(int, QString)), Qt::QueuedConnection);
	connect(m_client, SIGNAL(receiveData(int, int)), m_displayStats, SLOT(receiveData(int, int)), Qt::DirectConnection);
	connect(m_client, SIGNAL(newImage(int, SYNTRO_RECORD_VIDEO *)), this, SLOT(newImage(int, SYNTRO_RECORD_VIDEO *)), Qt::DirectConnection);
	connect(this, SIGNAL(deleteAllServices()), m_displayStats, SLOT(deleteAllServices()), Qt::QueuedConnection);
	connect(this, SIGNAL(deleteStreams()), m_client, SLOT(deleteStreams()), Qt::QueuedConnection);
	connect(this, SIGNAL(addStreams()), m_client, SLOT(addStreams()), Qt::QueuedConnection);
	connect(m_client, SIGNAL(newWindowLayout()), this, SLOT(newWindowLayout()), Qt::QueuedConnection);

	m_client->resumeThread();

	m_statusTimer = startTimer(2000);

	restoreWindowState();
	initStatusBar();
	initMenus();

	setWindowTitle(QString("%1 - %2")
		.arg(m_settings->value(SYNTRO_PARAMS_APPNAME).toString())
		.arg(m_settings->value(SYNTRO_PARAMS_COMPTYPE).toString()));

	m_enableServicesTimer = -1;
}

void SyntroView::onStats()
{
	m_displayStats->activateWindow();
	m_displayStats->show();
}

void SyntroView::closeEvent(QCloseEvent *)
{
	if (m_singleCamera) {
		disconnect(m_singleCamera, SIGNAL(closed()), this, SLOT(singleCameraClosed()));
		m_singleCamera->close();
	}

	killTimer(m_statusTimer);
	disconnect(m_client, SIGNAL(setServiceName(int, QString)), m_displayStats, SLOT(setServiceName(int, QString)));
	disconnect(m_client, SIGNAL(receiveData(int, int)), m_displayStats, SLOT(receiveData(int, int)));
	disconnect(m_client, SIGNAL(newImage(int, SYNTRO_RECORD_VIDEO *)), this, SLOT(newImage(int, SYNTRO_RECORD_VIDEO *)));
	disconnect(this, SIGNAL(deleteAllServices()), m_displayStats, SLOT(deleteAllServices()));
	disconnect(this, SIGNAL(deleteStreams()), m_client, SLOT(deleteStreams()));
	disconnect(this, SIGNAL(addStreams()), m_client, SLOT(addStreams()));
	disconnect(m_client, SIGNAL(newWindowLayout()), this, SLOT(newWindowLayout()));
	m_client->exitThread();
	saveWindowState();
	if (m_server != NULL)
		m_server->exitThread();
	syntroAppExit();
}

void SyntroView::timerEvent(QTimerEvent *event)
{
	if (event->timerId() == m_enableServicesTimer) {
		emit addStreams();
		killTimer(m_enableServicesTimer);
		m_enableServicesTimer = -1;
	} else {
		m_controlStatus->setText(m_client->getLinkState());
	}
}

void SyntroView::newImage(int slot, SYNTRO_RECORD_VIDEO *videoRecord)
{
	if (slot < 0 || slot >= m_windowList.count())
		return;

	m_windowList[slot]->newImage(videoRecord);

	if (m_singleCameraId == slot)
		m_singleCamera->newImage(videoRecord);
}

void SyntroView::singleCameraClosed()
{
	if (m_singleCamera) {
		delete m_singleCamera;

		m_windowList[m_singleCameraId]->setSelected(false);
		m_singleCamera = NULL;
		m_singleCameraId = -1;
	}
}

void SyntroView::imageMousePress(int id)
{
	if (m_singleCameraId < 0)
		return;

	m_singleCameraId = id;

	m_singleCamera->setSourceName(m_windowList[id]->sourceName());

	for (int i = 0; i < m_windowList.count(); i++)
		m_windowList[i]->setSelected(i == id);
}

void SyntroView::imageDoubleClick(int id)
{
	if (m_singleCameraId >= 0)
		return;

	if (!m_singleCamera) {
		m_singleCamera = new ViewSingleCamera(NULL, m_settings, m_windowList[id]->sourceName());

		if (!m_singleCamera)
			return;

		connect(m_singleCamera, SIGNAL(closed()), this, SLOT(singleCameraClosed()));
		m_singleCamera->show();
	}
	else {
		m_singleCamera->setSourceName(m_windowList[id]->sourceName());
	}

	m_singleCameraId = id;
	m_windowList[id]->setSelected(true);
}

void SyntroView::onShowName()
{
	m_showName = ui.actionShow_name->isChecked();

	for (int i = 0; i < m_windowList.count(); i++)
		m_windowList[i]->setShowName(m_showName);
}

void SyntroView::onShowDate()
{
	m_showDate = ui.actionShow_date->isChecked();

	for (int i = 0; i < m_windowList.count(); i++)
		m_windowList[i]->setShowDate(m_showDate);
}

void SyntroView::onShowTime()
{
	m_showTime = ui.actionShow_time->isChecked();

	for (int i = 0; i < m_windowList.count(); i++)
		m_windowList[i]->setShowTime(m_showTime);
}

void SyntroView::onTextColor()
{
	m_textColor = QColorDialog::getColor(m_textColor, this);

	for (int i = 0; i < m_windowList.count(); i++)
		m_windowList[i]->setTextColor(m_textColor);
}

void SyntroView::newStreams()
{
	if (m_enableServicesTimer != -1)
		return;												// already waiting to clear things
	deleteGrid();
	emit deleteStreams();
	emit deleteAllServices();
	m_enableServicesTimer = startTimer(500);				// give Endpoint time to clear up - finish off in timer event

}

void SyntroView::newWindowLayout()
{
	layoutGrid(m_client->streamSources());
}

void SyntroView::layoutGrid(QStringList sourceList)
{
	int rows;

	m_grid = new QGridLayout(ui.centralWidget);
	m_grid->setSpacing(GRID_SPACING);
	m_grid->setContentsMargins(2, 2, 2, 2);

	int numSources = sourceList.count();

	if (numSources > 30)
		numSources = 30;
	
	if (numSources < 4)
		rows = 1;
	else if (numSources < 9)
		rows = 2;
	else if (numSources < 13)
		rows = 3;
	else if (numSources < 23)
		rows = 4;
	else
		rows = 5;

	int cols = (numSources / rows);

	if (numSources % rows)
		cols++;

	for (int i = 0, id = 0; i < rows && id < numSources; i++) {
		for (int j = 0; j < cols && id < numSources; j++) {
			ImageWindow *iw = new ImageWindow(id, sourceList.at(id), m_showName, m_showDate, m_showTime, m_textColor);
			m_windowList.append(iw);
			m_grid->addWidget(iw, i, j);
			connect(iw, SIGNAL(imageMousePress(int)), this, SLOT(imageMousePress(int)));
			connect(iw, SIGNAL(imageDoubleClick(int)), this, SLOT(imageDoubleClick(int)));
			id++;
		}
	}

	for (int i = 0; i < rows; i++)
		m_grid->setRowStretch(i, 1);

	for (int i = 0; i < cols; i++)
		m_grid->setColumnStretch(i, 1);
}

void SyntroView::deleteGrid()
{
	ImageWindow *iw;

	for (int i = 0; i < m_windowList.count(); i++) {
		iw = m_windowList.at(i);
		disconnect(iw, SIGNAL(imageMousePress(int)), this, SLOT(imageMousePress(int)));
		disconnect(iw, SIGNAL(imageDoubleClick(int)), this, SLOT(imageDoubleClick(int)));
		delete m_windowList.at(i);
	}
	delete m_grid;
	m_windowList.clear();
}


void SyntroView::initStatusBar()
{
	m_controlStatus = new QLabel(this);
	m_controlStatus->setAlignment(Qt::AlignLeft);
	ui.statusBar->addWidget(m_controlStatus, 1);
}

void SyntroView::initMenus()
{
	connect(ui.actionAbout, SIGNAL(triggered()), this, SLOT(onAbout()));
	connect(ui.actionBasicSetup, SIGNAL(triggered()), this, SLOT(onBasicSetup()));
	connect(ui.actionSelectStreams, SIGNAL(triggered()), this, SLOT(onSelectStreams()));

	connect(ui.onStats, SIGNAL(triggered()), this, SLOT(onStats()));
	connect(ui.actionShow_name, SIGNAL(triggered()), this, SLOT(onShowName()));
	connect(ui.actionShow_date, SIGNAL(triggered()), this, SLOT(onShowDate()));
	connect(ui.actionShow_time, SIGNAL(triggered()), this, SLOT(onShowTime()));
	connect(ui.actionText_color, SIGNAL(triggered()), this, SLOT(onTextColor()));

	ui.actionShow_name->setChecked(m_showName);
	ui.actionShow_date->setChecked(m_showDate);
	ui.actionShow_time->setChecked(m_showTime);
}

void SyntroView::saveWindowState()
{
	if (m_settings) {
		m_settings->beginGroup("Window");
		m_settings->setValue("Geometry", saveGeometry());
		m_settings->setValue("State", saveState());
		m_settings->setValue("showName", m_showName);
		m_settings->setValue("showDate", m_showDate);
		m_settings->setValue("showTime", m_showTime);
		m_settings->setValue("textColor", m_textColor);
		m_settings->endGroup();
	}
}

void SyntroView::restoreWindowState()
{
	if (m_settings) {
		m_settings->beginGroup("Window");
		restoreGeometry(m_settings->value("Geometry").toByteArray());
		restoreState(m_settings->value("State").toByteArray());

		if (m_settings->contains("showName")) 
			m_showName = m_settings->value("showName").toBool();
		else
			m_showName = true;

		if (m_settings->contains("showDate"))
			m_showDate = m_settings->value("showDate").toBool();
		else
			m_showDate = true;

		if (m_settings->contains("showTime"))
			m_showTime = m_settings->value("showTime").toBool();
		else
			m_showTime = true;

		if (m_settings->contains("textColor"))
			m_textColor = m_settings->value("textColor").value<QColor>();
		else
			m_textColor = Qt::white;

		m_settings->endGroup();
	}
}

void SyntroView::onAbout()
{
	SyntroAbout *dlg = new SyntroAbout(this, m_settings);
	dlg->show();
}

void SyntroView::onBasicSetup()
{
	BasicSetupDlg *dlg = new BasicSetupDlg(this, m_settings);
	dlg->show();
}

void SyntroView::onSelectStreams()
{
	SelectStreamsDlg *dlg = new SelectStreamsDlg(this, m_settings);
	connect(dlg, SIGNAL(newStreams()), this, SLOT(newStreams()), Qt::QueuedConnection);
	dlg->show();
}
