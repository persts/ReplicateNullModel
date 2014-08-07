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
*/
#include "calibrationtest.h"

#include <QList>

CalibrationTest::CalibrationTest( RasterModel* theModel ) : ReplicateNullModelTest( theModel )
{
}

CalibrationTest::CalibrationTest( RasterModel* theModel, const LocalityCollection &theLocalities ) : ReplicateNullModelTest( theModel, theLocalities )
{
}

void CalibrationTest::run()
{
  QString lvCalibrationReport = "";
  LocalityList lvCalibrationPoints = cvLocalities.calibrationPresenceLocalities();

  if( !cvRasterModel->isValid() )
  {
    cvLastError = tr( "The raster model is not valid" );
    emit testComplete( false );
    return;
  }

  if( 0 == lvCalibrationPoints.size() )
  {
    cvLastError = tr( "Calibration list is empty." );
    emit testComplete( false );
    return;
  }

  Locality lvLocality;
  double lvRawModelData;
  QList< double > lvCalibrationValues;
  for( int lvLocalityRunner = 0; lvLocalityRunner < lvCalibrationPoints.size(); lvLocalityRunner++ )
  {
    lvLocality = lvCalibrationPoints.locality( lvLocalityRunner );
    lvRawModelData = cvRasterModel->rawDataAtRealWorldCoordinate( lvLocality.realWorldCoordinates );
    if( !lvCalibrationValues.contains( lvRawModelData ) )
    {
      lvCalibrationValues.append( lvRawModelData );
    }
  }

  qSort( lvCalibrationValues );

  double myOmissionStep = 100.0 / ( double )lvCalibrationValues.size();
  lvCalibrationReport += tr( "Calibration Report" ) + "\n";
  lvCalibrationReport += tr( "Omission" ) + "\t" + tr( "Threshold" );
  for( int lvListItem = 0; lvListItem < lvCalibrationValues.size(); lvListItem++ )
  {
    lvCalibrationReport += "\n" + QString::number( lvListItem * myOmissionStep, 'f', 2 ) + "%\t" + QString::number( lvCalibrationValues[ lvListItem ], 'f' );
  }

  emit message( lvCalibrationReport );
  emit testComplete( true );
}
