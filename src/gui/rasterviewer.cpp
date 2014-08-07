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
#include "rasterviewer.h"

#include "ui_rasterviewerwidget.h"

#include "locality.h"
#include "gradientshader.h"

#include <QDebug>

#include <QMatrix>
#include <QPixmap>
#include <QMessageBox>
#include <QGraphicsPixmapItem>

RasterViewer::RasterViewer( QWidget *parent, bool loadRasterOnSelect ) :
  QWidget( parent )
{
  cvLoadRasterOnSelect = loadRasterOnSelect;
  init();

}

RasterViewer::RasterViewer( QString theRasterFileName, QWidget *parent ) :
  QWidget( parent )
{
  cvLoadRasterOnSelect = true;
  init();
  acceptRasterFileSelection( theRasterFileName );
}


RasterViewer::~RasterViewer()
{
  if( 0 != cvGraphicsScene )
  {
    delete cvGraphicsScene;
  }

  if( cvLoadRasterOnSelect && cvRasterModel != 0 )
  {
    delete cvRasterModel;
  }
}

/*
 * PUBLIC FUNCTIONS
 */
void RasterViewer::enableControls( bool enable )
{
  cvControlsEnabled = enable;
  cvUi->pbtnReload->setEnabled( enable );

}

void RasterViewer::setCoordinatesVisible( bool theState )
{
  cvUi->frameCoordinates->setVisible( theState );
}

void RasterViewer::setLocalityCollection( const LocalityCollection &theCollection )
{
  cvLocalities = theCollection;
  updateGui();
}

void RasterViewer::setMinimumMaximumVisible( bool theState )
{
  cvUi->frameMinimumMaximum->setVisible( theState );
}

void RasterViewer::setRasterModel( RasterModel* const theModel )
{
  if( 0 == theModel ) { return; }

  if( cvLoadRasterOnSelect )
  {
    cvLoadRasterOnSelect = false;
    if( 0 != cvRasterModel )
    {
      delete cvRasterModel;
      cvRasterModel = 0;
    }
  }
  cvRasterModel = theModel;
  updateGui();
}

void RasterViewer::setRasterNameVisible( bool theState )
{
  cvUi->frameRasterName->setVisible( theState );
}

void RasterViewer::setThresholdVisible( bool theState )
{
  cvUi->frameThreshold->setVisible( theState );
}

/*
 * PRIVATE FUNCTIONS
 */
void RasterViewer::acceptRasterFileSelection( QString theRasterFileName )
{
  if( cvLoadRasterOnSelect )
  {
    if( 0 != cvRasterModel )
    {
      delete cvRasterModel;
    }

    cvRasterModel = new RasterModel( theRasterFileName );

    if( cvRasterModel->isValid() )
    {
      updateGui();
    }
    else
    {
      QMessageBox::critical( this, tr( "Invalid Raster" ), cvRasterModel->lastError() );
    }

  }
  else
  {
    emit rasterFileSelected( theRasterFileName );
  }
}

void RasterViewer::displayLocalityList( LocalityList theLocalityList, Qt::GlobalColor theColor )
{
  if( !cvRasterModel->isValid() ) { return; }

  int lvTotalLocalities = theLocalityList.size();
  Locality lvLocality;
  QPointF lvPoint;
  for( int lvIterator = 0; lvIterator < lvTotalLocalities; lvIterator++ )
  {
    lvLocality = theLocalityList.locality( lvIterator );
    if( !lvLocality.isValid )
    {
      continue;
    }
    lvPoint = cvRasterModel->realWorldToPixel( lvLocality.realWorldCoordinates );
    QGraphicsRectItem* lvRectangle = cvGraphicsScene->addRect( QRectF(( int )lvPoint.x(), ( int )lvPoint.y(), 1, 1 ), QPen( QColor( theColor ) ), QBrush( QColor( theColor ) ) );
    lvRectangle->setZValue( 10 );
  }
}

void RasterViewer::init()
{
  this->setAttribute( Qt::WA_DeleteOnClose );
  cvUi = new Ui::RasterViewerWidget();
  cvUi->setupUi( this );

  cvControlsEnabled = true;

  cvRasterModel = 0;
  cvUseGradientToggled = false;
  cvThresholdSet = false;

  cvGraphicsScene = new QGraphicsScene();
  cvUi->gvRasterDisplay->setAlignment( Qt::AlignLeft | Qt::AlignTop );
  cvUi->gvRasterDisplay->fitInView( cvGraphicsScene->sceneRect() );
  cvUi->gvRasterDisplay->setScene( cvGraphicsScene );

  connect( cvUi->gvRasterDisplay, SIGNAL( rasterFileSelected( QString ) ), this, SLOT( acceptRasterFileSelection( QString ) ) );
  connect( cvUi->gvRasterDisplay, SIGNAL( mouseMoved( double, double ) ), this, SLOT( mouseCoordinates( double, double ) ) );
  connect( cvUi->frameColorStop1, SIGNAL( colorChanged() ), this, SLOT( setShader() ) );
  connect( cvUi->frameColorStop2, SIGNAL( colorChanged() ), this, SLOT( setShader() ) );
  connect( cvUi->frameColorStop3, SIGNAL( colorChanged() ), this, SLOT( setShader() ) );
  connect( cvUi->frameColorStop4, SIGNAL( colorChanged() ), this, SLOT( setShader() ) );
}

void RasterViewer::mouseCoordinates( double x, double y )
{
  if( 0 == cvRasterModel ) { return; }

  QPointF lvRealWorld = cvRasterModel->pixelToRealWorld( x, y );
  cvUi->labelCoordinates->setText( QString( tr( "Pixe X: %1\tY: %2\n\nReal X: %3\tY: %4" ) ) .arg( QString::number( x, 'f', 2 ), 10 ) .arg( QString::number( y, 'f', 2 ), 10 ) .arg( QString::number( lvRealWorld.x(), 'f', 2 ), 10 ) .arg( QString::number( lvRealWorld.y(), 'f', 2 ), 10 ) );
}

