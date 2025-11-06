#ifndef __DEMO_BUFFER_H__
#define __DEMO_BUFFER_H__

#include "basic.h"
#include <atomic>
#include <mutex>
#include <qcontainerfwd.h>
#include <qobject.h>
#include <qtmetamacros.h>


class DemoBuffer : public QObject
{
    Q_OBJECT
public:
signals:
    void DemoProcessor_RecvData();

public:
    DemoBuffer(size_t winSize_ms);

    void Exits();

    void Buffer();

    void SetWindowSize(int winSize);
    void PushRowData(QVector<channel>& frameData);

    void PushFrameFilterData();

    QVector<channel>& GetRowData();
    QVector<channel>& GetFilteredData();

    void UpdateFrameRowData();
    void UpdateProcessedData();

public:
    QVector<channel> mFrameRowData;     // 单帧原始数据
    QVector<channel> mFrameFilteredData;// 单帧滤波后数据
private:
    QVector<channel> mFilteredData;// 整个窗口
    QVector<channel> mRowData;
    int mWinSize;// plot 窗口大小
    int mSamRate;

    std::atomic<bool> mGetFrameRowData;
    std::atomic<bool> mGetProcessedData;
    std::atomic<bool> mExit;
    std::atomic<bool> mContinue;
    std::mutex mMutex;
};

#endif
