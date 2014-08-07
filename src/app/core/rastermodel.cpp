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
#include "rastermodel.h"

#define MODEL_NEGATIVE qRgba( 0,0,0,255 )
#define MODEL_POSITIVE qRgba( 0,128,128,255 )
#define MODEL_NODATA qRgba( 225,225,225,0 )
#define DOUBLE_DIFF_THRESHOLD 0.0000000001

#include <QDebug>

#include <time.h>
#include <limits>

#include <QObject>
#include <QFileInfo>

RasterModel::RasterModel()
{
  init();
}

RasterModel::RasterModel( QString theRasterFile )
{
  init();
  this->load( theRasterFile );
}

RasterModel::RasterModel( QString theRasterFile, const GradientShader &theShader )
{
  init();
  cvShader = theShader;
  this->load( theRasterFile );
}

RasterModel::~RasterModel()
{
  if( 0 != cvGDALDataSet )
  {
    GDALClose( cvGDALDataSet );
  }

  if( 0 != cvImage )
  {
    delete cvImage;
  }

  if( 0 != cvRNG )
  {
    delete cvRNG;
  }
}

/*
 *
 * PUBLIC FUNCTIONS
 *
 */
QString RasterModel::fileName( bool justFileName )
{
  if( justFileName )
  {
    QFileInfo lvFileInfo( cvRasterFileName );
    return lvFileInfo.fileName();
  }

  return cvRasterFileName;
}

QList< QPoint > RasterModel::generateRandomDistributionOfPoints( int theNumberOfSamples, bool withReplacement )
{
  QList< QPoint > lvPointList;
  // If layer has not been loaded return an empty list
  if( !cvIsValid )
  {
    return lvPointList;
  }

  //If the number of points is more or equal the available point list just return the main list
  if( theNumberOfSamples >= cvModelDataPoints.size() )
  {
    return cvModelDataPoints;
  }

  if( withReplacement )
  {
    //Randomly select a point and take it out of the original set of points
    double lvSize = ( double )( cvModelDataPoints.size() );
    int lvPoint = 0;
    for( int lvIterator = 0; lvIterator < theNumberOfSamples; lvIterator++ )
    {
      lvPoint = ( int )( cvRNG->randExc() * lvSize );
      lvPointList.append( cvModelDataPoints.at( lvPoint ) );
    }
  }
  else
  {
    //Randomly select a point and take it out of the original set of points
    for( int lvIterator = 0; lvIterator < theNumberOfSamples; lvIterator++ )
    {
      lvPointList.append( cvModelDataPoints.takeAt(( int )( cvRNG->randExc() *(( double ) cvModelDataPoints.size() ) ) ) );
    }

    //Put a copy of all of the selected points back into the original set of points
    for( int lvIterator = 0; lvIterator < theNumberOfSamples; lvIterator++ )
    {
      cvModelDataPoints.append( lvPointList.at( lvIterator ) );
    }
  }

  return lvPointList;
}

QString RasterModel::lastError()
{
  return cvLastError;
}

