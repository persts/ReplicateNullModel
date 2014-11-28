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
              lvTemplateMapElement += lvDataCloudInnerIterator.key() + "-" + cvTemplate->labelCell( lvNewPoint ) + "|"; /* */
            }
            lvTemplateMapElement.chop( 1 );
            cvDistinctReplicates << lvTemplateMapElement; /**/
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
