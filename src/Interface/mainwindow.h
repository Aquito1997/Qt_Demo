#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include "CSV_Saver.h"
#include "DemoBuffer.h"
#include "DemoProcessor.h"
#include "DemoTimer.h"
#include "Sampler.h"
#include "qcustomplot.h"


#include <QMainWindow>
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
    void Warning();

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();
public slots:
    void Draw();
    void SetChannel(int viewIdx, int channelIdx);

private:
    DemoTimer* mTimer;
    CSV_Saver* mFileSaver;
    Sampler* mSampler;
    DemoProcessor* mProcessor;
    DemoBuffer* mBuffer;
    std::vector<QCustomPlot*> mCustomPlots;
    QVector<std::thread*> mThreads;

    int mFilterWinSize;
    bool mDrawPlot;

    Ui::MainWindow* ui;
    std::atomic<bool> mExitDraw;
    std::vector<int> mChannels;// widget  channel
    QVector<channel> mFilteredData;
};
#endif// MAINWINDOW_H
