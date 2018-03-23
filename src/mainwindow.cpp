#include "mainwindow.h"
#include <iostream>
#include <fstream>
#include <QHBoxLayout>
#include <QCloseEvent>
#include <QTimer>
#include <QDir>
#include <QJsonDocument>
#include <QDebug>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    readSettings();
    experimentsStorage = new ExperimentsStorage(this);
    experimentsStorage->setSettings(&settingsJson_["Storage"]);
    if (experimentsStorage->pathCalibrationFolder().isEmpty())
        experimentsStorage->setPathCalibrationFolder("/home/calorimeter/MCMIX/Amp");
    if (experimentsStorage->pathSampleFolder().isEmpty())
        experimentsStorage->setPathSampleFolder("/home/calorimeter/MCMIX/Data");
    if (experimentsStorage->pathTarFolder().isEmpty())
        experimentsStorage->setPathTarFolder("/home/calorimeter/MCMIX/Tar");
    if (experimentsStorage->pathMCMIXFolder().isEmpty())
        experimentsStorage->setPathMCMIXFolder("/home/calorimeter/MCMIX");

    viewExperiments = new ViewExperiments;
    experimentInfo =  new ExperimentInfo;
    experimentInfo->setSettings(&settingsJson_["GUI"]["ExperimentInfo"]);
    resultsExperimentWidget = new ResultsExperimentWidget;

    fotoView = new FotoView;
    fotoView->setSettings(&settingsJson_["GUI"]["FotoView"]);
    splitterInfoFotoExperiments = new QSplitter(Qt::Vertical);
    splitterInfoFotoExperiments->addWidget(experimentInfo);
    splitterInfoFotoExperiments->addWidget(fotoView);


    experimentWidget = new QWidget;
    QTabWidget * experimentTabWidget = new QTabWidget;
    experimentTabWidget->addTab(splitterInfoFotoExperiments,tr("Инфо"));
    experimentTabWidget->addTab(resultsExperimentWidget,tr("Обработка"));
    QVBoxLayout * tabLayout = new QVBoxLayout;
    tabLayout->addWidget(experimentTabWidget);

    experimentWidget->setLayout(tabLayout);
    experimentWidget->hide();


    splitterViewInfoExperiments = new QSplitter(Qt::Horizontal);
    splitterViewInfoExperiments->addWidget(viewExperiments);
    splitterViewInfoExperiments->addWidget(experimentWidget);


    setCentralWidget(splitterViewInfoExperiments);

    connect(viewExperiments, &ViewExperiments::showExperiment,
            this, &MainWindow::showExperimentInfo);
    connect(viewExperiments, &ViewExperiments::showExperiment,
            resultsExperimentWidget, &ResultsExperimentWidget::setCurrenExperiment);




    createActions();
    createMenu();

    splitterViewInfoExperiments->restoreState(splitterViewInfoTreeExperimentsSize_);
    splitterInfoFotoExperiments->restoreState(splitterInfoFotoTreeExperimentsSize_);
    currentTabShow = "Experiments";

    connect(viewExperiments, &ViewExperiments::treeExperimentsClicked,
            this, &MainWindow::tabTreeExperimentsIsVisible);
    connect(viewExperiments, &ViewExperiments::allExperimentsTableClicked,
            this, &MainWindow::tabAllExperimentsIsVisible);
    connect(viewExperiments, &ViewExperiments::tarClicked,
            this, &MainWindow::tabTarIsVisible);

    showExperiments();
}

MainWindow::~MainWindow()
{

}

void MainWindow::closeEvent(QCloseEvent *event)
{
    experimentsStorage->close();

    writeSettings();
    event->accept();
}

