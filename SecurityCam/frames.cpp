
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
#include "frames.hpp"


namespace SecurityCam {

//
// Frames
//

Frames::Frames( QObject * parent )
	:	QAbstractVideoSurface( parent )
{
}

bool
Frames::present( const QVideoFrame & frame )
{
	if( !isActive() )
		return false;

	QVideoFrame f = frame;
	f.map( QAbstractVideoBuffer::ReadOnly );

	QImage image( f.bits(), f.width(), f.height(), f.bytesPerLine(),
		QVideoFrame::imageFormatFromPixelFormat( f.pixelFormat() ) );

	f.unmap();

	emit newFrame( image.copy( image.rect() ) );

	return true;
}

QList< QVideoFrame::PixelFormat >
Frames::supportedPixelFormats( QAbstractVideoBuffer::HandleType type ) const
{
	Q_UNUSED( type )

	return QList< QVideoFrame::PixelFormat > ()
		<< QVideoFrame::Format_ARGB32
		<< QVideoFrame::Format_ARGB32_Premultiplied
		<< QVideoFrame::Format_RGB32
		<< QVideoFrame::Format_RGB24
		<< QVideoFrame::Format_RGB565
		<< QVideoFrame::Format_RGB555
		<< QVideoFrame::Format_ARGB8565_Premultiplied;
}

} /* namespace SecurityCam */
