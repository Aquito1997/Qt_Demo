#ifndef __Processor_HPP__
#define __Processor_HPP__


#include "basic.h"

#include <QObject>
#include <QVector>
#include <atomic>
#include <cassert>
#include <mutex>
#include <qcontainerfwd.h>
#include <qobject.h>


class DemoProcessor : public QObject
{
    Q_OBJECT
public:
signals:
    void DemoBuffer_FetchData();
    void Processor_Warning();

public:
    void Check();
    // 计算最后一帧数据，放入mFilterData中
    void Filter();


    void SetWindowSize(int WinSize);
    void SetThreshold(float threshold);
    void SetFilterVal(float filterVal);

    void Exits();
    void RecvData();

    DemoProcessor() = delete;
    DemoProcessor(int winSize, QVector<channel>* frameRowData, QVector<channel>* frameFilterData);
    ~DemoProcessor() = default;

private:
    std::atomic<bool> mExit;
    std::atomic<bool> mRecvFrameData;
    int mThreshold;                   // 阈值
    int mWinSize;                     // 滤波参考个数
    QVector<channel> mFilterData;     // 滤波后的一帧的数据
    QVector<channel> mRowData;        // 缓冲数据
    QVector<channel> mFrameRowData;   // 单帧原始数据
    QVector<channel>* mInFrameRowData;// 入参 单帧原始数据
    QVector<channel>* mOutFilterData; // 出参 滤波后的数据
    std::mutex mProcessorMutex;
};
#endif