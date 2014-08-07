/*
** Copyright (C) 2009 Peter J. Ersts (ersts at amnh.org)
** Creation Date: 2009-12-10
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
**
** This work was partially funded by the National Aeronautics and Space Administration
** under award(s) NNX09AK19G
**
** This work was partially funded by the National Science Foundation under Grant No.DEB-0641023
**
** Any opinions, findings and conclusions or recommendations expressed in this material are those
** of the author(s) and do not necessarily reflect the views of the National Science Foundation (NSF)
*/
#include "gradientshader.h"

#include <QBrush>
#include <QPainter>
#include <QLinearGradient>

#include <QDebug>

GradientShader::GradientShader()
{
  cvMinimum = 0.0;
  cvMaximum = 255.0;
  cvRange = cvMaximum - cvMinimum;
  QImage lvTemp( 101, 1, QImage::Format_RGB32 );
  cvCanvas = lvTemp;
  cvCanvas.fill( Qt::white );
  setColors( Qt::black, Qt::white );
}

GradientShader::GradientShader( double theMinimum, double theMaximum )
{
  cvMinimum = theMinimum;
  cvMaximum = theMaximum;
  cvRange = cvMaximum - cvMinimum;
  QImage lvTemp( 101, 1, QImage::Format_RGB32 );
  cvCanvas = lvTemp;
  cvCanvas.fill( Qt::white );
  setColors( Qt::black, Qt::white );
}

GradientShader::~GradientShader()
{
}

/*
 * PUBLIC FUNCTIONS
 */

void GradientShader::setColors( QColor color1, QColor color2 )
{
  QLinearGradient lvGradient( 0, 0, 101, 1.0 );
  lvGradient.setColorAt( 0.0, color1 );
  lvGradient.setColorAt( 1.0, color2 );

  QPainter lvPainter( &cvCanvas );
  lvPainter.fillRect( 0, 0 , 101, 1, QBrush( lvGradient ) );
}

void GradientShader::setColors( QColor color1, QColor color2, QColor color3 )
{
  QLinearGradient lvGradient( 0, 0, 101, 1.0 );
  lvGradient.setColorAt( 0.0, color1 );
  lvGradient.setColorAt( 0.5, color2 );
  lvGradient.setColorAt( 1.0, color3 );

  QPainter lvPainter( &cvCanvas );
  lvPainter.fillRect( 0, 0 , 101, 1, QBrush( lvGradient ) );
}

void GradientShader::setColors( QColor color1, QColor color2, QColor color3, QColor color4 )
{
  QLinearGradient lvGradient( 0, 0, 101, 1.0 );
  lvGradient.setColorAt( 0.0, color1 );
  lvGradient.setColorAt( 0.33, color2 );
  lvGradient.setColorAt( 0.66, color3 );
  lvGradient.setColorAt( 1.0, color4 );

  QPainter lvPainter( &cvCanvas );
  lvPainter.fillRect( 0, 0 , 101, 1, QBrush( lvGradient ) );
}

void GradientShader::setInterval( double theMinimum, double theMaximum )
{
  if( theMinimum > theMaximum )
  {
    double lvTemp = theMaximum;

    cvMaximum = theMinimum;
    cvMinimum = lvTemp;
  }
  else
  {
    cvMaximum = theMaximum;
    cvMinimum = theMinimum;
  }

  cvRange = cvMaximum - cvMinimum;

}

QColor GradientShader::shade( double theValue )
{
  if( 0 == cvRange ) { return Qt::white; }

  if( theValue < cvMinimum )
  {
    return cvCanvas.pixel( 0, 0 );
  }

  if( theValue > cvMaximum )
  {
    return cvCanvas.pixel( 100, 0 );
  }

  return cvCanvas.pixel(( int )((( theValue - cvMinimum ) / cvRange ) * 100 ) , 0 );

}


