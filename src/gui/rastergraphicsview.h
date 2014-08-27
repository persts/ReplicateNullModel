/*
** Copyright (C) 2009 Peter J. Ersts (ersts at amnh.org)
** Creation Date: 2009-02-11
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
#ifndef RASTERGRAPHICSVIEW_H
#define RASTERGRAPHICSVIEW_H

#include "rastergraphicsview.h"

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QDragMoveEvent>
#include <QDropEvent>

class RasterGraphicsView: public QGraphicsView
{
    Q_OBJECT

  public:
    RasterGraphicsView( QWidget* theParent = 0 );
    RasterGraphicsView( QGraphicsScene* theScene, QWidget* theParent = 0 );
    void dragEnterEvent( QDragEnterEvent* );
    void dropEvent( QDropEvent* );
    void dragLeaveEvent( QDragLeaveEvent* ) {}
    void dragMoveEvent( QDragMoveEvent* );
    void mouseDoubleClickEvent( QMouseEvent* );
    void mouseMoveEvent( QMouseEvent* );

  signals:
    void mouseMoved( double, double );
    void rasterFileSelected( QString );
};
#endif
