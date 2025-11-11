#include "mainwindow.h"

#include "CSV_Saver.h"
#include "DemoProcessor.h"
#include "basic.h"
#include "ui_mainwindow.h"

#include <QtCharts/QtCharts>
#include <QtCharts/qchartview.h>
#include <QtCharts/qsplineseries.h>
#include <QtWidgets>
#include <cstddef>
#include <numeric>
#include <qaction.h>
#include <qboxlayout.h>
#include <qbrush.h>
#include <qcombobox.h>
#include <qcontainerfwd.h>
#include <qcoreevent.h>
#include <qdatetime.h>
#include <qdebug.h>
#include <qelapsedtimer.h>
#include <qflags.h>
#include <qlayout.h>
#include <qlayoutitem.h>
#include <qlogging.h>
#include <qmath.h>
#include <qmenu.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qobjectdefs.h>
#include <qpen.h>
#include <qpushbutton.h>
#include <qthread.h>
#include <qtimer.h>
#include <qtmetamacros.h>

#include <cassert>
#include <stdlib.h>
#include <thread>


int CalFilterWindowSize(int ms, int samRate)
{
    return ms * samRate / 1000;
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), mExitDraw(true), mDrawPlot(true)
{
    ui->setupUi(this);
    Init();
    Connect();
}

void MainWindow::Init()
{
    {// ui
        this->mCustomPlots.push_back(ui->widget_1);
        this->mCustomPlots.push_back(ui->widget_2);
        this->mCustomPlots.push_back(ui->widget_3);
        this->mCustomPlots.push_back(ui->widget_4);

        for (size_t iCnt = 0; iCnt < 32; iCnt++)
        {
            ui->comboBox_1->addItem(QString("Channel ") + QString::number(iCnt + 1));
            ui->comboBox_2->addItem(QString("Channel ") + QString::number(iCnt + 1));
            ui->comboBox_3->addItem(QString("Channel ") + QString::number(iCnt + 1));
            ui->comboBox_4->addItem(QString("Channel ") + QString::number(iCnt + 1));
        }

        ui->comboBox_1->setCurrentIndex(0);
        ui->comboBox_2->setCurrentIndex(1);
        ui->comboBox_3->setCurrentIndex(2);
        ui->comboBox_4->setCurrentIndex(3);

        ui->lineEdit_1->setText("51200");
        ui->lineEdit_2->setText("0.666");
        ui->lineEdit_3->setText("10");
    }


    for (size_t chn = 0; chn < 33; chn++)
    {
        mFltData.push_back(channel());
        mFltData[chn].push_back(0);
    }


    {


        mChannels.push_back(0);// widget_1
        mChannels.push_back(1);// widget_2
        mChannels.push_back(2);// widget_3
        mChannels.push_back(3);// widget_4

        mProcessor = new DemoProcessor(100, 51200, 0.97, 15);
        mFileSaver = new CSV_Saver(QString("./"), mProcessor->GetRowData());

        mFilterWinSize = CalFilterWindowSize(100, 51200);
        mXAxis = channel(mFilterWinSize);
        std::iota(mXAxis.begin(), mXAxis.end(), 0);

        mSampleTimer = new QTimer();
        mSampleTimer->start(5);

        mReplotTimer = new QTimer();
        mReplotTimer->start(40);
    }


    mThreads.push_back(new std::thread([this]() { mProcessor->Filter(); }));
    mThreads.push_back(new std::thread([this]() { mFileSaver->SaveToFile(); }));
    for (auto& thread : mThreads)
        thread->detach();
}

