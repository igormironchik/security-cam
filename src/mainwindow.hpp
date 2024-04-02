
/*
	SPDX-FileCopyrightText: 2016-2024 Igor Mironchik <igor.mironchik@gmail.com>
	SPDX-License-Identifier: GPL-3.0-or-later
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