void RasterViewer::on_pbtnReload_clicked()
{

  if( cvUi->leThreshold->text() != "" && cvRasterModel != 0 )
  {
    cvThresholdSet = true;
    cvUi->rbtnUseGradient->setChecked( false );
    if( cvRasterModel->reload( cvUi->leThreshold->text().toDouble() ) )
    {
      updateGui();
    }
    else
    {
      emit consoleMessage( cvRasterModel->lastError() );
    }
  }
  else if( cvUi->leThreshold->text() == "" && cvRasterModel != 0 )
  {
    if( cvRasterModel->reload() )
    {
      updateGui();
    }
    else
    {
      emit consoleMessage( cvRasterModel->lastError() );
    }
  }
}

void RasterViewer::on_pbtnZoomIn_clicked()
{
  cvUi->gvRasterDisplay->scale( 2, 2 );
}

void RasterViewer::on_pbtnZoomOut_clicked()
{
  cvUi->gvRasterDisplay->scale( 0.5, 0.5 );
}

void RasterViewer::on_rbtnUseGradient_toggled()
{
  cvUseGradientToggled = true;
  setShader();
}

void RasterViewer::setShader()
{
  if( 0 != cvRasterModel )
  {
    QList<QColor> lvColors;
    if( cvUi->rbtnUseGradient->isChecked() )
    {
      if( cvUi->frameColorStop1->color().isValid() )
      {
        lvColors << cvUi->frameColorStop1->color();
      }

      if( cvUi->frameColorStop2->color().isValid() )
      {
        lvColors << cvUi->frameColorStop2->color();
      }

      if( cvUi->frameColorStop3->color().isValid() )
      {
        lvColors << cvUi->frameColorStop3->color();
      }

      if( cvUi->frameColorStop4->color().isValid() )
      {
        lvColors << cvUi->frameColorStop4->color();
      }
    }

    if( 0 == lvColors.length() )
    {
      cvRasterModel->shader()->setColors( Qt::black, Qt::white );
    }
    else if( 1 == lvColors.length() )
    {
      cvRasterModel->shader()->setColors( lvColors.at( 0 ), Qt::white );
    }
    else if( 2 == lvColors.length() )
    {
      cvRasterModel->shader()->setColors( lvColors.at( 0 ), lvColors.at( 1 ) );
    }
    else if( 3 == lvColors.length() )
    {
      cvRasterModel->shader()->setColors( lvColors.at( 0 ), lvColors.at( 1 ), lvColors.at( 2 ) );
    }
    else if( 4 == lvColors.length() )
    {
      cvRasterModel->shader()->setColors( lvColors.at( 0 ), lvColors.at( 1 ), lvColors.at( 2 ), lvColors.at( 3 ) );
    }

    if(( cvUi->rbtnUseGradient->isChecked() || cvUseGradientToggled ) && !cvThresholdSet )
    {
      cvRasterModel->reload();
      updateGui();
    }
    cvUseGradientToggled = false;
    cvThresholdSet = false;
  }
}

void RasterViewer::updateGui()
{
  if( 0 == cvRasterModel || !cvRasterModel->isValid() ) { return; }

  QList<QGraphicsItem *> lvItems = cvGraphicsScene->items();
  int lvTotalItems = lvItems.length();
  for( int lvIterator = 0; lvIterator < lvTotalItems; lvIterator++ )
  {
    QGraphicsItem* lvGraphicsItem = lvItems.takeLast();
    cvGraphicsScene->removeItem( lvGraphicsItem );
    delete lvGraphicsItem;
  }

  QPixmap lvPixmap = QPixmap::fromImage( *( cvRasterModel->toImage() ) );
  cvGraphicsScene->addPixmap( lvPixmap );
  cvGraphicsScene->setSceneRect( 0, 0, lvPixmap.width(), lvPixmap.height() );

  QMatrix lvResetMatrix;
  cvUi->gvRasterDisplay->setMatrix( lvResetMatrix );

  double lvScaleFactor = 0.0;
  if( 0 == lvPixmap.width() || 0 == lvPixmap.height() )
  {
    lvScaleFactor = 1.0;
  }
  else if( lvPixmap.width() > lvPixmap.height() )
  {
    lvScaleFactor = (( double )cvUi->gvRasterDisplay->width() - 3 ) / ( double )lvPixmap.width();   //Take off a couple of pixels from the display size
  }
  else
  {
    lvScaleFactor = (( double )cvUi->gvRasterDisplay->height() - 3 ) / ( double )lvPixmap.height();   //Take off a couple of pixels from the display size
  }
  cvUi->gvRasterDisplay->scale( lvScaleFactor, lvScaleFactor );

  cvUi->labelRasterName->setText( cvRasterModel->fileName( true ) );
  cvUi->labelProportionPositive->setText( QString::number( cvRasterModel->positiveResponseProbability(), 'f', 4 ) );
  cvUi->labelRasterMinimum->setText( QString::number( cvRasterModel->minimum(), 'f', 2 ) );
  cvUi->labelRasterMaximum->setText( QString::number( cvRasterModel->maximum(), 'f', 2 ) );

  displayLocalityList( cvLocalities.testPresenceLocalities(), Qt::green );
  displayLocalityList( cvLocalities.testAbsenceLocalities(), Qt::red );
  displayLocalityList( cvLocalities.calibrationPresenceLocalities(), Qt::yellow );
  displayLocalityList( cvLocalities.calibrationAbsenceLocalities(), Qt::cyan );

}

