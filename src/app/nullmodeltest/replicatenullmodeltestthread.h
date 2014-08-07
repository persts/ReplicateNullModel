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
#ifndef REPLICATENULLMODELTESTTHREAD_H
#define REPLICATENULLMODELTESTTHREAD_H

#include "probability.h"

#include <QSet>
#include <QThread>
#include <QString>

class ReplicateNullModelTestThread : public QThread
{
    Q_OBJECT

  public:
    ReplicateNullModelTestThread( QString, bool showProgress = true );
    ~ReplicateNullModelTestThread();

    void abort() { cvAbort = true; }
    QString id() { return cvId; }
    virtual void run() { emit finished( cvId ); }

    const QSet< QString > &distinctReplicates() { return cvDistinctReplicates; }

    Probability probability();
    void progress( int, int );

  signals:
    void finished( QString );
    void progressUpdate( QString, int );

  protected:
    QString cvId;
    bool cvAbort;
    int cvLastProgressValue;
    Probability* cvProbability;
    QSet< QString > cvDistinctReplicates;
    bool cvShowProgress;
};

#endif // REPLICATENULLMODELTESTTHREAD_H
