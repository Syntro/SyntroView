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

#ifndef DISPLAYSTATS_H
#define DISPLAYSTATS_H

#include <QDialog>
#include "ui_DisplayStats.h"
#include "SyntroLib.h"
#include "DisplayStatsData.h"

#define DEFAULT_ROW_HEIGHT 20

class DisplayStats : public QDialog
{
	Q_OBJECT

public:
	DisplayStats(QWidget *parent, bool receive, bool transmit, QSettings *settings = NULL);
	~DisplayStats();

public slots:
	void receiveData(int slot, int bytes);
	void transmitData(int slot, int bytes);
	void setServiceName(int slot, QString name);
	void deleteAllServices();

protected:
	void showEvent(QShowEvent *event);
	void closeEvent(QCloseEvent *event);
	void timerEvent(QTimerEvent *event);

private:
	void saveDialogState();
	void restoreDialogState();

	QSettings *m_settings;
	int	m_timer;
	bool m_RX;
	bool m_TX;
	QList<DisplayStatsData> m_data;
	QMutex m_dataMutex;

	Ui::CDisplayStats ui;
};

#endif // DISPLAYSTATS_H
