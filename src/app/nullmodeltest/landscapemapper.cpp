#include "landscapemapper.h"

#define HASH_SEED 1298712093
#define ACCUMULATION 500000

#include <QDebug>

#include <QDir>
#include <QDateTime>
#include <QStringList>
#include <QSetIterator>
/**//**/
LandscapeMapper::LandscapeMapper( RasterModel* theRasterModel, QString theOutputDirectory, bool plotAccumulationCurve )
{
  cvIsValid = false;

  cvRasterModel = theRasterModel;
  cvPlotAccumulationCurve = plotAccumulationCurve;

  cvTemplateMap = 0;
  cvTemplateSize = 0;
  cvLastProgressValue = -1;
  cvProbability = 0;
  cvTotalReplicatesInLandscape = 0;
  cvTotalAddRequests = 0;

  cvDistinctLandscapeReplicatesFile = 0;
  cvAccumulationCurveFile = 0;

  cvDistinctReplicatesStream = 0;
  cvAccumulationCurveStream = 0;

  if( 0 != cvRasterModel && !cvRasterModel->isValid() )
  {
    cvLastError = tr( "The raster model is not valid" );
    return;
  }

  if( !cvRasterModel->isThreshholdApplied() )
  {
    cvLastError = tr( "A threshhold has not been applied to the raster model" );
    return;
  }

  //Open output files
  QString lvOutputDirectoryName = theOutputDirectory.isEmpty() ? QDir::tempPath() : theOutputDirectory;
  QString lvBaseFileName = lvOutputDirectoryName + QDir::separator() + QDateTime::currentDateTime().toString( "yyyyMMdd_hhmmss" );

  cvDistinctLandscapeReplicatesFile = new QFile( lvBaseFileName + "_distinct_replicates.txt" );
  if( cvPlotAccumulationCurve )
  {
    cvAccumulationCurveFile = new QFile( lvBaseFileName + "_accumulation.txt" );
  }

  if( cvDistinctLandscapeReplicatesFile->open( QIODevice::ReadWrite | QIODevice::Text ) )
  {
    if( cvPlotAccumulationCurve )
    {
      if( cvAccumulationCurveFile->open( QIODevice::ReadWrite | QIODevice::Text ) )
      {
        cvIsValid = true;
        cvDistinctReplicatesStream = new QTextStream( cvDistinctLandscapeReplicatesFile );
        cvAccumulationCurveStream = new QTextStream( cvAccumulationCurveFile );
      }
    }
    else
    {
      cvIsValid = true;
      cvDistinctReplicatesStream = new QTextStream( cvDistinctLandscapeReplicatesFile );
    }
  }



  if( !cvIsValid )
  {
    cvLastError = tr( "Unable to create replicate files" );
  }

  //Initialization done, set run type to unknown and wait
  cvRunType = LandscapeMapper::Unknown;
}

LandscapeMapper::~LandscapeMapper()
{
  cvLandscapeReplicateHashTable.clear();

  if( 0 != cvTemplateMap )
  {
    for( int lvIterator = 0; lvIterator < cvTemplateSize; lvIterator++ )
    {
      if( 0 != cvTemplateMap[ lvIterator ] )
      {
        delete[] cvTemplateMap[ lvIterator ];
        cvTemplateMap[ lvIterator ] = 0;
      }
    }

    delete[] cvTemplateMap;
    cvTemplateMap = 0;
  }

  if( 0 != cvProbability )
  {
    delete cvProbability;
    cvProbability = 0;
  }

  if( 0 != cvDistinctLandscapeReplicatesFile )
  {
    cvDistinctLandscapeReplicatesFile->close();
    delete cvDistinctLandscapeReplicatesFile;
  }

  if( 0 != cvAccumulationCurveFile )
  {
    cvAccumulationCurveFile->close();
    delete cvAccumulationCurveFile;
  }
}

/*
 *
 * PUBLIC FUNCTIONS
 *
 */
