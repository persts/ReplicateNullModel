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
#ifndef LOCALITYCOLLECTION_H
#define LOCALITYCOLLECTION_H

#include "localitylist.h"
#include "locality.h"

#include <QStringList>

/*
 * This a bit of a convenience class that that holds all four list of locality types
 */
class LocalityCollection
{

  public:

    /** \brief Constuctor */
    LocalityCollection();

    /** \brief Constuctor */
    LocalityCollection( QString theSpecies, QString theLocalitiesFileName = "NOT SET" );

    void add( Locality, QString );

    /** \brief Accessor for the calibration-absence localities list */
    LocalityList calibrationAbsenceLocalities() { return cvCalibrationAbsenceLocalities; }

    /** \brief Accessor for the calibration-presence localities list */
    LocalityList calibrationPresenceLocalities() { return cvCalibrationPresenceLocalities; }

    /** \brief Clear the collection */
    void clear();

    /** \brief The header fields from the localities file */
    const QStringList &headerLabels() { return cvHeaderLabels; }

    /** \brief Returns the last error message */
    const QString &lastError() { return cvLastError; }

    /** \brief Returns File name of the currently located loalites */
    const QString &localitiesFileName() { return cvLocalitiesFileName; }

    bool save( QString theOutputDirectory, bool singleTypeFormat = false );

    /** \brief Set the species name of the collection*/
    void setSpecies( QString theSpecies );

    QString species() { return cvSpecies; }

    /** \brief Accessor for the absence localities list */
    LocalityList testAbsenceLocalities() { return cvTestAbsenceLocalities; }

    /** \brief Accessor for the presence localties list */
    LocalityList testPresenceLocalities() { return cvTestPresenceLocalities; }



  private:
    QString cvLocalitiesFileName;

    LocalityList cvTestAbsenceLocalities;
    LocalityList cvTestPresenceLocalities;
    LocalityList cvCalibrationAbsenceLocalities;
    LocalityList cvCalibrationPresenceLocalities;

    QString cvLastError;
    QStringList cvHeaderLabels;
    QString cvSpecies;
};
#endif
