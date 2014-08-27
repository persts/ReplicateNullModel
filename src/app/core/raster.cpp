#include "raster.h"

#include <QFileInfo>
#include <QDebug>

Raster::Raster()
{
  cvIsValid = false;
  cvIsReadOnly = true;
  cvGDALDataSet = 0;
  cvBands = 0;
  cvRowBuffer = 0;
  cvCurrentRowBuffered = 0;

  cvHeight = 0;
  cvWidth = 0;
  cvNumberOfBands = 0;
  cvNoDataValue = 0;
  cvValidNoData = 0;
  cvGeotransform[0] = 0;
  cvGeotransform[1] = 0;
  cvGeotransform[2] = 0;
  cvGeotransform[3] = 0;
  cvGeotransform[4] = 0;
  cvGeotransform[5] = 0;

}

Raster::Raster( QString theFileName, bool readOnly )
{
  cvIsValid = false;
  qDebug("raster");
  cvIsReadOnly = readOnly;
  GDALAllRegister();
  cvBands = 0;
  cvRowBuffer = 0;
  cvCurrentRowBuffered = 0;

  cvHeight = 0;
  cvWidth = 0;
  cvNumberOfBands = 0;
  cvNoDataValue = 0;
  cvValidNoData = 0;
  cvGeotransform[0] = 0;
  cvGeotransform[1] = 0;
  cvGeotransform[2] = 0;
  cvGeotransform[3] = 0;
  cvGeotransform[4] = 0;
  cvGeotransform[5] = 0;

  if( readOnly )
  {
    cvGDALDataSet = GDALOpen( theFileName.toLocal8Bit().data(), GA_ReadOnly );
  }
  else
  {
    cvGDALDataSet = GDALOpen( theFileName.toLocal8Bit().data(), GA_Update );
  }

  if( 0 == cvGDALDataSet )
  {
    QFileInfo lvFileInfo( theFileName );
    cvLastError = QObject::tr( "%1 was not a recognized raster format." ) .arg( lvFileInfo.fileName() );
    return;
  }

  if( GDALGetGeoTransform( cvGDALDataSet, cvGeotransform ) != CE_None )
  {
    cvLastError = QObject::tr( "Unable to read the data set's geotransform." );
    return;
  }

  cvNumberOfBands = GDALGetRasterCount( cvGDALDataSet );
  cvWidth = GDALGetRasterXSize( cvGDALDataSet );
  cvHeight = GDALGetRasterYSize( cvGDALDataSet );

  cvBands = new GDALRasterBandH[ cvNumberOfBands ];
  cvRowBuffer = new float*[ cvNumberOfBands ];
  cvCurrentRowBuffered = new int[ cvNumberOfBands ];
  for( int lvIterator = 1; lvIterator <= cvNumberOfBands; lvIterator++ )
  {
    cvRowBuffer[ lvIterator - 1 ] = 0;
    cvBands[ lvIterator - 1 ] = GDALGetRasterBand( cvGDALDataSet, lvIterator );
    cvCurrentRowBuffered[ lvIterator - 1 ] = -1;
  }

  cvNoDataValue = GDALGetRasterNoDataValue( cvBands[0], &cvValidNoData );
  cvDataType = GDALGetRasterDataType( cvBands[0] );
  cvDataTypeSize = GDALGetDataTypeSize( cvDataType );

  cvIsValid = true;
}

Raster::~Raster()
{
  if( 0 != cvGDALDataSet )
  {
    GDALClose( cvGDALDataSet );
  }

  if( 0 != cvBands )
  {
    delete[] cvBands;
  }

  if( 0 != cvRowBuffer )
  {
    for( int lvIterator = 0; lvIterator < cvNumberOfBands; lvIterator++ )
    {
      if( 0 != cvRowBuffer[ lvIterator ] )
      {
        delete[] cvRowBuffer[ lvIterator ];
      }
    }
    delete[] cvRowBuffer;
  }

  if( 0 != cvCurrentRowBuffered )
  {
    delete[] cvCurrentRowBuffered;
  }
}

float* Raster::rawData( int theBand, int theRow )
{
  if( theBand < 1 || theBand > cvNumberOfBands )
  {
    cvLastError = QObject::tr( "Invalid band number" );
    return 0;
  }

  if( 0 > theRow || theRow >= cvHeight )
  {
    cvLastError = QObject::tr( "Invalid row number" );
    return 0;
  }

  if( theRow != cvCurrentRowBuffered[ theBand - 1 ] )
  {
    cvCurrentRowBuffered[ theBand - 1 ] = theRow;
    if( 0 == cvRowBuffer[ theBand - 1 ] )
    {
      cvRowBuffer[ theBand - 1 ] = new float[ cvWidth ];
      if( 0 == cvRowBuffer[ theBand - 1 ] )
      {
        cvLastError = QObject::tr( "Unable to allocate row buffer" );
        return 0;
      }
    }
    GDALRasterIO( cvBands[ theBand - 1 ], GF_Read, 0, theRow, cvWidth, 1, cvRowBuffer[ theBand - 1 ], cvWidth, 1, GDT_Float32, 0, 0 );
  }

  return cvRowBuffer[ theBand - 1 ];
}

bool Raster::operator==( const Raster &that )
{
  if( !this->cvIsValid || !that.cvIsValid )
  {
    return false;
  }

  for( int lvIterator = 0; lvIterator < 6; lvIterator++ )
  {

    if( this->cvGeotransform[ lvIterator ] != that.cvGeotransform[ lvIterator ] )
    {
      return false;
    }
  }

  if( this->cvWidth != that.cvWidth || this->cvHeight != that.cvHeight || this->cvNumberOfBands != that.cvNumberOfBands || this->cvDataType != that.cvDataType )
  {
    return false;
  }

  if( this->cvValidNoData != that.cvValidNoData || this->cvNoDataValue != that.cvNoDataValue )
  {
    return false;
  }

  return true;
}

bool Raster::operator!=( const Raster &that )
{
  if( !this->cvIsValid || !that.cvIsValid )
  {
    return true;
  }

  return !( *this == that );
}

