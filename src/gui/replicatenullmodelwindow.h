/*
** Copyright (C) 2009 Peter J. Ersts (ersts at amnh.org)
** Creation Date: 2009-06-03
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
#ifndef REPLICATENULLMODELWINDOW_H
#define REPLICATENULLMODELWINDOW_H

#include "replicatenullmodelconfigdialog.h"
#include "replicatenullmodeltest.h"
#include "specieslocalityset.h"
#include "rastermodel.h"

#include <QTextEdit>
#include <QMainWindow>
#include <QDockWidget>

namespace Ui
{
  class ReplicateNullModelWindow;
}

class ReplicateNullModelWindow : public QMainWindow
{
    Q_OBJECT

  public:
    ReplicateNullModelWindow( QMainWindow* theParent = 0, Qt::WindowFlags theFlags = 0 );
    ~ReplicateNullModelWindow();

  private slots:
    void consoleMessage( QString theMessage );
    void localityFileSelected( QString );
    void on_pbtnAbout_clicked();
    void on_pbtnCancel_clicked();
    void on_pbtnConfigure_clicked();
    void on_pbtnRun_clicked();
    void rasterFileSelected( QString theFilename );
    void testComplete( bool );

  private:
    void displayLocalityList( LocalityList, Qt::GlobalColor );
    void enableGuiControls( bool );

    ReplicateNullModelConfigDialog cvConfigurationDialog;
    QTextEdit cvConsole;
    QDockWidget* cvConsoleDock;
    ReplicateNullModelTest* cvNullModelTest;
    SpeciesLocalitySet cvLocalitySet;
    RasterModel cvRasterModel;
    Ui::ReplicateNullModelWindow* cvUi;

};

#endif // REPLICATENULLMODELWINDOW_H
