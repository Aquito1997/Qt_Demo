#ifndef __CSV_Saver_HPP__
#define __CSV_Saver_HPP__

#include "basic.h"
#include <QObject>
#include <QString>
#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <mutex>
#include <qcontainerfwd.h>
#include <qdatetime.h>
#include <qdebug.h>
#include <qdir.h>
#include <qobject.h>
#include <qobjectdefs.h>
#include <qtimer.h>
#include <qtimezone.h>
#include <qtmetamacros.h>
#include <qtypes.h>


class CSV_Saver : public QObject
{
    Q_OBJECT

public:
signals:
    void WriteDone();

public:
    void CreateFile(qint64 time);
    void SaveToFile();
    void Exits();

    void RecvData();
    void Pause();
    void Continue();
    CSV_Saver(const QString& prepath, QVector<channel>* rowData);
    ~CSV_Saver();

private:
    QFile* mFile;
    QString mPath;
    QString mSuffix;
    QString mFileHeader;

    std::atomic_bool mRecvData;
    QVector<channel> mRowData;
    QVector<channel>* mData;

    size_t mSavedIdx;// 已经保存了的数据的条数

    qint64 mDuration;
    QDateTime* mDateTime;
    QTimeZone mTimeZone;

    std::atomic<bool> mContinue;
    std::atomic<bool> mExit;

    std::mutex mMutex;
};

#endif