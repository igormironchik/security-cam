
/*
	SPDX-FileCopyrightText: 2016-2024 Igor Mironchik <igor.mironchik@gmail.com>
	SPDX-License-Identifier: GPL-3.0-or-later
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
