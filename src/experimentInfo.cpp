#include "experimentInfo.h"
#include <QVBoxLayout>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QUrl>
#include <QDesktopServices>
#include <QTextStream>
#include <QEvent>
#include <QWheelEvent>
#include <QDebug>

ExperimentInfo::ExperimentInfo(QWidget *parent) : QWidget(parent)
{
    experimentInfoText = new QTextEdit;
    experimentInfoText->setReadOnly(true);
    experimentInfoText->installEventFilter(this);

    buttonOpenExperimentFolder = new QPushButton(tr("Открыть директорию эксперимента"));

    QVBoxLayout * layout = new QVBoxLayout;
    layout->addWidget(buttonOpenExperimentFolder);
    layout->addWidget(experimentInfoText);

    connect(buttonOpenExperimentFolder, &QPushButton::clicked,
            this, &ExperimentInfo::openExperimentFolder);

    setLayout(layout);
}

void ExperimentInfo::showExperiment(QString fileName)
{
    QFileInfo fileInfo(fileName);
    folderName_ = fileInfo.absolutePath();

    experimentInfoText->clear();

    QFile infoFile (fileName);
    if (!infoFile.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug() << fileName << "file not open";
        return;
    }


    QTextStream in(&infoFile);
    in.setCodec("CP1251");
    while (!in.atEnd()) {
        QString line = in.readLine();
        experimentInfoText->append(line);

    }

    infoFile.close();
}

void ExperimentInfo::setSettings(json *settingsJson)
{
    settingsJson_ = settingsJson;
    experimentInfoText->setFontPointSize((*settingsJson_)["fontPointSize"].is_number() ? static_cast <double>((*settingsJson_)["fontPointSize"]) : 12.0);
}

void ExperimentInfo::openExperimentFolder()
{
    QString path = QDir::toNativeSeparators(folderName_);
    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

bool ExperimentInfo::eventFilter(QObject *obj, QEvent *event)
{

    if (obj == experimentInfoText){
        if (event->type() == QEvent::Wheel){
            QWheelEvent * wheelEv = static_cast <QWheelEvent *> (event);
            if (wheelEv->modifiers() == Qt::ControlModifier){
                experimentInfoText->selectAll();
                if (wheelEv->delta() >0){
                    experimentInfoText->setFontPointSize(experimentInfoText->fontPointSize() + 1);
                } else {
                    experimentInfoText->setFontPointSize(experimentInfoText->fontPointSize() - 1);
                }
                QTextCursor cursor = experimentInfoText->textCursor();
                cursor.movePosition( QTextCursor::End);
                experimentInfoText->setTextCursor( cursor );
                if(settingsJson_!=nullptr)
                    (*settingsJson_)["fontPointSize"] = experimentInfoText->fontPointSize();
            }
            return true;
        } else {
            return false;
        }
    }

    return QWidget::eventFilter(obj, event);
}