bool LandscapeMapper::addReplicate( QString theReplicate, bool storeAssociation )
{
  bool lvReplicateAdded = false;
  if( !theReplicate.isEmpty() )
  {
    cvTotalAddRequests++;

    //Pad the replicate to be an even multiple of 8 for the has algorithm
    QString lvHashString = QString( "%1" ) .arg( theReplicate, -1 * ( theReplicate.length() + ( 8 - theReplicate.length() % 8 ) ), 'X' );
    quint64 lvHash = murmurHash( lvHashString.toLocal8Bit().data(), lvHashString.length(), 1298712093 );
    //*cvAllReplicatesStream << QString::number( lvHash ) << "," << theReplicate << "\n";
    int lvSize = cvLandscapeReplicateHashTable.size();
    cvLandscapeReplicateHashTable.insert( lvHash );
    if( lvSize != cvLandscapeReplicateHashTable.size() )
    {
      *cvDistinctReplicatesStream << theReplicate << "\n";
      cvTotalReplicatesInLandscape++;

      if( storeAssociation )
      {
        cvLandscapeRowHashAssociation[2] << lvHash;
      }
      lvReplicateAdded = true;
    }

    if( cvPlotAccumulationCurve )
    {
      if( cvTotalAddRequests % ACCUMULATION == 0 )
      {
        *cvAccumulationCurveStream << QDateTime::currentDateTime().toString( "yyyy-MM-dd hh:mm:ss" ) << "," << cvTotalAddRequests / ACCUMULATION << "," << cvTotalReplicatesInLandscape << "\n";
        cvAccumulationCurveStream->flush();
      }
    }
  }

  return lvReplicateAdded;
}

void LandscapeMapper::calculateProbability()
{
  cvRunType = LandscapeMapper::CalculateProbability;
  this->start();
}

void LandscapeMapper::calculateProbabilityBlocking()
{
  cvRunType = LandscapeMapper::CalculateProbability;
  run();
}

void LandscapeMapper::generateLandscapeMap()
{
  cvRunType = LandscapeMapper::GenerateLandscapeMap;
  this->start();
}

bool LandscapeMapper::initRelabeler( int theTemplateSize, const QSet< QString > &theReplicates )
{
  if( !cvIsValid ) { return false; }

  cvTemplateSize = theTemplateSize;
  //Clean out the template map if it already exists
  if( 0 != cvTemplateMap )
  {
    for( int lvIterator = 0; lvIterator < cvTemplateSize; lvIterator++ )
    {
      if( 0 != cvTemplateMap[ lvIterator ] )
      {
        delete[] cvTemplateMap[ lvIterator ];
        cvTemplateMap[ lvIterator ] = 0;
      }
    }

    delete[] cvTemplateMap;
    cvTemplateMap = 0;
  }

  cvTemplateMap = new QString*[ cvTemplateSize ];
  if( 0 == cvTemplateMap ) { return false; }

  for( int lvIterator = 0; lvIterator < cvTemplateSize; lvIterator++ )
  {
    cvTemplateMap[ lvIterator ] = new QString[ cvTemplateSize ];
    if( 0 == cvTemplateMap[ lvIterator ] ) { return false; }
  }

  for( int lvRow = 0; lvRow < theTemplateSize; lvRow++ )
  {
    for( int lvColumn = 0; lvColumn < theTemplateSize; lvColumn++ )
    {
      cvTemplateMap[ lvRow ][ lvColumn ] = "0";
    }
  }

  QSetIterator< QString > lvReplicates( theReplicates );
  while( lvReplicates.hasNext() )
  {
    ReplicateRelabeler lvNewRelabeler( lvReplicates.next() );
    QStringList lvComponents = lvNewRelabeler.id().split( '|' );
    int lvSize = lvComponents.size();
    for( int lvIterator = 0; lvIterator < lvSize; lvIterator++ )
    {
      QString lvPointId = lvComponents[ lvIterator ].section( '-', 0, 0 );
      QStringList lvCoordinates = lvComponents[ lvIterator ].section( '-', -1 ).split( '_' );
      if( 2 != lvCoordinates.size() )
      {
        cvReplicates.clear();
        cvLastError = tr( "Unexpected format encountered in replicate" ) + ": " + lvNewRelabeler.id();
        return false;
      }
      int x = lvCoordinates[ 0 ].toInt();
      int y = lvCoordinates[ 1 ].toInt();

      if( 0 > x || 0 > y || x >= cvTemplateSize || y >= cvTemplateSize )
      {
        cvReplicates.clear();
        cvLastError = tr( "Replicate has coordinate outside of template" ) + ": " + lvNewRelabeler.id();
        return false;
      }
      lvNewRelabeler.add( lvPointId, &cvTemplateMap[( cvTemplateSize - 1 ) -  y ][ x ] );

    }

    cvReplicates << lvNewRelabeler;
  }
  return true;
}

