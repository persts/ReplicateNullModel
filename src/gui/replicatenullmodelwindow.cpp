/*
** Copyright (C) 2009 Peter J. Ersts (ersts at amnh.org)
** Creation Date: 2009-06-03
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
#include "aboutdialog.h"
#include "binomialtest.h"
#include "calibrationtest.h"
#include "localitycollection.h"
#include "pyramidtemplatetest.h"
#include "replicatenullmodelwindow.h"
#include "randomdistributiontest.h"
#include "randomtranslationrotationtest.h"

#include "ui_replicatenullmodelwindow.h"

#include <QRectF>
#include <QThread>
#include <QPointF>
#include <QDateTime>

ReplicateNullModelWindow::ReplicateNullModelWindow( QMainWindow* theParent, Qt::WindowFlags theFlags ) : QMainWindow( theParent, theFlags )
{
  this->setAttribute( Qt::WA_DeleteOnClose );
  cvUi = new Ui::ReplicateNullModelWindow();
  cvUi->setupUi( this );

  cvUi->rasterViewer->setMinimumMaximumVisible( false );
  cvUi->rasterViewer->adjustSize();

  cvNullModelTest = 0;

  cvConsoleDock = new QDockWidget( "Console", this );
  if( 0 == cvConsoleDock ) { return; }
  cvConsoleDock->setMinimumWidth( 500 );
  cvConsoleDock->setWidget( &cvConsole );
  cvConsoleDock->setFeatures( QDockWidget::DockWidgetMovable);
  //TODO: Missing something simple here cannot dock in all areas specified, either left and right - top or bottom, but not a mix
  cvConsoleDock->setAllowedAreas( Qt::BottomDockWidgetArea | Qt::RightDockWidgetArea | Qt::LeftDockWidgetArea );
  addDockWidget( Qt::RightDockWidgetArea, cvConsoleDock );

  cvUi->pBar->setMinimum( 0 );
  cvUi->pBar->setMaximum( 100 );
  cvUi->pBar->setValue( 0 );

  connect( cvUi->tableLocalities, SIGNAL( fileSelected( QString ) ), this, SLOT( localityFileSelected( QString ) ) );
  connect( cvUi->rasterViewer, SIGNAL( rasterFileSelected( QString ) ), this, SLOT( rasterFileSelected( QString ) ) );
}

ReplicateNullModelWindow::~ReplicateNullModelWindow()
{
  if( 0 != cvConsoleDock )
  {
    removeDockWidget( cvConsoleDock );
    //TODO: figure out why this causes segfault
    //delete cvConsoleDock;
  }

  if( 0 != cvNullModelTest )
  {
    delete cvNullModelTest;
  }
}
/*
 *
 * PUBLIC FUNCTIONS
 *
 */


/*
 *
 * PRIVATE FUNCTIONS
 *
 */

void ReplicateNullModelWindow::consoleMessage( QString theMessage )
{
  QDateTime lvDateTime = QDateTime::currentDateTime();
  cvConsole.append( "[ " + lvDateTime.toString( "yyyy-MM-dd hh:mm:ss " ) + " ]: " + theMessage );
}

void ReplicateNullModelWindow::enableGuiControls( bool theState )
{
  cvUi->pbtnConfigure->setEnabled( theState );
  cvUi->pbtnRun->setEnabled( theState );
  cvUi->rasterViewer->enableControls( theState );
}

void ReplicateNullModelWindow::localityFileSelected( QString theFilename )
{
  if( cvLocalitySet.load( theFilename ) )
  {
    cvUi->tableLocalities->loadLocalities( cvLocalitySet.collection( 0 ) );
    cvUi->rasterViewer->setLocalityCollection( cvLocalitySet.collection( 0 ) );
  }
  else
  {
    consoleMessage( cvLocalitySet.lastError() );
  }
}

void ReplicateNullModelWindow::on_pbtnAbout_clicked()
{
  new AboutDialog();
}

void ReplicateNullModelWindow::on_pbtnCancel_clicked()
{
  if( 0 != cvNullModelTest )
  {
    cvNullModelTest->abort();
  }
}

void ReplicateNullModelWindow::on_pbtnConfigure_clicked()
{
  cvConfigurationDialog.show();
}

void ReplicateNullModelWindow::on_pbtnRun_clicked()
{
  if( 0 != cvNullModelTest )
  {
    delete cvNullModelTest;
  }

  consoleMessage( tr( "Starting test" ) );
  enableGuiControls( false );
  if( cvUi->rbtnCalibrationReport->isChecked() )
  {
    CalibrationTest* lvTheTest = new CalibrationTest( &cvRasterModel, cvLocalitySet.collection( 0 ) );
    cvNullModelTest = lvTheTest;
  }
  else if( cvUi->rbtnBinomialTest->isChecked() )
  {
    BinomialTest* lvTheTest = new BinomialTest( &cvRasterModel, cvLocalitySet.collection( 0 ) );
    cvNullModelTest = lvTheTest;
  }
  else if( cvUi->rbtnRandomDistribution->isChecked() )
  {
    RandomDistributionTest* lvTheTest = new RandomDistributionTest( &cvRasterModel );
    lvTheTest->setParameters( cvConfigurationDialog.randomDistributionIterations(), cvConfigurationDialog.randomDistributionSamples() );
    cvNullModelTest = lvTheTest;
  }
  else if( cvUi->rbtnRandomTranslation->isChecked() )
  {
    RandomTranslationRotationTest* lvTheTest = new RandomTranslationRotationTest( &cvRasterModel, cvLocalitySet.collection( 0 ), true );
    lvTheTest->setParameters( cvConfigurationDialog.randomTranslationIterations(), cvConfigurationDialog.distinctReplicates(), cvConfigurationDialog.timeLimit(), cvConfigurationDialog.outputDirectory() );
    cvNullModelTest = lvTheTest;
  }
  else if( cvUi->rbtnTemplate->isChecked() && cvConfigurationDialog.pyramid() )
  {
    PyramidTemplateTest* lvTheTest = new PyramidTemplateTest( &cvRasterModel, cvLocalitySet.collection( 0 ), true );
    lvTheTest->setParameters( 0, cvConfigurationDialog.templatePyramidLevels(), cvConfigurationDialog.newResultMinimumThreshold(), cvConfigurationDialog.outputDirectory() );
    cvNullModelTest = lvTheTest;
  }
  else
  {
    cvNullModelTest = new ReplicateNullModelTest( &cvRasterModel );
  }

  connect( cvNullModelTest, SIGNAL( testComplete( bool ) ), this, SLOT( testComplete( bool ) ) );
  connect( cvNullModelTest, SIGNAL( message( QString ) ), this, SLOT( consoleMessage( QString ) ) );
  connect( cvNullModelTest, SIGNAL( progressUpdate( int ) ), cvUi->pBar, SLOT( setValue( int ) ) );
  cvNullModelTest->start();
}

void ReplicateNullModelWindow::rasterFileSelected( QString theFilename )
{
  if( cvRasterModel.load( theFilename ) )
  {
    cvUi->rasterViewer->setRasterModel( &cvRasterModel );
  }
  else
  {
    consoleMessage( cvRasterModel.lastError() );
  }
}

void ReplicateNullModelWindow::testComplete( bool theState )
{
  enableGuiControls( true );
  cvUi->pBar->setValue( 0 );
  if( !theState )
  {
    consoleMessage( cvNullModelTest->lastError() );
  }

  //free the memeory
  while( cvNullModelTest->isRunning() ) { cvNullModelTest->wait( 500 ); }
  delete cvNullModelTest;
  cvNullModelTest = 0;
}
