
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
#include <QVideoSink>
#include <QCamera>
#include <QMediaCaptureSession>
#include <QMutex>

// SecurityCam include.
#include "cfg.hpp"


namespace SecurityCam {

//! Count of processed frames when key farme changes.
static const int c_keyFrameChangesOn = 10;


//
// Frames
//

//! Frames listener.
class Frames
	:	public QVideoSink
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
	explicit Frames( const Cfg::Cfg & cfg, QObject * parent = nullptr );
	~Frames() override;

	bool present( const QVideoFrame & frame );

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

private slots:
	//! Camera settings changed.
	void camSettingsChanged();
	//! Init camera.
	void initCam();
	//! Stop camera.
	void stopCam();
	//! Video frame changed.
	void frame( const QVideoFrame & frame );
	//! No frames timeout.
	void noFramesTimeout();
	//! 1 second.
	void second();

private:
	//! Detect motion.
	void detectMotion( const QImage & key, const QImage & image );

private:
	Q_DISABLE_COPY( Frames )

	//! Camera.
	QCamera * m_cam;
	//! Counter.
	int m_counter;
	//! Key frame counter.
	int m_keyFrameCounter;
	//! Current frame.
	QImage m_currentFrame;
	//! Key frame.
	QImage m_keyFrame;
	//! Transform.
	QTransform m_transform;
	//! Capture.
	QMediaCaptureSession m_capture;
	//! Motions was detected.
	bool m_motion;
	//! Mutex.
	mutable QMutex m_mutex;
	//! Transformation applied.
	bool m_transformApplied;
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
