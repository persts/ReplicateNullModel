/*
** Copyright (C) 2009 Peter J. Ersts (ersts at amnh.org)
** Creation Date: 2009-06-05
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
#include "randomtranslationrotationtest.h"
#include "randomtranslationrotationthread.h"

#include <QDebug>

RandomTranslationRotationTest::RandomTranslationRotationTest( RasterModel* theModel, const LocalityCollection &theLocalities, bool showProgress ) : ReplicateNullModelTest( theModel, theLocalities, showProgress )
{
  cvIterations = 0;
  cvCountDistinctReplicates = false;
  cvUseTimeLimit = false;
  cvLandscapeMapper = 0;
}

RandomTranslationRotationTest::~RandomTranslationRotationTest()
{
  if( 0 != cvLandscapeMapper )
  {
    delete cvLandscapeMapper;
  }
}


/*
 *
 * PUBLIC FUNCTIONS
 *
 */
void RandomTranslationRotationTest::run()
{
  //Do all error checking before creating threads.
  if( !cvRasterModel->isValid() )
  {
    cvLastError = tr( "The raster model is not valid" );
    emit testComplete( false );
    return;
  }

  if( !cvRasterModel->isThreshholdApplied() )
  {
    cvLastError = tr( "A threshhold has not been applied to the raster model" );
    emit testComplete( false );
    return;
  }

  if( fabs( cvRasterModel->xPixelResolution() ) != fabs( cvRasterModel->yPixelResolution() ) )
  {
    cvLastError = tr( "This test will only run with raster models that have the same x and y pixel resolution" );
    emit testComplete( false );
    return;
  }

  if( !arePresenceLocalitiesValid() )
  {
    cvLastError = tr( "Presence locality list is empty or not valid" );
    emit testComplete( false );
    return;
  }

  if( 0 == cvLandscapeMapper )
  {
    cvLastError = tr( "The landscape mapper is null" );
    emit testComplete( false );
    return;
  }

  initProgress();
  cvThreadsCreated = 0;
  for( int lvThreadMaker = 0; lvThreadMaker < cvIdealThreadCount; lvThreadMaker++ )
  {
    if( cvAbort ) { return; }

    cvThreadsCreated++;
    RandomTranslationRotationThread* lvThread = new RandomTranslationRotationThread( this->newThreadId( lvThreadMaker ), cvShowProgress );
    if( cvCountDistinctReplicates )
    {
      lvThread->setParamters( cvRasterModel, cvLocalities.testPresenceLocalities(), cvIterations, cvCountDistinctReplicates, cvLandscapeMapper, &cvMutex );
    }
    else if( cvUseTimeLimit )
    {
      lvThread->setParamters( cvRasterModel, cvLocalities.testPresenceLocalities(), cvIterations, cvCountDistinctReplicates, cvLandscapeMapper, &cvMutex, &cvStartTime );
    }
    else
    {
      lvThread->setParamters( cvRasterModel, cvLocalities.testPresenceLocalities(), cvIterations / cvIdealThreadCount, cvCountDistinctReplicates, cvLandscapeMapper, &cvMutex );
    }
    this->addThread( lvThread );
  }

}

void RandomTranslationRotationTest::setParameters( int theIterations, bool countDistinctReplicates, bool useTimeLimit, QString theOutputDirectory )
{
  cvIterations = theIterations;
  cvCountDistinctReplicates = countDistinctReplicates;
  cvUseTimeLimit = useTimeLimit;
  cvUnifiedThreadGoal = cvCountDistinctReplicates || cvUseTimeLimit ? true : false;
  cvDisplayEstimatedTimeToCompletion = !cvCountDistinctReplicates;
  if( 0 != cvLandscapeMapper )
  {
    delete cvLandscapeMapper;
  }
  cvLandscapeMapper = new LandscapeMapper( cvRasterModel, theOutputDirectory, true );
}

/*
 *
 * PROTECTED FUNCTIONS
 *
 */
void RandomTranslationRotationTest::allThreadsFinished()
{
  if( 0 == cvProbability ) { delete cvProbability; }
  cvProbability = new Probability( cvLocalities.testPresenceLocalities().size() );
  cvProbability->testName = "Random Translation Rotation ( all replicates )";
  cvProbability->isValid = true;

  QMapIterator< QString, ReplicateNullModelTestThread* > lvIterator( cvThreads );
  while( lvIterator.hasNext() )
  {
    lvIterator.next();
    Probability lvProbability = lvIterator.value()->probability();
    if( !lvProbability.isValid )
    {
      cvProbability->errorMessage = lvProbability.errorMessage;
      cvProbability->isValid = false;
      break;
    }

    cvProbability->totalInputs += lvProbability.totalInputs;
    cvProbability->discardedInputs += lvProbability.discardedInputs;
    QMapIterator< int, int > lvIterator( lvProbability.frequency );
    while( lvIterator.hasNext() )
    {
      lvIterator.next();
      cvProbability->frequency[ lvIterator.key()] +=  lvIterator.value();
    }
  }

  //Print Frequencies
  message( cvProbability->generateReport() );

  //TODO: Make this part of the thread, show progress and allow for abort
  cvLandscapeMapper->calculateProbabilityBlocking();
  message( cvLandscapeMapper->probability().generateReport() );

  emit testComplete( true );
}

