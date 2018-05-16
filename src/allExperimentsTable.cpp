#include "allExperimentsTable.h"
#include <QVBoxLayout>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonObject>
#include <QDate>
#include <QScrollBar>
#include <QEvent>
#include <QKeyEvent>
#include "tableWidgetItemDate.h"
#include <QDebug>

AllExperimentsTable::AllExperimentsTable(QWidget *parent) : QWidget(parent)
{

    QStringList tableHeader;
    tableHeader << tr("Дата") << tr("№") << tr("Ампула") << tr("Вещество")<< tr("Тем-ра, %1C").arg(QChar(186));

    experimentsTable_ = new QTableWidget;
    experimentsTable_->setColumnCount(5);
    experimentsTable_->setColumnWidth(1, 30);
    experimentsTable_->setRowCount(0);
    experimentsTable_->setSortingEnabled(true);
    experimentsTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    experimentsTable_->setHorizontalHeaderLabels(tableHeader);
    experimentsTable_->verticalHeader()->setDefaultAlignment(Qt::AlignCenter);
    experimentsTable_->setFrameShape(QFrame::NoFrame);
    experimentsTable_->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    experimentsTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    experimentsTable_->horizontalHeader()->setHighlightSections(false);
    experimentsTable_->installEventFilter(this);

    QVBoxLayout * layout = new QVBoxLayout;
    layout->addWidget(experimentsTable_);

    setLayout(layout);

    connect(experimentsTable_, &QTableWidget::itemClicked,
            this, &AllExperimentsTable::experimentSelected);
}

void AllExperimentsTable::addAllExperiments(const QJsonArray & dropsArray)
{
    experimentsTable_->setRowCount(dropsArray.size());
    int i =0;
    for (auto experimentInfo : dropsArray){
        TableWidgetItemDate * itemDate =new TableWidgetItemDate;
        itemDate->setInFormat("dd_MM_yyyy H");
        itemDate->setOutFormat("dd.MM.yyyy");
        itemDate->setDate(experimentInfo.toObject()["date"].toString() + " "
                + experimentInfo.toObject()["experimentNumber"].toString());
        itemDate->setTextAlignment( Qt::AlignCenter);
        itemDate->setData(Qt::UserRole, experimentInfo.toObject()["id"].toString());
        experimentsTable_->setItem(i,0,itemDate);

        QTableWidgetItem * itemExperimentNumber =new QTableWidgetItem(experimentInfo.toObject()["experimentNumber"].toString());
        itemExperimentNumber->setTextAlignment(Qt::AlignCenter);
        itemExperimentNumber->setData(Qt::UserRole, experimentInfo.toObject()["id"].toString());
        experimentsTable_->setItem(i,1,itemExperimentNumber);


        QString ampouleInfo;
        ampouleInfo.append(experimentInfo.toObject()["ampouleMaterial"].toString() + " ");
        ampouleInfo.append(experimentInfo.toObject()["ampouleDemountableType"].toString().replace(" ", "_") + " ");
        ampouleInfo.append(QString::number(experimentInfo.toObject()["ampouleType"].toInt()) + "-");
        ampouleInfo.append(QString::number(experimentInfo.toObject()["ampouleBatch"].toInt()) + "-");
        ampouleInfo.append(QString::number(experimentInfo.toObject()["ampouleNumber"].toInt()));
        QTableWidgetItem * itemAmpoule =new QTableWidgetItem(ampouleInfo);
        itemAmpoule->setTextAlignment(Qt::AlignCenter);
        itemAmpoule->setData(Qt::UserRole, experimentInfo.toObject()["id"].toString());
        experimentsTable_->setItem(i,2,itemAmpoule);

        QTableWidgetItem * itemSample =new QTableWidgetItem(experimentInfo.toObject()["sample"].toString());
        itemSample->setTextAlignment(Qt::AlignCenter);
        itemSample->setData(Qt::UserRole, experimentInfo.toObject()["id"].toString());
        experimentsTable_->setItem(i,3,itemSample);

        QTableWidgetItem * itemTemperature =new QTableWidgetItem;
        itemTemperature->setData(Qt::DisplayRole, experimentInfo.toObject()["temperatureFurnace"].toDouble());
        itemTemperature->setTextAlignment(Qt::AlignCenter);
        itemTemperature->setData(Qt::UserRole, experimentInfo.toObject()["id"].toString());
        experimentsTable_->setItem(i,4,itemTemperature);

        i++;
    }

    experimentsTable_->sortByColumn(0, Qt::AscendingOrder);
    experimentsTable_->resizeRowsToContents();
    experimentsTable_->setCurrentCell(i - 1, 0);
    experimentsTable_->clearSelection();
    experimentsTable_->setColumnWidth(1, 30);
}

void AllExperimentsTable::experimentSelected(QTableWidgetItem * item)
{
    emit experimentNeedShow(item->data(Qt::UserRole).toString());
}

bool AllExperimentsTable::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == experimentsTable_){
        if (event->type() == QEvent::KeyRelease){
            QKeyEvent * keyEvent = static_cast <QKeyEvent *>(event);
            int key = keyEvent->key();
            if (key == Qt::Key_Up || Qt::Key_Down){
                experimentSelected(experimentsTable_->currentItem());
            }
        } else {
            return false;
        }
    }

    return QWidget::eventFilter(obj, event);
}
