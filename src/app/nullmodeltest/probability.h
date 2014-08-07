/*
** Coppyright (C) 2009 Peter J. Ersts (ersts at amnh.org)
** Creation Date: 2009-03-24
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
#ifndef PROBABILITY_H
#define PROBABILITY_H

#include <QMap>
#include <QObject>

class Probability
{

  public:
    Probability( int theInitialBins )
    {
      errorMessage = QObject::tr( "uninitialized object" );
      isValid = false;
      discardedInputs = 0;
      totalInputs = 0;
      testName = "";
      for( int lvListRunner = 0; lvListRunner <= theInitialBins;  lvListRunner++ )
      {
        frequency[ lvListRunner ] = 0;
      }
    }

    /** \brief Convience function to general a basic report */
    QString generateReport() const
    {
      QString myReport = QObject::tr( "Probability Report" ) + "\n";
      myReport += "===============================\n";
      myReport += testName + "\n";
      if( isValid )
      {
        myReport += QString::number( discardedInputs ) + " inputs discarded during test\n";
        myReport += QString::number( totalInputs ) + " total iterations/inputs\n";
        int lvTotalFrequency = 0;
        for( int lvListRunner = frequency.size() - 1; lvListRunner >= 0;  lvListRunner-- )
        {
          lvTotalFrequency += frequency[ lvListRunner ];
          myReport += QString::number( lvListRunner );
          if( lvListRunner != frequency.size() - 1 )
          {
            myReport += "+ ";
          }
          myReport += "\tp = " + QString::number(( double ) lvTotalFrequency / ( double )totalInputs, 'f', 15 ) + "\n";
        }
      }
      else
      {
        myReport += QObject::tr( "The probability result object is not valid" ) + "\n";
        if( !errorMessage.isEmpty() )
        {
          myReport += QObject::tr( "Error message: " ) + errorMessage + "\n";
        }
      }
      myReport += "===============================";

      return myReport;
    }


    QString errorMessage;

    bool isValid;

    QString testName;

    int discardedInputs;

    int totalInputs;

    QMap< int, int > frequency;

};
#endif // PROBABILITY_H
