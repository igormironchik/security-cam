
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
#include "cfg.hpp"
#include "options.hpp"

// QtConfFile include.
#include <QtConfFile/Utils>
#include <QtConfFile/Exceptions>

// Qt include.
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QCamera>
#include <QApplication>
#include <QCloseEvent>

#include <QDebug>


namespace SecurityCam {

//
// MainWindowPrivate
//

class MainWindowPrivate {
public:
	MainWindowPrivate( MainWindow * parent, const QString & cfgFileName )
		:	m_sysTray( Q_NULLPTR )
		,	m_cfgFileName( cfgFileName )
		,	q( parent )
	{
	}

	//! Init.
	void init();
	//! Read cfg.
	bool readCfg();
	//! Init camera.
	void initCamera();
	//! Init UI.
	void initUi();
	//! Save cfg.
	void saveCfg();
	//! Stop camera.
	void stopCamera();

	//! System tray icon.
	QSystemTrayIcon * m_sysTray;
	//! Configuration.
	Cfg::Cfg m_cfg;
	//! Cfg file.
	QString m_cfgFileName;
	//! Parent.
	MainWindow * q;
}; // class MainWindowPrivate

void
MainWindowPrivate::init()
{
	initUi();

	if( readCfg() )
		initCamera();
	else
		q->options();
}

bool
MainWindowPrivate::readCfg()
{
	if( QFileInfo::exists( m_cfgFileName ) )
	{
		try {
			Cfg::TagCfg tag;

			QtConfFile::readQtConfFile( tag, m_cfgFileName,
				QTextCodec::codecForName( "UTF-8" ) );

			m_cfg = tag.getCfg();

			return true;
		}
		catch( const QtConfFile::Exception & x )
		{
			QMessageBox::critical( q,
				MainWindow::tr( "Unable to load configuration..." ),
				MainWindow::tr( "Unable to load configuration.\n"
					"%1\n"
					"Please configure application." )
				.arg( x.whatAsQString() ) );
		}
	}
	else
		QMessageBox::information( q,
			MainWindow::tr( "Unable to load configuration..." ),
			MainWindow::tr( "Unable to load configuration.\n"
				"No such file: \"%1\"\n"
				"Please configure application." ).arg( m_cfgFileName ) );

	return false;
}

void
MainWindowPrivate::initCamera()
{

}

void
MainWindowPrivate::initUi()
{
	QMenu * file = q->menuBar()->addMenu( MainWindow::tr( "&File" ) );

	file->addAction( QIcon( ":/img/application-exit.png" ),
		MainWindow::tr( "&Quit" ), q, &MainWindow::quit );

	QMenu * opts = q->menuBar()->addMenu( MainWindow::tr( "&Options" ) );

	opts->addAction( QIcon( ":/img/configure.png" ),
		MainWindow::tr( "&Settings"), q, &MainWindow::options );

	if( QSystemTrayIcon::isSystemTrayAvailable() )
	{
		m_sysTray = new QSystemTrayIcon( q );

		QIcon icon( ":/img/icon_256x256.png" );
		icon.addFile( ":/img/icon_128x128.png" );
		icon.addFile( ":/img/icon_64x64.png" );
		icon.addFile( ":/img/icon_48x48.png" );
		icon.addFile( ":/img/icon_32x32.png" );
		icon.addFile( ":/img/icon_22x22.png" );
		icon.addFile( ":/img/icon_16x16.png" );

		m_sysTray->setIcon( icon );

		QMenu * ctx = new QMenu( q );

		ctx->addAction(
			QIcon( ":/img/application-exit.png" ),
			MainWindow::tr( "Quit" ), q, &MainWindow::quit );

		m_sysTray->setContextMenu( ctx );

		MainWindow::connect( m_sysTray, &QSystemTrayIcon::activated,
			q, &MainWindow::sysTrayActivated );

		m_sysTray->show();
	}
}

void
MainWindowPrivate::saveCfg()
{
	QFileInfo info( m_cfgFileName );
	QDir dir( info.absolutePath() );

	if( !dir.exists() )
		dir.mkpath( info.absolutePath() );

	try {
		Cfg::TagCfg tag( m_cfg );

		QtConfFile::writeQtConfFile( tag, m_cfgFileName,
			QTextCodec::codecForName( "UTF-8" ) );
	}
	catch( const QtConfFile::Exception & x )
	{
		QMessageBox::critical( q,
			MainWindow::tr( "Unable to save configuration..." ),
			MainWindow::tr( "Unable to save configuration.\n"
				"%1" ).arg( x.whatAsQString() ) );
	}
}

void
MainWindowPrivate::stopCamera()
{

}


//
// MainWindow
//

MainWindow::MainWindow( const QString & cfgFileName )
	:	d( new MainWindowPrivate( this, cfgFileName ) )
{
	d->init();
}

MainWindow::~MainWindow()
{
}

void
MainWindow::quit()
{
	d->saveCfg();

	d->stopCamera();

	QApplication::quit();
}

void
MainWindow::options()
{
	Options opts( d->m_cfg, this );

	if( QDialog::Accepted == opts.exec() )
	{
		d->m_cfg = opts.cfg();
	}
	else if( d->m_cfg.camera().isEmpty() )
		d->m_cfg = opts.cfg();
}

void
MainWindow::sysTrayActivated( QSystemTrayIcon::ActivationReason reason )
{
	if( reason == QSystemTrayIcon::Trigger )
		show();
}

void
MainWindow::closeEvent( QCloseEvent * e )
{
	e->ignore();

	if( d->m_sysTray )
		hide();
	else
		showMinimized();
}

} /* namespace SecurityCam */
