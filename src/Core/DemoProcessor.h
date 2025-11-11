#ifndef __Processor_HPP__
#define __Processor_HPP__


#include "basic.h"

#include <QObject>
#include <QVector>
#include <atomic>
#include <cassert>
#include <cstdint>
#include <mutex>
#include <qcontainerfwd.h>
#include <qobject.h>
#include <qtypes.h>


class DemoProcessor : public QObject
{
    Q_OBJECT
public:
signals:
    void FetchData();
    void Processor_Warning();

public:
    // 计算最后一帧数据，放入mFilterData中
    void Filter();

    void SetDrawLen(int drawLen, int smpRate);

    void Exits();
    void SetFltWinLen(int WinSize);
    void SetThreshold(float threshold);
    void SetFilterVal(float filterVal);

    void GetFilteredData(QVector<channel>& buffer);
    QVector<channel>* GetRowData();

    void EmptyRowData();
    void GenerateData();// 固定生成4ms的数据
    inline void GenChannelData();

    void Sample();
    void Process();

    DemoProcessor() = delete;
    DemoProcessor(int drawlen, int sampleRate, float threshold, int fltWinLen);
    ~DemoProcessor() = default;

private:
    std::atomic<bool> mExit;

    float mThreshold;  // 阈值
    int mDrawLen;      // 绘制时窗口长度
    int mFltHalfWinLen;// 计算时窗口半长
    int mFltWinLen;    // 计算时窗口长度

    int mSampleLen;           // 每次采样长度
    uint64_t mSampleIdx;      // 采样过的下标
    QVector<channel> mSmpData;// 采样的原始数据

    int mFltIdx;              // 处理过的idx
    QVector<channel> mFltData;// 滤波后的数据

    std::atomic_bool mContinue;
    std::atomic_bool mSample;

    // std::atomic_uint64_t mIdx;// 计数
    std::mutex mMutex;
};
#endif