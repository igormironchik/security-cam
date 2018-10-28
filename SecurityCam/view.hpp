
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

#ifndef SECURITYCAM_VIEW_HPP_INCLUDED
#define SECURITYCAM_VIEW_HPP_INCLUDED

// Qt include.
#include <QWidget>
#include <QScopedPointer>


namespace SecurityCam {

//
// View
//

class ViewPrivate;

//! View of the video data from the camera.
class View final
	:	public QWidget
{
	Q_OBJECT

public:
	explicit View( QWidget * parent );
	~View() noexcept override;

public slots:
	//! Draw image.
	void draw( const QImage & image );

protected:
	void paintEvent( QPaintEvent * ) override;
	void resizeEvent( QResizeEvent * e ) override;

private:
	Q_DISABLE_COPY( View )

	QScopedPointer< ViewPrivate > d;
}; // class View

} /* namespace SecurityCam */

#endif // SECURITYCAM_VIEW_HPP_INCLUDED
