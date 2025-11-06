#ifndef __TIMER__H__
#define __TIMER__H__

#include <QObject>
#include <qelapsedtimer.h>
#include <qfloat16.h>
#include <qtypes.h>


class DemoTimer : public QObject
{
    Q_OBJECT
public:
signals:
    void TimeOut_Sample();
    void TimeOut_Redraw();

public:
    DemoTimer(int samRate);
    ~DemoTimer();
    int CalWindowSize(int ms, int samRate = 0);
    void UpdateSampleRate(int samRate);
    void Timer();
    void Exits();

private:
    int gcd_iterative(int a, int b);

private:
    qint64 mDrawCnt;   // mGCD * mDrawCnt 到重新绘制 QCustomPlot 的时间
    qint64 mSampleCnt; // mGCD * mSampleCnt 到采集数据的时间
    float mSampleUsage;// 采样一次需要多少纳秒
    int mLastSamRate;  // 采样率
    int mGCD;          // 重绘一帧 和 采集一次数据 的公约数
    QElapsedTimer* mTimer;
    bool mContinue;
};

#endif