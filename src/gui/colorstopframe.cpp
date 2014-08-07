/*
** Copyright (C) 2010 Peter J. Ersts (ersts at amnh.org)
** Creation Date: 2010-01-11
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
#include "colorstopframe.h"

#include <QDebug>

#include <QBrush>
#include <QPalette>
#include <QMouseEvent>
#include <QColorDialog>

ColorStopFrame::ColorStopFrame( QFrame *parent ) :
  QFrame( parent )
{
  this->setAutoFillBackground( true );
  this->setBackgroundRole( QPalette::Base ); //NoRole does not seem to beworking so use an alternate
}

/*
 * PUBLIC FUNCTIONS
 */
QColor ColorStopFrame::color()
{
  if( QPalette::Base == this->backgroundRole() )
  {
    return QColor();
  }
  return this->palette().brush( QPalette::Window ).color();
}

void ColorStopFrame::mousePressEvent( QMouseEvent* theEvent )
{
  if( Qt::LeftButton == theEvent->button() )
  {
    QColor lvSelectedColor = QColorDialog::getColor( this->palette().brush( QPalette::Window ).color() );
    if( lvSelectedColor.isValid() )
    {
      this->setBackgroundRole( QPalette::Window );
      QPalette lvPalette;
      lvPalette.setBrush( QPalette::Window, QBrush( lvSelectedColor ) );
      this->setPalette( lvPalette );
      emit colorChanged();
    }
  }
  else
  {
    if( QPalette::Base != this->backgroundRole() )
    {
      this->setBackgroundRole( QPalette::Base );
      QPalette lvPalette;
      lvPalette.setBrush( QPalette::Window, QBrush( QColor( 0, 0, 0, 0 ) ) );
      this->setPalette( lvPalette );
      emit colorChanged();
    }
  }
}
