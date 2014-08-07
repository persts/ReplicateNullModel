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
#include "randomtranslationrotationthread.h"

#include "mtrand.h"

#include <QMatrix>
#include <QPointF>
#include <QStringList>

#include <QDebug>

RandomTranslationRotationThread::RandomTranslationRotationThread( QString theId, bool showProgress ) : ReplicateNullModelTestThread( theId, showProgress )
{
  cvIterations = 0;
  cvCountDistinctReplicates = false;
  cvLandscapeMapper = 0;
  cvMutex = 0;
  cvStartTime = 0;
}

RandomTranslationRotationThread::~RandomTranslationRotationThread()
{
}

void RandomTranslationRotationThread::run()
{
  MTRand lvRNG( time( 0 ) );
  QPointF lvCentroid = cvPresence.centroid();
  QMatrix lvTranslateToZero;
  lvTranslateToZero.translate( 0 - lvCentroid.x(), 0 - lvCentroid.y() );

  if( 0 != cvProbability ) { delete cvProbability; }
  cvProbability = new Probability( cvPresence.size() );
  cvProbability->testName = "Random Translation Rotation";
  RasterModel::ModelResponse lvModelResponse;
  int lvListSize = cvPresence.size();
  int lvReplicatesFound = 0;
  int lvPositiveResponses = 0;
  int lvReplicatesDiscarded = 0;
  bool lvKeepRunning = true;
  QStringList lvReplicateList;
  while( lvKeepRunning )
  {
    QString lvReplicateString = "";
    QMatrix lvRotation;
    lvRotation.rotate( lvRNG.randExc() * 360.0 );
    QPointF lvNewPoint = cvRasterModel->pixelToRealWorld( lvRNG.randExc() * ( double ) cvRasterModel->width(), lvRNG.randExc() * ( double ) cvRasterModel->height() );
    QMatrix lvNewTranslation;
    lvNewTranslation.translate( lvNewPoint.x(), lvNewPoint.y() );
    lvPositiveResponses = 0;
    for( int lvIterator = 0; lvIterator < lvListSize; lvIterator++ )
    {
      if( cvAbort )
      {
        return;
      }
      lvNewPoint = cvPresence.locality( lvIterator ).realWorldCoordinates;
      lvNewPoint = lvTranslateToZero.map( lvNewPoint );
      lvNewPoint = lvRotation.map( lvNewPoint );
      lvNewPoint = lvNewTranslation.map( lvNewPoint );
      lvModelResponse = cvRasterModel->modelResponseAtRealWorldCoordinate( lvNewPoint );

      lvNewPoint = cvRasterModel->realWorldToPixel( lvNewPoint );
      lvReplicateString += "P" + QString::number( cvPresence.locality( lvIterator ).id ) + "-" + QString::number(( int ) lvNewPoint.x() ) + "_" + QString::number(( int ) lvNewPoint.y() ) + "|";

      if( RasterModel::NoData == lvModelResponse || RasterModel::OutOfBounds == lvModelResponse )
      {
        lvPositiveResponses = -1;
        lvReplicatesDiscarded++;
        break;
      }
      else if( RasterModel::Positive == lvModelResponse )
      {
        lvPositiveResponses++;
      }
    }

    if( lvPositiveResponses >= 0 )
    {
      cvProbability->frequency[ lvPositiveResponses ]++;
      lvReplicatesFound++;

      lvReplicateString.chop( 1 );
      lvReplicateList << lvReplicateString;

      bool lvHaveLock = false;
      //must get lock for straight iteration run
      if( !cvCountDistinctReplicates && 0 == cvStartTime && lvReplicatesFound == cvIterations )
      {
        lvHaveLock = true;
        cvMutex->lock();
      }
      else if( lvReplicateList.size() > 40 )
      {
        lvHaveLock = cvMutex->tryLock();
      }
      //else if( lvReplicateList.size() > 50 )
      //{
      //  lvHaveLock = true;
      //  cvMutex->lock();
      //}

      if( lvHaveLock )
      {
        while( lvReplicateList.size() > 0 )
        {
          if( cvCountDistinctReplicates && cvLandscapeMapper->replicatesInLandscape() == cvIterations )
          {
            break;
          }
          else if( 0 != cvStartTime && cvStartTime->secsTo( QDateTime::currentDateTime() ) >= cvIterations )
          {
            break;
          }

          cvLandscapeMapper->addReplicate( lvReplicateList.takeLast(), false );
        }

        //Show progress
        if( cvShowProgress )
        {
          if( 0 != cvStartTime )
          {
            progress( cvStartTime->secsTo( QDateTime::currentDateTime() ), cvIterations );
          }
          else if( cvCountDistinctReplicates )
          {
            progress( cvLandscapeMapper->replicatesInLandscape(), cvIterations );
          }
          else
          {
            progress( lvReplicatesFound, cvIterations );
          }
        }

        //Stop criteria
        if( 0 != cvStartTime )
        {
          lvKeepRunning = cvStartTime->secsTo( QDateTime::currentDateTime() ) < cvIterations ? true : false;
        }
        else if( cvCountDistinctReplicates )
        {
          lvKeepRunning = cvLandscapeMapper->replicatesInLandscape() < cvIterations ? true : false;
        }
        else
        {
          lvKeepRunning = lvReplicatesFound < cvIterations ? true : false;
        }

        cvMutex->unlock();
      }
    }
  }

  cvProbability->totalInputs = lvReplicatesFound;
  cvProbability->isValid = true;
  cvProbability->discardedInputs = lvReplicatesDiscarded;
  emit finished( cvId );
}

void RandomTranslationRotationThread::setParamters( RasterModel* theModel, const LocalityList &theLocalities, int theIterations, bool countDistinctReplicates, LandscapeMapper* theLandscapeMapper, QMutex* theMutex, QDateTime* startTime )
{
  cvRasterModel = theModel;
  cvPresence = theLocalities;
  cvIterations = theIterations;
  cvCountDistinctReplicates = countDistinctReplicates;
  cvLandscapeMapper = theLandscapeMapper;
  cvMutex = theMutex;
  cvStartTime = startTime;
}

