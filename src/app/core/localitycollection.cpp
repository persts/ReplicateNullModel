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
#include "localitycollection.h"

#define FIELDS 4
#define SPECIES_FIELD 0
#define X_FIELD 1
#define Y_FIELD 2
#define LOCALITY_TYPE 3

#include <QFile>
#include <QObject>
#include <QTextStream>

LocalityCollection::LocalityCollection()
{
  cvHeaderLabels.clear();
  cvHeaderLabels << "Species" << "x" << "y" << "Locality Type";
  cvSpecies = "UNDEFINED";
  cvLocalitiesFileName = "NOT SET";
  cvLastError = "";
}

LocalityCollection::LocalityCollection( QString theSpecies, QString theLocalitiesFileName )
{
  cvHeaderLabels.clear();
  cvHeaderLabels << "Species" << "x" << "y" << "Locality Type";
  cvSpecies = theSpecies;
  setSpecies( theSpecies );
  cvLocalitiesFileName = theLocalitiesFileName;
  cvLastError = "";
  //load( theFileName );
}

void LocalityCollection::add( Locality theLocality, QString theType )
{
  if( theType.compare( "ta" ) == 0 )
  {
    cvTestAbsenceLocalities.append( theLocality );
  }
  else if( theType.compare( "tp" ) == 0 )
  {
    cvTestPresenceLocalities.append( theLocality );
  }
  else if( theType.compare( "cp" ) == 0 )
  {
    cvCalibrationPresenceLocalities.append( theLocality );
  }
  else if( theType.compare( "ca" ) == 0 )
  {
    cvCalibrationAbsenceLocalities.append( theLocality );
  }
}


void LocalityCollection::clear()
{
  cvTestAbsenceLocalities.clear();
  cvTestPresenceLocalities.clear();
  cvCalibrationAbsenceLocalities.clear();
  cvCalibrationPresenceLocalities.clear();
}

bool LocalityCollection::save( QString theOutputDirectory, bool singleTypeFormat )
{
  return true;
}

void LocalityCollection::setSpecies( QString theSpecies )
{
  cvTestAbsenceLocalities.setSpecies( theSpecies );
  cvTestPresenceLocalities.setSpecies( theSpecies );
  cvCalibrationAbsenceLocalities.setSpecies( theSpecies );
  cvCalibrationPresenceLocalities.setSpecies( theSpecies );
}
