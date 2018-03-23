#include "mainwindow.h"
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
    experimentsStorage->setSettings(settings_["Storage"].toObject());
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
    experimentInfo->setSettings(experimentInfoSettings_);
    resultsExperimentWidget = new ResultsExperimentWidget;

    fotoView = new FotoView;
    fotoView->setSettings(fotoViewSettings_);
    splitterInfoFotoExperiments = new QSplitter(Qt::Vertical, this);
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


    splitterViewInfoExperiments = new QSplitter(this);
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

    connect(fotoView, &FotoView::settingsChanged, this, &MainWindow::updateSettings);
    connect(experimentInfo, &ExperimentInfo::settingsChanged, this, &MainWindow::updateSettings);


    showExperiments();
}

MainWindow::~MainWindow()
{

}

void MainWindow::closeEvent(QCloseEvent *event)
{
    experimentsStorage->close();
    QJsonObject storageSettings = settings_["Storage"].toObject();
    storageSettings = experimentsStorage->getSettings();
    settings_["Storage"] = storageSettings;



    writeSettings();
    event->accept();
}

void MainWindow::updateSettings(QJsonObject settings)
{
    if (sender() == fotoView){
        fotoViewSettings_ = settings;
    } else if(sender() == experimentInfo){
        experimentInfoSettings_ = settings;
    }

}

void MainWindow::readSettings()
{
    QFile jsonSettingsFile("settings/MCMIX.json");
    if(jsonSettingsFile.exists()){
        if(jsonSettingsFile.open(QIODevice::ReadOnly)){
            settings_ = QJsonDocument::fromJson(jsonSettingsFile.readAll()).object();
            QJsonObject mainWindow = settings_["GUI"].toObject()["MainWindow"].toObject();
            QJsonObject mainWindowPosition = mainWindow["Position"].toObject();
            if (mainWindow["maximized"].toBool()){
                showMaximized();
            } else {
                resize(mainWindowPosition["width"].toInt(), mainWindowPosition["height"].toInt());
                move(mainWindowPosition["x"].toInt(), mainWindowPosition["y"].toInt());
            }
            QString splitterViewInfo;
            QString splitterInfoFoto;
            splitterViewInfo = settings_["GUI"].toObject()["MainWindow"].toObject()["Tabs"].toObject()["TreeExperiments"].toObject()["SplitterViewInfo"].toObject()["state"].toString();
            splitterInfoFoto = settings_["GUI"].toObject()["MainWindow"].toObject()["Tabs"].toObject()["TreeExperiments"].toObject()["SplitterInfoFoto"].toObject()["state"].toString();
            splitterViewInfoTreeExperimentsSize_ = QByteArray::fromHex(splitterViewInfo.toLatin1());
            splitterInfoFotoTreeExperimentsSize_ = QByteArray::fromHex(splitterInfoFoto.toLatin1());
            splitterViewInfo = settings_["GUI"].toObject()["MainWindow"].toObject()["Tabs"].toObject()["AllExperiments"].toObject()["SplitterViewInfo"].toObject()["state"].toString();
            splitterInfoFoto = settings_["GUI"].toObject()["MainWindow"].toObject()["Tabs"].toObject()["AllExperiments"].toObject()["SplitterInfoFoto"].toObject()["state"].toString();
            splitterViewInfoAllExperimentsSize_ = QByteArray::fromHex(splitterViewInfo.toLatin1());
            splitterInfoFotoAllExperimentsSize_ = QByteArray::fromHex(splitterInfoFoto.toLatin1());
            splitterViewInfo = settings_["GUI"].toObject()["MainWindow"].toObject()["Tabs"].toObject()["TarExperiments"].toObject()["SplitterViewInfo"].toObject()["state"].toString();
            splitterViewInfoTarExperimentsSize_ = QByteArray::fromHex(splitterViewInfo.toLatin1());

            experimentInfoSettings_ = settings_["GUI"].toObject()["ExperimentInfo"].toObject();
            fotoViewSettings_ = settings_["GUI"].toObject()["FotoView"].toObject();
            jsonSettingsFile.close();
        }
    }
}

