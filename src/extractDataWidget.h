#ifndef EXTRACTDATAWIDGET_H
#define EXTRACTDATAWIDGET_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>

class ExtractDataWidget : public QDialog
{
    Q_OBJECT
public:
    explicit ExtractDataWidget(QWidget *parent = nullptr);
    void setDataFileName(const QString & name);
    void setExperimentFolderName(const QString & folderName);

signals:

public slots:
    void extractSampleTemperature();

private:
    QString dataFileName_;
    QString experimentFolderName_;

    QLabel * dataFileNameLabel_;
    QLabel * statusLabel_;
    QLabel * fromLabel_;
    QLabel * toLabel_;
    QPushButton * extractButton_;
    QSpinBox * fromSpinBox_;
    QSpinBox * toSpinBox_;

};

#endif // EXTRACTDATAWIDGET_H
