#include "DemoProcessor.h"
#include <cassert>
#include <cstddef>
#include <qdebug.h>
#include <qlogging.h>
#include <qtmetamacros.h>

DemoProcessor::DemoProcessor(int winSize, QVector<channel>* frameRowData,
                             QVector<channel>* frameFilterData)
    : mWinSize(winSize), mExit(false)
{
    assert(frameRowData && frameFilterData);
    assert(frameRowData->size() == 33);
    assert((*frameRowData)[32].size() == 1);
    assert(frameFilterData->size() == 33);
    assert((*frameFilterData)[32].size() == 1);

    mInFrameRowData = frameRowData;
    mOutFilterData = frameFilterData;

    for (size_t chn = 0; chn < 33; chn++)
    {
        mRowData.push_back(channel());

        mFrameRowData.push_back(channel());
        mFrameRowData[chn].push_back(0);

        mFilterData.push_back(channel());
    }
}

void DemoProcessor::Check()
{
    emit Processor_Warning();
};

void DemoProcessor::Filter()
{
    while (!mExit)
    {
        if (mRecvFrameData)
        {
            mRecvFrameData = false;
            mInFrameRowData->swap(mFrameRowData);

            {                                      // 更新数据
                if (mRowData[32].size() < mWinSize)// 填充数据
                {
                    for (size_t idx = 0; idx < 33; idx++)
                        mRowData[idx].push_back(mFrameRowData[idx][0]);
                }
                else// 更新数据
                {
                    for (size_t idx = 0; idx < 33; idx++)
                    {
                        mRowData[idx].push_back(mFrameRowData[idx][0]);
                        mRowData[idx].pop_front();
                    }
                }
            }

            if (mRowData[32].size() < mWinSize) continue;// 原始数据不够计算一次均值滤波

            {// 因果滤波器
                float filteredData = 0;
                for (size_t chn = 0; chn < 32; chn++)
                {
                    for (size_t idx = mRowData[32].size() - 1; idx < mWinSize; idx--)
                        filteredData += mRowData[chn][idx];
                    filteredData /= mWinSize;
                    mFilterData[chn].push_back(filteredData);
                    (*mOutFilterData)[chn][0] = filteredData;
                }
                mFilterData[32].push_back(mRowData[32].last());
                (*mOutFilterData)[32][0] = mRowData[32].last();
                // qDebug() << "emit DemoBuffer_FetchData ";
                emit DemoBuffer_FetchData();

                for (auto& data : mFilterData)
                    data.pop_front();
            }
        }
    }
};

void DemoProcessor::RecvData()
{
    mRecvFrameData = true;
}

void DemoProcessor::SetThreshold(float threshold)
{
    mThreshold = threshold;
}

void DemoProcessor::Exits()
{
    mExit = true;
}

void DemoProcessor::SetWindowSize(int winSize)
{
    mWinSize = winSize;
    if (mWinSize / 2 != 0)
        mWinSize++;
}
