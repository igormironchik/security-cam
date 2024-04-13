
/*
	SPDX-FileCopyrightText: 2016-2024 Igor Mironchik <igor.mironchik@gmail.com>
	SPDX-License-Identifier: GPL-3.0-or-later
*/

// Stock include.
#include "frames.hpp"

// Qt include.
#include <QCameraDevice>

// Qt include.
#include <QMutexLocker>
#include <QTimer>
#include <QMediaDevices>
#include <QDateTime>
#include <QDir>
#include <QImageCapture>


namespace SecurityCam {

static const int c_noFramesTimeout = 3000;


//
// Frames
//

Frames::Frames( const Cfg::Cfg & cfg, QObject * parent )
	:	QVideoSink( parent )
	,	m_cam( nullptr )
	,	m_counter( 0 )
	,	m_keyFrameCounter( 0 )
	,	m_motion( false )
	,	m_threshold( cfg.threshold() )
	,	m_rotation( cfg.rotation() )
	,	m_mirrored( cfg.mirrored() )
	,	m_timer( new QTimer( this ) )
	,	m_secTimer( new QTimer( this ) )
	,	m_fps( 0 )
	,	m_imgCapture( nullptr )
{
	if( cfg.applyTransform() )
		applyTransform();

	m_timer->setInterval( c_noFramesTimeout );
	m_secTimer->setInterval( 1000 );

	connect( m_timer, &QTimer::timeout, this, &Frames::noFramesTimeout );
	connect( m_secTimer, &QTimer::timeout, this, &Frames::second );
	connect( this, &QVideoSink::videoFrameChanged, this, &Frames::frame );

	m_secTimer->start();
}

Frames::~Frames()
{
	stopCam();
}

qreal
Frames::rotation() const
{
	return m_rotation;
}

void
Frames::setRotation( qreal a )
{
	m_rotation = a;
}

bool
Frames::mirrored() const
{
	return m_mirrored;
}

void
Frames::setMirrored( bool on )
{
	m_mirrored = on;
}

qreal
Frames::threshold() const
{
	QMutexLocker lock( &m_mutex );

	return m_threshold;
}

void
Frames::setThreshold( qreal v )
{
	QMutexLocker lock( &m_mutex );

	m_threshold = v;
}

void
Frames::applyTransform( bool on )
{
	QMutexLocker lock( &m_mutex );

	if( on )
	{
		m_transform = QTransform();

		m_transform.rotate( m_rotation );

		if( qAbs( m_rotation ) > 0.01 )
			m_transformApplied = true;

		if( m_mirrored )
		{
			m_transform.scale( -1.0, 1.0 );

			m_transformApplied = true;
		}
	}
	else
	{
		m_transformApplied = false;

		m_transform = QTransform();
	}
}

QCameraFormat
Frames::cameraFormat() const
{
	if( m_cam )
		return m_cam->cameraFormat();
	else
		return QCameraFormat();
}

QCameraDevice
Frames::cameraDevice() const
{
	if( m_cam )
		return m_cam->cameraDevice();
	else
		return QCameraDevice();
}

void
Frames::frame( const QVideoFrame & frame )
{
	QVideoFrame f = frame;
	f.map( QVideoFrame::ReadOnly );

	if( f.isValid() )
	{
		QImage image = f.toImage();

		f.unmap();

		if( m_counter == c_keyFrameChangesOn )
			m_counter = 0;

		QImage tmp = ( m_transformApplied ? image.transformed( m_transform )
			:	image.copy() );

		if( m_counter == 0 )
		{
			if( !m_keyFrame.isNull() )
				detectMotion( m_keyFrame, tmp );

			m_keyFrame = tmp;

			emit newFrame( m_keyFrame );
		}
		else if( m_motion )
			emit newFrame( tmp );

		++m_counter;
		++m_fps;

		m_timer->start();
	}
}

void
Frames::detectMotion( const QImage & key, const QImage & image )
{
	bool detected = false;
	
	if( key.size() == image.size() )
	{
		double errorL2 = 0.0;
		
		// Calculate the L2 relative error between images.
		for( int x = 0; x < key.width(); ++x )
		{
			for( int y = 0; y < key.height(); ++y )
			{
				const auto p1 = key.pixelColor( x, y );
				const auto p2 = image.pixelColor( x, y );
				
				const auto r = p1.redF() - p2.redF();
				const auto r2 = r * r;
				
				const auto g = p1.greenF() - p2.greenF();
				const auto g2 = g * g;
				
				const auto b = p1.blueF() - p2.blueF();
				const auto b2 = b * b;
				
				errorL2 += std::sqrt( r2 + g2 + b2 );
			}
		}
		
		// Convert to a reasonable scale, since L2 error is summed across
		// all pixels of the image.
		const double similarity = errorL2 / (double)( key.width() * key.height() );
	
		detected = similarity > m_threshold;
	
		emit imgDiff( similarity );
	}

	if( m_motion && !detected )
	{
		m_motion = false;

		emit noMoreMotions();
	}
	else if( !m_motion && detected )
	{
		m_motion = true;

		emit motionDetected();
	}
}

void
Frames::noFramesTimeout()
{
	QMutexLocker lock( &m_mutex );

	m_timer->stop();

	emit noFrames();
}

void
Frames::second()
{
	QMutexLocker lock( &m_mutex );

	emit fps( m_fps );

	m_fps = 0;
}

void
Frames::initCam( const QString & name )
{
	const auto cameras = QMediaDevices::videoInputs();

	if( !cameras.isEmpty() )
	{
		QCameraDevice dev = cameras.at( 0 );

		for( const auto & cameraInfo : cameras )
		{
			if( cameraInfo.description() == name )
			{
				dev = cameraInfo;
				break;
			}
		}

		if( m_cam )
			m_cam->deleteLater();

		m_cam = new QCamera( dev, this );

		if( !m_imgCapture )
		{
			m_imgCapture = new QImageCapture( this );
			m_imgCapture->setFileFormat( QImageCapture::JPEG );

			connect( m_imgCapture, &QImageCapture::imageCaptured,
				this, &Frames::imageCaptured );
		}

		m_cam->setFocusMode( QCamera::FocusModeAuto );
		m_capture.setCamera( m_cam );
		m_capture.setVideoSink( this );
		m_capture.setImageCapture( m_imgCapture );

		m_cam->start();
	}
}

void
Frames::imageCaptured( int id, const QImage & img )
{
	const auto fileName = m_fileNames[ id ];
	m_fileNames.remove( id );
	const auto toSave = ( m_transformApplied ? img.transformed( m_transform ) : img );
	toSave.save( fileName );
}

void
Frames::stopCam()
{
	if( m_cam )
	{
		m_cam->stop();

		disconnect( m_cam, 0, 0, 0 );
		m_cam->setParent( nullptr );

		delete m_cam;

		m_cam = nullptr;
	}
}

void
Frames::setResolution( const QCameraFormat & fmt )
{
	if( m_cam )
	{
		m_cam->stop();
		m_cam->setCameraFormat( fmt );
		m_cam->start();
	}
}

void
Frames::takeImage( const QString & dirName )
{
	const QDateTime current = QDateTime::currentDateTime();

	QDir dir( dirName );

	const QString path = dir.absolutePath() +
		current.date().toString( QLatin1String( "/yyyy/MM/dd/" ) );

	dir.mkpath( path );

	const auto fileName = path +
		current.toString( QStringLiteral( "hh.mm.ss" ) ) + QStringLiteral( ".jpg" );

	const auto id = m_imgCapture->capture();

	m_fileNames.insert( id, fileName );
}

} /* namespace Stock */
