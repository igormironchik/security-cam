
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
#include <QRegExp>

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
	ResolutionDialogPrivate( QCamera * cam, Frames * frames,
		const QCameraViewfinderSettings & s,
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
	QCamera * m_cam;
	//! Frames.
	Frames * m_frames;
	//! Settings.
	QCameraViewfinderSettings m_settings;
	//! Ui.
	Ui::ResolutionDialog m_ui;
	//! Parent.
	ResolutionDialog * q;
}; // class ResolutionDialogPrivate

void
ResolutionDialogPrivate::init()
{
	m_ui.setupUi( q );

	const auto settings = m_cam->supportedViewfinderSettings();

	for( const auto & s : settings )
	{
		if( m_frames->supportedPixelFormats().contains( s.pixelFormat() ) )
		{
			const QString data = QString::number( s.resolution().width() ) +
				QLatin1Char( 'x' ) + QString::number( s.resolution().height() ) +
				QLatin1Char( ' ' ) + QString::number( s.maximumFrameRate(), 'f', 0 ) +
				QLatin1String( " fps" );

			m_ui.m_res->addItem( data );

			if( s.resolution() == m_settings.resolution() &&
				s.maximumFrameRate() == m_settings.maximumFrameRate() )
					m_ui.m_res->setCurrentIndex( m_ui.m_res->count() - 1 );
		}
	}
}


//
// ResolutionDialog
//

ResolutionDialog::ResolutionDialog( QCamera * cam, Frames * frames,
	const QCameraViewfinderSettings & s,
	QWidget * parent )
	:	QDialog( parent )
	,	d( new ResolutionDialogPrivate( cam, frames, s, this ) )
{
	d->init();
}

ResolutionDialog::~ResolutionDialog()
{
}

QCameraViewfinderSettings
ResolutionDialog::settings() const
{
	QCameraViewfinderSettings res;

	const auto settings = d->m_cam->supportedViewfinderSettings();

	const auto data = d->m_ui.m_res->currentText();

	static const QRegExp r( "(\\d+)x(\\d+)\\s+(\\d*\\.?\\d*)\\s+fps" );

	r.exactMatch( data );

	const auto width = r.cap( 1 ).toInt();
	const auto height = r.cap( 2 ).toInt();
	const auto fps = r.cap( 3 ).toDouble();

	for( const auto & s : settings )
	{
		if( s.resolution().width() == width &&
			s.resolution().height() == height &&
			qAbs( s.maximumFrameRate() - fps ) < 0.001 )
		{
			res = s;

			break;
		}
	}

	return res;
}

} /* namespace SecurityCam */
