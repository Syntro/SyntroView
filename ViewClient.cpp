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

#include "ViewClient.h"
#include "SyntroLib.h"

ViewClient::ViewClient(QObject *parent, QSettings *settings)
	: Endpoint(parent, settings, VIEWCLIENT_BACKGROUND_INTERVAL)
{
}

QStringList ViewClient::streamSources()
{
	return m_sources;
}

void ViewClient::appClientInit()
{
	addStreams();
}

void ViewClient::appClientReceiveMulticast(int servicePort, SYNTRO_EHEAD *multiCast, int len)
{
	int slot;

	for (slot = 0; slot < m_sources.count(); slot++) {
		if (servicePort == m_ports.at(slot))
			break;
	}
	if (slot == m_sources.count()) {
		logWarn(QString("Multicast received to invalid port %1").arg(servicePort));
	}
	else {
		emit newImage(slot, reinterpret_cast<SYNTRO_RECORD_VIDEO *>(multiCast + 1));
		emit receiveData(slot, len);
		clientSendMulticastAck(servicePort);
	}

	free(multiCast);
}

void ViewClient::addStreams()
{
	int port;
	
	loadStreamSources(SYNTRO_PARAMS_STREAM_SOURCES, SYNTRO_PARAMS_STREAM_SOURCE);
	for (int i = 0; i < m_sources.count(); i++) {
		port = clientAddService(m_sources.at(i), SERVICETYPE_MULTICAST, false);
		m_ports.append(port);
		logDebug(QString("Port = %1 count = %2").arg(port).arg(m_ports.size()));
		emit setServiceName(i, m_sources.at(i));
	}
	emit newWindowLayout();
}

void ViewClient::deleteStreams()
{

	for (int i = 0; i < m_sources.count(); i++) {
		clientRemoveService(m_ports.at(i));
	}
	m_ports.clear();
	m_sources.clear();
}


void ViewClient::loadStreamSources(QString group, QString src)
{
	int count = m_settings->beginReadArray(group);

	for (int i = 0; i < count; i++) {
		m_settings->setArrayIndex(i);

		QString s = m_settings->value(src).toString();
		m_sources.append(s);
	}

	m_settings->endArray();
}
