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

#include "SyntroView.h"
#include <QApplication>

#include "SyntroUtils.h"

QSettings *loadSettings(QStringList arglist);


int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

#ifdef Q_OS_WIN
	a.addLibraryPath(QDir::currentPath() + "/plugins");
#endif

	QSettings *settings = loadSettings(a.arguments());

	SyntroView *w = new SyntroView(settings);

	w->show();

	return a.exec();
}


QSettings *loadSettings(QStringList arglist)
{
	QSettings *settings = loadStandardSettings(PRODUCT_TYPE, arglist);

	settings->setValue(SYNTRO_PARAMS_COMPTYPE, PRODUCT_TYPE);

	//	check to see if the array of sources exists

	int	nSize = settings->beginReadArray(SYNTRO_PARAMS_STREAM_SOURCES);
	settings->endArray();

	if (nSize == 0) {
		settings->beginWriteArray(SYNTRO_PARAMS_STREAM_SOURCES);

		settings->setArrayIndex(0);
		settings->setValue(SYNTRO_PARAMS_STREAM_SOURCE, "Video");

		settings->setArrayIndex(1);
		settings->setValue(SYNTRO_PARAMS_STREAM_SOURCE, "Video2");

		settings->endArray();
	}

	return settings;
}
