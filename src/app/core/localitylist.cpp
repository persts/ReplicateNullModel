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
#include "localitylist.h"

#include <limits>

LocalityList::LocalityList()
{
  cvSpecies = "UNDEFINED";
  cvFileName = "UNDEFINED";
  cvXMaximum = -1 * std::numeric_limits< double >::max();
  cvXMinimum = std::numeric_limits< double >::max();
  cvYMaximum = -1 * std::numeric_limits< double >::max();
  cvYMinimum = std::numeric_limits< double >::max();
}

LocalityList::LocalityList( QString theSpecies, QString theFileName )
{
  cvSpecies = theSpecies;
  cvFileName = theFileName;
  cvXMaximum = -1 * std::numeric_limits< double >::max();
  cvXMinimum = std::numeric_limits< double >::max();
  cvYMaximum = -1 * std::numeric_limits< double >::max();
  cvYMinimum = std::numeric_limits< double >::max();
}

void LocalityList::append( Locality theLocality )
{
  cvXMaximum = qMax( cvXMaximum, theLocality.realWorldCoordinates.x() );
  cvXMinimum = qMin( cvXMinimum, theLocality.realWorldCoordinates.x() );
  cvYMaximum = qMax( cvYMaximum, theLocality.realWorldCoordinates.y() );
  cvYMinimum = qMin( cvYMinimum, theLocality.realWorldCoordinates.y() );

  cvCentroid.setX( cvXMaximum - (( cvXMaximum - cvXMinimum ) / 2.0 ) );
  cvCentroid.setY( cvYMaximum - (( cvYMaximum - cvYMinimum ) / 2.0 ) );

  cvLocalityList.append( theLocality );
}

void LocalityList::append( int theId, double x, double y )
{
  Locality lvLocality;
  lvLocality.id = theId;
  lvLocality.isValid = true;
  lvLocality.realWorldCoordinates = QPointF( x, y );

  cvXMaximum = qMax( cvXMaximum, x );
  cvXMinimum = qMin( cvXMinimum, x );
  cvYMaximum = qMax( cvYMaximum, y );
  cvYMinimum = qMin( cvYMinimum, y );

  cvCentroid.setX( cvXMaximum - (( cvXMaximum - cvXMinimum ) / 2.0 ) );
  cvCentroid.setY( cvYMaximum - (( cvYMaximum - cvYMinimum ) / 2.0 ) );

  cvLocalityList.append( lvLocality );
}

void LocalityList::clear()
{
  cvLocalityList.clear();
  cvXMaximum = -1 * std::numeric_limits< double >::max();
  cvXMinimum = std::numeric_limits< double >::max();
  cvYMaximum = -1 * std::numeric_limits< double >::max();
  cvYMinimum = std::numeric_limits< double >::max();
}

void  LocalityList::insert( int theIndex, Locality theLocality )
{
  if( 0 > theIndex )
  {
    return;
  }

  if( theIndex > cvLocalityList.size() )
  {
    cvLocalityList.append( theLocality );
  }
  else
  {
    cvLocalityList.insert( theIndex, theLocality );
  }
  //TODO: Updated XYMinMax
}

const Locality LocalityList::locality( int theIndex ) const
{
  Locality lvEmpty;
  if( size() == 0 || theIndex <  0 || theIndex > size() )
  {
    return lvEmpty;
  }

  return cvLocalityList[ theIndex ];
}

Locality LocalityList::take( int theIndex )
{
  Locality lvEmpty;
  if( size() == 0 || theIndex < 0 || theIndex > size() )
  {
    return lvEmpty;
  }

  return cvLocalityList.takeAt( theIndex );
  //TODO: Updated XYMinMax
}

