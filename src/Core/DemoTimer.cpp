#include "DemoTimer.h"

#include <qdebug.h>
#include <qelapsedtimer.h>
#include <qtmetamacros.h>
#include <qtypes.h>

static const qint64 x1sec = 1000000000;
static const qint64 x40ms = 5000000;

DemoTimer::DemoTimer(int samRate)
{
    mContinue = true;
    mLastSamRate = samRate;
    mSampleUsage = (float)x1sec / (float)samRate;
    mGCD = gcd_iterative(x40ms, mSampleUsage);// 采样一次 和 draw 一次耗时的 公约数
    mDrawCnt = x40ms / mGCD;
    mSampleCnt = mSampleUsage / mGCD;
    mTimer = new QElapsedTimer;
}


DemoTimer::~DemoTimer()
{
    if (!mTimer) delete mTimer;
}


void DemoTimer::UpdateSampleRate(int samRate)
{
    mLastSamRate = samRate;
    mSampleUsage = (float)x1sec / (float)samRate;
    mGCD = gcd_iterative(x40ms, mSampleUsage);
    mDrawCnt = x40ms / mGCD;
    mSampleCnt = mSampleUsage / mGCD;
}

int DemoTimer::CalWindowSize(int ms, int samRate)
{
    int sampleRete = 0;
    if (samRate)
        sampleRete = samRate;
    else
        sampleRete = mLastSamRate;

    float SampleUsage = (float)x1sec / (float)sampleRete;
    int winSize = (ms * 1000 * 1000) / SampleUsage;
    return winSize;
}


void DemoTimer::Timer()
{
    static int DrawCnt = 0;
    static int SampleCnt = 0;
    while (mContinue)
    {
        mTimer->restart();
        while (mTimer->nsecsElapsed() < mGCD);
        DrawCnt++;
        SampleCnt++;
        if (DrawCnt >= mDrawCnt)// QCustomplot绘制一次
        {
            emit TimeOut_Redraw();
            DrawCnt = 0;
        }
        if (SampleCnt >= mSampleCnt)// Sample 一次
        {
            emit TimeOut_Sample();
            SampleCnt = 0;
        }
    }
};

void DemoTimer::Exits()
{
    mContinue = false;
}


int DemoTimer::gcd_iterative(int a, int b)
{
    while (b != 0)
    {
        int temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}