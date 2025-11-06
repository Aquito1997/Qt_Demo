#include "CSV_Saver.h"
#include "basic.h"

#include <QString>
#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <qcontainerfwd.h>
#include <qdatetime.h>
#include <qdebug.h>
#include <qdir.h>
#include <qlogging.h>
#include <qobject.h>
#include <qobjectdefs.h>
#include <qtimer.h>
#include <qtimezone.h>
#include <qtmetamacros.h>
#include <qtypes.h>
#include <string>


void CSV_Saver::CreateFile()
{
    assert(mFile);
    mFile->close();
    delete mFile;

    for (size_t iCnt = 0; iCnt < 10; iCnt++)
    {
        QString time = QString(mDateTime->currentDateTime(mTimeZone).toString("yyyy-MM-dd_hh_mm_ss"));
        mFile = new QFile(mPath + time + mSuffix);
        mFile->open(QIODevice::WriteOnly);
        if (mFile->isOpen())
        {
            mFile->write(mFileHeader.toStdString().c_str());
            mFile->flush();
            return;
        }

        qDebug() << "文件创建失败，尝试" << iCnt + 1 << "/10:"
                 << &_fullpath << "错误:" << mFile->errorString();
        mFile->close();
        delete mFile;
    }
    assert(false);
}


void CSV_Saver::SaveToFile(QVector<channel>& rowData)
{
    while (!mExit)
    {
        while (!mContinue);

        qint64 time = mDateTime->currentDateTime(mTimeZone).toSecsSinceEpoch();
        if (time - mDuration >= 300)// 5min
        {
            mDuration = time;
            CreateFile();
        }

        if (!mFile->isOpen()) continue;

        mMutex.lock();
        {
            int loopCnt = rowData[32].size();
            for (size_t idx = mSavedIdx; idx < loopCnt; idx++)
            {
                for (size_t chnIdx = 0; chnIdx < 33; chnIdx++)
                {
                    auto& xAxis = rowData[32][idx];
                    QString singleData = QString::number(xAxis) + ","  // 采样次数
                        + QString::number(chnIdx) + ","                // 通道号
                        + QString::number(rowData[chnIdx][idx]) + "\n";// 某个采样次数 下 某个通道号的 value
                    mFile->write(singleData.toStdString().c_str());
                }
            }
            mSavedIdx = loopCnt;
        }
        mMutex.unlock();
    }
}


void CSV_Saver::Pause()
{
    mContinue = false;
};
void CSV_Saver::Continue()
{
    mContinue = true;
};


void CSV_Saver::Exits()
{
    mExit = true;
}


CSV_Saver::CSV_Saver(const QString& prepath)
    : mExit(false), mSavedIdx(0), mContinue(true), mDuration(0),
      mSuffix(".csv"), mTimeZone(QTimeZone::systemTimeZone())
{
    QDir path(prepath);
    mPath = path.absolutePath() + QString("/data_");

    mFileHeader = QString("Index,Channel,value\n");
    mFile = new QFile();
    CreateFile();

    mDateTime = new QDateTime;
}


CSV_Saver::~CSV_Saver()
{
    if (!mFile) delete mFile;
    if (!mDateTime) delete mDateTime;
}
