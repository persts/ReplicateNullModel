/*
** Copyright (C) 2009 Peter J. Ersts (ersts at amnh.org)
** Creation Date: 2009-06-09
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
#ifndef REPLICATETEMPLATE_H
#define REPLICATETEMPLATE_H

#include "localitylist.h"
#include "rastermodel.h"

#include "math.h"

#include <QMap>
#include <QSet>
#include <QDebug>
#include <QList>
#include <QPointF>
#include <QObject>

#define EPSILON 0.0000001

#ifdef WIN32
#define M_PI 3.14159265358979323846
#endif

class ReplicateTemplate : public QObject
{
    Q_OBJECT

  public:
    ReplicateTemplate( LocalityList const &theDataCloud, double theCellSize );

    /*
     * Enums, structs, typdefs
     */
    enum Boundary
    {
      North,
      East,
      South,
      West
    };

    struct DataCloudPoint
    {
      QPointF dataPoint;
      double distanceFromCentroid;
    };

    double angleToPoint( QPointF const &theCentroid, double const &theRadius, QPointF const &theIntersection );

    QList< double > anglesToPoints( QPointF const &theCentroid, double const &theRadius );

    const QMap< QString, DataCloudPoint > &dataCloud() { return cvDataCloud; }

    double dataCloudRadius() { return cvDataCloudRadius; }

    double cellSize() { return cvCellSize; }

    bool isValid() { return cvIsValid; }

    /*! \brief Get the lable for the template cell that thePoint is located in */
    QString labelCell( QPointF const &thePoint );

    QString lastError() { return cvLastError; }

    double maximumRotation( QList< double > const &theArcLengths );

    double maximumRotation( QPointF const &theCentroid );

    double maximumRotation( QPointF const &theCentroid, double const &theRadius );

    /*! \brief Get the boundaries for the template cell that contains thePoint */
    QMap<ReplicateTemplate::Boundary, double> templateCellBoundaries( QPointF const &thePoint );

    QPointF templateCentroid() { return cvTemplateCentroid; }

    int templateSize() { return cvTemplateSize; }




  private:
    QMap< QString, DataCloudPoint > cvDataCloud;
    double cvDataCloudRadius;
    double cvCellSize;
    bool cvIsValid;
    QString cvLastError;
    QPointF cvTemplateCentroid;
    int cvTemplateSize;


    /*! \brief Secant.x = theCentroid.x +- deltaXToIntersection( theCentroid, theLatitude ) */
    double deltaXToIntersection( QPointF const &theCentroid, double const &theLatitude, double const &theRadius );

    /*! \brief Secant.y = theCentroid.y +- deltaYToIntersection( theCentroid, theLongitude ) */
    double deltaYToIntersection( QPointF const &theCentroid, double const &theLongitude, double const &theRadius );

    /*! \brief calculate the distance between two points */
    double distance( QPointF const &PointOne, QPointF const &PointTwo );

    QList< QPointF > intersections( QPointF const &theCentroid, double const &theRadius );

    /*! \brief Generate a list of latitudinal boundaries that a circle, of radius r centered on theCentroid, will intersect */
    QList<double> latitudeBoundaries( QPointF const &theCentroid, double const &theRadius );

    /*! \brief Generate a list of longitudinal boundaries that a circle, of radius r centered on theCentroid, will intersect */
    QList<double> longitudeBoundaries( QPointF const &theCentroid, double const &theRadius );

    /*! \brief Transforms the locality list to coordinates relative to the centroid of the data cloud */
    void transformRelativeToCentroid( LocalityList const &theDataCloud );

};
#endif
