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
#include "binomialtest.h"
#include "statisticsengine.h"

BinomialTest::BinomialTest( RasterModel* theModel ) : ReplicateNullModelTest( theModel )
{
}

BinomialTest::BinomialTest( RasterModel* theModel, const LocalityCollection &theLocalities ) : ReplicateNullModelTest( theModel, theLocalities )
{
}

void BinomialTest::run()
{

  cvResult.validResult = false;

  if( !cvRasterModel->isValid() )
  {
    cvLastError = tr( "The raster model is not valid" );
    emit testComplete( false );
    return;
  }

  LocalityList lvLocalities = cvLocalities.testPresenceLocalities();
  if( 0 == lvLocalities.size() )
  {
    cvLastError = tr( "Presence locality list is empty." );
    emit testComplete( false );
    return;
  }

  Locality lvLocality;
  int lvPositive = 0;
  int lvNegative = 0;
  RasterModel::ModelResponse lvModelResponse;
  for( int lvIterator = 0; lvIterator < lvLocalities.size(); lvIterator++ )
  {
    lvLocality = lvLocalities.locality( lvIterator );
    if( !lvLocality.isValid )
    {
      cvLastError = tr( "A locality in the presence list was invalid " );
      emit testComplete( false );
      return;
    }

    lvModelResponse = cvRasterModel->modelResponseAtRealWorldCoordinate( lvLocality.realWorldCoordinates );
    if( RasterModel::NoData == lvModelResponse )
    {
      cvLastError  = tr( "The model return a [NoData] value for a presence locality, which is not allowed in a binomial test " );
      emit testComplete( false );
      return;
    }
    else if( RasterModel::OutOfBounds == lvModelResponse )
    {
      cvLastError  = tr( "The model return an [OutOfBounds] value for a presence locality, which is not allowed in a binomial test " );
      emit testComplete( false );
      return;
    }
    else if( RasterModel::RawData == lvModelResponse )
    {
      cvLastError  = tr( "The model return a [RawData] value for a presence locality, which is not allowed in a binomial test. Try setting the threshold and reloading the model." );
      emit testComplete( false );
      return;
    }
    else if( RasterModel::Positive == lvModelResponse )
    {
      lvPositive++;
    }
    else if( RasterModel::Negative == lvModelResponse )
    {
      lvNegative++;
    }

  }

  cvResult.validResult = true;
  cvResult.k = lvPositive;
  cvResult.n = lvPositive + lvNegative;
  cvResult.positiveLocalities = lvPositive;
  cvResult.negativeLocalities = lvNegative;
  cvResult.positiveResponseProbability = cvRasterModel->positiveResponseProbability();
  cvResult.p = StatisticsEngine::binomialProbability( cvResult.k, cvResult.n, cvResult.positiveResponseProbability );


  emit message( "Binomial test results\n" + cvResult.generateReport() );
  emit testComplete( true );
}
