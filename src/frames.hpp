
/*
	SPDX-FileCopyrightText: 2016-2024 Igor Mironchik <igor.mironchik@gmail.com>
	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef SECURITYCAM_FRAMES_HPP_INCLUDED
#define SECURITYCAM_FRAMES_HPP_INCLUDED

// Qt include.
#include <QVideoSink>
#include <QCamera>
#include <QMediaCaptureSession>
#include <QMutex>
#include <QMap>

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

	//! \return Current format of the camera.
	QCameraFormat cameraFormat() const;

	//! \return Current camera device.
	QCameraDevice cameraDevice() const;

public slots:
	//! Init camera.
	void initCam( const QString & name );
	//! Stop camera.
	void stopCam();
	//! Set resolution.
	void setResolution( const QCameraFormat & fmt );
	//! Take image.
	void takeImage( const QString & dirName );

private slots:
	//! Video frame changed.
	void frame( const QVideoFrame & frame );
	//! No frames timeout.
	void noFramesTimeout();
	//! 1 second.
	void second();
	//! Image captured.
	void imageCaptured( int id, const QImage & img );

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
	//! Image capture.
	QImageCapture * m_imgCapture;
	//! Map of file names.
	QMap< int, QString > m_fileNames;
}; // class Frames

} /* namespace SecurityCam */

#endif // SECURITYCAM_FRAMES_HPP_INCLUDED
