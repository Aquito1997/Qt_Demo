#ifndef __SAMPLER_HPP__
#define __SAMPLER_HPP__


#include "basic.h"
#include <QObject>
#include <atomic>
#include <mutex>
#include <qtmetamacros.h>
#include <qtypes.h>
#include <qvector.h>


class Sampler : public QObject
{
    Q_OBJECT
public slots:
    void SetStatus();

signals:
    void DataReady();

public:
    void Exits();
    void Collect();
    Sampler(QVector<channel>* outData);
    ~Sampler() = default;

private:
    inline float RandomFloat(float min, float max);

private:
    qint64 mIdx;
    std::atomic<bool> mExit;
    std::atomic<bool> mContinue;// 暂停 继续标志
    std::atomic<bool> mEnable;  // 控制是否采集数据
    QVector<channel> mTempData; // 缓冲数据
    QVector<channel>* mOutFrameData;
    std::mutex mMutex;
};


#endif//__SAMPLER_HPP__