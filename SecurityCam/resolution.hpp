
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

#ifndef SECURITYCAM__RESOLUTION_HPP__INCLUDED
#define SECURITYCAM__RESOLUTION_HPP__INCLUDED

// Qt include.
#include <QDialog>
#include <QCameraViewfinderSettings>
#include <QScopedPointer>

QT_BEGIN_NAMESPACE
class QCamera;
QT_END_NAMESPACE


namespace SecurityCam {

//
// ResolutionDialog
//

class ResolutionDialogPrivate;

//! Dialog with resolution settings.
class ResolutionDialog Q_DECL_FINAL
	:	public QDialog
{
	Q_OBJECT

public:
	ResolutionDialog( QCamera * cam, const QCameraViewfinderSettings & s,
		QWidget * parent );
	~ResolutionDialog();

	//! \return Selected settings.
	QCameraViewfinderSettings settings() const;

private:
	Q_DISABLE_COPY( ResolutionDialog )

	QScopedPointer< ResolutionDialogPrivate > d;
}; // class ResolutionDialog

} /* namespace SecurityCam */

#endif // SECURITYCAM__RESOLUTION_HPP__INCLUDED
