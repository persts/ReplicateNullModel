/*
** Copyright (C) 2008 Peter J. Ersts (ersts at amnh.org)
** Creation Date: 2008-02-11
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
**/
#ifndef BINOMIALTESTRESULT_H
#define BINOMIALTESTRESULT_H

#include <QObject>
#include <QString>
#include <QMetaType>

/*
 * This class is really nothing much more that an convience container for holding result
 * data from a binomial test.
 */
class BinomialTestResult
{
  public:
    /** \brief Constructor */
    BinomialTestResult()
    {
      validResult = false;
      errorMessage = "";
      rasterFilename = "";
      localitiesFilename = "";
      positiveLocalities = 0;
      negativeLocalities = 0;
      invalidLocalities = 0;
      k = 0;
      n = 0;
      positiveResponseProbability = 0.0;
      p = 0.0;

      qRegisterMetaType<BinomialTestResult>( "BinomialTestResult" );
    }

    /** \brief Convience function to general a basic report */
    QString generateReport()
    {
      QString myReport = "";
      myReport += "===============================\n";
      myReport += QObject::tr( "Binomial Test Results\n" );
      if( validResult )
      {
        myReport += QObject::tr( "Positives: " ) + QString::number( positiveLocalities ) + QString( "\t(%1 %)\t" ) .arg( QString::number((( double )positiveLocalities / (( double )positiveLocalities + ( double )negativeLocalities ) ) * 100, 'f', 2 ) ) + QObject::tr( "Negatives: " ) + QString::number( negativeLocalities ) + "\n";
        myReport += QString( "k: %1\tn: %2\tPositiveResponseProbability: %3\n" ) .arg( k ) .arg( n ) .arg( positiveResponseProbability );
        myReport += QString( "p ( %1 or better) = %2\n" ) .arg( k ) .arg( QString::number( p, 'f', 15 ) );
      }
      else
      {
        myReport += QObject::tr( "Test did not produce valid results." );
        myReport += QObject::tr( "Error message: " ) + errorMessage + "\n";
      }
      myReport += "===============================";

      return myReport;
    }

    /** \brief Flag to indicate if the test produced a valid result */
    bool validResult;
    /** \brief The error message for debugging */
    QString errorMessage;
    /** \brief the file name for the raster layer that was loaded at the time of running the test*/
    QString rasterFilename;
    /** \brief The file name of the localities file that was loaded at the time of running the test */
    QString localitiesFilename;
    /** \brief Number of test localities that were conatined within the positive portion of the predicted distirbution */
    int positiveLocalities;
    /** \brief Number of test localities that were contained within the negative portion of the predicted distirbution */
    int negativeLocalities;
    /** \brief Number of test localities that were contained that feel in non-data areas*/
    int invalidLocalities;
    /** \brief Binomial parameter, number of positive tests, k = positiveLocalities */
    int k;
    /** \brief Binomial paramter, number of samples, n = positiveLocalities + negativeLocalities */
    int n;
    /** \brief Binomial parameter, proportion of predicted distribution that is positive */
    double positiveResponseProbability;
    /** \brief Bionomial result, the p value */
    double p;
};
#endif
