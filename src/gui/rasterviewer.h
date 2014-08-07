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
#ifndef RASTERVIEWER_H
#define RASTERVIEWER_H

#include "rastermodel.h"
#include "localitylist.h"
#include "localitycollection.h"

#include <QWidget>
#include <QGraphicsScene>

namespace Ui
{
  class RasterViewerWidget;
}

class RasterViewer : public QWidget
{
    Q_OBJECT
  public:
    explicit RasterViewer( QWidget *parent = 0, bool loadRasterOnSelect = false );
    explicit RasterViewer( QString theRasterFileName, QWidget *parent = 0 );
    ~RasterViewer();

    void enableControls( bool );
    void setCoordinatesVisible( bool theState );
    void setLocalityCollection( const LocalityCollection &theCollection );
    void setMinimumMaximumVisible( bool theState );
    void setRasterModel( RasterModel* const theModel );
    void setRasterNameVisible( bool theState );
    void setThresholdVisible( bool theState );


  signals:
    void rasterFileSelected( QString theRasterFileName );
    void consoleMessage( QString theMessage );

  public slots:

  private:
    bool cvControlsEnabled;
    QGraphicsScene* cvGraphicsScene;
    bool cvLoadRasterOnSelect;
    LocalityCollection cvLocalities;
    RasterModel* cvRasterModel;
    Ui::RasterViewerWidget* cvUi;
    bool cvUseGradientToggled;
    bool cvThresholdSet;


    void init();
    void displayLocalityList( LocalityList theLocalityList, Qt::GlobalColor theColor );
    void updateGui();

  private slots:
    void acceptRasterFileSelection( QString theRasterFileName );
    void mouseCoordinates( double, double );
    void on_pbtnReload_clicked();
    void on_pbtnZoomIn_clicked();
    void on_pbtnZoomOut_clicked();
    void on_rbtnUseGradient_toggled();
    void setShader();

};

#endif // RASTERVIEWER_H