void MainWindow::readSettings()
{
    std::ifstream file("settings/MCMIX.json");
    if(file.is_open()){
        file >> settingsJson_;
        file.close();
        if (settingsJson_["/GUI/MainWindow/maximized"_json_pointer].is_boolean() ? static_cast<bool>(settingsJson_["/GUI/MainWindow/maximized"_json_pointer]) : false){
            showMaximized();
        } else {
            resize(settingsJson_["/GUI/MainWindow/Position/width"_json_pointer].is_number() ? static_cast<unsigned int>(settingsJson_["/GUI/MainWindow/Position/width"_json_pointer]) : 1200,
                settingsJson_["/GUI/MainWindow/Position/height"_json_pointer].is_number() ?static_cast<unsigned int>(settingsJson_["/GUI/MainWindow/Position/height"_json_pointer]) : 900);
            move(settingsJson_["/GUI/MainWindow/Position/x"_json_pointer].is_number() ? static_cast<unsigned int>(settingsJson_["/GUI/MainWindow/Position/x"_json_pointer]) : 50,
                settingsJson_["/GUI/MainWindow/Position/y"_json_pointer].is_number() ? static_cast<unsigned int>(settingsJson_["/GUI/MainWindow/Position/y"_json_pointer]) : 50);
        }
        std::string splitterViewInfo;
        std::string splitterInfoFoto;
        splitterViewInfo = settingsJson_["/GUI/MainWindow/Tabs/TreeExperiments/SplitterViewInfo/state"_json_pointer].is_string()
                ? settingsJson_["/GUI/MainWindow/Tabs/TreeExperiments/SplitterViewInfo/state"_json_pointer] : "";
        splitterInfoFoto = settingsJson_["/GUI/MainWindow/Tabs/TreeExperiments/SplitterInfoFoto/state"_json_pointer].is_string()
                ? settingsJson_["/GUI/MainWindow/Tabs/TreeExperiments/SplitterInfoFoto/state"_json_pointer] : "";
        splitterViewInfoTreeExperimentsSize_ = QByteArray::fromHex(splitterViewInfo.data());
        splitterInfoFotoTreeExperimentsSize_ = QByteArray::fromHex(splitterInfoFoto.data());
        splitterViewInfo = settingsJson_["/GUI/MainWindow/Tabs/AllExperiments/SplitterViewInfo/state"_json_pointer].is_string()
                ? settingsJson_["/GUI/MainWindow/Tabs/AllExperiments/SplitterViewInfo/state"_json_pointer] : "";
        splitterInfoFoto = settingsJson_["/GUI/MainWindow/Tabs/AllExperiments/SplitterInfoFoto/state"_json_pointer].is_string()
                ? settingsJson_["/GUI/MainWindow/Tabs/AllExperiments/SplitterInfoFoto/state"_json_pointer] : "";
        splitterViewInfoAllExperimentsSize_ = QByteArray::fromHex(splitterViewInfo.data());
        splitterInfoFotoAllExperimentsSize_ = QByteArray::fromHex(splitterInfoFoto.data());
        splitterViewInfo = settingsJson_["/GUI/MainWindow/Tabs/TarExperiments/SplitterViewInfo/state"_json_pointer].is_string()
                ? settingsJson_["/GUI/MainWindow/Tabs/TarExperiments/SplitterViewInfo/state"_json_pointer] : "";
        splitterViewInfoTarExperimentsSize_ = QByteArray::fromHex(splitterViewInfo.data());
    }

}

void MainWindow::readGuiSettings()
{
    std::string tabName = settingsJson_["/GUI/MainWindow/Tabs/lastOpened"_json_pointer].is_string() ? settingsJson_["/GUI/MainWindow/Tabs/lastOpened"_json_pointer] : "Experiments";
    if(tabName == "AllExperiments"){
        viewExperiments->showTab(ViewExperiments::TabName::allExperiment);
    } else if(tabName == "Tar"){
        viewExperiments->showTab(ViewExperiments::TabName::tar);
    } else if(tabName == "Experiments"){
        viewExperiments->showTab(ViewExperiments::TabName::experiment);
    }
}

