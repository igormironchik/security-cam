
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

// Widgets include.
#include <Widgets/LicenseDialog>


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
	help->addAction( MainWindow::tr( "Licenses" ), q, &MainWindow::licenses );

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
MainWindow::licenses()
{
	LicenseDialog msg( this );
	msg.addLicense( QStringLiteral( "LibYuv" ),
		QStringLiteral( "<p>Copyright 2011 The <b>LibYuv</b> Project Authors. All rights reserved.\n</p>"
		"\n"
		"<p>Redistribution and use in source and binary forms, with or without "
		"modification, are permitted provided that the following conditions are "
		"met:\n</p>"
		"\n"
		"<p>  <ul><li>Redistributions of source code must retain the above copyright "
		"notice, this list of conditions and the following disclaimer.\n</li>"
		"\n"
		"  <li>Redistributions in binary form must reproduce the above copyright "
		"notice, this list of conditions and the following disclaimer in "
		"the documentation and/or other materials provided with the "
		"distribution.\n</li>"
		"\n"
		"  <li>Neither the name of Google nor the names of its contributors may "
		"be used to endorse or promote products derived from this software "
		"without specific prior written permission.\n</li></ul></p>"
		"\n"
		"<p>THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "
		"\"AS IS\" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT "
		"LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR "
		"A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT "
		"HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, "
		"SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT "
		"LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, "
		"DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY "
		"THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT "
		"(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE "
		"OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.</p>" ) );
	msg.addLicense( QStringLiteral( "The Oxygen Icon Theme" ),
		QStringLiteral( "<p><b>The Oxygen Icon Theme</b>\n\n</p>"
		"<p>Copyright (C) 2007 Nuno Pinheiro &lt;nuno@oxygen-icons.org&gt;\n</p>"
		"<p>Copyright (C) 2007 David Vignoni &lt;david@icon-king.com&gt;\n</p>"
		"<p>Copyright (C) 2007 David Miller &lt;miller@oxygen-icons.org&gt;\n</p>"
		"<p>Copyright (C) 2007 Johann Ollivier Lapeyre &lt;johann@oxygen-icons.org&gt;\n</p>"
		"<p>Copyright (C) 2007 Kenneth Wimer &lt;kwwii@bootsplash.org&gt;\n</p>"
		"<p>Copyright (C) 2007 Riccardo Iaconelli &lt;riccardo@oxygen-icons.org&gt;\n</p>"
		"<p>\nand others\n</p>"
		"\n"
		"<p>This library is free software; you can redistribute it and/or "
		"modify it under the terms of the GNU Lesser General Public "
		"License as published by the Free Software Foundation; either "
		"version 3 of the License, or (at your option) any later version.\n</p>"
		"\n"
		"<p>This library is distributed in the hope that it will be useful, "
		"but WITHOUT ANY WARRANTY; without even the implied warranty of "
		"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU "
		"Lesser General Public License for more details.\n</p>"
		"\n"
		"<p>You should have received a copy of the GNU Lesser General Public "
		"License along with this library. If not, see "
		"<a href=\"http://www.gnu.org/licenses/\">&lt;http://www.gnu.org/licenses/&gt;</a>.\n</p>"
		"\n"
		"<p>Clarification:\n</p>"
		"\n"
		"<p>The GNU Lesser General Public License or LGPL is written for "
		"software libraries in the first place. We expressly want the LGPL to "
		"be valid for this artwork library too.\n</p>"
		"\n"
		"<p>KDE Oxygen theme icons is a special kind of software library, it is an "
		"artwork library, it's elements can be used in a Graphical User Interface, or "
		"GUI.\n</p>"
		"\n"
		"<p>Source code, for this library means:\n</p>"
		"<p><ul> <li>where they exist, SVG;\n</li>"
		" <li>otherwise, if applicable, the multi-layered formats xcf or psd, or "
		"otherwise png.\n</li></ul></p>"
		"\n"
		"<p>The LGPL in some sections obliges you to make the files carry "
		"notices. With images this is in some cases impossible or hardly useful.\n</p>"
		"\n"
		"<p>With this library a notice is placed at a prominent place in the directory "
		"containing the elements. You may follow this practice.\n</p>"
		"\n"
		"<p>The exception in section 5 of the GNU Lesser General Public License covers "
		"the use of elements of this art library in a GUI.\n</p>"
		"\n"
		"<p>kde-artists [at] kde.org\n</p>"
		"\n"
		"<p><b>GNU LESSER GENERAL PUBLIC LICENSE</b>\n</p>"
		"<p>Version 3, 29 June 2007\n</p>"
		"\n"
		"<p>Copyright (C) 2007 Free Software Foundation, Inc. <a href=\"http://fsf.org/\">&lt;http://fsf.org/&gt;</a> "
		"Everyone is permitted to copy and distribute verbatim copies "
		"of this license document, but changing it is not allowed.\n</p>"
		"\n"
		"\n"
		"<p>This version of the GNU Lesser General Public License incorporates "
		"the terms and conditions of version 3 of the GNU General Public "
		"License, supplemented by the additional permissions listed below.\n</p>"
		"\n"
		"<p><b>0.</b> Additional Definitions.\n</p>"
		"\n"
		"<p>As used herein, \"this License\" refers to version 3 of the GNU Lesser "
		"General Public License, and the \"GNU GPL\" refers to version 3 of the GNU "
		"General Public License.\n</p>"
		"\n"
		"<p>\"The Library\" refers to a covered work governed by this License, "
		"other than an Application or a Combined Work as defined below.\n</p>"
		"\n"
		"<p>An \"Application\" is any work that makes use of an interface provided "
		"by the Library, but which is not otherwise based on the Library. "
		"Defining a subclass of a class defined by the Library is deemed a mode "
		"of using an interface provided by the Library.\n</p>"
		"\n"
		"<p>A \"Combined Work\" is a work produced by combining or linking an "
		"Application with the Library.  The particular version of the Library "
		"with which the Combined Work was made is also called the \"Linked "
		"Version\".\n</p>"
		"\n"
		"<p>The \"Minimal Corresponding Source\" for a Combined Work means the "
		"Corresponding Source for the Combined Work, excluding any source code "
		"for portions of the Combined Work that, considered in isolation, are "
		"based on the Application, and not on the Linked Version.\n</p>"
		"\n"
		"<p>The \"Corresponding Application Code\" for a Combined Work means the "
		"object code and/or source code for the Application, including any data "
		"and utility programs needed for reproducing the Combined Work from the "
		"Application, but excluding the System Libraries of the Combined Work.\n</p>"
		"\n"
		"<p><b>1.</b> Exception to Section 3 of the GNU GPL.\n</p>"
		"\n"
		"<p>You may convey a covered work under sections 3 and 4 of this License "
		"without being bound by section 3 of the GNU GPL.\n</p>"
		"\n"
		"<p><b>2.</b> Conveying Modified Versions.\n</p>"
		"\n"
		"<p>If you modify a copy of the Library, and, in your modifications, a "
		"facility refers to a function or data to be supplied by an Application "
		"that uses the facility (other than as an argument passed when the "
		"facility is invoked), then you may convey a copy of the modified "
		"version:\n</p>"
		"\n"
		"<p><b>a)</b> under this License, provided that you make a good faith effort to "
		"ensure that, in the event an Application does not supply the "
		"function or data, the facility still operates, and performs "
		"whatever part of its purpose remains meaningful, or\n</p>"
		"\n"
		"<p><b>b)</b> under the GNU GPL, with none of the additional permissions of "
		"this License applicable to that copy.\n</p>"
		"\n"
		"<p><b>3.</b> Object Code Incorporating Material from Library Header Files.\n</p>"
		"\n"
		"<p>The object code form of an Application may incorporate material from "
		"a header file that is part of the Library.  You may convey such object "
		"code under terms of your choice, provided that, if the incorporated "
		"material is not limited to numerical parameters, data structure "
		"layouts and accessors, or small macros, inline functions and templates "
		"(ten or fewer lines in length), you do both of the following:\n</p>"
		"\n"
		"<p><b>a)</b> Give prominent notice with each copy of the object code that the "
		"Library is used in it and that the Library and its use are "
		"covered by this License.\n</p>"
		"\n"
		"<p><b>b)</b> Accompany the object code with a copy of the GNU GPL and this license "
		"document.\n</p>"
		"\n"
		"<p><b>4.</b> Combined Works.\n</p>"
		"\n"
		"<p>You may convey a Combined Work under terms of your choice that, "
		"taken together, effectively do not restrict modification of the "
		"portions of the Library contained in the Combined Work and reverse "
		"engineering for debugging such modifications, if you also do each of "
		"the following:\n</p>"
		"\n"
		"<p><b>a)</b> Give prominent notice with each copy of the Combined Work that "
		"the Library is used in it and that the Library and its use are "
		"covered by this License.\n</p>"
		"\n"
		"<p><b>b)</b> Accompany the Combined Work with a copy of the GNU GPL and this license "
		"document.\n</p>"
		"\n"
		"<p><b>c)</b> For a Combined Work that displays copyright notices during "
		"execution, include the copyright notice for the Library among "
		"these notices, as well as a reference directing the user to the "
		"copies of the GNU GPL and this license document.\n</p>"
		"\n"
		"<p><b>d)</b> Do one of the following:\n</p>"
		"\n"
		"<p>    <b>0)</b> Convey the Minimal Corresponding Source under the terms of this "
		"License, and the Corresponding Application Code in a form "
		"suitable for, and under terms that permit, the user to "
		"recombine or relink the Application with a modified version of "
		"the Linked Version to produce a modified Combined Work, in the "
		"manner specified by section 6 of the GNU GPL for conveying "
		"Corresponding Source.\n</p>"
		"\n"
		"<p>    <b>1)</b> Use a suitable shared library mechanism for linking with the "
		"Library.  A suitable mechanism is one that (a) uses at run time "
		"a copy of the Library already present on the user's computer "
		"system, and (b) will operate properly with a modified version "
		"of the Library that is interface-compatible with the Linked "
		"Version.\n</p>"
		"\n"
		"<p><b>e)</b> Provide Installation Information, but only if you would otherwise "
		"be required to provide such information under section 6 of the "
		"GNU GPL, and only to the extent that such information is "
		"necessary to install and execute a modified version of the "
		"Combined Work produced by recombining or relinking the "
		"Application with a modified version of the Linked Version. (If "
		"you use option 4d0, the Installation Information must accompany "
		"the Minimal Corresponding Source and Corresponding Application "
		"Code. If you use option 4d1, you must provide the Installation "
		"Information in the manner specified by section 6 of the GNU GPL "
		"for conveying Corresponding Source.)\n</p>"
		"\n"
		"<p><b>5.</b> Combined Libraries.\n</p>"
		"\n"
		"<p>You may place library facilities that are a work based on the "
		"Library side by side in a single library together with other library "
		"facilities that are not Applications and are not covered by this "
		"License, and convey such a combined library under terms of your "
		"choice, if you do both of the following:\n</p>"
		"\n"
		"<p><b>a)</b> Accompany the combined library with a copy of the same work based "
		"on the Library, uncombined with any other library facilities, "
		"conveyed under the terms of this License.\n</p>"
		"\n"
		"<p><b>b)</b> Give prominent notice with the combined library that part of it "
		"is a work based on the Library, and explaining where to find the "
		"accompanying uncombined form of the same work.\n</p>"
		"\n"
		"<p><b>6.</b> Revised Versions of the GNU Lesser General Public License.\n</p>"
		"\n"
		"<p>The Free Software Foundation may publish revised and/or new versions "
		"of the GNU Lesser General Public License from time to time. Such new "
		"versions will be similar in spirit to the present version, but may "
		"differ in detail to address new problems or concerns.\n</p>"
		"\n"
		"<p>Each version is given a distinguishing version number. If the "
		"Library as you received it specifies that a certain numbered version "
		"of the GNU Lesser General Public License \"or any later version\" "
		"applies to it, you have the option of following the terms and "
		"conditions either of that published version or of any later version "
		"published by the Free Software Foundation. If the Library as you "
		"received it does not specify a version number of the GNU Lesser "
		"General Public License, you may choose any version of the GNU Lesser "
		"General Public License ever published by the Free Software Foundation.\n</p>"
		"\n"
		"<p>If the Library as you received it specifies that a proxy can decide "
		"whether future versions of the GNU Lesser General Public License shall "
		"apply, that proxy's public statement of acceptance of any version is "
		"permanent authorization for you to choose that version for the "
		"Library.</p>" ) );

	msg.exec();
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
