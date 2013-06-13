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

#ifndef VIEWCLIENT_H
#define VIEWCLIENT_H

#include "SyntroLib.h"

#define	VIEWCLIENT_BACKGROUND_INTERVAL (SYNTRO_CLOCKS_PER_SEC/100)

class ViewClient : public Endpoint
{
	Q_OBJECT

public:
	ViewClient(QObject *parent, QSettings *settings);

	QStringList streamSources();

public slots:
	void deleteStreams();
	void addStreams();

signals:
	void newImage(int slot, SYNTRO_RECORD_VIDEO *videoRecord);
	void newWindowLayout();
	void receiveData(int slot, int bytes);
	void setServiceName(int slot, QString name);

protected:
	void appClientInit();
	void appClientReceiveMulticast(int servicePort, SYNTRO_EHEAD *multiCast, int len);

private:
	void loadStreamSources(QString group, QString src);

	QStringList m_sources;
	QList <int> m_ports;
};

#endif // VIEWCLIENT_H

