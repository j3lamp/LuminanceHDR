/**
 * @brief Writing QImage from PFS stream (which is a tonemapped LDR)
 *
 * This file is a part of LuminanceHDR package.
 * ----------------------------------------------------------------------
 * Copyright (C) 2006 Giuseppe Rota
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * ----------------------------------------------------------------------
 *
 * @author Giuseppe Rota <grota@users.sourceforge.net>
 *
 */

#include <QImage>
#include <QSysInfo>
#include <iostream>
#include <assert.h>

#include "Libpfs/pfs.h"
#include "Libpfs/colorspace.h"
#include "Common/msec_timer.h"

inline int clamp2int( const float v, const float minV, const float maxV )
{
    if ( v < minV ) return (int)minV;
    if ( v > maxV ) return (int)maxV;
    return (int)v;
}

// Davide Anastasia <davideanastasia@users.sourceforge.net>
// This new implementation avoid the creation of a temporary buffer
QImage* fromLDRPFStoQImage( pfs::Frame* inpfsframe , pfs::ColorSpace display_colorspace )
{
#ifdef TIMER_PROFILING
    msec_timer __timer;
    __timer.start();
#endif

    assert( inpfsframe != NULL );

    pfs::Channel *Xc, *Yc, *Zc;
    inpfsframe->getXYZChannels( Xc, Yc, Zc );
    assert( Xc != NULL && Yc != NULL && Zc != NULL );

    pfs::Array2DImpl  *X = Xc->getChannelData();
    pfs::Array2DImpl  *Y = Yc->getChannelData();
    pfs::Array2DImpl  *Z = Zc->getChannelData();

    // Back to CS_RGB for the Viewer
    pfs::transformColorSpace(pfs::CS_XYZ, X, Y, Z, display_colorspace, X, Y, Z);

    const int width   = Xc->getWidth();
    const int height  = Xc->getHeight();
    //const int elems   = width*height;

    QImage * temp_qimage = new QImage (width, height, QImage::Format_ARGB32);

    float* p_R = X->getRawData();
    float* p_G = Y->getRawData();
    float* p_B = Z->getRawData();

    int red, green, blue;

    for (int r = 0; r < height; r++)
    {
        QRgb* pixel   = (QRgb*)temp_qimage->scanLine(r);

        for ( int c = 0; c < width; c++ )
        {
            red = clamp2int((*p_R)*255.f, 0.0f, 255.f);
            green = clamp2int((*p_G)*255.f, 0.0f, 255.f);
            blue = clamp2int((*p_B)*255.f, 0.0f, 255.f);

            *pixel = qRgb(red, green, blue);

            pixel++;
            p_R++;
            p_G++;
            p_B++;
        }
    }


#ifdef TIMER_PROFILING
    __timer.stop_and_update();
    std::cout << "fromLDRPFStoQImage() = " << __timer.get_time() << " msec" << std::endl;
#endif

    return temp_qimage;
}
