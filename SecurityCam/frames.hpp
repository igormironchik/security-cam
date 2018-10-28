
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

#ifndef SECURITYCAM_FRAMES_HPP_INCLUDED
#define SECURITYCAM_FRAMES_HPP_INCLUDED

// Qt include.
#include <QAbstractVideoSurface>
#include <QTransform>
#include <QMutex>
#include <QTimer>

// SecurityCam include.
#include "cfg.hpp"


namespace SecurityCam {

//! Count of processed frames when key farme changes.
static const int c_keyFrameChangesOn = 10;


//
// Frames
//

//! Frames listener.
class Frames final
	:	public QAbstractVideoSurface
{
	Q_OBJECT

signals:
	//! New frame arrived.
	void newFrame( QImage );
	//! Motion detected.
	void motionDetected();
	//! No more motions.
	void noMoreMotions();
	//! Images difference.
	void imgDiff( qreal diff );
	//! No frames.
	void noFrames();
	//! FPS.
	void fps( int v );

public:
	Frames( const Cfg::Cfg & cfg, QObject * parent );

	//! \return Rotation.
	qreal rotation() const;
	//! Set rotation.
	void setRotation( qreal a );

	//! \return Mirrored?
	bool mirrored() const;
	//! Set mirrored.
	void setMirrored( bool on );

	//! \return Threshold.
	qreal threshold() const;
	//! Set threshold.
	void setThreshold( qreal v );

	//! Apply new transformations.
	void applyTransform( bool on = true );

	bool present( const QVideoFrame & frame ) override;

	QList< QVideoFrame::PixelFormat > supportedPixelFormats(
		QAbstractVideoBuffer::HandleType type =
			QAbstractVideoBuffer::NoHandle ) const override;

private:
	//! Detect motion.
	void detectMotion( const QImage & key, const QImage & image );

private slots:
	//! No frames timeout.
	void noFramesTimeout();
	//! 1 second.
	void second();

private:
	//! Key frame.
	QImage m_keyFrame;
	//! Frames counter.
	int m_counter;
	//! Motions was detected.
	bool m_motion;
	//! Mutex.
	mutable QMutex m_mutex;
	//! Transformation applied.
	bool m_transformApplied;
	//! Transformation.
	QTransform m_transform;
	//! Threshold.
	qreal m_threshold;
	//! Rotation.
	qreal m_rotation;
	//! Mirrored.
	bool m_mirrored;
	//! Timer.
	QTimer * m_timer;
	//! 1 second timer.
	QTimer * m_secTimer;
	//! FPS.
	int m_fps;
}; // class Frames

} /* namespace SecurityCam */

#endif // SECURITYCAM_FRAMES_HPP_INCLUDED
