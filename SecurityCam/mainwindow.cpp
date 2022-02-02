
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
#include "frames.hpp"
#include "view.hpp"
#include "resolution.hpp"

// cfgfile include.
#include <cfgfile/all.hpp>

// Qt include.
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QTimer>
#include <QTime>
#include <QDate>
#include <QTextStream>
#include <QFile>
#include <QStatusBar>
#include <QLabel>


namespace SecurityCam {

//
// MainWindowPrivate
//

class MainWindowPrivate {
public:
	MainWindowPrivate( MainWindow * parent, const QString & cfgFileName )
		:	m_sysTray( Q_NULLPTR )
		,	m_isRecording( false )
		,	m_takeImageInterval( 1500 )
		,	m_takeImagesYetInterval( 3 * 1000 )
		,	m_fps( 0 )
		,	m_stopTimer( Q_NULLPTR )
		,	m_timer( Q_NULLPTR )
		,	m_cleanTimer( Q_NULLPTR )
		,	m_frames( Q_NULLPTR )
		,	m_view( Q_NULLPTR )
		,	m_status( Q_NULLPTR )
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
	//! Start clean timer.
	void startCleanTimer();
	//! Configure frames.
	void configureFrames();

	//! System tray icon.
	QSystemTrayIcon * m_sysTray;
	//! Camera device.
	QCameraDevice m_cam;
	//! Recording?
	bool m_isRecording;
	//! Interval between images.
	int m_takeImageInterval;
	//! How long should images be taken after no motion.
	int m_takeImagesYetInterval;
	//! Current FPS.
	int m_fps;
	//! Stop timer.
	QTimer * m_stopTimer;
	//! Take image timer.
	QTimer * m_timer;
	//! Clean timer.
	QTimer * m_cleanTimer;
	//! Surface.
	Frames * m_frames;
	//! View.
	View * m_view;
	//! Status label.
	QLabel * m_status;
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
	{
		initCamera();

		startCleanTimer();

		configureFrames();
	}
	else
		q->options();
}

void
MainWindowPrivate::configureFrames()
{
	m_takeImageInterval = m_cfg.snapshotTimeout();

	m_takeImagesYetInterval = m_cfg.stopTimeout();

	if( m_cfg.applyTransform() )
	{
		m_frames->setRotation( m_cfg.rotation() );

		m_frames->setMirrored( m_cfg.mirrored() );

		m_frames->applyTransform( true );
	}
	else
		m_frames->applyTransform( false );

	m_frames->setThreshold( m_cfg.threshold() );

	const auto settings = m_cam.videoFormats();

	for( const auto & s : settings )
	{
		if( s.resolution().width() == m_cfg.resolution().width() &&
			s.resolution().height() == m_cfg.resolution().height() &&
			qAbs( s.maxFrameRate() - m_cfg.resolution().fps() ) < 0.01 &&
			s.pixelFormat() == stringToPixelFormat( m_cfg.resolution().format() ) )
		{
			m_frames->setResolution( s );

			break;
		}
	}
}

void
MainWindowPrivate::startCleanTimer()
{
	m_cleanTimer->stop();

	if( m_cfg.storeDays() > 0 )
	{
		QTime t = QTime::fromString( m_cfg.clearTime(),
			QLatin1String( "hh:mm" ) );

		int s = QTime::currentTime().secsTo( t );

		if( s < 0 )
			s = 24 * 60 * 60 + s;

		m_cleanTimer->start( s * 1000 );
	}
}

bool
MainWindowPrivate::readCfg()
{
	if( QFileInfo::exists( m_cfgFileName ) )
	{
		QFile file( m_cfgFileName );

		if( file.open( QIODevice::ReadOnly ) )
		{
			try {
				Cfg::tag_Cfg< cfgfile::qstring_trait_t > tag;

				QTextStream stream( &file );

				cfgfile::read_cfgfile( tag, stream, m_cfgFileName );

				file.close();

				m_cfg = tag.get_cfg();

				return true;
			}
			catch( const cfgfile::exception_t< cfgfile::qstring_trait_t > & x )
			{
				file.close();

				QMessageBox::critical( q,
					MainWindow::tr( "Unable to load configuration..." ),
					MainWindow::tr( "Unable to load configuration.\n"
						"%1\n"
						"Please configure application." )
					.arg( x.desc() ) );
			}
		}
		else
		{
			QMessageBox::critical( q,
				MainWindow::tr( "Unable to load configuration..." ),
				MainWindow::tr( "Unable to load configuration.\n"
					"Unable to open file \"%1\"." )
				.arg( m_cfgFileName ) );
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

static const int c_cameraReinitTimeout = 1000;

void
MainWindowPrivate::initCamera()
{
	if( !m_cfg.camera().isEmpty() )
	{
		m_frames->initCam( m_cfg.camera() );

		m_cam = m_frames->cameraDevice();

		q->setStatusLabel();
	}
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

	opts->addAction( QIcon( ":/img/transform-scale.png" ),
		MainWindow::tr( "&Resolution" ), q,
		&MainWindow::resolution );

	QMenu * help = q->menuBar()->addMenu( MainWindow::tr( "&Help" ) );
	help->addAction( QIcon( ":/logo/img/icon_22x22.png" ),
		MainWindow::tr( "About" ), q, &MainWindow::about );
	help->addAction( QIcon( ":/img/qt.png" ),
		MainWindow::tr( "About Qt" ), q, &MainWindow::aboutQt );

	if( QSystemTrayIcon::isSystemTrayAvailable() )
	{
		m_sysTray = new QSystemTrayIcon( q );

		QIcon icon( ":/logo/img/icon_256x256.png" );
		icon.addFile( ":/logo/img/icon_128x128.png" );
		icon.addFile( ":/logo/img/icon_64x64.png" );
		icon.addFile( ":/logo/img/icon_48x48.png" );
		icon.addFile( ":/logo/img/icon_32x32.png" );
		icon.addFile( ":/logo/img/icon_22x22.png" );
		icon.addFile( ":/logo/img/icon_16x16.png" );

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

	m_cfg.set_applyTransform( false );
	m_cfg.set_rotation( 0.0 );
	m_cfg.set_mirrored( false );
	m_cfg.set_threshold( 0.02 );
	m_cfg.set_snapshotTimeout( 1500 );
	m_cfg.set_stopTimeout( 3000 );
	m_cfg.set_storeDays( 0 );

	m_frames = new Frames( m_cfg, q );

	m_view = new View( q );

	q->setCentralWidget( m_view );

	m_stopTimer = new QTimer( q );

	m_timer = new QTimer( q );

	m_cleanTimer = new QTimer( q );

	m_status = new QLabel( q );
	m_status->setText( MainWindow::tr( "Camera is not ready." ) );

	q->statusBar()->addPermanentWidget( m_status );

	MainWindow::connect( m_frames, &Frames::newFrame,
		m_view, &View::draw, Qt::QueuedConnection );
	MainWindow::connect( m_frames, &Frames::motionDetected,
		q, &MainWindow::motionDetected, Qt::QueuedConnection );
	MainWindow::connect( m_frames, &Frames::noMoreMotions,
		q, &MainWindow::noMoreMotion, Qt::QueuedConnection );
	MainWindow::connect( m_stopTimer, &QTimer::timeout,
		q, &MainWindow::stopRecording );
	MainWindow::connect( m_timer, &QTimer::timeout,
		q, &MainWindow::takeImage );
	MainWindow::connect( m_cleanTimer, &QTimer::timeout,
		q, &MainWindow::clean );
	MainWindow::connect( m_frames, &Frames::fps,
		q, &MainWindow::fps, Qt::QueuedConnection );
}

void
MainWindowPrivate::saveCfg()
{
	QFileInfo info( m_cfgFileName );
	QDir dir( info.absolutePath() );

	if( !dir.exists() )
		dir.mkpath( info.absolutePath() );

	QFile file( m_cfgFileName );

	if( file.open( QIODevice::WriteOnly ) )
	{
		try {
			Cfg::tag_Cfg< cfgfile::qstring_trait_t > tag( m_cfg );

			QTextStream stream( &file );

			cfgfile::write_cfgfile( tag, stream );

			file.close();
		}
		catch( const cfgfile::exception_t< cfgfile::qstring_trait_t > & x )
		{
			file.close();

			QMessageBox::critical( q,
				MainWindow::tr( "Unable to save configuration..." ),
				MainWindow::tr( "Unable to save configuration.\n"
					"%1" ).arg( x.desc() ) );
		}
	}
	else
		QMessageBox::critical( q,
			MainWindow::tr( "Unable to save configuration..." ),
			MainWindow::tr( "Unable to save configuration.\n"
				"Unable to open file \"%1\"." ).arg( m_cfgFileName ) );
}

void
MainWindowPrivate::stopCamera()
{
	m_stopTimer->stop();

	m_frames->stopCam();
}


//
// MainWindow
//

MainWindow::MainWindow( const QString & cfgFileName )
	:	d( new MainWindowPrivate( this, cfgFileName ) )
{
	d->init();
}

MainWindow::~MainWindow() noexcept
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
	Options opts( d->m_cfg, d->m_frames->cameraDevice(), this );

	connect( d->m_frames, &Frames::imgDiff,
		&opts, &Options::imgDiff, Qt::QueuedConnection );

	if( QDialog::Accepted == opts.exec() )
	{
		const Cfg::Cfg c = opts.cfg();

		bool reinit = false;

		if( d->m_cfg.camera() != c.camera() )
		{
			reinit = true;

			d->m_cfg.resolution().set_width( 0 );
			d->m_cfg.resolution().set_height( 0 );
		}

		d->m_cfg = c;

		d->startCleanTimer();

		d->saveCfg();

		if( reinit )
		{
			d->stopCamera();

			d->initCamera();
		}

		d->configureFrames();
	}
	else if( d->m_cfg.camera().isEmpty() )
	{
		d->m_cfg = opts.cfg();

		d->initCamera();

		d->startCleanTimer();

		d->saveCfg();

		d->configureFrames();
	}
}

void
MainWindow::resolution()
{
	ResolutionDialog dlg( d->m_frames, this );

	if( QDialog::Accepted == dlg.exec() )
	{
		const auto s = dlg.settings();

		d->m_frames->setResolution( s );

		d->m_cfg.resolution().width() = s.resolution().width();
		d->m_cfg.resolution().height() = s.resolution().height();
		d->m_cfg.resolution().fps() = s.maxFrameRate();
		d->m_cfg.resolution().set_format( pixelFormatToString( s.pixelFormat() ) );

		d->saveCfg();
	}
}

void
MainWindow::sysTrayActivated( QSystemTrayIcon::ActivationReason reason )
{
	if( reason == QSystemTrayIcon::Trigger )
	{
		show();
		raise();
		activateWindow();
	}
}

void
MainWindow::motionDetected()
{
	if( !d->m_isRecording )
	{
		d->m_isRecording = true;

		takeImage();

		d->m_timer->start( d->m_takeImageInterval );
	}
	else
		d->m_stopTimer->stop();
}

void
MainWindow::noMoreMotion()
{
	if( d->m_isRecording )
		d->m_stopTimer->start( d->m_takeImagesYetInterval );
}

void
MainWindow::stopRecording()
{
	d->m_stopTimer->stop();

	d->m_timer->stop();

	d->m_isRecording = false;
}

void
MainWindow::takeImage()
{
	d->m_frames->takeImage( d->m_cfg.folder() );
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

void
MainWindow::about()
{
	QMessageBox::about( this, tr( "About SecurityCam" ),
		tr( "SecurityCam.\n\n"
			"Author - Igor Mironchik (igor.mironchik at gmail dot com).\n\n"
			"Copyright (c) 2016 Igor Mironchik.\n\n"
			"Licensed under GNU GPL 3.0." ) );
}

void
MainWindow::aboutQt()
{
	QMessageBox::aboutQt( this );
}

void
MainWindow::clean()
{
	static auto isTimeLess =
		[] ( const QTime & t1, const QTime & t2 ) -> bool
		{
			return ( ( t1.hour() == t2.hour() && t1.minute() == t2.minute() ) ? false :
				( t2.hour() == 0 && t2.minute() == 0 ) ? true :
				( t1.hour() < t2.hour() ) ? true :
				( t1.hour() == t2.hour() ) ? t1.minute() < t2.minute() : false );
		};

	const QTime shouldBe = QTime::fromString( d->m_cfg.clearTime(),
		QLatin1String( "hh:mm" ) );

	while( isTimeLess( QTime::currentTime(), shouldBe ) )
	{
		QApplication::processEvents();
	}

	d->startCleanTimer();

	const QDate dt = QDate::currentDate().addDays( -d->m_cfg.storeDays() );

	QDir folder( d->m_cfg.folder() );

	QStringList years = folder.entryList( QDir::Dirs | QDir::NoDotAndDotDot );

	foreach( const QString & y, years )
	{
		bool ok = false;

		const int yValue = y.toInt( &ok );

		if( ok )
		{
			QDir year( d->m_cfg.folder() + QLatin1String( "/" ) + y );

			QStringList monthes =
				year.entryList( QDir::Dirs | QDir::NoDotAndDotDot );

			foreach( const QString & m, monthes )
			{
				bool ok = false;

				const int mValue = m.toInt( &ok );

				if( ok )
				{
					QDir month( d->m_cfg.folder() + QLatin1String( "/" ) + y +
						QLatin1String( "/" ) + m );

					QStringList days =
						month.entryList( QDir::Dirs | QDir::NoDotAndDotDot );

					foreach( const QString & dd, days )
					{
						bool ok = false;

						const int dValue = dd.toInt( &ok );

						if( ok && QDate( yValue, mValue, dValue ) <= dt )
						{
							QDir r( d->m_cfg.folder() +
								QLatin1String( "/" ) + y +
								QLatin1String( "/" ) + m +
								QLatin1String( "/" ) + dd );

							r.removeRecursively();
						}
					}

					days = month.entryList( QDir::Dirs | QDir::NoDotAndDotDot );

					if( days.isEmpty() )
						month.removeRecursively();
				}
			}

			monthes = year.entryList( QDir::Dirs | QDir::NoDotAndDotDot );

			if( monthes.isEmpty() )
				year.removeRecursively();
		}
	}
}

void
MainWindow::fps( int v )
{
	d->m_fps = v;

	setStatusLabel();
}

void
MainWindow::setStatusLabel()
{
	const auto s = d->m_frames->cameraFormat();

	d->m_status->setText( tr( "%1x%2 | %3 fps" )
		.arg( s.resolution().width() )
		.arg( s.resolution().height() )
		.arg( d->m_fps ) );
}

} /* namespace SecurityCam */