bool RasterModel::load( QString theFileName, bool estimateMinMax, bool useThreshold, double theThreshold, bool inverseRelationship )
{
  //TODO: Load and reload need to be rethought.

  //Free existing resource
  if( 0 != cvGDALDataSet )
  {
    GDALClose( cvGDALDataSet );
  }

  if( 0 != cvImage )
  {
    delete cvImage;
  }

  cvFileName = theFileName;
  cvIsThresholdApplied = useThreshold;
  cvThreshold = theThreshold;
  cvIsValid = false;
  GDALAllRegister();
  cvGDALDataSet = GDALOpen( theFileName.trimmed().toAscii().data(), GA_ReadOnly );
  if( 0 == cvGDALDataSet )
  {
    cvLastError = QObject::tr( "The input file was not a valid GDAL dataset" );
    return false;
  }

  cvNumberOfBands = GDALGetRasterCount( cvGDALDataSet );
  cvXSize = GDALGetRasterXSize( cvGDALDataSet );
  cvYSize = GDALGetRasterYSize( cvGDALDataSet );

  if( GDALGetGeoTransform( cvGDALDataSet, cvGeotransform ) == CE_None )
  {
    cvRasterFileName = theFileName.trimmed();

    float* lvScanline;
    cvModelNegative = 0;
    cvModelPositive = 0;
    cvModelDataPoints.clear();
    cvRasterBand = GDALGetRasterBand( cvGDALDataSet, 1 );
    cvImage = new QImage( cvXSize, cvYSize, QImage::Format_ARGB32 );

    int cvValidNoDataValue = 0;
    cvNoDataValue = GDALGetRasterNoDataValue( cvRasterBand, &cvValidNoDataValue );

    //TODO: If first load and estimateMinMax is false, the min double value and max double value will be used and will equal a black image.
    //If the diff between 0 and cvMaxium is > then Y, estimate?
    if( estimateMinMax )
    {
      double lvRasterMinMax[2];
      GDALComputeRasterMinMax( cvRasterBand, 1, lvRasterMinMax );
      cvShader.setInterval( lvRasterMinMax[0], lvRasterMinMax[1] );
      cvMaximum = std::numeric_limits<double>::max() * -1;
      cvMinimum = std::numeric_limits<double>::max();
    }
    else
    {
      cvShader.setInterval( cvMinimum, cvMaximum );
    }

    lvScanline = new float[cvXSize];
    if( 0 != lvScanline )
    {
      for( int yRunner = 0; yRunner < cvYSize; yRunner++ )
      {
        GDALRasterIO( cvRasterBand, GF_Read, 0, yRunner, cvXSize, 1, lvScanline, cvXSize, 1, GDT_Float32, 0, 0 );
        QRgb* lvBuffer = ( QRgb* ) cvImage->scanLine( yRunner );
        for( int xRunner = 0; xRunner < cvXSize; xRunner++ )
        {
          if( cvValidNoDataValue && fabs( lvScanline[xRunner] - cvNoDataValue ) > DOUBLE_DIFF_THRESHOLD )
          {
            cvMaximum = qMax( cvMaximum, ( double )lvScanline[xRunner] );
            cvMinimum = qMin( cvMinimum, ( double )lvScanline[xRunner] );
          }

          if( useThreshold )
          {
            if( cvValidNoDataValue && fabs( lvScanline[xRunner] - cvNoDataValue ) < DOUBLE_DIFF_THRESHOLD )
            {
              lvBuffer[ xRunner ] = MODEL_NODATA;
            }
            else
            {
              if( inverseRelationship )
              {
                if( lvScanline[xRunner] <= theThreshold || fabs( lvScanline[xRunner] - theThreshold ) < DOUBLE_DIFF_THRESHOLD)
                {
                  cvModelPositive++;
                  lvBuffer[ xRunner ] = MODEL_POSITIVE;
                  cvModelDataPoints.append( QPoint( xRunner, yRunner ) );
                }
                else
                {
                  cvModelNegative++;
                  lvBuffer[ xRunner ] = MODEL_NEGATIVE;
                  cvModelDataPoints.append( QPoint( xRunner, yRunner ) );
                }
              }
              else
              {
                if( lvScanline[xRunner] >= theThreshold || fabs( lvScanline[xRunner] - theThreshold ) < DOUBLE_DIFF_THRESHOLD)
                {
                  cvModelPositive++;
                  lvBuffer[ xRunner ] = MODEL_POSITIVE;
                  cvModelDataPoints.append( QPoint( xRunner, yRunner ) );
                }
                else
                {
                  cvModelNegative++;
                  lvBuffer[ xRunner ] = MODEL_NEGATIVE;
                  cvModelDataPoints.append( QPoint( xRunner, yRunner ) );
                }
              }
            }
          }
          else
          {
            if( cvValidNoDataValue && fabs( lvScanline[xRunner] - cvNoDataValue ) < DOUBLE_DIFF_THRESHOLD )
            {
              lvBuffer[ xRunner ] = MODEL_NODATA;
            }
            else
            {
              cvModelDataPoints.append( QPoint( xRunner, yRunner ) );
              lvBuffer[ xRunner ] = cvShader.shade( lvScanline[xRunner] ).rgba();
            }
          }
        }
      }
    }
    else
    {
      cvLastError = QObject::tr( "Cound not allocate enought memory to read the raster band" );
      return false;
    }
  }
  else
  {
    cvLastError = QObject::tr( "The geotransormation could not be read from the raster layer" );
    return false;
  }

  cvIsValid = true;
  return true;
}

