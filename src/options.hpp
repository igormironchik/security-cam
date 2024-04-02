
/*
	SPDX-FileCopyrightText: 2016-2024 Igor Mironchik <igor.mironchik@gmail.com>
	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef SECURITYCAM_OPTIONS_HPP_INCLUDED
#define SECURITYCAM_OPTIONS_HPP_INCLUDED

// Qt include.
#include <QDialog>
#include <QScopedPointer>
#include <QCameraDevice>

// SecurityCam include.
#include "cfg.hpp"


namespace SecurityCam {

//
// Options
//

class OptionsPrivate;

//! Options of the application.
class Options final
	:	public QDialog
{
	Q_OBJECT

public:
	Options( const Cfg::Cfg & cfg, const QCameraDevice & dev, QWidget * parent );
	~Options() noexcept override;

	//! \return Cfg.
	Cfg::Cfg cfg() const;

public slots:
	//! Img difference.
	void imgDiff( qreal value );

private slots:
	//! Choose folder.
	void chooseFolder();

private:
	friend class OptionsPrivate;

	Q_DISABLE_COPY( Options )

	QScopedPointer< OptionsPrivate > d;
}; // class Options

} // namespace SecurityCam

#endif // SECURITYCAM_OPTIONS_HPP_INCLUDED
