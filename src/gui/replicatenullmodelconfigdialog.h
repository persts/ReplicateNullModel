/*
** Copyright (C) 2009 Peter J. Ersts (ersts at amnh.org)
** Creation Date: 2009-03-23
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
#ifndef REPLICATENULLMODELCONFIGDIALOG_H
#define REPLICATENULLMODELCONFIGDIALOG_H

#include <QDialog>
#include <QString>

namespace Ui
{
  class ReplicateNullModelConfigDialog;
}


class ReplicateNullModelConfigDialog : public QDialog
{
    Q_OBJECT

  public:
    ReplicateNullModelConfigDialog();

    /* Quick, temporary accessors created while refactoring */
    int randomDistributionIterations();
    int randomDistributionSamples();
    bool distinctReplicates();
    float newResultMinimumThreshold();
    QString outputDirectory();
    bool pyramid();
    int randomTranslationIterations();
    void setOutputDirectory(QString);
    int templatePyramidLevels();
    bool timeLimit();


  private:
    Ui::ReplicateNullModelConfigDialog* cvUi;
  private slots:
    void on_pbtnSelectDirectory_clicked();
};

#endif // REPLICATENULLMODELCONFIGDIALOG_H
