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
#ifndef LOCALITYLIST_H
#define LOCALITYLIST_H

#include "locality.h"

#include <QList>
#include <QPointF>
#include <QString>

/*
 * This class provides a container for a series if like localities
 */
class LocalityList
{

  public:
    /** \brief constructor */
    LocalityList();

    /** \brief constructor */
    LocalityList( QString, QString );

    void append( Locality theLocality );

    /** \brief Add a new locality to the end of the list, take an id, x (real world), y(real world) as parameters  */
    void append( int, double, double );

    /** \brief Return the geographic mean of the localities in the list */
    QPointF centroid() const { return cvCentroid; }

    /** \brief Clears the list of localities */
    void clear();

    /** \brief Returns the file name for the original point list */
    QString fileName() { return cvFileName; }

    /** \brief Inserts a locality at the specific position */
    void  insert( int, Locality );

    /** \brief Returns the locality at a list position */
    const Locality locality( int ) const;

    double maximumX() { return cvXMaximum; }
    double maximumY() { return cvYMaximum; }
    double minimumX() { return cvXMinimum; }
    double minimumY() { return cvYMinimum; }

    /** \brief Sets the file name to the original point list */
    void setFileName( QString theFileName ) { cvFileName = theFileName; }

    void setSpecies( QString theSpecies ) { cvSpecies = theSpecies; }

    /** \brief Return the size of the list, same as the count of elements */
    int size() const { return cvLocalityList.size(); }

    QString species() { return cvSpecies; }

    /** \brief Removes a locality from the list and returns a copy */
    Locality take( int );

  private:
    QString cvSpecies;
    QPointF cvCentroid;
    QString cvFileName;
    QList<Locality> cvLocalityList;
    double cvXMaximum;
    double cvXMinimum;
    double cvYMaximum;
    double cvYMinimum;
};
#endif
