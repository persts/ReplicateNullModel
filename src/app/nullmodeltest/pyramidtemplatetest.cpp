/*
** Copyright (C) 2009 Peter J. Ersts (ersts at amnh.org)
** Creation Date: 2009-06-09
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
#include "pyramidtemplatetest.h"
#include "templatetestthread.h"

PyramidTemplateTest::PyramidTemplateTest( RasterModel* theModel, const LocalityCollection &theLocalities, bool showProgress )
  : ReplicateNullModelTest( theModel, theLocalities, showProgress )
{
  cvIterations = 0;
  cvPyramidLevels = 1;
  cvPercentNewResultThreshold = 0.0;
  cvCurrentPyramidLevel = 0;
  cvTemplate = new ReplicateTemplate( cvLocalities.testPresenceLocalities(), cvRasterModel->xPixelResolution() );
}

/*
 *
 * PUBLIC FUNCTIONS
 *
 */

void PyramidTemplateTest::abort()
{
  exit();
  ReplicateNullModelTest::abort();
}

void PyramidTemplateTest::run()
{
  cvDistinctReplicates.clear();

  if( 0 == cvTemplate )
  {
    cvLastError = tr( "The template pointer was null" );
    emit testComplete( false );
    return;
  }

  if( !cvTemplate->isValid() )
  {
    cvLastError = cvTemplate->lastError();
    emit testComplete( false );
    return;
  }

  if( !cvRasterModel->isThreshholdApplied() )
  {
    cvLastError = tr( "No threshold has been applied to the raster model" );
    emit testComplete( false );
    return;
  }

  emit message( "Pyramid method" );
  cvCurrentPyramidLevel = 0;
  runNextLevel();
  exec();
}

void PyramidTemplateTest::setParameters( int theIterations, int theMinimumLevels, double thePercentNewResultThreshold, QString theOutputDirectory )
{
  cvIterations = theIterations;
  cvPyramidLevels = theMinimumLevels;
  cvPercentNewResultThreshold = thePercentNewResultThreshold;
  cvOutputDirectory = theOutputDirectory;
}


/*
 *
 * PROTECTED FUNCTIONS
 *
 */
void PyramidTemplateTest::allThreadsFinished()
{
  if( cvRunLevel == ReplicateNullModelTest::InitialTest )
  {
    int lvLastSize = cvDistinctReplicates.size();
    QMapIterator< QString, ReplicateNullModelTestThread* > lvIterator( cvThreads );
    while( lvIterator.hasNext() )
    {
      lvIterator.next();
      cvDistinctReplicates += lvIterator.value()->distinctReplicates();
    }

    int lvNewReplicates = cvDistinctReplicates.size() - lvLastSize;
    double lvPercentNewResult = (( double )lvNewReplicates / ( double ) cvDistinctReplicates.size() ) * 100.0;

    if(( lvPercentNewResult < cvPercentNewResultThreshold || fabs( lvPercentNewResult - cvPercentNewResultThreshold ) < 0.000005 ) && cvCurrentPyramidLevel >= cvPyramidLevels )
    {
      emit message( QString::number( cvDistinctReplicates.size() - lvLastSize ) + " new replicates found" );
      emit message( "New replicates represent " + QString::number( lvPercentNewResult, 'f', 3 ) + "% of total replicates found" );
      emit message( "=== " + QString::number( cvDistinctReplicates.size() ) + " distinct replicates found in template ===" );
      applyReplicatesToLandscape();
    }
    else
    {
      emit message( QString::number( lvNewReplicates ) + " new replicates found" );
      emit message( "New replicates represent " + QString::number( lvPercentNewResult, 'f', 3 ) + "% of total replicates found" );
      runNextLevel();
    }

  }
  else if( cvRunLevel == ReplicateNullModelTest::GenerateLandscapeMap )
  {
    emit message( QString::number( cvLandscapeMapper->replicatesInLandscape() ) + " replicates in landscape" );
    if( 0 != cvLandscapeMapper )
    {
      initProgress();
      cvRunLevel = ReplicateNullModelTest::CalculateProbability;
      cvLandscapeMapper->calculateProbability();
    }
    else
    {
      cvLastError = tr( "Landscape mapper was null" );
      cvRunLevel = ReplicateNullModelTest::Failed;
    }
  }
  else if( cvRunLevel == ReplicateNullModelTest::CalculateProbability )
  {
    cvRunLevel = ReplicateNullModelTest::Completed;
    emit message( cvLandscapeMapper->probability().generateReport() );
  }


  if( cvRunLevel == ReplicateNullModelTest::Completed )
  {
    exit();
    emit testComplete( true );
  }

  if( cvRunLevel == ReplicateNullModelTest::Failed )
  {
    exit();
    emit testComplete( false );
  }

}