void MainWindow::writeSettings()
{
    bool maximized = isMaximized();
    QByteArray splitterViewInfoTreeExperimentsState;
    QByteArray splitterInfoFotoTreeExperimentsState;
    QByteArray splitterViewInfoAllExperimentsState;
    QByteArray splitterInfoFotoAllExperimentsState;
    QByteArray splitterViewInfoTarExperimentsState;
    if (currentTabShow == "Experiments"){
        splitterViewInfoTreeExperimentsState = splitterViewInfoExperiments->saveState();
        splitterInfoFotoTreeExperimentsState = splitterInfoFotoExperiments->saveState();
        splitterViewInfoAllExperimentsState = splitterViewInfoAllExperimentsSize_;
        splitterInfoFotoAllExperimentsState = splitterInfoFotoAllExperimentsSize_;
        splitterViewInfoTarExperimentsState = splitterViewInfoTarExperimentsSize_;
    } else if (currentTabShow == "AllExperiments"){
        splitterViewInfoTreeExperimentsState = splitterViewInfoTreeExperimentsSize_;
        splitterInfoFotoTreeExperimentsState = splitterInfoFotoTreeExperimentsSize_;
        splitterViewInfoAllExperimentsState = splitterViewInfoExperiments->saveState();
        splitterInfoFotoAllExperimentsState = splitterInfoFotoExperiments->saveState();
        splitterViewInfoTarExperimentsState = splitterViewInfoTarExperimentsSize_;
    } else if (currentTabShow == "Tar"){
        splitterViewInfoTreeExperimentsState = splitterViewInfoTreeExperimentsSize_;
        splitterInfoFotoTreeExperimentsState = splitterInfoFotoTreeExperimentsSize_;
        splitterViewInfoAllExperimentsState = splitterViewInfoAllExperimentsSize_;
        splitterInfoFotoAllExperimentsState = splitterInfoFotoAllExperimentsSize_;
        splitterViewInfoTarExperimentsState = splitterViewInfoExperiments->saveState();
    }


    QDir settingsDir(QDir::currentPath());
    if(!settingsDir.exists("settings"))
        settingsDir.mkdir("settings");

    settingsJson_["/GUI/MainWindow/maximized"_json_pointer]=maximized;
    if (!maximized){
        settingsJson_["/GUI/MainWindow/Position/height"_json_pointer] = size().height();
        settingsJson_["/GUI/MainWindow/Position/width"_json_pointer] = size().width();
        settingsJson_["/GUI/MainWindow/Position/x"_json_pointer] = pos().x();
        settingsJson_["/GUI/MainWindow/Position/y"_json_pointer] = pos().y();
    }

    if (currentTabShow == "Experiments"){
        settingsJson_["/GUI/MainWindow/Tabs/lastOpened"_json_pointer] = "Experiments";
    } else if (currentTabShow == "AllExperiments"){
        settingsJson_["/GUI/MainWindow/Tabs/lastOpened"_json_pointer] = "AllExperiments";
    } else if (currentTabShow == "Tar"){
        settingsJson_["/GUI/MainWindow/Tabs/lastOpened"_json_pointer] = "Tar";
    }

    settingsJson_["/GUI/MainWindow/Tabs/TreeExperiments/SplitterViewInfo/state"_json_pointer]=std::string(splitterViewInfoTreeExperimentsState.toHex());
    settingsJson_["/GUI/MainWindow/Tabs/TreeExperiments/SplitterInfoFoto/state"_json_pointer]=std::string(splitterInfoFotoTreeExperimentsState.toHex());
    settingsJson_["/GUI/MainWindow/Tabs/AllExperiments/SplitterViewInfo/state"_json_pointer]=std::string(splitterViewInfoAllExperimentsState.toHex());
    settingsJson_["/GUI/MainWindow/Tabs/AllExperiments/SplitterInfoFoto/state"_json_pointer]=std::string(splitterInfoFotoAllExperimentsState.toHex());
    settingsJson_["/GUI/MainWindow/Tabs/TarExperiments/SplitterViewInfo/state"_json_pointer]=std::string(splitterViewInfoTarExperimentsState.toHex());

    std::ofstream file("settings/MCMIX.json");
    if(file.is_open()){
        file << std::setw(4) << settingsJson_;
        file.close();
    }
}

void MainWindow::createMenu()
{
    fileMenu = menuBar()->addMenu(tr("Файл"));
    fileMenu->addAction(exitAct);

    //helpMenu = menuBar()->addMenu(tr("Справка"));
    //helpMenu->addAction(infoAmpulesAct);
}

