
/*
	SPDX-FileCopyrightText: 2016-2024 Igor Mironchik <igor.mironchik@gmail.com>
	SPDX-License-Identifier: GPL-3.0-or-later
*/

// Qt include.
#include <QCamera>
#include <QRegularExpression>

// SecurityCam include.
#include "resolution.hpp"
#include "ui_resolution.h"
#include "frames.hpp"


namespace SecurityCam {

//
// ResolutionDialogPrivate
//

class ResolutionDialogPrivate {
public:
	ResolutionDialogPrivate( const QCameraDevice & cam, Frames * frames,
		const QCameraFormat & s,
		ResolutionDialog * parent )
		:	m_cam( cam )
		,	m_frames( frames )
		,	m_settings( s )
		,	q( parent )
	{
	}

	//! Init.
	void init();

	//! Camera.
	QCameraDevice m_cam;
	//! Frames.
	Frames * m_frames;
	//! Settings.
	QCameraFormat m_settings;
	//! Ui.
	Ui::ResolutionDialog m_ui;
	//! Parent.
	ResolutionDialog * q;
}; // class ResolutionDialogPrivate

void
ResolutionDialogPrivate::init()
{
	m_ui.setupUi( q );

	const auto settings = m_cam.videoFormats();

	for( const auto & s : settings )
	{
		const QString data = QString::number( s.resolution().width() ) +
			QLatin1Char( 'x' ) + QString::number( s.resolution().height() ) +
			QLatin1Char( ' ' ) + QString::number( s.maxFrameRate(), 'f', 0 ) +
			QLatin1String( " fps, " ) + pixelFormatToString( s.pixelFormat() );

		m_ui.m_res->addItem( data );

		if( s.resolution() == m_settings.resolution() &&
			qAbs( s.maxFrameRate() - m_settings.maxFrameRate() ) < 0.01 &&
			s.pixelFormat() == m_settings.pixelFormat() )
				m_ui.m_res->setCurrentIndex( m_ui.m_res->count() - 1 );
	}
}

//
// pixelFormatToString
//

QString
pixelFormatToString( QVideoFrameFormat::PixelFormat f )
{
	switch( f )
	{
		case QVideoFrameFormat::Format_ARGB8888 :
			return QStringLiteral( "ARGB8888" );

		case QVideoFrameFormat::Format_ARGB8888_Premultiplied :
			return QStringLiteral( "ARGB8888P" );

		case QVideoFrameFormat::Format_XRGB8888 :
			return QStringLiteral( "XRGB8888" );

		case QVideoFrameFormat::Format_BGRA8888 :
			return QStringLiteral( "BGRA8888" );

		case QVideoFrameFormat::Format_BGRA8888_Premultiplied :
			return QStringLiteral( "BGRA8888P" );

		case QVideoFrameFormat::Format_BGRX8888 :
			return QStringLiteral( "BGRX8888" );

		case QVideoFrameFormat::Format_ABGR8888 :
			return QStringLiteral( "ARGB8888" );

		case QVideoFrameFormat::Format_XBGR8888 :
			return QStringLiteral( "XBGR8888" );

		case QVideoFrameFormat::Format_RGBA8888 :
			return QStringLiteral( "RGBA8888" );

		case QVideoFrameFormat::Format_RGBX8888 :
			return QStringLiteral( "RGBX8888" );

		case QVideoFrameFormat::Format_AYUV :
			return QStringLiteral( "AYUV" );

		case QVideoFrameFormat::Format_AYUV_Premultiplied :
			return QStringLiteral( "AYUVP" );

		case QVideoFrameFormat::Format_YUV420P :
			return QStringLiteral( "YUV420P" );

		case QVideoFrameFormat::Format_YUV422P :
			return QStringLiteral( "YUV422P" );

		case QVideoFrameFormat::Format_YV12 :
			return QStringLiteral( "YV12" );

		case QVideoFrameFormat::Format_UYVY :
			return QStringLiteral( "UYVY" );

		case QVideoFrameFormat::Format_YUYV :
			return QStringLiteral( "YUYV" );

		case QVideoFrameFormat::Format_NV12 :
			return QStringLiteral( "NV12" );

		case QVideoFrameFormat::Format_NV21 :
			return QStringLiteral( "NV21" );

		case QVideoFrameFormat::Format_IMC1 :
			return QStringLiteral( "IMC1" );

		case QVideoFrameFormat::Format_IMC2 :
			return QStringLiteral( "IMC2" );

		case QVideoFrameFormat::Format_IMC3 :
			return QStringLiteral( "IMC3" );

		case QVideoFrameFormat::Format_IMC4 :
			return QStringLiteral( "IMC4" );

		case QVideoFrameFormat::Format_Y8 :
			return QStringLiteral( "Y8" );

		case QVideoFrameFormat::Format_Y16 :
			return QStringLiteral( "Y16" );

		case QVideoFrameFormat::Format_P010 :
			return QStringLiteral( "P010" );

		case QVideoFrameFormat::Format_P016 :
			return QStringLiteral( "P016" );

		case QVideoFrameFormat::Format_Jpeg :
			return QStringLiteral( "JPEG" );

		case QVideoFrameFormat::Format_SamplerExternalOES :
			return QStringLiteral( "SEOES" );

		case QVideoFrameFormat::Format_SamplerRect :
			return QStringLiteral( "SR" );

		case QVideoFrameFormat::Format_Invalid :
			return QStringLiteral( "Invalid" );

		default :
			return QStringLiteral( "Unknown" );
	}
}

//
// stringToPixelFormat
//

//! \return String representation of pixel format.
QVideoFrameFormat::PixelFormat
stringToPixelFormat( const QString & s )
{
	if( s == QStringLiteral( "ARGB8888" ) )
		return QVideoFrameFormat::Format_ARGB8888;
	else if( s == QStringLiteral( "ARGB8888P" ) )
		return QVideoFrameFormat::Format_ARGB8888_Premultiplied;
	else if( s == QStringLiteral( "XRGB8888" ) )
		return QVideoFrameFormat::Format_XRGB8888;
	else if( s == QStringLiteral( "BGRA8888" ) )
		return QVideoFrameFormat::Format_BGRA8888;
	else if( s == QStringLiteral( "BGRA8888P" ) )
		return QVideoFrameFormat::Format_BGRA8888_Premultiplied;
	else if( s == QStringLiteral( "BGRX8888" ) )
		return QVideoFrameFormat::Format_BGRX8888;
	else if( s == QStringLiteral( "ARGB8888" ) )
		return QVideoFrameFormat::Format_ABGR8888;
	else if( s == QStringLiteral( "XBGR8888" ) )
		return QVideoFrameFormat::Format_XBGR8888;
	else if( s == QStringLiteral( "RGBA8888" ) )
		return QVideoFrameFormat::Format_RGBA8888;
	else if( s == QStringLiteral( "RGBX8888" ) )
		return QVideoFrameFormat::Format_RGBX8888;
	else if( s == QStringLiteral( "AYUV" ) )
		return QVideoFrameFormat::Format_AYUV;
	else if( s == QStringLiteral( "AYUVP" ) )
		return QVideoFrameFormat::Format_AYUV_Premultiplied;
	else if( s == QStringLiteral( "YUV420P" ) )
		return QVideoFrameFormat::Format_YUV420P;
	else if( s == QStringLiteral( "YUV422P" ) )
		return QVideoFrameFormat::Format_YUV422P;
	else if( s == QStringLiteral( "YV12" ) )
		return QVideoFrameFormat::Format_YV12;
	else if( s == QStringLiteral( "UYVY" ) )
		return QVideoFrameFormat::Format_UYVY;
	else if( s == QStringLiteral( "YUYV" ) )
		return QVideoFrameFormat::Format_YUYV;
	else if( s == QStringLiteral( "NV12" ) )
		return QVideoFrameFormat::Format_NV12;
	else if( s == QStringLiteral( "NV21" ) )
		return QVideoFrameFormat::Format_NV21;
	else if( s == QStringLiteral( "IMC1" ) )
		return QVideoFrameFormat::Format_IMC1;
	else if( s == QStringLiteral( "IMC2" ) )
		return QVideoFrameFormat::Format_IMC2;
	else if( s == QStringLiteral( "IMC3" ) )
		return QVideoFrameFormat::Format_IMC3;
	else if( s == QStringLiteral( "IMC4" ) )
		return QVideoFrameFormat::Format_IMC4;
	else if( s == QStringLiteral( "Y8" ) )
		return QVideoFrameFormat::Format_Y8;
	else if( s == QStringLiteral( "Y16" ) )
		return QVideoFrameFormat::Format_Y16;
	else if( s == QStringLiteral( "P010" ) )
		return QVideoFrameFormat::Format_P010;
	else if( s == QStringLiteral( "P016" ) )
		return QVideoFrameFormat::Format_P016;
	else if( s == QStringLiteral( "JPEG" ) )
		return QVideoFrameFormat::Format_Jpeg;
	else if( s == QStringLiteral( "SEOES" ) )
		return QVideoFrameFormat::Format_SamplerExternalOES;
	else if( s == QStringLiteral( "SR" ) )
		return QVideoFrameFormat::Format_SamplerRect;
	else
		return QVideoFrameFormat::Format_Invalid;
}


//
// ResolutionDialog
//

ResolutionDialog::ResolutionDialog( Frames * frames, QWidget * parent )
	:	QDialog( parent )
	,	d( new ResolutionDialogPrivate( frames->cameraDevice(),
			frames, frames->cameraFormat(), this ) )
{
	d->init();
}

ResolutionDialog::~ResolutionDialog() noexcept
{
}

QCameraFormat
ResolutionDialog::settings() const
{
	const auto settings = d->m_cam.videoFormats();

	const auto data = d->m_ui.m_res->currentText();

	static const QRegularExpression r( "^(\\d+)x(\\d+)\\s+(\\d*\\.?\\d*)\\s+fps,\\s+(\\w+)$" );

	QRegularExpressionMatch match = r.match( data );

	const auto width = match.captured( 1 ).toInt();
	const auto height = match.captured( 2 ).toInt();
	const auto fps = match.captured( 3 ).toDouble();
	const auto fmt = stringToPixelFormat( match.captured( 4 ) );

	for( const auto & s : settings )
	{
		if( s.resolution().width() == width &&
			s.resolution().height() == height &&
			qAbs( s.maxFrameRate() - fps ) < 0.01 &&
			fmt == s.pixelFormat() )
		{
			return s;
		}
	}

	return settings.at( 0 );
}

} /* namespace SecurityCam */
