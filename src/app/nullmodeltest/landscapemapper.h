/*
** Copyright (C) 2009 Peter J. Ersts (ersts at amnh.org)
** Creation Date: 2009-06-17
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
#ifndef LANDSCAPEMAPPER_H
#define LANDSCAPEMAPPER_H

#include "probability.h"
#include "rastermodel.h"
#include "replicaterelabeler.h"

#include <QSet>
#include <QList>
#include <QFile>
#include <QString>
#include <QThread>
#include <QTextStream>

class LandscapeMapper : public QThread
{
    Q_OBJECT

  public:

    enum RunType
    {
      Unknown,
      GenerateLandscapeMap,
      CalculateProbability
    };

    LandscapeMapper( RasterModel* theRasterModel, QString theOutputDirectory = "", bool plotAccumulationCurve = false );
    ~LandscapeMapper();

    bool addReplicate( QString, bool );

    void calculateProbability();
    void calculateProbabilityBlocking();

    void generateLandscapeMap();

    bool initRelabeler( int theTemplateSize, const QSet< QString > &theReplicates );

    QString lastError() { return cvLastError; }

    const Probability &probability();

    quint64 replicatesInLandscape() { return cvTotalReplicatesInLandscape; }

    void run();

  signals:
    void progressUpdate( int, int );
    void message( QString );

  private:
    QFile* cvDistinctLandscapeReplicatesFile;
    QFile* cvAccumulationCurveFile;
    QTextStream* cvDistinctReplicatesStream;
    QTextStream* cvAccumulationCurveStream;

    bool cvPlotAccumulationCurve;
    bool cvIsValid;
    QString cvLastError;
    int cvLastProgressValue;
    Probability* cvProbability;
    RasterModel* cvRasterModel;
    LandscapeMapper::RunType cvRunType;
    QString** cvTemplateMap;
    int cvTemplateSize;
    quint64 cvTotalReplicatesInLandscape;
    QSet< quint64 > cvLandscapeReplicateHashTable;

    QList< quint64 > cvLandscapeRowHashAssociation[3];
    QList< ReplicateRelabeler > cvReplicates;

    quint64 cvTotalAddRequests;

    quint64 murmurHash( const void* key, int len, unsigned int seed );
    void progress( int, int );
};

#endif // LANDSCAPEMAPPER_H
