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
#include "tablewidget.h"

#include <QMimeData>
#include <QSettings>
#include <QFileInfo>
#include <QFileDialog>
#include <QTextStream>
#include <QStringList>

TableWidget::TableWidget( QWidget* theParent ) : QTableWidget( DEFAULT_NUMBER_OF_ROWS, DEFAULT_NUMBER_OF_COLUMNS, theParent )
{
  cvMaximumFields = 0;
  cvMinimumFields = 0;

  cvAcceptDoubleClick = true;
  setAcceptDrops( true );
  setDragDropMode( QAbstractItemView::DropOnly );
}

void TableWidget::dragEnterEvent( QDragEnterEvent* theEvent )
{
  theEvent->acceptProposedAction();
}

void TableWidget::dragMoveEvent( QDragMoveEvent* theEvent )
{
  theEvent->accept();
}

void TableWidget::dropEvent( QDropEvent* theEvent )
{
  //TODO: Update for Microsoft and Mac?
  if( theEvent->mimeData()->text().startsWith( "file://", Qt::CaseInsensitive ) )
  {
    emit fileSelected( theEvent->mimeData()->text().remove( "file://", Qt::CaseInsensitive ).trimmed() );
  }
}

bool TableWidget::load( QString theFileName )
{
  QFile lvFile( theFileName );
  if( !lvFile.open( QIODevice::ReadOnly | QIODevice::Text ) )
  {
    emit consoleMessage( tr( "Unable to open file [ %1 ]" ) .arg( theFileName ) );
    return false;
  }

  QString lvData;
  QTextStream lvInputStream( &lvFile );

  /*
   * Read first line, assume headers exist
   */
  lvData = lvInputStream.readLine();
  int lvTabCount = lvData.count( "\t" );
  int lvCommaCount = lvData.count( "," );

  if( 0 == lvTabCount && 0 == lvCommaCount )
  {
    emit consoleMessage( tr( "Input data must be tab or comma delimited" ) );
    return false;
  }

  QString lvDelimiter = "\t";
  int lvTotalEntries = lvTabCount + 1;
  if( 0 == lvTabCount )
  {
    lvDelimiter = ',';
    lvTotalEntries = lvCommaCount + 1;
  }

  if( lvTotalEntries < cvMinimumFields )
  {
    consoleMessage( tr( "A minimum of %1 fields are expected" ) .arg( cvMinimumFields ) );
    return false;
  }

  if( 0 != cvMaximumFields && lvTotalEntries > cvMaximumFields )
  {
    consoleMessage( tr( "A maximum of %1 fields are allowed" ) .arg( cvMaximumFields ) );
    return false;
  }

  clear();
  setRowCount( 0 );
  setColumnCount( lvTotalEntries );
  setHorizontalHeaderLabels( lvData.split( lvDelimiter ) );

  int lvTotalRowCount = 0;
  QTableWidgetItem *lvNewItem;
  while( !lvInputStream.atEnd() )
  {
    lvData = lvInputStream.readLine();
    QStringList lvDataElements = lvData.split( lvDelimiter );
    if( lvDataElements.size() != lvTotalEntries )
    {
      continue;
      consoleMessage( tr( "Skipping invalid input [ %1 ]" ) .arg( lvData ) );
    }

    insertRow( lvTotalRowCount );
    for( int lvIterator = 0; lvIterator < lvTotalEntries; lvIterator++ )
    {
      lvNewItem = new QTableWidgetItem( lvDataElements.at( lvIterator ) );
      lvNewItem->setFlags( Qt::NoItemFlags );
      setItem( lvTotalRowCount, lvIterator, lvNewItem );
    }

    lvTotalRowCount++;
  }

}

void TableWidget::setMaximumFields( int theMaximum )
{
  cvMaximumFields = theMaximum;
  reInitialize();
}

void TableWidget::setMinimumMaximumFields( int theMinimum, int theMaximum )
{
  cvMinimumFields = theMinimum;
  cvMaximumFields = theMaximum;
  reInitialize();
}

void TableWidget::mouseDoubleClickEvent( QMouseEvent* theEvent )
{
  if( !cvAcceptDoubleClick ) { return; }

  QSettings lvQSettings;
  QString lvFileName = QFileDialog::getOpenFileName( this, tr( "Open input file" ), lvQSettings.value( "lastInputDirectory", "" ).toString(), tr( "Text file ( *.txt *.csv )" ) );
  QFileInfo lvFileInfo( lvFileName );

  if( lvFileName.isEmpty() )
  {
    return;
  }

  lvQSettings.setValue( "lastInputDirectory", lvFileInfo.absoluteDir().absolutePath() );
  emit fileSelected( lvFileName );
  load( lvFileName );
}

void TableWidget::reInitialize()
{
  clear();
  setRowCount( DEFAULT_NUMBER_OF_ROWS );
  0 == cvMaximumFields ? setColumnCount( DEFAULT_NUMBER_OF_COLUMNS ) : setColumnCount( cvMaximumFields );
}

