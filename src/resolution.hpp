
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

#ifndef SECURITYCAM_RESOLUTION_HPP_INCLUDED
#define SECURITYCAM_RESOLUTION_HPP_INCLUDED

// Qt include.
#include <QDialog>
#include <QCameraDevice>
#include <QScopedPointer>

QT_BEGIN_NAMESPACE
class QCamera;
QT_END_NAMESPACE


namespace SecurityCam {

class Frames;

//
// pixelFormatToString
//

//! \return String representation of pixel format.
QString
pixelFormatToString( QVideoFrameFormat::PixelFormat f );


//
// stringToPixelFormat
//

//! \return String representation of pixel format.
QVideoFrameFormat::PixelFormat
stringToPixelFormat( const QString & s );


//
// ResolutionDialog
//

class ResolutionDialogPrivate;

//! Dialog with resolution settings.
class ResolutionDialog final
	:	public QDialog
{
	Q_OBJECT

public:
	ResolutionDialog( Frames * frames, QWidget * parent );
	~ResolutionDialog() noexcept override;

	//! \return Selected settings.
	QCameraFormat settings() const;

private:
	Q_DISABLE_COPY( ResolutionDialog )

	QScopedPointer< ResolutionDialogPrivate > d;
}; // class ResolutionDialog

} /* namespace SecurityCam */

#endif // SECURITYCAM_RESOLUTION_HPP_INCLUDED
