#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QSplitter>
#include <QTabWidget>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include "include/externals/nlohmann/json/json.hpp"
#include "ViewExperiments.h"
#include "experimentsStorage.h"
#include "experimentInfo.h"
#include "fotoView.h"
#include "resultsExperimentWidget.h"

using json = nlohmann::json;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void readGuiSettings();
    void showExperiments();
    void showExperimentInfo(const QString & experimentId);
    void tabAllExperimentsIsVisible();
    void tabTreeExperimentsIsVisible();
    void tabTarIsVisible();

private:
    void createMenu();
    void createActions();
    void readSettings();
    void writeSettings();

    QWidget * experimentWidget;
    ViewExperiments * viewExperiments;
    ExperimentInfo * experimentInfo;
    FotoView * fotoView;
    QSplitter * splitterViewInfoExperiments;
    QSplitter * splitterInfoFotoExperiments;

    ExperimentsStorage * experimentsStorage;
    ResultsExperimentWidget * resultsExperimentWidget;
    QByteArray splitterViewInfoTreeExperimentsSize_;
    QByteArray splitterInfoFotoTreeExperimentsSize_;
    QByteArray splitterViewInfoAllExperimentsSize_;
    QByteArray splitterInfoFotoAllExperimentsSize_;
    QByteArray splitterViewInfoTarExperimentsSize_;
    QString currentTabShow;
    bool fotoViewIsVisible = false;
    bool experimentWidgetIsVisible = false;

    QMenu * fileMenu;
    QMenu * helpMenu;

    QAction * exitAct;
    QAction * infoAmpulesAct;

    json settingsJson_;
};


#endif // MAINWINDOW_H
