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
#ifndef REPLICATENULLMODELTEST_H
#define REPLICATENULLMODELTEST_H

#include <QMap>
#include <QSet>
#include <QTime>
#include <QThread>
#include <QString>
#include <QDateTime>

#include "probability.h"
#include "rastermodel.h"
#include "landscapemapper.h"
#include "localitycollection.h"
#include "replicatenullmodeltestthread.h"

class ReplicateNullModelTest : public QThread
{
    Q_OBJECT

  public:

    enum RunLevel
    {
      InitialTest,
      GenerateLandscapeMap,
      CalculateProbability,
      Completed,
      Failed
    };

    ReplicateNullModelTest( RasterModel* theModel, bool showProgress = true );
    ReplicateNullModelTest( RasterModel* theModel, const LocalityCollection &theLocalities, bool showProgress = true );
    ~ReplicateNullModelTest();

    virtual void abort();

    QString lastError() { return cvLastError; }

    virtual void run() { emit testComplete( false ); }

    Probability probability();

  signals:
    void testComplete( bool );
    void message( QString );
    void progressUpdate( int );

  protected slots:
    virtual void allThreadsFinished() {}
    void progress( int theCurrentProgress, int theTarget );
    virtual void threadFinished( QString );
    virtual void threadProgress( QString, int );


  protected:
    void addThread( ReplicateNullModelTestThread* );
    bool arePresenceLocalitiesValid();
    void initProgress();
    QString newThreadId( int threadNumber ) { return QString::number( threadNumber ) + QDateTime::currentDateTime().toString( "yyyyMMddhhmmsszzz" ); }
    void purgeThreads();


    LandscapeMapper* cvLandscapeMapper;

    ReplicateNullModelTest::RunLevel cvRunLevel;
    bool cvAbort;
    int cvIdealThreadCount;
    QString cvLastError;
    int cvLastProgressValue;
    LocalityCollection cvLocalities;
    Probability* cvProbability;
    RasterModel* cvRasterModel;
    bool cvShowProgress;
    QMap< QString, ReplicateNullModelTestThread* > cvThreads;
    int cvThreadsCompleted;
    int cvThreadsCreated;
    int cvThreadProgress;

    bool cvUnifiedThreadGoal;

    bool cvDisplayEstimatedTimeToCompletion;

    QDateTime cvStartTime;
};

#endif // REPLICATENULLMODELTEST_H
