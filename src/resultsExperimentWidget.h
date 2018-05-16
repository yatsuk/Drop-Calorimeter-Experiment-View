#ifndef RESULTSEXPERIMENTWIDGET_H
#define RESULTSEXPERIMENTWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QTableWidget>
#include <QProcess>
#include "include/externals/nlohmann/json/json.hpp"

using json = nlohmann::json;


class ResultsExperimentWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ResultsExperimentWidget(QWidget *parent = 0);

signals:

public slots:
    void setCurrenExperiment(const QString &experimentId);
    void setSettings(json * settingsJson);

private slots:
    void newCalculateExperimentButtonClicked();
    void extractExperimentDataButtonClicked();
    void openExperimentResult(QTableWidgetItem * item);

private:
    void openExperimentJsonFile();
    void fillExperimentTable();

    QString  currentExperimentId_;
    QPushButton * newCalculateExperimentButton_;
    QTableWidget * calculateExperimentTable_;
    QPushButton * extractExperimentDataButton_;

    int calcCount_;
    json * settingsJson_;
};

#endif // RESULTSEXPERIMENTWIDGET_H
