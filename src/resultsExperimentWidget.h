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

private slots:
    void newCalculateExperimentButtonClicked();
    void extractExperimentDataButtonClicked();

private:
    void openExperimentJsonFile();

    QString  currentExperimentId_;
    QPushButton * newCalculateExperimentButton_;
    QPushButton * openCalculateExperimentButton_;
    QPushButton * openMathcadFileButton_;
    QTableWidget * calculateExperimentTable_;
    QPushButton * extractExperimentDataButton_;

    int calcCount_;
};

#endif // RESULTSEXPERIMENTWIDGET_H
