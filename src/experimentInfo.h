#ifndef EXPERIMENTINFO_H
#define EXPERIMENTINFO_H

#include <QWidget>
#include <QTextEdit>
#include <QPushButton>
#include "include/externals/nlohmann/json/json.hpp"

using json = nlohmann::json;

class ExperimentInfo : public QWidget
{
    Q_OBJECT
public:
    explicit ExperimentInfo(QWidget *parent = 0);
    void setSettings(json * settingsJson);

public slots:
    void showExperiment(QString fileName);

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void openExperimentFolder();

private:
    QTextEdit * experimentInfoText;
    QPushButton * buttonOpenExperimentFolder;

    QString folderName_;
    json * settingsJson_;

};

#endif // EXPERIMENTINFO_H