void MainWindow::Connect()
{
    connect(mSampleTimer, &QTimer::timeout, mProcessor, &DemoProcessor::Process);
    connect(mReplotTimer, &QTimer::timeout, this, &MainWindow::Draw);

    connect(ui->comboBox_1, &QComboBox::currentIndexChanged, this, [this](int chnIdx) { this->SetChannel(0, chnIdx); });
    connect(ui->comboBox_2, &QComboBox::currentIndexChanged, this, [this](int chnIdx) { this->SetChannel(1, chnIdx); });
    connect(ui->comboBox_3, &QComboBox::currentIndexChanged, this, [this](int chnIdx) { this->SetChannel(2, chnIdx); });
    connect(ui->comboBox_4, &QComboBox::currentIndexChanged, this, [this](int chnIdx) { this->SetChannel(3, chnIdx); });

    connect(this->ui->pushButton_1, &QPushButton::pressed, this, [this]() { this->mDrawPlot = true; });
    connect(this->ui->pushButton_2, &QPushButton::pressed, this, [this]() { this->mDrawPlot = false; });
    connect(this->ui->pushButton_3, &QPushButton::pressed, this, [this]() { this->mFileSaver->Continue(); });
    connect(this->ui->pushButton_4, &QPushButton::pressed, this, [this]() { this->mFileSaver->Pause(); });
    connect(this->ui->pushButton_5, &QPushButton::pressed, this, [this]() {
        int samRate = ui->lineEdit_1->text().toInt();      // 采样率
        int filter = ui->lineEdit_3->text().toFloat();     // 滤波参数(ms)
        float threshold = ui->lineEdit_2->text().toFloat();// 阈值

        if (samRate > 0)
            mProcessor->SetDrawLen(100, samRate);

        if (filter > 0 && samRate > 0)
        {
            mFilterWinSize = CalFilterWindowSize(ui->lineEdit_3->text().toInt(), ui->lineEdit_1->text().toInt());
            mXAxis = channel(mFilterWinSize);
            std::iota(mXAxis.begin(), mXAxis.end(), 0);

            mProcessor->SetFltWinLen(mFilterWinSize);
        }

        if (threshold > 0)
            mProcessor->SetThreshold(threshold);
    });

    connect(mProcessor, &DemoProcessor::FetchData, this, [this]() { mFileSaver->RecvData(); });
    connect(mFileSaver, &CSV_Saver::WriteDone, mProcessor, &DemoProcessor::Sample);
    connect(mProcessor, &DemoProcessor::Processor_Warning, this, [this]() {
        ui->statusbar->showMessage("越界警告");
    });

    for (auto& plot : mCustomPlots)
    {
        connect(plot->xAxis, SIGNAL(rangeChanged(QCPRange)), plot->xAxis2, SLOT(setRange(QCPRange)));
        connect(plot->yAxis, SIGNAL(rangeChanged(QCPRange)), plot->yAxis2, SLOT(setRange(QCPRange)));
    }
}


void MainWindow::SetChannel(int viewIdx, int channelIdx)
{
    assert(viewIdx >= 0 && viewIdx < 4);
    mChannels[viewIdx] = channelIdx;
}


void MainWindow::FetchFilteredData()
{
    QVector<channel> fltdData(33);
    mProcessor->GetFilteredData(fltdData);
    if (!fltdData[32].size()) return;
    // qDebug() << "Filter Data Size: " << fltdData[32].size()
    //          << "Filter Begin: " << fltdData[32].first()
    //          << "mFltData Last: " << mFltData[32].last();

    // 预先计算各个通道的数据量
    QVector<int> fltChnSize;
    for (size_t chn = 0; chn < 33; chn++)
        fltChnSize.push_back(fltdData[chn].size());

    // 增量更新
    int lastIdx = mFltData[32].last() > fltdData[32][0] ? mFltData[32].last() : fltdData[32][0];
    int copyLen = fltdData[32].last() - lastIdx;
    for (int chn = 0; chn < 33; chn++)
        for (auto idx = copyLen; idx > 0; idx--)
            mFltData[chn].push_back(fltdData[chn][fltChnSize[chn] - idx]);

    // 只保留窗口大小的数据
    if (mFltData[0].size() > mFilterWinSize)
    {
        for (auto& chn : mFltData)
        {
            if (chn.size() > mFilterWinSize)
                chn.erase(chn.begin(), chn.begin() + (chn.size() - mFilterWinSize));
        }
    }
}


// 绘制图表
void MainWindow::Draw()
{
    // auto start_time = std::chrono::high_resolution_clock::now();
    FetchFilteredData();
    if (!mDrawPlot) return;// 绘制开关


    for (int idx = 0; idx < 4; idx++)
    {
        auto& yfAxis = mFltData[mChannels[idx]];
        auto* plot = mCustomPlots[idx];
        plot->replot();

        plot->addGraph();
        plot->graph(0)->setPen(QPen(Qt::blue));
        plot->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20)));

        plot->setOpenGl(true, 0);
        plot->setAntialiasedElements(QCP::aeNone);// 禁用抗锯齿以提高性能
        plot->setNotAntialiasedElements(QCP::aeAll);

        plot->xAxis2->setVisible(true);
        plot->xAxis2->setTickLabels(false);
        plot->yAxis2->setVisible(true);
        plot->yAxis2->setTickLabels(false);
        plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);

        plot->graph(0)->setData(mXAxis, yfAxis);
        plot->graph(0)->rescaleAxes();
        plot->replot();
    }
    // auto end = std::chrono::high_resolution_clock::now();
    // qDebug() << "usage: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start_time).count();
}


MainWindow::~MainWindow()
{
    mExitDraw = false;
    mFileSaver->Exits();
    mProcessor->Exits();

    if (!mFileSaver) delete mFileSaver;
    if (!mProcessor) delete mProcessor;

    for (auto& plot : mCustomPlots)
        if (!plot) delete plot;
    delete ui;
}
