/*
** Copyright (C) 2010 Peter J. Ersts (ersts at amnh.org)
** Creation Date: 2010-01-15
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
#define FIELDS 4
#define SPECIES_FIELD 0
#define X_FIELD 1
#define Y_FIELD 2
#define LOCALITY_TYPE 3

#include "specieslocalityset.h"
#include "locality.h"

#include <QDir>
#include <QFile>
#include <QObject>
#include <QTextStream>

#include <QDebug>

SpeciesLocalitySet::SpeciesLocalitySet()
{
  cvLastError = "";
}

bool SpeciesLocalitySet::add( QString theSpecies, LocalityCollection const &theCollection )
{
  if( cvLocalitySet.contains( theSpecies ) )
  {
    cvLastError = QObject::tr( "The locality set already contains the species: %1" ) .arg( theSpecies );
    return false;
  }

  cvLocalitySet.insert( theSpecies, theCollection );
  return true;
}

LocalityCollection SpeciesLocalitySet::collection( QString theKey )
{
  if( cvLocalitySet.contains( theKey ) )
  {
    return cvLocalitySet[ theKey ];
  }

  return LocalityCollection();
}

LocalityCollection SpeciesLocalitySet::collection( int theIndex )
{
  //This really does not have much meaning in a map as the keys are stored in a tree, but is needed for backward compatability
  //TODO: remove this function
  if( theIndex >= 0 && theIndex < cvLocalitySet.size() )
  {
    return cvLocalitySet[ cvLocalitySet.keys().at( theIndex )];
  }

  return LocalityCollection();
}

bool SpeciesLocalitySet::load( QString theFileName )
{
  cvLocalitySet.clear();

  //Try opening the file
  QFile lvFile( theFileName );
  if( !lvFile.open( QIODevice::ReadOnly | QIODevice::Text ) )
  {
    cvLastError = QString( "[ %1 ] %2" ) .arg( theFileName ) .arg( QObject::tr( "Could not be opened." ) );
    return false;
  }

  QString lvData;
  QStringList lvDataElements;
  QTextStream lvInputStream( &lvFile );

  /*
   * Read first line, strict format
   */
  lvData = lvInputStream.readLine();
  int lvTabCount = lvData.count( "\t" );
  int lvCommaCount = lvData.count( "," );

  if( lvTabCount > FIELDS - 1 )
  {
    cvLastError = QObject::tr( "Only four fields are expected, found " ) + QString::number( lvTabCount + 1 );
    return false;
  }

  if( lvCommaCount > FIELDS - 1 )
  {
    cvLastError = QObject::tr( "Only four fields are expected, found " ) + QString::number( lvCommaCount + 1 );
    return false;
  }

  if( lvTabCount <= FIELDS - 3 && lvCommaCount <= FIELDS - 3 )
  {
    cvLastError = QObject::tr( "A minimum of three fields are required ( Species, x, y, [ Locality Type ] )" );
    return false;
  }

  bool lvUseTab = false;
  if( lvTabCount == FIELDS - 1 || lvTabCount == FIELDS - 2 )
  {
    lvUseTab = true;
  }

  //Pull in all the points
  int lvCurrentId = 1;
  Locality lvLocality;
  QString lvSpecies = "";
  bool lvFirstLine = true;
  while( !lvInputStream.atEnd() )
  {
    if( lvFirstLine )
    {
      lvFirstLine = false;
      if( lvData.contains( "species", Qt::CaseInsensitive ) )
      {
        continue;
      }
    }
    else
    {
      lvData = lvInputStream.readLine();
    }

    if( lvUseTab )
    {
      lvDataElements = lvData.split( "\t" );
    }
    else
    {
      lvDataElements = lvData.split( "," );
    }

    if( lvDataElements.size() != FIELDS && lvDataElements.size() != FIELDS - 1 )
    {
      continue;
    }


    lvSpecies = lvDataElements[ SPECIES_FIELD ].remove( QChar( '"' ) ).trimmed();
    if( !cvLocalitySet.contains( lvSpecies ) )
    {
      LocalityCollection lvNewCollection( lvSpecies, theFileName );
      cvLocalitySet[ lvSpecies ] = lvNewCollection;
    }

    lvLocality.isValid = true;
    lvLocality.id = lvCurrentId;
    lvLocality.realWorldCoordinates = QPointF( lvDataElements[ X_FIELD ].toDouble(), lvDataElements[ Y_FIELD ].toDouble() );
    if( lvDataElements.size() == FIELDS - 1 )
    {
      cvLocalitySet[ lvSpecies ].add( lvLocality, "cp" );
    }
    else
    {
      cvLocalitySet[ lvSpecies ].add( lvLocality, lvDataElements[ LOCALITY_TYPE ].remove( QChar( '"' ) ).toLower().trimmed() );
    }

    lvCurrentId++;
  }

  return true;
}

