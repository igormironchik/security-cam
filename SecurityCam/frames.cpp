
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
#include "frames.hpp"

// OpenCV include.
#include <opencv2/core/core.hpp>
#include <opencv2/core/core_c.h>

// Qt include.
#include <QMutexLocker>
#include <QTimer>


namespace SecurityCam {

static const int c_noFramesTimeout = 3000;

//
// Frames
//

Frames::Frames( const Cfg::Cfg & cfg, QObject * parent )
	:	QAbstractVideoSurface( parent )
	,	m_counter( 0 )
	,	m_motion( false )
	,	m_threshold( cfg.threshold() )
	,	m_rotation( cfg.rotation() )
	,	m_mirrored( cfg.mirrored() )
	,	m_timer( new QTimer( this ) )
	,	m_secTimer( new QTimer( this ) )
	,	m_fps( 0 )
{
	if( cfg.applyTransform() )
		applyTransform();

	m_timer->setInterval( c_noFramesTimeout );
	m_secTimer->setInterval( 1000 );

	connect( m_timer, &QTimer::timeout, this, &Frames::noFramesTimeout );
	connect( m_secTimer, &QTimer::timeout, this, &Frames::second );

	m_secTimer->start();
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

bool
Frames::present( const QVideoFrame & frame )
{
	if( !isActive() )
		return false;

	QMutexLocker lock( &m_mutex );

	QVideoFrame f = frame;
	f.map( QAbstractVideoBuffer::ReadOnly );

	QImage image( f.bits(), f.width(), f.height(), f.bytesPerLine(),
		QVideoFrame::imageFormatFromPixelFormat( f.pixelFormat() ) );

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

	return true;
}

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

QList< QVideoFrame::PixelFormat >
Frames::supportedPixelFormats( QAbstractVideoBuffer::HandleType type ) const
{
	Q_UNUSED( type )

	return QList< QVideoFrame::PixelFormat > ()
		<< QVideoFrame::Format_ARGB32
		<< QVideoFrame::Format_ARGB32_Premultiplied
		<< QVideoFrame::Format_RGB32
		<< QVideoFrame::Format_RGB24;
}

void
Frames::noFramesTimeout()
{
	m_timer->stop();

	emit noFrames();
}

void
Frames::second()
{
	emit fps( m_fps );

	m_fps = 0;
}

} /* namespace SecurityCam */