/*
 *
 * PRIVATE FUNCTIONS
 *
 */
void PyramidTemplateTest::applyReplicatesToLandscape()
{
  emit message( tr( "Applying template to landscape" ) );
  cvRunLevel = ReplicateNullModelTest::GenerateLandscapeMap;

  //Free the test threads and any existing landscape mapper
  purgeThreads();
  if( 0 != cvLandscapeMapper ) { delete cvLandscapeMapper; }

  cvLandscapeMapper = new LandscapeMapper( cvRasterModel, cvOutputDirectory, true );
  if( cvLandscapeMapper->initRelabeler( cvTemplate->templateSize(), cvDistinctReplicates ) )
  {
    connect( cvLandscapeMapper, SIGNAL( progressUpdate( int, int ) ), this, SLOT( progress( int, int ) ) );
    connect( cvLandscapeMapper, SIGNAL( finished() ), this, SLOT( allThreadsFinished() ) );
    initProgress();
    cvLandscapeMapper->generateLandscapeMap();
  }
  else
  {
    cvLastError = cvLandscapeMapper->lastError();
    cvRunLevel = ReplicateNullModelTest::Failed;
  }
}

bool PyramidTemplateTest::isPreviousPyramidPoint( double theNumber, double theCellSize, int thePyramidLevel )
{
  if( thePyramidLevel == 1 )
  {
    return false;
  }
  else if( 0.0 == fmod( theNumber , theCellSize / pow( 2.0, double( thePyramidLevel - 1 ) ) ) )
  {
    return true;
  }

  return isPreviousPyramidPoint( theNumber, theCellSize, thePyramidLevel - 1 );
}

void PyramidTemplateTest::runNextLevel()
{
  cvCurrentPyramidLevel++;

  //Just purge the thread list each run to make sure memory is released.
  purgeThreads();

  //Create a list of point list, one for each possible thread
  QList< QList< QPointF > > lvTestPoints;
  for( int lvIterator = 0; lvIterator < cvIdealThreadCount; lvIterator++ )
  {
    lvTestPoints.append( QList< QPointF >() );
  }

  QMap<ReplicateTemplate::Boundary, double> lvCellBoundaries = cvTemplate->templateCellBoundaries( cvTemplate->templateCentroid() );

  int lvCurrentList = 0;
  double lvStepSize = 0.0;
  double lvCellSize = cvTemplate->cellSize();
  double lvSouth = lvCellBoundaries[ ReplicateTemplate::South ];
  double lvEast = lvCellBoundaries[ ReplicateTemplate::East ];

  emit message( QString( "Generating test points -  pyramid level %1 ( %2 )" ) .arg( cvCurrentPyramidLevel ) .arg( pow( 2.0, double ( cvCurrentPyramidLevel ) ) ) );
  lvStepSize = lvCellSize / ( double ) pow( 2.0, double( cvCurrentPyramidLevel ) );

  for( double yRunner =  lvSouth + lvStepSize; yRunner < lvCellBoundaries[ ReplicateTemplate::North ]; yRunner += lvStepSize )
  {
    for( double xRunner = lvCellBoundaries[ ReplicateTemplate::West ] + lvStepSize; xRunner < lvEast; xRunner += lvStepSize )
    {
      //TODO: Test To make sure we are actually getting a speed improvement, this is a kind of heavy call
      if( isPreviousPyramidPoint( xRunner, lvCellSize, cvCurrentPyramidLevel ) &&  isPreviousPyramidPoint( yRunner, lvCellSize, cvCurrentPyramidLevel ) )
      {
        //do nothing
      }
      else
      {
        lvTestPoints[ lvCurrentList ].append( QPointF( xRunner, yRunner ) );
        if( lvCurrentList == cvIdealThreadCount - 1 )
        {
          lvCurrentList = 0;
        }
        else
        {
          lvCurrentList++;
        }
      }
    }
  }

  //create and launch threads
  initProgress();
  for( int lvIterator = 0; lvIterator < cvIdealThreadCount; lvIterator++ )
  {
    QString lvId = QString::number( lvIterator );
    TemplateTestThread* lvThread = new TemplateTestThread( lvId, true );
    lvThread->setParameters( lvTestPoints[ lvIterator ], cvTemplate );
    addThread( lvThread );
  }

}