RasterModel::ModelResponse RasterModel::modelResponseAtPixelCoordinate( int x, int y ) const
{
  if( x < 0 || x >= cvXSize )
    return OutOfBounds;

  if( y < 0 || y >= cvYSize )
    return OutOfBounds;

  QRgb lvPixelValue = cvImage->pixel( x, y );
  if( cvValidNoDataValue )
  {
    if( lvPixelValue == MODEL_NODATA )
    {
      return NoData;
    }
  }

  if( !cvIsThresholdApplied )
  {
    return RawData;
  }
  else if( lvPixelValue == MODEL_NEGATIVE )
  {
    return Negative;
  }
  else if( lvPixelValue == MODEL_POSITIVE )
  {
    return Positive;
  }

  //Can't tell, make it no data
  return NoData;
}

RasterModel::ModelResponse RasterModel::modelResponseAtRealWorldCoordinate( double x, double y )
{
  QPointF lvPoint = realWorldToPixel( x, y );

  //Cast causes with values like -0.1 to be 0
  if( lvPoint.x() < 0 || lvPoint.y() < 0 ) { return OutOfBounds; }

  return modelResponseAtPixelCoordinate(( int )lvPoint.x(), ( int )lvPoint.y() );
}

RasterModel::ModelResponse RasterModel::modelResponseAtRealWorldCoordinate( const QPointF &thePoint )
{
  return modelResponseAtRealWorldCoordinate( thePoint.x(), thePoint.y() );
}

double RasterModel::positiveResponseProbability()
{
  if( cvModelPositive == 0 && cvModelNegative == 0 )
  {
    return 0.0;
  }
  else if( cvModelNegative == 0 )
  {
    return 1.0;
  }

  return ( double )cvModelPositive / ( double )( cvModelPositive + cvModelNegative );
}

//Omission keyed on threshold value, i.e., producing a list of distinct threshold values
QMap< QString, QString > RasterModel::omissionList( LocalityList theCalibrationList )
{
  QMap< QString, QString > lvOmissionList;
  if( !cvIsValid )
  {
    cvLastError = QObject::tr( "The raster model is not valid" );
  }
  else if( 0 == theCalibrationList.size() )
  {
    cvLastError = QObject::tr( "The test calibration list is empty" );
  }
  else
  {
    Locality lvLocality;
    QList< double > lvCalibrationValues;
    for( int lvLocalityRunner = 0; lvLocalityRunner < theCalibrationList.size(); lvLocalityRunner++ )
    {
      lvLocality = theCalibrationList.locality( lvLocalityRunner );
      lvCalibrationValues << rawDataAtRealWorldCoordinate( lvLocality.realWorldCoordinates );
    }
    qSort( lvCalibrationValues );
    int lvSize = lvCalibrationValues.size();
    lvOmissionList[ "0.00000000" ] = "  0.000";
    QString lvKey;
    QString lvValue;
    for( int lvIterator = 1; lvIterator <= lvSize; lvIterator++ )
    {
      lvKey = QString::number( lvCalibrationValues[ lvIterator -1 ], 'f', 8 );
      lvValue = QString("%1").arg( ( (double)lvIterator / (double)lvSize ) * 100.0, 7, 'f', 3, QLatin1Char(' ') );
      lvOmissionList[ lvKey ] = lvValue;
    }
  }

  return lvOmissionList;

}

