
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

// Stock include.
#include "frames.hpp"

// Qt include.
#include <QCameraDevice>

// OpenCV include.
#include <opencv2/core.hpp>
#include <opencv2/core/core_c.h>

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

namespace /* anonymous */ {

inline cv::Mat QImageToCvMat( const QImage & inImage )
{
	switch ( inImage.format() )
	{
		case QImage::Format_ARGB32:
		case QImage::Format_ARGB32_Premultiplied:
		{
			cv::Mat mat( inImage.height(), inImage.width(),
				CV_8UC4,
				const_cast< uchar* >( inImage.bits() ),
				static_cast< size_t >( inImage.bytesPerLine() ) );

			return mat;
		}

		case QImage::Format_RGB32:
		case QImage::Format_RGB888:
		{
			QImage swapped;

			if( inImage.format() == QImage::Format_RGB32 )
				swapped = inImage.convertToFormat( QImage::Format_RGB888 );

			swapped = inImage.rgbSwapped();

			return cv::Mat( swapped.height(), swapped.width(),
				CV_8UC3,
				const_cast< uchar* >( swapped.bits() ),
				static_cast< size_t >( swapped.bytesPerLine() ) ).clone();
		}

		default:
			break;
	}

	return cv::Mat();
}

} /* namespace anonymous */

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

	try {
		const cv::Mat A = QImageToCvMat( key );
		const cv::Mat B = QImageToCvMat( image );

		// Calculate the L2 relative error between images.
		const double errorL2 = cv::norm( A, B, CV_L2 );
		// Convert to a reasonable scale, since L2 error is summed across
		// all pixels of the image.
		const double similarity = errorL2 / (double)( A.rows * A.cols );

		detected = similarity > m_threshold;

		emit imgDiff( similarity );
	}
	catch( const cv::Exception & )
	{
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
