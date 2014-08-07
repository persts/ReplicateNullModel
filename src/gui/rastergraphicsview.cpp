/*
** Copyright (C) 2008 Peter J. Ersts (ersts at amnh.org)
** Creation Date: 2008-02-11
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
#include "rastergraphicsview.h"

#include <QDebug>

#include <QFileDialog>
#include <QSettings>
#include <QFileInfo>

RasterGraphicsView::RasterGraphicsView( QWidget* theParent ) : QGraphicsView( theParent )
{
  setMouseTracking( true );
}

RasterGraphicsView::RasterGraphicsView( QGraphicsScene* theScene, QWidget* theParent ) : QGraphicsView( theScene, theParent )
{
  setMouseTracking( true );
}

void RasterGraphicsView::dragEnterEvent( QDragEnterEvent* theEvent )
{
  theEvent->acceptProposedAction();
}

void RasterGraphicsView::dropEvent( QDropEvent* theEvent )
{
  if( theEvent->mimeData()->text().startsWith( "file://", Qt::CaseInsensitive ) )
  {
    QString lvFilename = theEvent->mimeData()->text().remove( "file://", Qt::CaseInsensitive );
    emit rasterFileSelected( lvFilename );
  }
}

void RasterGraphicsView::dragMoveEvent( QDragMoveEvent* theEvent )
{
  theEvent->accept();
}

void RasterGraphicsView::mouseDoubleClickEvent( QMouseEvent* theEvent )
{
  QSettings lvQSettings;
  QString lvFilename = QFileDialog::getOpenFileName( this, tr( "Open raster layer" ), lvQSettings.value( "lastInputDirectory", "" ).toString(), tr( "All files ( *.* )" ) );
  QFileInfo lvFileInfo( lvFilename );

  if( lvFilename.isEmpty() )
  {
    return;
  }

  lvQSettings.setValue( "lastInputDirectory", lvFileInfo.absoluteDir().absolutePath() );
  emit rasterFileSelected( lvFilename );
}

void RasterGraphicsView::mouseMoveEvent( QMouseEvent* theEvent )
{
  QPointF lvPoint = mapToScene( theEvent->x(), theEvent->y() );
  emit mouseMoved( lvPoint.x(), lvPoint.y() );
}
