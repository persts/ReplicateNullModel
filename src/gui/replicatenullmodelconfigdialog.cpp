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
#include "replicatenullmodelconfigdialog.h"
#include "ui_replicatenullmodelconfigdialog.h"

#include <QDir>
#include <QSettings>
#include <QFileDialog>

ReplicateNullModelConfigDialog::ReplicateNullModelConfigDialog()
{
  cvUi = new Ui::ReplicateNullModelConfigDialog();
  cvUi->setupUi( this );
}

void ReplicateNullModelConfigDialog::on_pbtnSelectDirectory_clicked()
{
  QSettings lvQSettings;
  QString lvLastDirectory = lvQSettings.value( "lastOutputDirectory", "" ).toString() + QDir::separator() + "..";
  QString lvDirectory = QFileDialog::getExistingDirectory( this, tr( "Select the output directory" ), lvLastDirectory );

  setOutputDirectory( lvDirectory );
}


int ReplicateNullModelConfigDialog::randomDistributionIterations()
{
  return cvUi->sboxRandomDistributionIterations->value();
}

int ReplicateNullModelConfigDialog::randomDistributionSamples()
{
  return cvUi->sboxRandomDistributionSamples->value();
}

bool ReplicateNullModelConfigDialog::distinctReplicates()
{
  return cvUi->rbtnDistinctReplicates->isChecked();
}

float ReplicateNullModelConfigDialog::newResultMinimumThreshold()
{
  return cvUi->dsboxNewResultMinimumTheshold->value();
}

bool ReplicateNullModelConfigDialog::pyramid()
{
  return cvUi->rbtnPyramid->isChecked();
}

int ReplicateNullModelConfigDialog::randomTranslationIterations()
{
  return cvUi->sboxRandomTranslationIterations->value();
}

void ReplicateNullModelConfigDialog::setOutputDirectory(QString theValue)
{
  cvUi->leOutputDirectory->setText(theValue);
}

int ReplicateNullModelConfigDialog::templatePyramidLevels()
{
  return cvUi->sboxTemplatePyramidLevels->value();
}

bool ReplicateNullModelConfigDialog::timeLimit()
{
  return cvUi->rbtnTimeLimit->isChecked();
}

QString ReplicateNullModelConfigDialog::outputDirectory()
{
  return cvUi->leOutputDirectory->text();
}
