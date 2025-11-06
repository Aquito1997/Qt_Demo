#include "Sampler.h"
#include "basic.h"

#include <atomic>
#include <cassert>
#include <cstddef>
#include <mutex>
#include <qcontainerfwd.h>
#include <qdebug.h>
#include <qfloat16.h>
#include <qlogging.h>
#include <qtmetamacros.h>
#include <qvector.h>
#include <random>


void Sampler::Collect()
{
    while (!mExit)
    {
        if (!mContinue) continue;
        mMutex.lock();
        {
            if (mEnable)
            {
                for (size_t idx = 0; idx < 32; idx++)
                    mTempData[idx][0] = RandomFloat(0, 1);
                mTempData[32][0] = mIdx++;// 更新 xAxis 的值

                mOutFrameData->swap(mTempData);
                emit DataReady();

                mEnable = false;
            }
        }
        mMutex.unlock();
    }
}


void Sampler::SetStatus()
{
    mEnable = true;
}


void Sampler::Exits()
{
    mExit = true;
}


Sampler::Sampler(QVector<channel>* outFrameData)
    : mIdx(0), mExit(false), mEnable(false), mContinue(true)
{
    assert(outFrameData);
    assert(outFrameData->size() == 33);
    assert((*outFrameData)[32].size() == 1);

    mOutFrameData = outFrameData;
    for (size_t chn = 0; chn < 33; chn++)
    {
        mTempData.push_back(channel());
        mTempData[chn].push_back(0);
    }
};


inline float Sampler::RandomFloat(float min, float max)
{
    static std::mt19937_64 generator(std::random_device{}());
    std::uniform_real_distribution<float> distribution(min, max);
    return distribution(generator);
};