bool SpeciesLocalitySet::save( QString theOutputDirectory, bool singleTypeFormat )
{
  QList< QString > lvKeys = cvLocalitySet.keys();
  QList< QString >::const_iterator lvKeyIterator = lvKeys.begin();


  QFile lvOutputFile( QDir::toNativeSeparators( QString( "%1/calibration_and_test_localities.csv" ) .arg( theOutputDirectory ) ) );
  if( lvOutputFile.exists() )
  {
    cvLastError = QObject::tr( "The file [calibration_and_test_localities.csv] already exists in the target directory" );
    return false;
  }

  if( !lvOutputFile.open( QIODevice::WriteOnly | QIODevice::Text ) )
  {
    cvLastError = QObject::tr( "Unable to write to the target directory [%1]" ) .arg( theOutputDirectory );
    return false;
  }

  QTextStream lvOutStream( &lvOutputFile );
  lvOutStream << "Species,x,y,point_type";
  while( lvKeyIterator != lvKeys.end() )
  {
    LocalityCollection lvCollection = cvLocalitySet[ *lvKeyIterator ];
    LocalityList lvList = lvCollection.testPresenceLocalities();
    Locality lvLocality;
    int lvSize = lvList.size();
    int lvCounter = 0;
    while( lvCounter < lvSize )
    {
      lvLocality = lvList.locality( lvCounter );
      lvOutStream << "\n" << *lvKeyIterator << "," << lvLocality.realWorldCoordinates.x() << "," << lvLocality.realWorldCoordinates.y() << ",tp";
      lvCounter++;
    }

    lvList = lvCollection.testAbsenceLocalities();
    lvSize = lvList.size();
    lvCounter = 0;
    while( lvCounter < lvSize )
    {
      lvLocality = lvList.locality( lvCounter );
      lvOutStream << "\n" << *lvKeyIterator << "," << lvLocality.realWorldCoordinates.x() << "," << lvLocality.realWorldCoordinates.y() << ",ta";
      lvCounter++;
    }

    lvList = lvCollection.calibrationPresenceLocalities();
    lvSize = lvList.size();
    lvCounter = 0;
    while( lvCounter < lvSize )
    {
      lvLocality = lvList.locality( lvCounter );
      lvOutStream << "\n" << *lvKeyIterator << "," << lvLocality.realWorldCoordinates.x() << "," << lvLocality.realWorldCoordinates.y() << ",cp";
      lvCounter++;
    }

    lvList = lvCollection.calibrationAbsenceLocalities();
    lvSize = lvList.size();
    lvCounter = 0;
    while( lvCounter < lvSize )
    {
      lvLocality = lvList.locality( lvCounter );
      lvOutStream << "\n" << *lvKeyIterator << "," << lvLocality.realWorldCoordinates.x() << "," << lvLocality.realWorldCoordinates.y() << ",ca";
      lvCounter++;
    }

    lvKeyIterator++;
  }

  lvOutStream.flush();
  lvOutputFile.close();



  lvKeyIterator = lvKeys.begin();
  if( singleTypeFormat )
  {
    while( lvKeyIterator != lvKeys.end() )
    {
      LocalityCollection lvCollection = cvLocalitySet[ *lvKeyIterator ];
      LocalityList lvList = lvCollection.testPresenceLocalities();
      Locality lvLocality;

      int lvSize = lvList.size();
      int lvCounter = 0;
      if( lvSize > 0 )
      {
        QFile lvOutputFile( QDir::toNativeSeparators( QString( "%1/%2_tp.csv" ) .arg( theOutputDirectory ) .arg( *lvKeyIterator ) ) );
        if( lvOutputFile.exists() )
        {
          cvLastError = QObject::tr( "The file [%1] already exists in the target directory") .arg( lvOutputFile.fileName() );
          return false;
        }

        if( !lvOutputFile.open( QIODevice::WriteOnly | QIODevice::Text ) )
        {
          cvLastError = QObject::tr( "Unable to write to the target directory [%1]" ) .arg( theOutputDirectory );
          return false;
        }

        QTextStream lvOutStream( &lvOutputFile );
        lvOutStream << "Species,x,y";
        while( lvCounter < lvSize )
        {
          lvLocality = lvList.locality( lvCounter );
          lvOutStream << "\n" << *lvKeyIterator << "," << lvLocality.realWorldCoordinates.x() << "," << lvLocality.realWorldCoordinates.y();
          lvCounter++;
        }
        lvOutStream.flush();
        lvOutputFile.close();
      }

      lvList = lvCollection.testAbsenceLocalities();
      lvSize = lvList.size();
      lvCounter = 0;
      if( lvSize > 0 )
      {
        QFile lvOutputFile( QDir::toNativeSeparators( QString( "%1/%2_ta.csv" ) .arg( theOutputDirectory ) .arg( *lvKeyIterator ) ) );
        if( lvOutputFile.exists() )
        {
          cvLastError = QObject::tr( "The file [%1] already exists in the target directory") .arg( lvOutputFile.fileName() );
          return false;
        }

        if( !lvOutputFile.open( QIODevice::WriteOnly | QIODevice::Text ) )
        {
          cvLastError = QObject::tr( "Unable to write to the target directory [%1]" ) .arg( theOutputDirectory );
          return false;
        }

        QTextStream lvOutStream( &lvOutputFile );
        lvOutStream << "Species,x,y";
        while( lvCounter < lvSize )
        {
          lvLocality = lvList.locality( lvCounter );
          lvOutStream << "\n" << *lvKeyIterator << "," << lvLocality.realWorldCoordinates.x() << "," << lvLocality.realWorldCoordinates.y();
          lvCounter++;
        }
        lvOutStream.flush();
        lvOutputFile.close();
      }

      lvList = lvCollection.calibrationPresenceLocalities();
      lvSize = lvList.size();
      lvCounter = 0;
      if( lvSize > 0 )
      {
        QFile lvOutputFile( QDir::toNativeSeparators( QString( "%1/%2_cp.csv" ) .arg( theOutputDirectory ) .arg( *lvKeyIterator ) ) );
        if( lvOutputFile.exists() )
        {
          cvLastError = QObject::tr( "The file [%1] already exists in the target directory") .arg( lvOutputFile.fileName() );
          return false;
        }

        if( !lvOutputFile.open( QIODevice::WriteOnly | QIODevice::Text ) )
        {
          cvLastError = QObject::tr( "Unable to write to the target directory [%1]" ) .arg( theOutputDirectory );
          return false;
        }

        QTextStream lvOutStream( &lvOutputFile );
        lvOutStream << "Species,x,y";
        while( lvCounter < lvSize )
        {
          lvLocality = lvList.locality( lvCounter );
          lvOutStream << "\n" << *lvKeyIterator << "," << lvLocality.realWorldCoordinates.x() << "," << lvLocality.realWorldCoordinates.y();
          lvCounter++;
        }
        lvOutStream.flush();
        lvOutputFile.close();
      }

      lvList = lvCollection.calibrationAbsenceLocalities();
      lvSize = lvList.size();
      lvCounter = 0;
      if( lvSize > 0 )
      {
        QFile lvOutputFile( QDir::toNativeSeparators( QString( "%1/%2_ca.csv" ) .arg( theOutputDirectory ) .arg( *lvKeyIterator ) ) );
        if( lvOutputFile.exists() )
        {
          cvLastError = QObject::tr( "The file [%1] already exists in the target directory") .arg( lvOutputFile.fileName() );
          return false;
        }

        if( !lvOutputFile.open( QIODevice::WriteOnly | QIODevice::Text ) )
        {
          cvLastError = QObject::tr( "Unable to write to the target directory [%1]" ) .arg( theOutputDirectory );
          return false;
        }

        QTextStream lvOutStream( &lvOutputFile );
        lvOutStream << "Species,x,y";
        while( lvCounter < lvSize )
        {
          lvLocality = lvList.locality( lvCounter );
          lvOutStream << "\n" << *lvKeyIterator << "," << lvLocality.realWorldCoordinates.x() << "," << lvLocality.realWorldCoordinates.y();
          lvCounter++;
        }
        lvOutStream.flush();
        lvOutputFile.close();
      }

      lvKeyIterator++;
    }
  }

  return true;
}
