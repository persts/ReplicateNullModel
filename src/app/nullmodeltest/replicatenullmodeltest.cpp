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
#include "replicatenullmodeltest.h"

#include <QMapIterator>

#include <QDebug>

ReplicateNullModelTest::ReplicateNullModelTest( RasterModel* theModel, bool showProgress )
{
  cvRasterModel = theModel;
  cvShowProgress = showProgress;

  cvAbort = false;
  cvIdealThreadCount = QThread::idealThreadCount();
  cvLandscapeMapper = 0;
  cvLastError = tr( "Uninitialized" );
  cvLastProgressValue = -1;
  cvRunLevel = ReplicateNullModelTest::InitialTest;
  cvProbability = 0;
  cvThreadsCompleted = 0;
  cvThreadsCreated = 0;
  cvUnifiedThreadGoal = false;
  cvDisplayEstimatedTimeToCompletion = true;

  if( cvIdealThreadCount == -1 )
  {
    cvIdealThreadCount = 1;
  }
}

ReplicateNullModelTest::ReplicateNullModelTest( RasterModel* theModel, const LocalityCollection &theLocalities, bool showProgress )
{
  cvRasterModel = theModel;
  cvLocalities = theLocalities;
  cvShowProgress = showProgress;

  cvAbort = false;
  cvIdealThreadCount = QThread::idealThreadCount();
  cvLandscapeMapper = 0;
  cvLastError = tr( "Uninitialized" );
  cvLastProgressValue = -1;
  cvRunLevel = ReplicateNullModelTest::InitialTest;
  cvProbability = 0;
  cvThreadsCompleted = 0;
  cvThreadsCreated = 0;
  cvUnifiedThreadGoal = false;
  cvDisplayEstimatedTimeToCompletion = true;

  if( cvIdealThreadCount == -1 )
  {
    cvIdealThreadCount = 1;
  }
}

ReplicateNullModelTest::~ReplicateNullModelTest()
{
  if( 0 != cvProbability )
  {
    delete cvProbability;
    cvProbability = 0;
  }

  if( 0 != cvLandscapeMapper )
  {
    delete cvLandscapeMapper;
    cvLandscapeMapper = 0;
  }

  QMapIterator< QString, ReplicateNullModelTestThread* > lvIterator( cvThreads );
  while( lvIterator.hasNext() )
  {
    lvIterator.next();
    if( 0 != lvIterator.value() )
    {
      lvIterator.value()->abort();
      while( lvIterator.value()->isRunning() ) { this->wait( 100 ); }
      delete lvIterator.value();
    }
  }
  cvThreads.clear();
}

/*
 *
 * PUBLIC FUNCTIONS
 *
 */
void ReplicateNullModelTest::abort()
{
  cvAbort = true;
  cvLastError = tr( "Test canceled by user" );

  purgeThreads();

  emit testComplete( false );
}

Probability ReplicateNullModelTest::probability()
{
  if( 0 == cvProbability )
  {
    return Probability( 0 );
  }

  return *cvProbability;
}

/*
 *
 * PROTECTED FUNCTIONS
 *
 */
void ReplicateNullModelTest::addThread( ReplicateNullModelTestThread* theThread )
{
  if( 0 != theThread )
  {
    //If abort has been called and we are here, it is this functions responsibility to clean up the memory
    if( cvAbort )
    {
      delete theThread;
      return;
    }
    cvThreads.insert( theThread->id(), theThread );
    connect( theThread, SIGNAL( finished( QString ) ), this, SLOT( threadFinished( QString ) ) );
    connect( theThread, SIGNAL( progressUpdate( QString, int ) ), this, SLOT( threadProgress( QString, int ) ) );
    theThread->start();
  }
}


bool ReplicateNullModelTest::arePresenceLocalitiesValid()
{
  LocalityList lvPresenceList = cvLocalities.testPresenceLocalities();
  if( 0 == cvRasterModel || !cvRasterModel->isValid() ) { return false; }
  if( 0 == lvPresenceList.size() ) { return false; }

  RasterModel::ModelResponse lvModelResponse;
  Locality lvLocality;
  for( int lvListRunner = 0; lvListRunner < lvPresenceList.size(); lvListRunner++ )
  {
    lvLocality = lvPresenceList.locality( lvListRunner );
    lvModelResponse = cvRasterModel->modelResponseAtRealWorldCoordinate( lvLocality.realWorldCoordinates.x(), lvLocality.realWorldCoordinates.y() );
    if( RasterModel::NoData == lvModelResponse || RasterModel::OutOfBounds == lvModelResponse )
    {
      return false;
    }
  }

  return true;
}

void ReplicateNullModelTest::initProgress()
{
  cvStartTime = QDateTime::currentDateTime();
  cvThreadsCompleted = 0;
  cvThreadProgress = 0;
}

void ReplicateNullModelTest::progress( int theCurrentProgress, int theTarget )
{
  if( !cvShowProgress ) { return; }

  int lvProgress = ( int )((( double ) theCurrentProgress / ( double ) theTarget ) * 100.0 );
  if( lvProgress != cvLastProgressValue )
  {
    cvLastProgressValue = lvProgress;
    emit progressUpdate( lvProgress );

    if( cvDisplayEstimatedTimeToCompletion )
    {
      if( cvLastProgressValue == 10 )
      {
        QDateTime lvNow = QDateTime::currentDateTime();
        QDateTime lvEndTime = cvStartTime.addSecs( cvStartTime.secsTo( lvNow )  * 10 );
        message( tr( "Estimated completion" ) + ": " +  lvEndTime.toString( "yyyy-MM-dd hh:mm:ss" ) );
      }
    }
  }
}

void ReplicateNullModelTest::purgeThreads()
{
  QMapIterator< QString, ReplicateNullModelTestThread* > lvIterator( cvThreads );
  while( lvIterator.hasNext() )
  {
    lvIterator.next();
    if( 0 != lvIterator.value() )
    {
      lvIterator.value()->abort();
      while( lvIterator.value()->isRunning() ) { this->wait( 100 ); }
      delete lvIterator.value();
    }
  }
  cvThreads.clear();
}

void ReplicateNullModelTest::threadFinished( QString theThreadId )
{
  cvThreadsCompleted++;
  if( cvThreadsCompleted == cvIdealThreadCount )
  {
    //reset the completed thread count, and call all threads finished method
    cvThreadsCompleted = 0;
    allThreadsFinished();
  }
}

void ReplicateNullModelTest::threadProgress( QString theId, int theProgress )
{
  if( !cvShowProgress ) { return; }


  if( cvUnifiedThreadGoal )
  {
    progress( theProgress, 100 );
  }
  else
  {
    cvThreadProgress++;
    if( cvThreadProgress % cvIdealThreadCount == 0 )
    {
      progress( cvThreadProgress, cvIdealThreadCount * 100 );
    }
  }
}
