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
#include "replicatetemplate.h"

#include "mtrand.h"

#include <QMatrix>
#include <QStringList>


ReplicateTemplate::ReplicateTemplate( LocalityList const &theDataCloud, double theCellSize )
{
  cvIsValid = false;
  cvCellSize = 0.0;
  cvDataCloudRadius = 0.0;

  //Transform data cloud from real world to template coordinates
  transformRelativeToCentroid( theDataCloud );

  if( 1 >= theDataCloud.size() )
  {
    cvLastError = tr( "The presence locality list must contain two or more points" );
    return ;
  }

  if( 1 > cvDataCloudRadius )
  {
    cvLastError = tr( "The replicate template requires the data cloud radius to be greater than 1.0" );
    return ;
  }

  if( 1 > theCellSize )
  {
    cvLastError = tr( "The cell size of the raster model has to be greater than 1" );
    return ;
  }
  cvCellSize = theCellSize;

  //Calculate the dimensions of the template
  double lvWholeNumber;
  modf( cvDataCloudRadius / theCellSize, &lvWholeNumber );
  cvTemplateSize = 1 + ( 2 * (( int ) lvWholeNumber + 1 ) );
  cvTemplateCentroid.setX(( double )(( cvTemplateSize / 2.0 ) * cvCellSize ) );
  cvTemplateCentroid.setY( cvTemplateCentroid.x() );
  cvIsValid = true;
}

/*
 *
 * PUBLIC FUNCTIONS
 *
 */

double ReplicateTemplate::angleToPoint( QPointF const &theCentroid, double const &theRadius, QPointF const &theIntersection )
{
  //TODO: add a isValid flag? Should check to see if theIntersection is theRadius away from theCentroid
  double lvDeltaX = theIntersection.x() - theCentroid.x();
  double lvDeltaY = theIntersection.y() - theCentroid.y();
  double lvAngle = 0.0;
  if( lvDeltaX < 0.0 && lvDeltaY >= 0.0 ) //UL QUAD
  {
    if( lvDeltaY == 0.0 )
    {
      lvAngle = 90.0;
    }
    else
    {
      lvAngle = fabs( asin( lvDeltaX / theRadius ) * ( 180.0 / M_PI ) );
    }
  }
  else if( lvDeltaX < 0.0 && lvDeltaY < 0.0 ) //LL QUAD
  {
    lvAngle = 180.0 - fabs(( asin( lvDeltaX / theRadius ) * ( 180.0 / M_PI ) ) );
  }
  else if( lvDeltaX >= 0.0 && lvDeltaY < 0.0 ) //LR QUAD
  {
    if( lvDeltaX == 0.0 )
    {
      lvAngle = 180.0;
    }
    else
    {
      lvAngle = 180.0 + fabs(( asin( lvDeltaX / theRadius ) * ( 180.0 / M_PI ) ) );
    }
  }
  else if( lvDeltaX >= 0.0 && lvDeltaY >= 0.0 ) //UR QUAD
  {
    if( lvDeltaY == 0.0 )
    {
      lvAngle = 270.0;
    }
    else
    {
      lvAngle = 360.0 - fabs(( asin( lvDeltaX / theRadius ) * ( 180.0 / M_PI ) ) );
    }
  }

  return lvAngle;
}

QList< double > ReplicateTemplate::anglesToPoints( QPointF const &theCentroid, double const &theRadius )
{
  QList<QPointF> lvIntersections = intersections( theCentroid, theRadius );

  QList< double > lvAngles;
  for( int lvIterator = 0; lvIterator < lvIntersections.size(); lvIterator++ )
  {
    lvAngles << angleToPoint( theCentroid, theRadius, lvIntersections[ lvIterator ] );
  }

  qSort( lvAngles );

  return lvAngles;
}

QString ReplicateTemplate::labelCell( QPointF const &thePoint )
{
  if( thePoint.x() < 0.0 ||
      thePoint.x() > ( cvTemplateSize * cvCellSize ) ||
      thePoint.y()  < 0.0 ||
      thePoint.y() > ( cvTemplateSize * cvCellSize ) )
  {
    return "invalid_invalid";
  }

  QString lvLabel = "";
  double theWholeNumber;
  modf( thePoint.x() / cvCellSize, &theWholeNumber );
  lvLabel += QString::number(( int ) theWholeNumber ) + "_";
  modf( thePoint.y() / cvCellSize, &theWholeNumber );
  lvLabel += QString::number(( int ) theWholeNumber );

  return lvLabel;
}

