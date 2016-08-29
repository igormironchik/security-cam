
/*!
	\file

	\author Igor Mironchik (igor.mironchik at gmail dot com).

	Copyright (c) 2016 Igor Mironchik

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// SecurityCam include.
#include "mainwindow.hpp"

// Qt include.
#include <QApplication>
#include <QDebug>
#include <QStandardPaths>

// QtArg include.
#include <QtArg/CmdLine>
#include <QtArg/Arg>
#include <QtArg/Help>
#include <QtArg/Exceptions>


int main( int argc, char ** argv )
{
	QString cfgFileName =
		QStandardPaths::writableLocation( QStandardPaths::AppConfigLocation ) +
		QLatin1String( "/security-cam.cfg" );

	try {
		QtArgCmdLine cmd( argc, argv );

		QtArg cfg( QLatin1Char( 'c' ), QLatin1String( "cfg" ),
			QLatin1String( "Configuration file." ), false, true );
		QtArgHelp help;
		help.printer()->setExecutableName( argv[ 0 ] );
		help.printer()->setProgramDescription(
			QLatin1String( "Security USB camera." ) );

		cmd.addParseable( cfg );
		cmd.addParseable( help );

		cmd.parse();

		if( cfg.isDefined() )
			cfgFileName = cfg.value();
	}
	catch( const QtArgHelpHasPrintedEx & )
	{
		return 0;
	}
	catch( const QtArgBaseException & x )
	{
		qDebug() << x.whatAsQString();

		return 1;
	}

	QApplication app( argc, argv );

	QIcon appIcon( ":/img/icon_256x256.png" );
	appIcon.addFile( ":/img/icon_128x128.png" );
	appIcon.addFile( ":/img/icon_64x64.png" );
	appIcon.addFile( ":/img/icon_48x48.png" );
	appIcon.addFile( ":/img/icon_32x32.png" );
	appIcon.addFile( ":/img/icon_22x22.png" );
	appIcon.addFile( ":/img/icon_16x16.png" );

	app.setWindowIcon( appIcon );
	app.setApplicationDisplayName( QObject::tr( "SecurityCam" ) );
	app.setApplicationName( QObject::tr( "SecurityCam" ) );

	SecurityCam::MainWindow w( cfgFileName );
	w.show();

	return app.exec();
}
