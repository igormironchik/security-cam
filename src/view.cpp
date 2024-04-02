
/*
	SPDX-FileCopyrightText: 2016-2024 Igor Mironchik <igor.mironchik@gmail.com>
	SPDX-License-Identifier: GPL-3.0-or-later
*/

// SecurityCam include.
#include "view.hpp"

// Qt include.
#include <QPainter>
#include <QResizeEvent>


namespace SecurityCam {

//
// ViewPrivate
//

class ViewPrivate {
public:
	explicit ViewPrivate( View * parent )
		:	m_resized( false )
		,	q( parent )
	{
	}

	//! Image.
	QImage m_image;
	//! Resized?
	bool m_resized;
	//! Parent.
	View * q;
}; // class ViewPrivate


//
// View
//

View::View( QWidget * parent )
	:	QWidget( parent )
	,	d( new ViewPrivate( this ) )
{
}

View::~View() noexcept
{
}

void
View::draw( const QImage & image )
{
	d->m_resized = false;

	d->m_image = image;

	update();
}

void
View::paintEvent( QPaintEvent * )
{
	if( isVisible() )
	{
		QPainter p( this );

		if( !d->m_image.isNull() )
		{
			if( !d->m_resized )
			{
				d->m_image = d->m_image.scaled( size(), Qt::KeepAspectRatio );

				d->m_resized = true;
			}

			const int x = rect().x() + ( size().width() - d->m_image.width() ) / 2;
			const int y = rect().y() + ( size().height() - d->m_image.height() ) / 2;

			p.drawImage( x, y, d->m_image );
		}
	}
}

void
View::resizeEvent( QResizeEvent * e )
{
	e->accept();

	d->m_resized = false;

	update();
}

} /* namespace SecurityCam */
