#include "randomdistributiontest.h"

RandomDistributionTest::RandomDistributionTest( RasterModel* theModel ) : ReplicateNullModelTest( theModel )
{
  cvIterations = 0;
  cvSamples = 0;
}

RandomDistributionTest::~RandomDistributionTest()
{
}


/* PUBLIC FUNCTIONS */

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

