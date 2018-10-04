
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

// Args include.
#include <Args/all.hpp>


int main( int argc, char ** argv )
{
	QString cfgFileName;

	try {
		Args::CmdLine cmd;

		cmd.addArgWithFlagAndName( QLatin1Char( 'c' ), QLatin1String( "cfg" ),
			true, false, QLatin1String( "Configuration file." ) )
			.addHelp( true, argv[ 0 ], QLatin1String( "Security USB camera." ) );

		cmd.parse( argc, argv );

		if( cmd.isDefined( QLatin1String( "-c" ) ) )
			cfgFileName = cmd.value( QLatin1String( "-c" ) );
	}
	catch( const Args::HelpHasBeenPrintedException & )
	{
		return 0;
	}
	catch( const Args::BaseException & x )
	{
		qDebug() << x.desc() << "\n";

		return 1;
	}

	QApplication app( argc, argv );

	QIcon appIcon( ":/logo/img/icon_256x256.png" );
	appIcon.addFile( ":/logo/img/icon_128x128.png" );
	appIcon.addFile( ":/logo/img/icon_64x64.png" );
	appIcon.addFile( ":/logo/img/icon_48x48.png" );
	appIcon.addFile( ":/logo/img/icon_32x32.png" );
	appIcon.addFile( ":/logo/img/icon_22x22.png" );
	appIcon.addFile( ":/logo/img/icon_16x16.png" );

	app.setWindowIcon( appIcon );
	app.setApplicationDisplayName( QObject::tr( "SecurityCam" ) );
	app.setApplicationName( QObject::tr( "SecurityCam" ) );

	if( cfgFileName.isEmpty() )
		cfgFileName =
			QStandardPaths::writableLocation( QStandardPaths::AppConfigLocation ) +
			QLatin1String( "/security-cam.cfg" );

	SecurityCam::MainWindow w( cfgFileName );
	w.resize( 640, 480 );
	w.show();

	return app.exec();
}