double ReplicateTemplate::maximumRotation( QList< double > const &theAngleList )
{
  double lvMaxRotation = 360.0;
  if( 1 == theAngleList.size() )
  {
    lvMaxRotation = theAngleList[ 0 ];
  }
  else
  {
    for( int lvIterator = 1; lvIterator < theAngleList.size(); lvIterator++ )
    {
      if( theAngleList[ lvIterator ] - theAngleList[ lvIterator - 1 ] > EPSILON )
      {
        lvMaxRotation = qMin( lvMaxRotation, theAngleList[ lvIterator ] - theAngleList[ lvIterator - 1 ] );
      }
    }

    lvMaxRotation = qMin( lvMaxRotation, ( 360.0 + theAngleList[ 0 ] ) - theAngleList[ theAngleList.size() - 1 ] );
  }

  return lvMaxRotation;
}

double ReplicateTemplate::maximumRotation( QPointF const &theCentroid )
{
  QMap< QString, ReplicateTemplate::DataCloudPoint >::const_iterator lvIterator;
  double lvMinimum = HUGE;

  lvIterator = cvDataCloud.begin();
  while( lvIterator != cvDataCloud.end() )
  {
    lvMinimum = qMin( lvMinimum, maximumRotation( theCentroid, lvIterator.value().distanceFromCentroid ) );
    lvIterator++;
  }

  return lvMinimum;
}

double ReplicateTemplate::maximumRotation( QPointF const &theCentroid, double const &theRadius )
{
  QList<QPointF> lvIntersections = intersections( theCentroid, theRadius );

  QPointF lvPointOne;
  QPointF lvPointTwo;
  bool lvSolution = false;
  double lvDistance = 0.0;
  double lvMinimumDistance = 1.0e200;
  for( int lvIterator = 0; lvIterator < lvIntersections.size(); lvIterator++ )
  {
    for( int lvListRunner = lvIterator + 1; lvListRunner < lvIntersections.size(); lvListRunner++ )
    {

      lvDistance = distance( lvIntersections[ lvIterator ], lvIntersections[ lvListRunner ] );
      if( lvDistance > EPSILON && lvDistance < lvMinimumDistance )
      {
        lvSolution = true;
        lvMinimumDistance = lvDistance;
        lvPointOne = lvIntersections[ lvIterator ];
        lvPointTwo = lvIntersections[ lvListRunner ];
      }
    }
  }

  double lvMaxRotationStep = ( asin(( lvMinimumDistance / 2.0 ) / theRadius )  * ( 180 / M_PI ) ) * 2.0;
  if( false == lvSolution || ( theRadius < cvCellSize && ( fabs( 90.0 - lvMaxRotationStep ) < EPSILON ) ) )
  {
    lvMaxRotationStep = 360.0;
  }

  return  lvMaxRotationStep;
}

QMap<ReplicateTemplate::Boundary, double> ReplicateTemplate::templateCellBoundaries( QPointF const &thePoint )
{
  QMap<ReplicateTemplate::Boundary, double> lvBoundaryMap;
  if( thePoint.x() < 0 || thePoint.y() < 0 ) { return lvBoundaryMap; }

  double theWholeNumber;
  modf( thePoint.y() / cvCellSize, &theWholeNumber );
  lvBoundaryMap[ South ] = theWholeNumber * cvCellSize;
  lvBoundaryMap[  North ] = ( theWholeNumber + 1 ) * cvCellSize;

  modf( thePoint.x() / cvCellSize, &theWholeNumber );
  lvBoundaryMap[ West ] = theWholeNumber * cvCellSize;
  lvBoundaryMap[ East ] = ( theWholeNumber + 1 ) * cvCellSize;

  return lvBoundaryMap;
}

/*
 *
 * PRIVATE FUNCTIONS
 *
 */
double ReplicateTemplate::deltaXToIntersection( QPointF const &theCentroid, double const &theLatitude, double const &theRadius )
{
  //TODO: This should have error checking
  return sqrt( pow( theRadius, 2 ) - pow( theLatitude - theCentroid.y(), 2 ) );
}

double ReplicateTemplate::deltaYToIntersection( QPointF const &theCentroid, double const &theLongitude, double const &theRadius )
{
  //TODO: This should have error checking -- check if Longitude is within radius from centroid
  return sqrt( pow( theRadius, 2 ) - pow( theLongitude - theCentroid.x(), 2 ) );
}

double ReplicateTemplate::distance( QPointF const &PointOne, QPointF const &PointTwo )
{
  return sqrt( pow( PointOne.x() - PointTwo.x(), 2 ) + pow( PointOne.y() - PointTwo.y(), 2 ) );
}

