
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
	ViewPrivate( View * parent )
		:	m_resized( false )
		,	q( parent )
	{
	}

	//! Init.
	void init();

	//! Image.
	QImage m_image;
	//! Resized?
	bool m_resized;
	//! Parent.
	View * q;
}; // class ViewPrivate

void
ViewPrivate::init()
{
	q->setAutoFillBackground( false );
}


//
// View
//

View::View( QWidget * parent )
	:	QWidget( parent )
	,	d( new ViewPrivate( this ) )
{
	d->init();
}

View::~View()
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
	QPainter p( this );
	p.setBackground( Qt::black );

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

void
View::resizeEvent( QResizeEvent * e )
{
	e->accept();

	d->m_resized = false;

	update();
}

} /* namespace SecurityCam */