const Probability &LandscapeMapper::probability()
{
  if( 0 == cvProbability )
  {
    cvProbability = new Probability( 0 );
  }

  return *cvProbability;
}

void LandscapeMapper::run()
{
  if( cvIsValid && LandscapeMapper::GenerateLandscapeMap == cvRunType && 0 != cvTemplateMap )
  {
    QListIterator< ReplicateRelabeler > lvIterator( cvReplicates );

    cvTotalReplicatesInLandscape = 0;

    RasterModel::ModelResponse lvModelResponse;
    int lvTemplateOffset = cvTemplateSize / 2;
    int lvWidth = cvRasterModel->width();
    int lvHeight = cvRasterModel->height();
    int lvCurrentProgress = 0;
    int lvTarget = lvWidth * lvHeight;
    for( int lvRasterYRunner = 0; lvRasterYRunner < lvHeight; lvRasterYRunner++ )
    {
      //Clean out the hases from two rows above the current row being processed to save memory
      QListIterator< quint64 > lvHashTableCleaner( cvLandscapeRowHashAssociation[0] );
      while( lvHashTableCleaner.hasNext() )
      {
        cvLandscapeReplicateHashTable.remove( lvHashTableCleaner.next() );
      }
      cvLandscapeRowHashAssociation[0].clear();
      cvLandscapeRowHashAssociation[0] = cvLandscapeRowHashAssociation[1];
      cvLandscapeRowHashAssociation[1].clear();
      cvLandscapeRowHashAssociation[1] = cvLandscapeRowHashAssociation[2];
      cvLandscapeRowHashAssociation[2].clear();
      //end cleaning

      for( int lvRasterXRunner = 0; lvRasterXRunner < lvWidth; lvRasterXRunner++ )
      {
        lvCurrentProgress++;
        progress( lvCurrentProgress, lvTarget );
        for( int lvTemplateYRunner = 0; lvTemplateYRunner < cvTemplateSize; lvTemplateYRunner++ )
        {
          for( int lvTemplateXRunner = 0; lvTemplateXRunner < cvTemplateSize; lvTemplateXRunner++ )
          {
            QString lvCoordinates;
            int lvX = lvRasterXRunner + ( lvTemplateXRunner - lvTemplateOffset );
            int lvY = lvRasterYRunner + ( lvTemplateYRunner - lvTemplateOffset );
            lvModelResponse = cvRasterModel->modelResponseAtPixelCoordinate( lvX, lvY );
            if( RasterModel::OutOfBounds == lvModelResponse || RasterModel::NoData == lvModelResponse )
            {
              lvCoordinates = "NODATA";
            }
            else
            {
              lvCoordinates = QString::number( lvX ) + "_" + QString::number( lvY );
            }
            cvTemplateMap[ lvTemplateYRunner ][ lvTemplateXRunner ] = lvCoordinates;
          }
        }

        lvIterator.toFront();
        while( lvIterator.hasNext() )
        {
          QString lvReplicate = lvIterator.next().print();
          addReplicate( lvReplicate, true );
        }
      }
    }
  }
  else if( cvIsValid && LandscapeMapper::CalculateProbability == cvRunType )
  {
    if( 0 != cvProbability )
    {
      delete cvProbability;
      cvProbability = 0;
    }

    if( 0 < cvTotalReplicatesInLandscape )
    {
      cvDistinctReplicatesStream->seek( 0 );
      int lvExpectedPoints = cvDistinctReplicatesStream->readLine().split( "|" ).size();
      cvDistinctReplicatesStream->seek( 0 );

      cvProbability = new Probability( lvExpectedPoints );
      cvProbability->testName = tr( "Landscape Probability ( distinct replicates )" );
      cvProbability->totalInputs = cvTotalReplicatesInLandscape;

      bool lvHadError = false;
      int lvCurrentProgress = 0;
      while( !cvDistinctReplicatesStream->atEnd() )
      {
        lvCurrentProgress++;
        QStringList lvPoints = cvDistinctReplicatesStream->readLine().split( "|" );
        if( lvExpectedPoints != lvPoints.size() )
        {
          lvHadError = true;
          cvLastError = tr( "Inconsistent number of points found in landscape replicates" );
          cvProbability->errorMessage = cvLastError;
          break;
        }

        RasterModel::ModelResponse lvModelResponse;
        int lvTotalPositive = 0;
        for( int lvPointRunner = 0; lvPointRunner < lvExpectedPoints; lvPointRunner++ )
        {
          QStringList lvCoordinates = lvPoints[ lvPointRunner ].section( '-', -1 ).split( '_' );
          if( 2 != lvCoordinates.size() )
          {
            lvHadError = true;
            cvLastError = tr( "Unexpected format encountered in replicate" );
            cvProbability->errorMessage = cvLastError;
            break;
          }
          lvModelResponse = cvRasterModel->modelResponseAtPixelCoordinate( lvCoordinates[ 0 ].toInt(), lvCoordinates[ 1 ].toInt() );
          if( RasterModel::OutOfBounds == lvModelResponse || RasterModel::NoData == lvModelResponse )
          {
            lvHadError = true;
            cvLastError = tr( "Unexpected OutOfBound or NoData response" );
            cvProbability->errorMessage = cvLastError;
            break;
          }
          else if( RasterModel::Positive == lvModelResponse )
          {
            lvTotalPositive++;
          }
        }

        if( lvHadError ) { break; }
        cvProbability->frequency[ lvTotalPositive ]++;
        progress( lvCurrentProgress, cvProbability->totalInputs );
      }

      cvProbability->isValid = true;
    }
    else
    {
      cvProbability = new Probability( 0 );
      cvLastError = tr( "Landscape Map was empty" );
      cvProbability->errorMessage = cvLastError;
    }


  }

  cvRunType = LandscapeMapper::Unknown;
}