//Omission list keyed on percent of points
QMap< QString, QString > RasterModel::omissionListFull( LocalityList theCalibrationList )
{
  QMap< QString, QString > lvOmissionList;
  if( !cvIsValid )
  {
    cvLastError = QObject::tr( "The raster model is not valid" );
  }
  else if( 0 == theCalibrationList.size() )
  {
    cvLastError = QObject::tr( "The test calibration list is empty" );
  }
  else
  {
    Locality lvLocality;
    QList< double > lvCalibrationValues;
    for( int lvLocalityRunner = 0; lvLocalityRunner < theCalibrationList.size(); lvLocalityRunner++ )
    {
      lvLocality = theCalibrationList.locality( lvLocalityRunner );
      lvCalibrationValues << rawDataAtRealWorldCoordinate( lvLocality.realWorldCoordinates );
    }
    qSort( lvCalibrationValues );
    int lvSize = lvCalibrationValues.size();
    lvOmissionList[ "  0.000" ] = "0.00000000";
    QString lvKey;
    QString lvValue;
    for( int lvIterator = 1; lvIterator <= lvSize; lvIterator++ )
    {
      lvKey = QString("%1").arg( ( (double)lvIterator / (double)lvSize ) * 100.0, 7, 'f', 3, QLatin1Char(' ') );
      lvValue = QString::number( lvCalibrationValues[ lvIterator -1 ], 'f', 8 );
      lvOmissionList[ lvKey ] = lvValue;
    }
  }

  return lvOmissionList;
}

QPoint RasterModel::randomModelDataPoint()
{
  if( cvModelDataPoints.size() == 0 )
  {
    return QPoint();
  }

  return cvModelDataPoints.at(( int )( cvRNG->randExc() * ( cvModelDataPoints.size() ) ) );
}

int RasterModel::randomModelResponse( int theNumberOfSamples )
{
  int lvPositiveResponses = -1;
  int lvListSize = cvModelDataPoints.size();
  if( !cvIsValid )
  {
    cvLastError = QObject::tr( "The raster model is not valid" );
  }
  else if( !cvIsThresholdApplied )
  {
    cvLastError = QObject::tr( "The raster model will only respond with [RAW DATA] values, a threshold must be applied first" );
  }
  else if( 0 >= theNumberOfSamples )
  {
    cvLastError = QObject::tr( "The number of samples has to be greater than 0" );
  }
  else if( theNumberOfSamples >= lvListSize )
  {
    lvPositiveResponses = 0;
    for( int lvIterator = 0; lvIterator < lvListSize; lvIterator++ )
    {
      if( RasterModel::Positive == modelResponseAtPixelCoordinate( cvModelDataPoints[ lvIterator ].x(), cvModelDataPoints[ lvIterator ].y() ) )
      {
        lvPositiveResponses++;
      }
    }
  }
  else
  {
    lvPositiveResponses = 0;
    int lvPoint = 0;
    for( int lvIterator = 0; lvIterator < theNumberOfSamples; lvIterator++ )
    {
      lvPoint = ( int )( cvRNG->randExc() * ( double ) lvListSize );
      if( RasterModel::Positive == this->modelResponseAtPixelCoordinate( cvModelDataPoints[ lvPoint ].x(), cvModelDataPoints[ lvPoint ].y() ) )
      {
        lvPositiveResponses++;
      }
    }
  }

  return lvPositiveResponses;
}

double RasterModel::rawDataAtPixelCoordinate( int x, int y )
{
  //TODO there should really be a flag set if this is a valid response or not
  if( x < 0 || x >= cvXSize )
    return 0.0;

  if( y < 0 || y >= cvYSize )
    return 0.0;

  if( 0 == cvRasterBand )
    return 0.0;

  //TODO: This is really not efficient to get one pixel at a time, 3 to X lines should be loaded into a buffer
  float lvPixelValue = 0.0;
  GDALRasterIO( cvRasterBand, GF_Read, x, y, 1, 1, &lvPixelValue, 1, 1, GDT_Float32, 0, 0 );

  return ( double )lvPixelValue;
}

double RasterModel::rawDataAtPixelCoordinate( const QPoint &thePoint )
{
  return rawDataAtPixelCoordinate( thePoint.x(), thePoint.y() );
}

double RasterModel::rawDataAtRealWorldCoordinate( double x, double y )
{
  QPointF lvPoint = realWorldToPixel( x, y );
  return rawDataAtPixelCoordinate(( int )lvPoint.x(), ( int )lvPoint.y() );
}

double RasterModel::rawDataAtRealWorldCoordinate( const QPointF &thePoint )
{
  return rawDataAtRealWorldCoordinate( thePoint.x(), thePoint.y() );
}

