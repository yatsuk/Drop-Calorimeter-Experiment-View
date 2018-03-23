#include "resultsExperimentWidget.h"
#include "experimentsStorage.h"
#include "extractDataWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QRegExp>
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <QGroupBox>
#include <fstream>

ResultsExperimentWidget::ResultsExperimentWidget(QWidget *parent) : QWidget(parent)
{
    QGroupBox * calculateExperimentGroupBox = new QGroupBox(tr("Расчёт эксперимента"));

    newCalculateExperimentButton_ = new QPushButton(tr("Новый расчёт"));
    openCalculateExperimentButton_ = new QPushButton(tr("Открыть расчёт"));
    openCalculateExperimentButton_->setDisabled(true);
    openMathcadFileButton_ = new QPushButton(tr("Открыть файл Mathcad"));
    openMathcadFileButton_->setDisabled(true);
    QTableWidget * calculateExperimentTable_ = new QTableWidget;
    calculateExperimentTable_->setColumnCount(3);
    calculateExperimentTable_->hide();

    QHBoxLayout * horLayoutcalculateExperimentGroupBox = new QHBoxLayout;
    horLayoutcalculateExperimentGroupBox->addWidget(newCalculateExperimentButton_);
    horLayoutcalculateExperimentGroupBox->addWidget(openCalculateExperimentButton_);
    horLayoutcalculateExperimentGroupBox->addWidget(openMathcadFileButton_);
    QVBoxLayout * layoutcalculateExperimentGroupBox = new QVBoxLayout;
    layoutcalculateExperimentGroupBox->addLayout(horLayoutcalculateExperimentGroupBox);
    layoutcalculateExperimentGroupBox->addWidget(calculateExperimentTable_);
    calculateExperimentGroupBox->setLayout(layoutcalculateExperimentGroupBox);

    extractExperimentDataButton_ = new QPushButton(tr("Извлечь данные"));

    QVBoxLayout * layout = new QVBoxLayout;
    layout->addWidget(calculateExperimentGroupBox);
    layout->addStretch();
    layout->addWidget(extractExperimentDataButton_);
    setLayout(layout);

    connect(newCalculateExperimentButton_, &QPushButton::clicked,
            this, &ResultsExperimentWidget::newCalculateExperimentButtonClicked);
    connect(extractExperimentDataButton_, &QPushButton::clicked,
            this, &ResultsExperimentWidget::extractExperimentDataButtonClicked);
}

void ResultsExperimentWidget::newCalculateExperimentButtonClicked()
{
    QProcess * calculateProgram = new QProcess(this);
    ExperimentsStorage ExperimentsStorage;
    if (ExperimentsStorage.isInited()){
        QStringList arguments;
        arguments << "--experimentDir" << ExperimentsStorage.getFileInfoDirById(currentExperimentId_)
                  << "--MCMIXDir" << ExperimentsStorage.pathMCMIXFolder()
                  << "--calc_variant" << "-1";
        QString program("/home/calorimeter/Projects/Drop-Calorimeter-Experiment-Calc/builds/build-Drop-Calorimeter-Experiment-Calc-Desktop_Qt_5_10_0_GCC_64bit-Release/Drop-Calorimeter-Experiment-Calc");
        calculateProgram->start(program, arguments);
    }
}

void ResultsExperimentWidget::extractExperimentDataButtonClicked()
{
    ExperimentsStorage ExperimentsStorage;

    QRegExp dataRegExp(".+(\\d{2}_\\d{2}_\\d{4})_\\d+$");
    QString experimentFolder = ExperimentsStorage.getFileInfoDirById(currentExperimentId_);
    dataRegExp.indexIn(experimentFolder);
    QDir dir(experimentFolder);
    dir.cdUp();

    ExtractDataWidget * extractDataWidget = new ExtractDataWidget(this);
    extractDataWidget->setAttribute(Qt::WA_DeleteOnClose);
    extractDataWidget->setModal(true);
    extractDataWidget->setExperimentFolderName(experimentFolder);
    extractDataWidget->setDataFileName(dir.path() + "/Data/data_" +dataRegExp.cap(1) +".txt");
    extractDataWidget->show();
}

void ResultsExperimentWidget::setCurrenExperiment(const QString & experimentId)
{
    currentExperimentId_ = experimentId;
    openExperimentJsonFile();
}

void ResultsExperimentWidget::openExperimentJsonFile()
{
    ExperimentsStorage ExperimentsStorage;
    json experimentJson;
    std::ifstream file(ExperimentsStorage.getFileInfoDirById(currentExperimentId_).toStdString() + "/data.json");
    if(file.is_open()){
        file >> experimentJson;
        file.close();
    }


}
