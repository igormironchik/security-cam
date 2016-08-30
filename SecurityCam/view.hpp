
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

#ifndef SECURITYCAM__VIEW_HPP__INCLUDED
#define SECURITYCAM__VIEW_HPP__INCLUDED

// Qt include.
#include <QWidget>
#include <QScopedPointer>


namespace SecurityCam {

//
// View
//

class ViewPrivate;

//! View of the video data from the camera.
class View Q_DECL_FINAL
	:	public QWidget
{
	Q_OBJECT

public:
	explicit View( QWidget * parent );
	~View();

public slots:
	//! Draw image.
	void draw( const QImage & image );

protected:
	void paintEvent( QPaintEvent * ) Q_DECL_OVERRIDE;
	void resizeEvent( QResizeEvent * e ) Q_DECL_OVERRIDE;

private:
	Q_DISABLE_COPY( View )

	QScopedPointer< ViewPrivate > d;
}; // class View

} /* namespace SecurityCam */

#endif // SECURITYCAM__VIEW_HPP__INCLUDED