QPointF RasterModel::pixelToRealWorld( double x, double y )
{
  return QPointF(( double )cvGeotransform[0] + ( x * ( double )cvGeotransform[1] ), ( double )cvGeotransform[3] + ( y * ( double )cvGeotransform[5] ) );
}

QPointF RasterModel::pixelToRealWorld( const QPoint &thePoint )
{
  return pixelToRealWorld( (double)thePoint.x(), (double)thePoint.y() );
}

QPointF RasterModel::pixelToRealWorld( const QPointF &thePoint )
{
  return pixelToRealWorld( thePoint.x(), thePoint.y() );
}

QPointF RasterModel::realWorldToPixel( double x, double y )
{
  if(( double )cvGeotransform[1] == 0.0 || ( double )cvGeotransform[5] == 0.0 )
  {
    return QPointF( 0, 0 );
  }

  //In the grid/raster model west and south cell boundaries are considered in the cell
  //In pixel coordinates the Y is reversed to the south boundary is consider in the adjacent cell to the south
  //If the y is an even multiple of the cell size, we need to take off just a little, so when the pixel value is cast to an int it will end up in the right cell
  if( 0.0 == fmod( y, ( double )cvGeotransform[5] ) )
  {
    return QPointF(( x - ( double )cvGeotransform[0] ) / ( double )cvGeotransform[1], (( y - ( double )cvGeotransform[3] ) / ( double )cvGeotransform[5] ) - 0.00000001 );
  }

  return QPointF(( x - ( double )cvGeotransform[0] ) / ( double )cvGeotransform[1], ( y - ( double )cvGeotransform[3] ) / ( double )cvGeotransform[5] );
}

QPointF RasterModel::realWorldToPixel( const QPointF &thePoint )
{
  return realWorldToPixel( thePoint.x(), thePoint.y() );
}

bool RasterModel::reload()
{
  if( cvIsValid )
  {
    return load( cvRasterFileName, false );
  }
  else
  {
    cvLastError = QObject::tr( "A valid raster layer is not available yet" );
    return false;
  }
}

bool RasterModel::reload( double theThreshhold, bool inverseRelationship )
{
  if( cvIsValid )
  {
    return load( cvRasterFileName, false, true, theThreshhold, inverseRelationship );
  }
  else
  {
    cvLastError = QObject::tr( "A valid raster layer is not available yet" );
    return false;
  }
}


bool RasterModel::operator==( const RasterModel &that )
{
  if( this->cvNoDataValue != that.cvNoDataValue )
  {
    return false;
  }

  if( this->cvGeotransform[0] != that.cvGeotransform[0] )
  {
    return false;
  }

  if( this->cvGeotransform[1] != that.cvGeotransform[1] )
  {
    return false;
  }

  if( this->cvGeotransform[2] != that.cvGeotransform[2] )
  {
    return false;
  }

  if( this->cvGeotransform[3] != that.cvGeotransform[3] )
  {
    return false;
  }

  if( this->cvGeotransform[4] != that.cvGeotransform[4] )
  {
    return false;
  }

  if( this->cvGeotransform[5] != that.cvGeotransform[5] )
  {
    return false;
  }

  return true;
}

bool RasterModel::operator!=( const RasterModel &that )
{
  return !( this->operator==( that ) );
}

/*
 *
 * PRIVATE FUNCTIONS
 *
 */
void RasterModel::init()
{
  cvGDALDataSet = 0;
  cvNumberOfBands = 0;
  cvIsValid = false;
  cvIsThresholdApplied = false;
  cvMaximum = std::numeric_limits<double>::max() * -1;
  cvMinimum = std::numeric_limits<double>::max();
  cvThreshold = 0.0;
  cvXSize = 0;
  cvYSize = 0;
  cvModelNegative = 0;
  cvModelPositive = 0;
  cvLastError = "";
  cvImage = new QImage();
  cvGeotransform[0] = 0.0;
  cvGeotransform[1] = 0.0;
  cvGeotransform[2] = 0.0;
  cvGeotransform[3] = 0.0;
  cvGeotransform[4] = 0.0;
  cvGeotransform[5] = 0.0;
  cvFileName = "Undefined";
  cvRNG = new MTRand( time( 0 ) );
}
