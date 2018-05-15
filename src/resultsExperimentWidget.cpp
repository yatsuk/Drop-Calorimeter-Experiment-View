#include "resultsExperimentWidget.h"
#include "experimentsStorage.h"
#include "extractDataWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QRegExp>
#include <QDir>
#include <QFileInfo>
#include <QHeaderView>
#include <QGroupBox>
#include <fstream>
#include <QDebug>

ResultsExperimentWidget::ResultsExperimentWidget(QWidget *parent) : QWidget(parent)
{
    QGroupBox * calculateExperimentGroupBox = new QGroupBox(tr("Расчёт эксперимента"));

    newCalculateExperimentButton_ = new QPushButton(tr("Новый расчёт"));

    QStringList tableHeader;
    tableHeader <<  tr("Имя") << tr("Комментарий");
    calculateExperimentTable_ = new QTableWidget;
    calculateExperimentTable_->setColumnCount(2);
    calculateExperimentTable_->setColumnWidth(0, 200);
    calculateExperimentTable_->setColumnWidth(1, 400);
    calculateExperimentTable_->setSortingEnabled(true);
    calculateExperimentTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    calculateExperimentTable_->setHorizontalHeaderLabels(tableHeader);
    calculateExperimentTable_->verticalHeader()->setDefaultAlignment(Qt::AlignCenter);
    calculateExperimentTable_->setFrameShape(QFrame::NoFrame);
    calculateExperimentTable_->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    calculateExperimentTable_->setSelectionMode(QAbstractItemView::NoSelection);
    calculateExperimentTable_->horizontalHeader()->setHighlightSections(false);
    calculateExperimentTable_->installEventFilter(this);

    QHBoxLayout * horLayoutcalculateExperimentGroupBox = new QHBoxLayout;
    horLayoutcalculateExperimentGroupBox->addWidget(newCalculateExperimentButton_);
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

    connect(calculateExperimentTable_, &QTableWidget::itemDoubleClicked,
            this, &ResultsExperimentWidget::openExperimentResult);
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
    fillExperimentTable();
}

void ResultsExperimentWidget::openExperimentJsonFile()
{
    ExperimentsStorage experimentsStorage;
    json experimentJson;
    std::ifstream file(experimentsStorage.getFileInfoDirById(currentExperimentId_).toStdString() + "/data.json");
    if(file.is_open()){
        file >> experimentJson;
        file.close();
    }
}

void ResultsExperimentWidget::fillExperimentTable()
{
    ExperimentsStorage experimentsStorage;
    QDir dir(experimentsStorage.getFileInfoDirById(currentExperimentId_));
    QStringList resultsList = dir.entryList(QStringList() << "*.mcd", QDir::Files);
    if(resultsList.size() == 0){
        calculateExperimentTable_->hide();
    } else {
        calculateExperimentTable_->show();
        calculateExperimentTable_->setRowCount(resultsList.size());
    }

    for (int i = 0; i < resultsList.size(); ++i){
        QTableWidgetItem * item =new QTableWidgetItem(resultsList.at(i));
        item->setTextAlignment(Qt::AlignCenter);
        item->setData(Qt::UserRole, "Mathcad_calculation");
        calculateExperimentTable_->setItem(i,0,item);
    }

}

void ResultsExperimentWidget::openExperimentResult(QTableWidgetItem * item)
{
    if(item->data(Qt::UserRole).toString() == "Mathcad_calculation"){
        ExperimentsStorage experimentsStorage;
        qDebug() << (experimentsStorage.getFileInfoDirById(currentExperimentId_) + "/"+  item->text());
    }
}
