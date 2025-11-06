#include "DemoBuffer.h"
#include "basic.h"
#include <cassert>
#include <cstddef>
#include <mutex>
#include <qdebug.h>
#include <qlogging.h>

static constexpr int x1s = 1000;

DemoBuffer::DemoBuffer(size_t winSize_ms)
    : mGetFrameRowData(false), mGetProcessedData(false), mExit(false),
      mWinSize(winSize_ms)
{
    for (size_t chn = 0; chn < 33; chn++)
    {
        mFrameFilteredData.push_back(channel());
        mFrameFilteredData[chn].push_back(0);

        mFrameRowData.push_back(channel());
        mFrameRowData[chn].push_back(0);

        mFilteredData.push_back(channel());
    }


    for (int idx = 0; idx < 33; idx++)
        mRowData.push_back(channel());
}

QVector<channel>& DemoBuffer::GetRowData()
{
    std::lock_guard<std::mutex> guard(mMutex);
    return mRowData;
};

void DemoBuffer::PushFrameFilterData()
{
    for (int chn = 0; chn < 33; chn++)
        mFilteredData[chn].push_back(mFrameFilteredData[chn][0]);

    // pop_front 超出绘制窗口的数据
    if (mFilteredData[32].size() > mWinSize)
        for (int chn = 0; chn < 33; chn++)
            mFilteredData[chn].pop_front();
}

void DemoBuffer::Buffer()
{
    while (!mExit)
    {
        if (mGetProcessedData)
        {
            PushFrameFilterData();
            mGetProcessedData = false;
        }

        if (mGetFrameRowData)
        {
            this->PushRowData(mFrameRowData);
            mGetFrameRowData = false;
            //   qDebug() << "emit DemoProcessor_RecvData";
            emit DemoProcessor_RecvData();
        }
    }
}

QVector<channel>& DemoBuffer::GetFilteredData()
{
    std::lock_guard<std::mutex> guard(mMutex);
    return mFilteredData;
}

void DemoBuffer::UpdateFrameRowData()
{
    std::lock_guard<std::mutex> guard(mMutex);
    mGetFrameRowData = true;
}
void DemoBuffer::UpdateProcessedData()
{
    std::lock_guard<std::mutex> guard(mMutex);
    mGetProcessedData = true;
}

void DemoBuffer::SetWindowSize(int winSize)
{
    mWinSize = winSize;
}

void DemoBuffer::PushRowData(QVector<channel>& frameData)
{
    std::lock_guard<std::mutex> guard(mMutex);
    for (int chn = 0; chn < 33; chn++)
        mRowData[chn].push_back(frameData[chn][0]);
}

void DemoBuffer::Exits()
{
    mExit = true;
}
