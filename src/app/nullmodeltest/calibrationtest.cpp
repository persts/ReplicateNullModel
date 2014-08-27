#include "calibrationtest.h"

#include <QList>
#include<QDebug>

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
  qDebug() << lvCalibrationPoints.size();
  for( int lvLocalityRunner = 0; lvLocalityRunner < lvCalibrationPoints.size(); lvLocalityRunner++ )
  {
      //qDebug() << lvCalibrationPoints.locality( lvLocalityRunner );
    lvLocality = lvCalibrationPoints.locality( lvLocalityRunner );
    lvRawModelData = cvRasterModel->rawDataAtRealWorldCoordinate( lvLocality.realWorldCoordinates ); /**/
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
    qDebug() << lvCalibrationReport;
  emit message( lvCalibrationReport );
  emit testComplete( true );
}
