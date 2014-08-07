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
#include "templatetestthread.h"

TemplateTestThread::TemplateTestThread( QString theId, bool showProgress ) : ReplicateNullModelTestThread( theId, showProgress )
{
  cvTemplate = 0;
}

TemplateTestThread::~TemplateTestThread()
{
  cvTestPoints.clear();
  cvTemplate = 0;
}

void TemplateTestThread::setParameters( const QList< QPointF > &theList, ReplicateTemplate* const theTemplate )
{
  cvTestPoints.clear();
  cvTestPoints = theList;
  cvTemplate = theTemplate;
}

void TemplateTestThread::run()
{
  cvDistinctReplicates.clear();

  int lvListSize = cvTestPoints.size();

  QMapIterator< QString, ReplicateTemplate::DataCloudPoint > lvDataCloudIterator( cvTemplate->dataCloud() );
  QMapIterator< QString, ReplicateTemplate::DataCloudPoint > lvDataCloudInnerIterator( cvTemplate->dataCloud() );
  for( int lvIterator = 0; lvIterator < lvListSize; lvIterator++ )
  {
    if( cvAbort ) { return; }

    //Get all of the angles to intersections for each radius
    lvDataCloudIterator.toFront();
    QMap< double, QList< double > > lvIntersectionMap;
    while( lvDataCloudIterator.hasNext() )
    {
      if( cvAbort ) { return; }

      lvDataCloudIterator.next();
      if( !lvIntersectionMap.contains( lvDataCloudIterator.value().distanceFromCentroid ) )
      {
        lvIntersectionMap[ lvDataCloudIterator.value().distanceFromCentroid ] = cvTemplate->anglesToPoints( cvTestPoints[ lvIterator ], lvDataCloudIterator.value().distanceFromCentroid );
      }
    }

    //For each point in the data cloud
    double lvWholeNumber;
    double lvCurrentOffset;
    double lvSplitTheDifference;
    QPointF lvNewPoint;
    QString lvTemplateMapElement;

    QMatrix lvTranslateMatrix;
    lvTranslateMatrix.translate( cvTestPoints[ lvIterator ].x(), cvTestPoints[ lvIterator ].y() );

    lvDataCloudIterator.toFront();
    while( lvDataCloudIterator.hasNext() )
    {
      if( cvAbort ) { return; }

      lvDataCloudIterator.next();
      //don't try to compute the rotation for any point that is also the centroid of the data cloud
      if( fabs( 0.0 - lvDataCloudIterator.value().dataPoint.x() ) > EPSILON || fabs( 0.0 - lvDataCloudIterator.value().dataPoint.y() ) > EPSILON )
      {
        //Compute angle to get the data cloud point to sit on the radius at 0 degrees
        lvCurrentOffset = 360.0 * modf( cvTemplate->angleToPoint( QPointF( 0.0, 0.0 ), lvDataCloudIterator.value().distanceFromCentroid, lvDataCloudIterator.value().dataPoint ) / 360.0, &lvWholeNumber );

        //For each point the data cloud apply the transformation and rotations specific for the current ( outter iterator ) point
        QList< double > lvAnglesToIntersections = lvIntersectionMap[ lvDataCloudIterator.value().distanceFromCentroid ];
        if( lvAnglesToIntersections.size() > 1 )
        {
          //For each of the intersections that exists for the current ( outter iterator ) point's radius
          for( int lvAngleRunner = 1; lvAngleRunner < lvAnglesToIntersections.size(); lvAngleRunner++ )
          {
            lvTemplateMapElement = "";

            //Get the angle that will put you mid way between the two intersection points
            lvSplitTheDifference = lvAnglesToIntersections[ lvAngleRunner - 1 ] + (( lvAnglesToIntersections[ lvAngleRunner ] - lvAnglesToIntersections[ lvAngleRunner - 1 ] ) / 2.0 );

            QMatrix lvRotationMatrix;
            lvRotationMatrix.rotate( lvSplitTheDifference - lvCurrentOffset );
            lvDataCloudInnerIterator.toFront();
            while( lvDataCloudInnerIterator.hasNext() )
            {
              if( cvAbort ) { return; }

              lvDataCloudInnerIterator.next();
              lvNewPoint = lvRotationMatrix.map( lvDataCloudInnerIterator.value().dataPoint );
              lvNewPoint = lvTranslateMatrix.map( lvNewPoint );
              lvTemplateMapElement += lvDataCloudInnerIterator.key() + "-" + cvTemplate->labelCell( lvNewPoint ) + "|";
            }
            lvTemplateMapElement.chop( 1 );
            cvDistinctReplicates << lvTemplateMapElement;
          }
        }
        else if( 0 == lvAnglesToIntersections.size() && cvTemplate->dataCloudRadius() < cvTemplate->cellSize() )
        {
          //Rotate by 1 degree for kicks?
        }
      }
    }
    progress( lvIterator, lvListSize - 1 );
  }

  finished( cvId );
}
