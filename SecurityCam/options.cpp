
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
#include "options.hpp"
#include "ui_options.h"

// Qt include.
#include <QCameraInfo>
#include <QGroupBox>
#include <QComboBox>
#include <QToolButton>
#include <QLineEdit>
#include <QStandardPaths>
#include <QFileDialog>


namespace SecurityCam {

//
// OptionsPrivate
//

class OptionsPrivate {
public:
	OptionsPrivate( Options * parent, const Cfg::Cfg & cfg )
		:	m_cfg( cfg )
		,	q( parent )
	{
	}

	//! Init.
	void init();

	//! Ui.
	Ui::Options m_ui;
	//! Cfg.
	Cfg::Cfg m_cfg;
	//! Cameras.
	QList< QCameraInfo > m_cameras;
	//! Parent.
	Options * q;
}; // class OptionsPrivate

void
OptionsPrivate::init()
{
	m_ui.setupUi( q );

	m_cameras = QCameraInfo::availableCameras();

	if( !m_cameras.isEmpty() )
	{
		int i = 0;

		foreach( auto & c, m_cameras )
		{
			m_ui.m_camera->addItem( c.description() );

			if( c.deviceName() == m_cfg.camera() )
				m_ui.m_camera->setCurrentIndex( i );

			++i;
		}
	}
	else
		m_ui.m_cameraBox->setEnabled( false );

	if( m_cfg.folder().isEmpty() )
		m_ui.m_dir->setText( QStandardPaths::writableLocation(
			QStandardPaths::PicturesLocation ) );
	else
		m_ui.m_dir->setText( m_cfg.folder() );

	Options::connect( m_ui.m_selectDir, &QToolButton::clicked,
		q, &Options::chooseFolder );
}


//
// Options
//

Options::Options( const Cfg::Cfg & cfg, QWidget * parent )
	:	QDialog( parent )
	,	d( new OptionsPrivate( this, cfg ) )
{
	d->init();
}

Options::~Options()
{
}

Cfg::Cfg
Options::cfg() const
{
	d->m_cfg.setCamera( d->m_cameras.at(
		d->m_ui.m_camera->currentIndex() ).deviceName() );
	d->m_cfg.setFolder( d->m_ui.m_dir->text() );

	return d->m_cfg;
}

void
Options::chooseFolder()
{
	const QString folder = QFileDialog::getExistingDirectory( this,
		tr( "Choose folder..." ), d->m_ui.m_dir->text() );

	if( !folder.isEmpty() )
		d->m_ui.m_dir->setText( folder );
}

} // namespace SecurityCam
