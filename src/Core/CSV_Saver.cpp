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


CSV_Saver::CSV_Saver(const QString& prepath, QVector<channel>* rowData)
    : mExit(false), mSavedIdx(1), mContinue(true), mDuration(0), mData(rowData),
      mSuffix(".csv"), mTimeZone(QTimeZone::systemTimeZone()), mRecvData(false)
{
    assert(rowData);
    QDir path(prepath);
    mPath = path.absolutePath() + QString("/data_");

    mFileHeader = QString("Index,Channel,value\n");
    mFile = new QFile();
    qint64 time = mDateTime->currentDateTime(mTimeZone).toSecsSinceEpoch();
    CreateFile(time);

    mDateTime = new QDateTime;

    for (size_t idx = 0; idx < 33; idx++)
        mRowData.push_back(channel());
}


CSV_Saver::~CSV_Saver()
{
    if (!mFile) delete mFile;
    if (!mDateTime) delete mDateTime;
}


void CSV_Saver::CreateFile(qint64 time)
{
    mDuration = time;

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


void CSV_Saver::SaveToFile()
{
    while (!mExit)
    {
        qint64 time = mDateTime->currentDateTime(mTimeZone).toSecsSinceEpoch();
        if (time - mDuration >= 300) CreateFile(time);// 5min
        if (!mFile->isOpen())
            continue;

        if (!mContinue)
            continue;

        if (mRecvData)
        {
            QVector<size_t> vecInputSize;
            for (int chn = 0; chn < 33; ++chn)
                vecInputSize.push_back((*mData)[chn].size());

            for (size_t chn = 0; chn < 33; chn++)
                for (size_t idx = 0; idx < vecInputSize[chn] - 1; idx++)
                {
                    auto& val = (*mData)[chn][idx];
                    mRowData[chn].push_back(val);
                }

            emit WriteDone();
            mRecvData = false;
        }

        if (mRowData[32].size() < 1)
            continue;

        const int& xAxisStart = mRowData[32][0] > mSavedIdx - 1 ? mRowData[32][0] : mSavedIdx - 1;
        const int& xAxisEnd = mRowData[32].last();

        for (size_t val = xAxisStart; val < xAxisEnd - 1; val++)
        {
            int idx = 0;
            for (size_t chn = 0; chn < 33; chn++)
            {
                QString singleData = QString::number(mRowData[32][idx]) + ","
                    + QString::number(chn) + ","
                    + QString::number(mRowData[chn][idx]) + "\n";
                mFile->write(singleData.toStdString().c_str());
                idx++;
            }
        }
        mSavedIdx += xAxisEnd - xAxisStart;

        for (auto& chn : mRowData)
            chn.clear();
    }
}

void CSV_Saver::RecvData()
{
    mRecvData = true;
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
