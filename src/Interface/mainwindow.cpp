#include "mainwindow.h"
#include "DemoBuffer.h"
#include "DemoProcessor.h"
#include "Sampler.h"
#include "basic.h"
#include "ui_mainwindow.h"

#include <cassert>
#include <stdlib.h>
#include <thread>


#include <QtCharts/QtCharts>
#include <QtCharts/qchartview.h>
#include <QtCharts/qsplineseries.h>
#include <QtWidgets>
#include <qaction.h>
#include <qboxlayout.h>
#include <qbrush.h>
#include <qcombobox.h>
#include <qcontainerfwd.h>
#include <qcoreevent.h>
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
#include <qtmetamacros.h>


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
    this->mCustomPlots.push_back(ui->widget_1);
    this->mCustomPlots.push_back(ui->widget_2);
    this->mCustomPlots.push_back(ui->widget_3);
    this->mCustomPlots.push_back(ui->widget_4);

    for (size_t chn = 0; chn < 33; chn++)
    {
        mFilteredData.push_back(channel());
        mFilteredData[chn].push_back(0);
    }

    mChannels.push_back(0);// widget_1
    mChannels.push_back(1);// widget_2
    mChannels.push_back(2);// widget_3
    mChannels.push_back(3);// widget_4

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

    ui->lineEdit_1->setText("512000");
    ui->lineEdit_2->setText("0.666");
    ui->lineEdit_3->setText("10");

    mFilterWinSize = CalFilterWindowSize(ui->lineEdit_3->text().toInt(), ui->lineEdit_1->text().toInt());

    mBuffer = new DemoBuffer(mFilterWinSize);
    mTimer = new DemoTimer(512000);
    mSampler = new Sampler(&mBuffer->mFrameRowData);
    mProcessor = new DemoProcessor(100, &mBuffer->mFrameRowData, &mBuffer->mFrameFilteredData);
    mFileSaver = new CSV_Saver(QString("./"));

    mThreads.push_back(new std::thread([this]() { mTimer->Timer(); }));
    mThreads.push_back(new std::thread([this]() { mSampler->Collect(); }));
    mThreads.push_back(new std::thread([this]() { mProcessor->Filter(); }));
    mThreads.push_back(new std::thread([this]() { mFileSaver->SaveToFile(mBuffer->GetRowData()); }));
    mThreads.push_back(new std::thread([this]() { mBuffer->Buffer(); }));
    for (auto& thread : mThreads)
    {
        thread->detach();
    }
}

void MainWindow::Connect()
{
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
            mTimer->UpdateSampleRate(samRate);

        if (filter > 0 && samRate > 0)
        {
            mFilterWinSize = CalFilterWindowSize(ui->lineEdit_3->text().toInt(), ui->lineEdit_1->text().toInt());
            mBuffer->SetWindowSize(mFilterWinSize);
            mProcessor->SetWindowSize(mFilterWinSize);
        }

        if (threshold > 0)
            mProcessor->SetThreshold(threshold);
    });

    connect(mTimer, &DemoTimer::TimeOut_Redraw, this, [this]() { Draw(); });
    connect(mTimer, &DemoTimer::TimeOut_Sample, this, [this]() { this->mSampler->SetStatus(); });
    connect(mSampler, &Sampler::DataReady, this, [this]() { mBuffer->UpdateFrameRowData(); });
    connect(mBuffer, &DemoBuffer::DemoProcessor_RecvData, this, [this]() { mProcessor->RecvData(); });
    connect(mProcessor, &DemoProcessor::DemoBuffer_FetchData, this, [this]() { mBuffer->UpdateProcessedData(); });
    connect(mProcessor, &DemoProcessor::Processor_Warning, this, [this]() { Warning(); });

    for (auto& plot : mCustomPlots)
    {
        connect(plot->xAxis, SIGNAL(rangeChanged(QCPRange)), plot->xAxis2, SLOT(setRange(QCPRange)));
        connect(plot->yAxis, SIGNAL(rangeChanged(QCPRange)), plot->yAxis2, SLOT(setRange(QCPRange)));
    }
}

void MainWindow::Warning()
{
    QMessageBox::warning(this, tr("警告"), tr("采样值超过阈值"), QMessageBox::Ok);
}


void MainWindow::SetChannel(int viewIdx, int channelIdx)
{
    assert(viewIdx >= 0 && viewIdx < 4);
    mChannels[viewIdx] = channelIdx;
}


// 绘制图表
void MainWindow::Draw()
{
    if (!mDrawPlot) return;
    auto& filteredData = mBuffer->GetFilteredData();
    if (!filteredData[32].size()) return;

    for (int chn = 0; chn < 33; chn++)// 增量更新
    {
        for (int idx = filteredData[32].last() - mFilteredData[32].last(); idx > 0; idx--)
        {
            int index = std::abs(filteredData[chn].size() - idx);
            auto data = filteredData[chn][index];
            mFilteredData[chn].push_back(data);
        }
    }


    for (auto& chn : mFilteredData)
        if (chn.size() > mFilterWinSize)                              // 只保留窗口大小的数据
            for (size_t idx = mFilterWinSize; idx < chn.size(); idx++)// pop_   front的次数
                chn.pop_front();

    auto& xfAxis = mFilteredData[32];
    for (int idx = 0; idx < 4; idx++)
    {
        auto& yfAxis = mFilteredData[mChannels[idx]];
        auto* plot = mCustomPlots[idx];
        plot->clearGraphs();
        plot->clearItems();
        plot->replot();

        plot->addGraph();
        plot->graph(0)->setPen(QPen(Qt::blue));
        plot->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20)));

        // plot->setOpenGl(true, 0);
        // plot->setAntialiasedElements(QCP::aeNone);// 禁用抗锯齿以提高性能
        // plot->setNotAntialiasedElements(QCP::aeAll);

        plot->xAxis2->setVisible(true);
        plot->xAxis2->setTickLabels(false);
        plot->yAxis2->setVisible(true);
        plot->yAxis2->setTickLabels(false);
        plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);

        plot->graph(0)->setData(xfAxis, yfAxis);
        plot->graph(0)->rescaleAxes();
        plot->replot();
    }
}


MainWindow::~MainWindow()
{
    mExitDraw = false;
    mTimer->Exits();
    mFileSaver->Exits();
    mSampler->Exits();
    mProcessor->Exits();
    mBuffer->Exits();

    if (!mTimer) delete mTimer;
    if (!mFileSaver) delete mFileSaver;
    if (!mSampler) delete mSampler;
    if (!mProcessor) delete mProcessor;
    if (!mBuffer) delete mBuffer;

    for (auto& plot : mCustomPlots)
        if (!plot) delete plot;
    delete ui;
}
