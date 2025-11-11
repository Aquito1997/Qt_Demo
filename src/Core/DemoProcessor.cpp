#include "DemoProcessor.h"
#include "basic.h"
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <qcontainerfwd.h>
#include <qdebug.h>
#include <qlogging.h>
#include <qtimer.h>
#include <qtmetamacros.h>
#include <random>
#include <stdlib.h>


DemoProcessor::DemoProcessor(int drawlen, int sampleRate, float threshold, int fltWinLen)
{
    mExit = false;
    mContinue = true;
    mThreshold = threshold;

    mFltWinLen = fltWinLen;
    mFltHalfWinLen = fltWinLen / 2;
    mFltIdx = mFltHalfWinLen;

    mDrawLen = drawlen * sampleRate / 1000;

    mSample = true;
    mSampleIdx = 0;
    mSampleLen = 4 * sampleRate / 1000;


    for (size_t chn = 0; chn < 33; chn++)
    {
        mSmpData.push_back(channel());
        mSmpData[chn].push_back(0);
        mFltData.push_back(channel());
    }
}


void DemoProcessor::Filter()
{
    while (!mExit)
    {
        if (!mContinue) continue;

        EmptyRowData();// 先删除之前的数据，在生成新数据
        GenerateData();

        mMutex.lock();
        {
            if (mSmpData.size() < mFltWinLen) continue;
            const int& rowDataIdx = mSmpData[32].last();
            const int& size = mSmpData[32].size();

            int xAxisIndex = mFltHalfWinLen;
            for (; mFltIdx < rowDataIdx - mFltHalfWinLen; mFltIdx++)
            {
                for (size_t chn = 0; chn < 32; chn++)
                {
                    size_t cnt = 0;
                    float filteredData = 0;
                    size_t idx = xAxisIndex - mFltHalfWinLen;
                    for (; cnt < mFltWinLen; cnt++)
                    {
                        if (idx >= size)
                            break;
                        filteredData += mSmpData[chn][idx];
                        idx++;
                    }
                    filteredData /= cnt;
                    mFltData[chn].push_back(filteredData);
                    if (filteredData > mThreshold)// 峰值检测
                        emit Processor_Warning();
                }
                mFltData[32].push_back(mSmpData[32][xAxisIndex++]);
            }
            mContinue = false;
        }
        mMutex.unlock();
    }
};


void DemoProcessor::SetDrawLen(int drawLen, int smpRate)
{
    mMutex.lock();
    {
        mDrawLen = drawLen * smpRate / 1000;
    }
    mMutex.unlock();
}


// 保留下次计算所需要的数据，其余部分丢弃
void DemoProcessor::EmptyRowData()
{
    QVector<channel> buff(33);
    const int& size = mSmpData[32].size();
    for (size_t cnt = size - mFltWinLen; cnt < size - 1; cnt++)
        for (size_t chn = 0; chn < 33; chn++)
            buff[chn].push_back(mSmpData[chn][cnt]);

    for (size_t chn = 0; chn < 33; chn++)
        mSmpData[chn].swap(buff[chn]);
}


void DemoProcessor::GenerateData()
{
    while (!mSample);
    GenChannelData();

    const int end = mSampleIdx + mSampleLen;
    for (; mSampleIdx < end; mSampleIdx++)
        mSmpData[32].push_back(mSampleIdx);// 更新 xAxis 的值

    emit FetchData();
    mSample = false;
}


inline void DemoProcessor::GenChannelData()
{
    std::random_device rd; // 硬件随机数种子
    std::mt19937 gen(rd());// 梅森旋转算法生成器
    std::uniform_real_distribution<double> dis(0.0, 1.0);
    for (size_t chn = 0; chn < 32; chn++)
        for (size_t xCnt = 0; xCnt < mSampleLen; xCnt++)
        {
            auto val = dis(gen);
            mSmpData[chn].push_back(val);
        }

    // srand(time(NULL));
    // for (size_t chn = 0; chn < 32; chn++)
    //     for (size_t xCnt = 0; xCnt < mSampleLen; xCnt++)
    //     {
    //         auto val = (float)rand() / RAND_MAX;
    //         mSmpData[chn].push_back(val);
    //         if (val > 1) qDebug() << "random val: " << val;
    //     }
}


void DemoProcessor::Sample()
{
    mSample = true;
}


void DemoProcessor::Process()
{
    mContinue = true;
}


void DemoProcessor::GetFilteredData(QVector<channel>& buffer)
{
    mMutex.lock();
    {
        buffer.swap(mFltData);
        for (auto& chn : mFltData)
            chn = channel();
    }
    mMutex.unlock();
}


QVector<channel>* DemoProcessor::GetRowData()
{
    return &this->mSmpData;
}


void DemoProcessor::SetThreshold(float threshold)
{
    mThreshold = threshold;
}

void DemoProcessor::Exits()
{
    mExit = true;
}

void DemoProcessor::SetFltWinLen(int fltWinLen)
{
    mFltWinLen = fltWinLen;
}
