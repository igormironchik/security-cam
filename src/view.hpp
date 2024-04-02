
/*
	SPDX-FileCopyrightText: 2016-2024 Igor Mironchik <igor.mironchik@gmail.com>
	SPDX-License-Identifier: GPL-3.0-or-later
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