/*
 *
 * PRIVATE FUNCTIONS
 *
 */
//-----------------------------------------------------------------------------
// MurmurHash2, 64-bit versions, by Austin Appleby
// Adapted for Qt by PJE
quint64 LandscapeMapper::murmurHash( const void* key, int len, unsigned int seed )
{
  const quint64 m = Q_UINT64_C( 0xc6a4a7935bd1e995 );
  const int r = 47;

  quint64 h = seed ^( len * m );

  const quint64 * data = ( const quint64 * )key;
  const quint64 * end = data + ( len / 8 );

  while( data != end )
  {
    quint64 k = *data++;

    k *= m;
    k ^= k >> r;
    k *= m;

    h ^= k;
    h *= m;
  }

  const unsigned char * data2 = ( const unsigned char* )data;

  switch( len & 7 )
  {
    case 7: h ^= quint64( data2[6] ) << 48;
    case 6: h ^= quint64( data2[5] ) << 40;
    case 5: h ^= quint64( data2[4] ) << 32;
    case 4: h ^= quint64( data2[3] ) << 24;
    case 3: h ^= quint64( data2[2] ) << 16;
    case 2: h ^= quint64( data2[1] ) << 8;
    case 1: h ^= quint64( data2[0] );
      h *= m;
  };

  h ^= h >> r;
  h *= m;
  h ^= h >> r;

  return h;
}

void LandscapeMapper::progress( int theCurrentProgress, int theTarget )
{
  int lvProgress = ( int )((( double ) theCurrentProgress / ( double ) theTarget ) * 100.0 );
  if( lvProgress != cvLastProgressValue )
  {
    cvLastProgressValue = lvProgress;
    emit progressUpdate( lvProgress, 100 );
  }
}
