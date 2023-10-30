
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

#ifndef SECURITYCAM_MAINWINDOW_HPP_INCLUDED
#define SECURITYCAM_MAINWINDOW_HPP_INCLUDED

// Qt include.
#include <QMainWindow>
#include <QScopedPointer>
#include <QSystemTrayIcon>
#include <QCamera>


namespace SecurityCam {

//
// MainWindow
//

class MainWindowPrivate;

//! Main window of the application.
class MainWindow final
	:	public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow( const QString & cfgFileName );
	~MainWindow() noexcept override;

private slots:
	//! Quit.
	void quit();
	//! Options.
	void options();
	//! Change resolution.
	void resolution();
	//! System tray activated.
	void sysTrayActivated( QSystemTrayIcon::ActivationReason reason );
	//! Motion detected.
	void motionDetected();
	//! No more motion.
	void noMoreMotion();
	//! Stop recording on timeout.
	void stopRecording();
	//! Take image.
	void takeImage();
	//! About.
	void about();
	//! About Qt.
	void aboutQt();
	//! Licenses.
	void licenses();
	//! Clean.
	void clean();
	//! FPS.
	void fps( int v );
	//! Set status label.
	void setStatusLabel();


protected:
	void closeEvent( QCloseEvent * e ) override;

private:
	friend class MainWindowPrivate;

	Q_DISABLE_COPY( MainWindow )

	QScopedPointer< MainWindowPrivate > d;
}; // class MainWindow

} /* namespace SecurityCam */

#endif // SECURITYCAM_MAINWINDOW_HPP_INCLUDED