void MainWindow::createActions()
{
    exitAct = new QAction(tr("Выход"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    connect (exitAct, &QAction::triggered, this, &QWidget::close);

    infoAmpulesAct = new QAction(tr("Ампулы"), this);
}

void MainWindow::showExperiments()
{
    experimentsStorage->init();

    for (auto &sample : experimentsStorage->samples()){
        viewExperiments->addSampleGroup(sample);
        viewExperiments->addExperiments(sample, experimentsStorage->experiments(sample));

    }

    for (auto &ampoule : experimentsStorage->calibrationAmpules()){
        viewExperiments->addCalibrationGroup(ampoule);
        viewExperiments->addExperiments(ampoule, experimentsStorage->calibrations(ampoule));
    }

    viewExperiments->showAllExperiments();
    viewExperiments->showTarExperiments();

    QTimer::singleShot(200, this, &MainWindow::readGuiSettings);
}

void MainWindow::showExperimentInfo(const QString & experimentId)
{
    experimentWidget->show();

    QString experimentDir = experimentsStorage->getFileInfoDirById(experimentId);
    QString fileInfoName(experimentDir);
    fileInfoName.append("/info.txt");
    experimentInfo->showExperiment(fileInfoName);
    fotoView->addFotos(experimentDir);
    fotoViewIsVisible=fotoView->isVisible();

    experimentWidgetIsVisible = true;
}

void MainWindow::tabTreeExperimentsIsVisible()
{
    if (experimentWidgetIsVisible && experimentWidget->isHidden())
        experimentWidget->show();
    if (fotoViewIsVisible && fotoView->isHidden())
        fotoView->show();

    if(currentTabShow == "Tar"){
        splitterViewInfoTarExperimentsSize_ = splitterViewInfoExperiments->saveState();
        experimentWidget->hide();
        fotoView->hide();
    } else if (currentTabShow == "AllExperiments"){
        splitterViewInfoAllExperimentsSize_ = splitterViewInfoExperiments->saveState();
        splitterInfoFotoAllExperimentsSize_ = splitterInfoFotoExperiments->saveState();
    }

    splitterViewInfoExperiments->restoreState(splitterViewInfoTreeExperimentsSize_);
    splitterInfoFotoExperiments->restoreState(splitterInfoFotoTreeExperimentsSize_);

    currentTabShow = "Experiments";
}

void MainWindow::tabAllExperimentsIsVisible()
{
    if (experimentWidgetIsVisible && experimentWidget->isHidden())
        experimentWidget->show();
    if (fotoViewIsVisible && fotoView->isHidden())
        fotoView->show();


    if(currentTabShow == "Tar"){
        splitterViewInfoTarExperimentsSize_ = splitterViewInfoExperiments->saveState();
        experimentWidget->hide();
        fotoView->hide();
    } else if (currentTabShow == "Experiments"){
        splitterViewInfoTreeExperimentsSize_ = splitterViewInfoExperiments->saveState();
        splitterInfoFotoTreeExperimentsSize_ = splitterInfoFotoExperiments->saveState();
    }

    splitterViewInfoExperiments->restoreState(splitterViewInfoAllExperimentsSize_);
    splitterInfoFotoExperiments->restoreState(splitterInfoFotoAllExperimentsSize_);

    currentTabShow = "AllExperiments";
}

void MainWindow::tabTarIsVisible()
{
    fotoView->hide();
    experimentWidget->hide();

    if(currentTabShow == "AllExperiments"){
        splitterViewInfoAllExperimentsSize_ = splitterViewInfoExperiments->saveState();
        splitterInfoFotoAllExperimentsSize_ = splitterInfoFotoExperiments->saveState();
    } else if (currentTabShow == "Experiments"){
        splitterViewInfoTreeExperimentsSize_ = splitterViewInfoExperiments->saveState();
        splitterInfoFotoTreeExperimentsSize_ = splitterInfoFotoExperiments->saveState();
    }

    splitterViewInfoExperiments->restoreState(splitterViewInfoTarExperimentsSize_);

    currentTabShow = "Tar";
}
