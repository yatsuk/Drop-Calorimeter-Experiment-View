#ifndef VIEWEXPERIMENTS_H
#define VIEWEXPERIMENTS_H

#include <QWidget>
#include <QTabWidget>
#include <QTreeWidget>
#include <QTableWidget>
#include <QJsonObject>
#include <QJsonArray>
#include "allExperimentsTable.h"
#include "tarExperimentTable.h"

class ViewExperiments : public QWidget
{
    Q_OBJECT
public:
    enum class TabName {allExperiment, tar, experiment};
    explicit ViewExperiments(QWidget *parent = 0);

signals:
    void showExperiment(QString);
    void allExperimentsTableClicked();
    void treeExperimentsClicked();
    void tarClicked();

public slots:
    void addSampleGroup(const QString & sampleName);
    void addCalibrationGroup(const QString & ampouleName);
    void addExperiments(const QString & sampleName,const QJsonArray & experimentsInfo);
    void showAllExperiments();
    void showTarExperiments();
    void showTab(TabName name);

private slots:
    void experimentSelected(QTableWidgetItem * item);
    void tabBarClicked(int index);
    void clearSelectionNotActiveTable(QTableWidget * activeTable);

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    QTableWidget * createEmptyTable();

    QTreeWidget * tw_;
    QTreeWidgetItem * calibration_;
    QTreeWidgetItem * sample_;
    QTabWidget * tab_;
    AllExperimentsTable * allExperimentsTable_;
    TarExperimentTable * tarExperimentTable_;

    QVector <QTableWidget *> tables;
    QTableWidget * tarTable_;

};

#endif // VIEWEXPERIMENTS_H
