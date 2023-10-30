
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
#include <QGroupBox>
#include <QComboBox>
#include <QToolButton>
#include <QLineEdit>
#include <QStandardPaths>
#include <QFileDialog>
#include <QMediaDevices>


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
	void init( const QCameraDevice & dev );

	//! Ui.
	Ui::Options m_ui;
	//! Cfg.
	Cfg::Cfg m_cfg;
	//! Cameras.
	QList< QCameraDevice > m_cameras;
	//! Parent.
	Options * q;
}; // class OptionsPrivate

void
OptionsPrivate::init( const QCameraDevice & dev )
{
	m_ui.setupUi( q );

	m_cameras = QMediaDevices::videoInputs();

	if( !m_cameras.isEmpty() )
	{
		int i = 0;

		foreach( auto & c, m_cameras )
		{
			m_ui.m_camera->addItem( c.description() );

			if( c.description() == m_cfg.camera() )
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

	m_ui.m_cleanTime->setTime( QTime::fromString( m_cfg.clearTime(),
		QLatin1String( "hh:mm" ) ) );

	m_ui.m_storeDays->setValue( m_cfg.storeDays() );

	if( m_cfg.storeDays() <= 0 )
		m_ui.m_clean->setChecked( false );
	else
		m_ui.m_clean->setChecked( true );

	if( m_cfg.applyTransform() )
	{
		m_ui.m_transformGroup->setChecked( true );

		m_ui.m_rotation->setValue( m_cfg.rotation() );

		m_ui.m_mirrored->setChecked( m_cfg.mirrored() );
	}
	else
		m_ui.m_transformGroup->setChecked( false );

	m_ui.m_snapshotTimeout->setValue( m_cfg.snapshotTimeout() );

	m_ui.m_stopTimeout->setValue( m_cfg.stopTimeout() );

	m_ui.m_threshold->setValue( m_cfg.threshold() );


	Options::connect( m_ui.m_selectDir, &QToolButton::clicked,
		q, &Options::chooseFolder );
}


//
// Options
//

Options::Options( const Cfg::Cfg & cfg, const QCameraDevice & dev, QWidget * parent )
	:	QDialog( parent )
	,	d( new OptionsPrivate( this, cfg ) )
{
	d->init( dev );
}

Options::~Options() noexcept
{
}

Cfg::Cfg
Options::cfg() const
{
	d->m_cfg.set_camera( d->m_cameras.at(
		d->m_ui.m_camera->currentIndex() ).description() );
	d->m_cfg.set_folder( d->m_ui.m_dir->text() );
	d->m_cfg.set_storeDays( d->m_ui.m_clean->isChecked() ?
		d->m_ui.m_storeDays->value() : 0 );
	d->m_cfg.set_clearTime( d->m_ui.m_cleanTime->time()
		.toString( QLatin1String( "hh:mm" ) ) );
	d->m_cfg.set_applyTransform( d->m_ui.m_transformGroup->isChecked() );
	d->m_cfg.set_rotation( d->m_ui.m_rotation->value() );
	d->m_cfg.set_mirrored( d->m_ui.m_mirrored->isChecked() );
	d->m_cfg.set_snapshotTimeout( d->m_ui.m_snapshotTimeout->value() );
	d->m_cfg.set_stopTimeout( d->m_ui.m_stopTimeout->value() );
	d->m_cfg.set_threshold( d->m_ui.m_threshold->value() );

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

void
Options::imgDiff( qreal value )
{
	d->m_ui.m_diff->setText( QString::number( value, 'f', 5 ) );
}

} // namespace SecurityCam
