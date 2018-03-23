#include "extractDataWidget.h"
#include <QFile>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>

ExtractDataWidget::ExtractDataWidget(QWidget *parent) : QDialog(parent)
{
    dataFileNameLabel_ = new QLabel;
    statusLabel_ = new QLabel;
    fromLabel_ = new QLabel(tr("От, с"));
    toLabel_ = new QLabel(tr("До, с"));

    fromSpinBox_ = new QSpinBox;
    fromSpinBox_->setRange(-1, 1e+5);
    fromSpinBox_->setValue(-1);
    toSpinBox_ = new QSpinBox;
    toSpinBox_->setRange(-1, 1e+5);
    toSpinBox_->setValue(-1);
    extractButton_ = new QPushButton(tr("Извлечь температуру образца"));

    QHBoxLayout * hLayout = new QHBoxLayout;
    hLayout->addWidget(fromLabel_);
    hLayout->addWidget(fromSpinBox_);
    hLayout->addWidget(toLabel_);
    hLayout->addWidget(toSpinBox_);
    hLayout->addStretch();

    QVBoxLayout * layout = new QVBoxLayout;
    layout->addWidget(dataFileNameLabel_);
    layout->addWidget(statusLabel_);
    layout->addLayout(hLayout);
    layout->addWidget(extractButton_);

    setLayout(layout);

    connect(extractButton_, &QPushButton::clicked, this, &ExtractDataWidget::extractSampleTemperature);
}

void ExtractDataWidget::setDataFileName(const QString & name)
{
    dataFileName_ = name;
    dataFileNameLabel_->setText(tr("Файл эксперимента: ") + dataFileName_);
}

void ExtractDataWidget::setExperimentFolderName(const QString & folderName)
{
    experimentFolderName_ = folderName;
}

void ExtractDataWidget::extractSampleTemperature()
{
    statusLabel_->setText(tr("Извлечение"));

    QFile dataFile(dataFileName_);
    if(!dataFile.open(QIODevice::ReadOnly | QIODevice::Text)){
        statusLabel_->setText(tr("Файл не найден"));
        return;
    }

    QFile exportDataFile(experimentFolderName_ + "/TSampleExport.dat");
    if(!exportDataFile.open(QIODevice::WriteOnly | QIODevice::Text)){
        return;
    }
    exportDataFile.write("time, sec\tTemper sample, mV\r\n");

    int timeFrom = (fromSpinBox_->value()==-1) ? 0 : fromSpinBox_->value();
    int timeTo = (toSpinBox_->value()==-1) ? toSpinBox_->maximum() : toSpinBox_->value();

    QString timeStr;
    bool timeString2DoubleConvertOk;
    double time;

    QString valueStr;
    bool valueString2DoubleConvertOk;
    double value;

    QString sampleTemperatureMvId = "{0986e158-6266-4d5e-8498-fa5c3cd84bbe}";
    double sampleTemperatureMultiplier = 1e+3;

    while(!dataFile.atEnd()){
        QString line = dataFile.readLine();
        QStringList stringList = line.split('\t');
        if (stringList.size() == 5 && stringList.at(1)==sampleTemperatureMvId){
            timeStr = stringList.at(0);
            timeStr.trimmed().replace(",", ".");
            time = timeStr.toDouble(&timeString2DoubleConvertOk);

            valueStr = stringList.at(2);
            valueStr.trimmed().replace(",", ".");
            value = valueStr.toDouble(&valueString2DoubleConvertOk);
            if(timeString2DoubleConvertOk && valueString2DoubleConvertOk){
                if (time >= timeFrom && time <= timeTo){
                    exportDataFile.write(QString::number(time).toLatin1());
                    exportDataFile.write("\t");
                    exportDataFile.write(QString::number(value*sampleTemperatureMultiplier, 'g', 7).toLatin1());
                    exportDataFile.write("\r\n");
                }
            }
        }
    }

    dataFile.close();
    exportDataFile.close();

    statusLabel_->setText(tr("Выполнено:\n") + experimentFolderName_ + "/TSampleExport.dat");
}