void MainWindow::readGuiSettings()
{
    QString tabName = settings_["GUI"].toObject()["MainWindow"].toObject()["Tabs"].toObject()["lastOpened"].toString();
    if(tabName == "AllExperiments"){
        viewExperiments->showTab(ViewExperiments::TabName::allExperiment);
    } else if(tabName == "Tar"){
        viewExperiments->showTab(ViewExperiments::TabName::tar);
    } else {
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

    QJsonObject gui = settings_["GUI"].toObject();
    QJsonObject mainWindow = gui["MainWindow"].toObject();
    QJsonObject mainWindowPosition = mainWindow["Position"].toObject();
    mainWindow["maximized"]=maximized;
    if (!maximized){
        mainWindowPosition["height"] = size().height();
        mainWindowPosition["width"] = size().width();
        mainWindowPosition["x"] = pos().x();
        mainWindowPosition["y"] = pos().y();
    }
    mainWindow["Position"] = mainWindowPosition;

    QJsonObject tabs = mainWindow["Tabs"].toObject();
    if (currentTabShow == "Experiments"){
        tabs["lastOpened"] = "Experiments";
    } else if (currentTabShow == "AllExperiments"){
        tabs["lastOpened"] = "AllExperiments";
    } else if (currentTabShow == "Tar"){
        tabs["lastOpened"] = "Tar";
    }
    QJsonObject splitterViewInfo;
    QJsonObject splitterInfoFoto;
    QJsonObject tabTreeExperiments = tabs["TreeExperiments"].toObject();
    splitterViewInfo = tabTreeExperiments["SplitterViewInfo"].toObject();
    splitterInfoFoto = tabTreeExperiments["SplitterInfoFoto"].toObject();
    splitterViewInfo["state"]=QString(splitterViewInfoTreeExperimentsState.toHex());
    splitterInfoFoto["state"]=QString(splitterInfoFotoTreeExperimentsState.toHex());
    tabTreeExperiments["SplitterViewInfo"] = splitterViewInfo;
    tabTreeExperiments["SplitterInfoFoto"] = splitterInfoFoto;

    QJsonObject tabAllExperiments = tabs["AllExperiments"].toObject();
    splitterViewInfo = tabAllExperiments["SplitterViewInfo"].toObject();
    splitterInfoFoto = tabAllExperiments["SplitterInfoFoto"].toObject();
    splitterViewInfo["state"]=QString(splitterViewInfoAllExperimentsState.toHex());
    splitterInfoFoto["state"]=QString(splitterInfoFotoAllExperimentsState.toHex());
    tabAllExperiments["SplitterViewInfo"] = splitterViewInfo;
    tabAllExperiments["SplitterInfoFoto"] = splitterInfoFoto;

    QJsonObject tabTarExperiments = tabs["TarExperiments"].toObject();
    splitterViewInfo = tabTarExperiments["SplitterViewInfo"].toObject();
    splitterViewInfo["state"]=QString(splitterViewInfoTarExperimentsState.toHex());
    tabTarExperiments["SplitterViewInfo"] = splitterViewInfo;


    gui.insert("ExperimentInfo", experimentInfoSettings_);
    gui.insert("FotoView", fotoViewSettings_);


    tabs["TreeExperiments"] = tabTreeExperiments;
    tabs["AllExperiments"] = tabAllExperiments;
    tabs["TarExperiments"] = tabTarExperiments;
    mainWindow["Tabs"] = tabs;
    gui["MainWindow"]=mainWindow;

    settings_["GUI"]=gui;


    QFile jsonSettingsFile("settings/MCMIX.json");
    if(jsonSettingsFile.open(QIODevice::WriteOnly)){
        jsonSettingsFile.write(QJsonDocument(settings_).toJson());
    }

    jsonSettingsFile.close();
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
