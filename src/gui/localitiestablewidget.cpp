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
#include "localitiestablewidget.h"

LocalitiesTableWidget::LocalitiesTableWidget( QWidget* theParent ) : TableWidget( theParent )
{
}

void LocalitiesTableWidget::loadLocalities( LocalityCollection theLocalityCollection )
{

  clear();
  setRowCount( 0 );
  setColumnCount( theLocalityCollection.headerLabels().size() );
  setHorizontalHeaderLabels( theLocalityCollection.headerLabels() );

  int lvTotalRowCount = 0;
  Locality lvLocality;
  QTableWidgetItem *lvNewItem;
  QString lvSpecies = theLocalityCollection.species();
  for( int lvLocalityIterator = 0; lvLocalityIterator < theLocalityCollection.testPresenceLocalities().size(); lvLocalityIterator++ )
  {
    insertRow( lvTotalRowCount );
    lvLocality = theLocalityCollection.testPresenceLocalities().locality( lvLocalityIterator );
    lvNewItem = new QTableWidgetItem( lvSpecies );
    lvNewItem->setFlags( Qt::NoItemFlags );
    setItem( lvTotalRowCount, 0, lvNewItem );

    lvNewItem = new QTableWidgetItem( QString::number( lvLocality.realWorldCoordinates.x(), 'f', 8 ) );
    lvNewItem->setFlags( Qt::NoItemFlags );
    setItem( lvTotalRowCount, 1, lvNewItem );

    lvNewItem = new QTableWidgetItem( QString::number( lvLocality.realWorldCoordinates.y(), 'f', 8 ) );
    lvNewItem->setFlags( Qt::NoItemFlags );
    setItem( lvTotalRowCount, 2, lvNewItem );

    lvNewItem = new QTableWidgetItem( "Test Presence" );
    lvNewItem->setFlags( Qt::NoItemFlags );
    setItem( lvTotalRowCount, 3, lvNewItem );

    lvTotalRowCount++;
  }

  for( int lvLocalityIterator = 0; lvLocalityIterator < theLocalityCollection.testAbsenceLocalities().size(); lvLocalityIterator++ )
  {
    insertRow( lvTotalRowCount );
    lvLocality = theLocalityCollection.testAbsenceLocalities().locality( lvLocalityIterator );
    lvNewItem = new QTableWidgetItem( lvSpecies );
    lvNewItem->setFlags( Qt::NoItemFlags );
    setItem( lvTotalRowCount, 0, lvNewItem );

    lvNewItem = new QTableWidgetItem( QString::number( lvLocality.realWorldCoordinates.x(), 'f', 8 ) );
    lvNewItem->setFlags( Qt::NoItemFlags );
    setItem( lvTotalRowCount, 1, lvNewItem );

    lvNewItem = new QTableWidgetItem( QString::number( lvLocality.realWorldCoordinates.y(), 'f', 8 ) );
    lvNewItem->setFlags( Qt::NoItemFlags );
    setItem( lvTotalRowCount, 2, lvNewItem );

    lvNewItem = new QTableWidgetItem( "Test Absence" );
    lvNewItem->setFlags( Qt::NoItemFlags );
    setItem( lvTotalRowCount, 3, lvNewItem );

    lvTotalRowCount++;
  }

  for( int lvLocalityIterator = 0; lvLocalityIterator < theLocalityCollection.calibrationPresenceLocalities().size(); lvLocalityIterator++ )
  {
    insertRow( lvTotalRowCount );
    lvLocality = theLocalityCollection.calibrationPresenceLocalities().locality( lvLocalityIterator );
    lvNewItem = new QTableWidgetItem( lvSpecies );
    lvNewItem->setFlags( Qt::NoItemFlags );
    setItem( lvTotalRowCount, 0, lvNewItem );

    lvNewItem = new QTableWidgetItem( QString::number( lvLocality.realWorldCoordinates.x(), 'f', 8 ) );
    lvNewItem->setFlags( Qt::NoItemFlags );
    setItem( lvTotalRowCount, 1, lvNewItem );

    lvNewItem = new QTableWidgetItem( QString::number( lvLocality.realWorldCoordinates.y(), 'f', 8 ) );
    lvNewItem->setFlags( Qt::NoItemFlags );
    setItem( lvTotalRowCount, 2, lvNewItem );

    lvNewItem = new QTableWidgetItem( "Calibration Presence" );
    lvNewItem->setFlags( Qt::NoItemFlags );
    setItem( lvTotalRowCount, 3, lvNewItem );

    lvTotalRowCount++;
  }

  for( int lvLocalityIterator = 0; lvLocalityIterator < theLocalityCollection.calibrationAbsenceLocalities().size(); lvLocalityIterator++ )
  {
    insertRow( lvTotalRowCount );
    lvLocality = theLocalityCollection.calibrationAbsenceLocalities().locality( lvLocalityIterator );
    lvNewItem = new QTableWidgetItem( lvSpecies );
    lvNewItem->setFlags( Qt::NoItemFlags );
    setItem( lvTotalRowCount, 0, lvNewItem );

    lvNewItem = new QTableWidgetItem( QString::number( lvLocality.realWorldCoordinates.x(), 'f', 8 ) );
    lvNewItem->setFlags( Qt::NoItemFlags );
    setItem( lvTotalRowCount, 1, lvNewItem );

    lvNewItem = new QTableWidgetItem( QString::number( lvLocality.realWorldCoordinates.y(), 'f', 8 ) );
    lvNewItem->setFlags( Qt::NoItemFlags );
    setItem( lvTotalRowCount, 2, lvNewItem );

    lvNewItem = new QTableWidgetItem( "Calibration Absence" );
    lvNewItem->setFlags( Qt::NoItemFlags );
    setItem( lvTotalRowCount, 3, lvNewItem );

    lvTotalRowCount++;
  }
}
