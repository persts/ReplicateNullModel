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
#include "randomdistributiontest.h"

RandomDistributionTest::RandomDistributionTest( RasterModel* theModel ) : ReplicateNullModelTest( theModel )
{
  cvIterations = 0;
  cvSamples = 0;
}

RandomDistributionTest::~RandomDistributionTest()
{
}


/*
 *
 * PUBLIC FUNCTIONS
 *
 */

void RandomDistributionTest::run()
{
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

  //Get a new probability object
  if( 0 != cvProbability ) { delete cvProbability; }
  cvProbability = new Probability( cvSamples );
  if( 0 == cvProbability )
  {
    cvLastError = tr( "Memory allocation problem" );
    emit testComplete( false );
    return;
  }

  cvProbability->testName = "Random Distribution ( n = " + QString::number( cvSamples ) + " )";
  cvProbability->totalInputs = cvIterations;

  int lvDistributions = 0;
  int lvPositiveResponses;
  int lvListSize;
  RasterModel::ModelResponse lvModelResponse;
  initProgress();
  while( lvDistributions < cvIterations )
  {
    if( cvAbort )
    {
      cvLastError = tr( "Test canceled by user" );
      emit testComplete( false );
      return;
    }

    lvPositiveResponses = cvRasterModel->randomModelResponse( cvSamples );
    if( lvPositiveResponses >= 0 )
    {
      cvProbability->frequency[ lvPositiveResponses ]++;
      lvDistributions++;
      progress( lvDistributions, cvIterations );
    }
  }

  cvProbability->isValid = true;
  //Print Frequencies
  emit message( cvProbability->generateReport() );

  return testComplete( true );
}

void RandomDistributionTest::setParameters( int theIterations, int theNumberOfSamples )
{
  cvIterations = theIterations;
  cvSamples = theNumberOfSamples;
}

