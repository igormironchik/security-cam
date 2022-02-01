
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
			QLatin1String( " fps" );

		m_ui.m_res->addItem( data );

		if( s.resolution() == m_settings.resolution() &&
			s.maxFrameRate() == m_settings.maxFrameRate() )
				m_ui.m_res->setCurrentIndex( m_ui.m_res->count() - 1 );
	}
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

	static const QRegularExpression r( "^(\\d+)x(\\d+)\\s+(\\d*\\.?\\d*)\\s+fps$" );

	QRegularExpressionMatch match = r.match( data );

	const auto width = match.captured( 1 ).toInt();
	const auto height = match.captured( 2 ).toInt();
	const auto fps = match.captured( 3 ).toDouble();

	for( const auto & s : settings )
	{
		if( s.resolution().width() == width &&
			s.resolution().height() == height &&
			qAbs( s.maxFrameRate() - fps ) < 0.001 &&
			s.pixelFormat() != QVideoFrameFormat::Format_Jpeg )
		{
			return s;
		}
	}

	return settings.at( 0 );
}

} /* namespace SecurityCam */
