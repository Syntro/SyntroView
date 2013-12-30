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

#ifndef DISPLAYSTATSDATA_H
#define DISPLAYSTATSDATA_H

#include <qstring.h>
#include <qobject.h>

class DisplayStatsData : public QObject
{
	Q_OBJECT

public:
	DisplayStatsData();
	
	int totalRecords() const;
	qint64 totalBytes() const;
	qreal recordRate() const;
	qreal byteRate() const;	

	void clear();

public slots:
	void updateBytes(int bytes);

protected:
	void timerEvent(QTimerEvent *);

private:
	void updateRates(int secs);
	
	int m_timer;
	int m_totalRecords;
	qint64 m_totalBytes;
	qreal m_recordRate;
	qreal m_byteRate;
	int m_records;
	int m_bytes;
};

#endif // DISPLAYSTATSDATA_H