QList< QPointF > ReplicateTemplate::intersections( QPointF const &theCentroid, double const &theRadius )
{
  double lvDelta;
  QList<QPointF> lvIntersections;
  QList<double> lvLongitudes = longitudeBoundaries( theCentroid, theRadius );
  for( int lvIterator = 0; lvIterator < lvLongitudes.size(); lvIterator++ )
  {
    lvDelta = deltaYToIntersection( theCentroid, lvLongitudes[ lvIterator ], theRadius );
    lvIntersections << QPointF( lvLongitudes[ lvIterator ], theCentroid.y() + lvDelta );
    lvIntersections << QPointF( lvLongitudes[ lvIterator ], theCentroid.y() - lvDelta );
  }

  QList<double> lvLatitudes = latitudeBoundaries( theCentroid, theRadius );
  for( int lvIterator = 0; lvIterator < lvLatitudes.size(); lvIterator++ )
  {
    lvDelta = deltaXToIntersection( theCentroid, lvLatitudes[ lvIterator ], theRadius );
    lvIntersections << QPointF( theCentroid.x() + lvDelta, lvLatitudes[ lvIterator ] );
    lvIntersections << QPointF( theCentroid.x() - lvDelta, lvLatitudes[ lvIterator ] );
  }

  return lvIntersections;
}

QList<double> ReplicateTemplate::latitudeBoundaries( QPointF const &theCentroid, double const &theRadius )
{
  QList<double> lvLatitudes;
  double lvWholeNumber;

  if( 0 >= theRadius || theRadius > theRadius ) { return lvLatitudes; }

  modf(( theCentroid.y() - theRadius ) / cvCellSize, &lvWholeNumber );
  int lvSouthernMost = ( int ) lvWholeNumber * cvCellSize;

  modf(( theCentroid.y() + theRadius ) / cvCellSize, &lvWholeNumber );
  int lvNorthernMost = ( int ) lvWholeNumber * cvCellSize;

  //add cell boundaries
  for( int lvIterator = lvSouthernMost; lvIterator <= lvNorthernMost; lvIterator += cvCellSize )
  {
    //Need to check if the bottom boundary is actually
    if( lvIterator == lvSouthernMost )
    {
      //This is a special case when radius < cellsize
      if( theRadius < distance( theCentroid, QPoint( theCentroid.x(), lvIterator ) ) )
      {
        continue;
      }
    }

    lvLatitudes << lvIterator;
  }

  return lvLatitudes;
}

QList<double> ReplicateTemplate::longitudeBoundaries( QPointF const &theCentroid, double const &theRadius )
{
  QList<double> lvLongitudes;
  double lvWholeNumber;

  if( 0 >= theRadius || theRadius > theRadius ) { return lvLongitudes; }

  modf(( theCentroid.x() - theRadius ) / cvCellSize, &lvWholeNumber );
  int lvWesternMost = ( int ) lvWholeNumber * cvCellSize;

  modf(( theCentroid.x() + theRadius ) / cvCellSize, &lvWholeNumber );
  int lvEasternMost = ( int ) lvWholeNumber * cvCellSize;

  //add cell boundaries
  for( int lvIterator = lvWesternMost; lvIterator <= lvEasternMost; lvIterator += cvCellSize )
  {
    //TODO: Shouldn't thus check be for all????
    //Need to check if the left boundary is actually
    if( lvIterator == lvWesternMost )
    {
      //This is a special case when radius < cellsize
      if( theRadius < distance( theCentroid, QPoint( lvIterator, theCentroid.y() ) ) )
      {
        continue;
      }
    }

    lvLongitudes << lvIterator;
  }

  return lvLongitudes;
}

void ReplicateTemplate::transformRelativeToCentroid( LocalityList const &theDataCloud )
{
  cvDataCloud.clear();
  cvDataCloudRadius = 0.0;

  QPointF lvCentroid = theDataCloud.centroid();

  Locality lvLocality;
  ReplicateTemplate::DataCloudPoint lvNewPoint;
  for( int lvIterator = 0; lvIterator < theDataCloud.size(); lvIterator++ )
  {
    lvLocality = theDataCloud.locality( lvIterator );
    lvNewPoint.dataPoint.setX( lvLocality.realWorldCoordinates.x() - lvCentroid.x() );
    lvNewPoint.dataPoint.setY( lvLocality.realWorldCoordinates.y() - lvCentroid.y() );
    lvNewPoint.distanceFromCentroid = distance( QPointF( 0.0, 0.0 ), lvNewPoint.dataPoint );
    cvDataCloudRadius = qMax( cvDataCloudRadius, lvNewPoint.distanceFromCentroid );
    cvDataCloud[ "P"+ QString::number( lvLocality.id )] = lvNewPoint;
  }
}
