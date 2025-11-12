#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include "CSV_Saver.h"
#include "DemoProcessor.h"
#include "qcustomplot.h"


#include <QMainWindow>
#include <atomic>
#include <qaction.h>
#include <qcontainerfwd.h>
#include <qelapsedtimer.h>
#include <qmap.h>
#include <qobjectdefs.h>
#include <qtmetamacros.h>
#include <qvectornd.h>
#include <thread>
#include <vector>

QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    void Init();
    void Connect();
    void FetchFilteredData();

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();
public slots:
    void Draw();
    void SetChannel(int viewIdx, int channelIdx);

private:
    CSV_Saver* mFileSaver;
    DemoProcessor* mProcessor;
    QVector<std::thread*> mThreads;
    std::vector<QCustomPlot*> mCustomPlots;

    int mSampleRate;
    int mPlotLen;
    int mFilterWinSize;// 根据窗口大小产生的数据量

    std::atomic_bool mFetchData;// 线程退出开关
    std::atomic_bool mDrawPlot; // 绘制 QCustomPlot 开关
    std::atomic_bool mReplot;   // 绘制 QCustomPlot 开关

    QTimer* mSampleTimer;// 采样定时器
    QTimer* mReplotTimer;// 重绘定时器

    Ui::MainWindow* ui;
    std::atomic_bool mExitDraw;
    std::vector<int> mChannels;// widget  channel
    QVector<channel> mFltData; // 滤波后的函数
    channel mXAxis;
};
#endif// MAINWINDOW_H
