#include "tarExperimentTable.h"
#include "tableWidgetItemNumeric.h"
#include "tableWidgetItemDate.h"
#include <QVBoxLayout>
#include <QHeaderView>
#include <QEvent>
#include <QKeyEvent>
#include <QJsonObject>
#include <QDate>
#include <QFont>
#include <QDebug>

TarExperimentTable::TarExperimentTable(QWidget *parent) : QWidget(parent)
{
    treeWidget_ = new QTreeWidget;
    treeWidget_->setHeaderHidden(true);
    treeWidget_->setSelectionMode(QAbstractItemView::NoSelection);
    treeWidget_->setFrameShape(QFrame::NoFrame);


    QVBoxLayout * layout = new QVBoxLayout;
    layout->addWidget(treeWidget_);

    setLayout(layout);

}

QTableWidget * TarExperimentTable::createEmptyTable()
{
    QStringList tableHeader;
    tableHeader << tr("Дата") << tr("№") << tr("Тем-ра, %1C").arg(QChar(186)) << tr("Время, с") << tr("Напряжение, В") << tr("Изм. сопр., Ом");

    QTableWidget * table = new QTableWidget;
    table->setColumnCount(6);
    table->setColumnWidth(1, 30);
    table->setSortingEnabled(true);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setHorizontalHeaderLabels(tableHeader);
    table->verticalHeader()->setDefaultAlignment(Qt::AlignCenter);
    table->setFrameShape(QFrame::NoFrame);
    table->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    table->resizeRowsToContents();
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->horizontalHeader()->setHighlightSections(false);
    /*fix!!!*/table->setMaximumHeight(500);//fix!!!
    table->installEventFilter(this);

    connect(table, &QTableWidget::itemClicked,
            this, &TarExperimentTable::experimentSelected);

    return table;
}

void TarExperimentTable::addExperiments(QMap <int, QJsonArray> tarMap)
{

    QFont boldBigFont(treeWidget_->font());
    boldBigFont.setBold(true);
    boldBigFont.setPixelSize(18);

    int i;
    for(int key : tarMap.keys()){
        QJsonArray tarArray = tarMap[key];

        QTreeWidgetItem * newTarGroup =  new QTreeWidgetItem(treeWidget_);
        newTarGroup->setFont(0, boldBigFont);

        QTreeWidgetItem * child = new QTreeWidgetItem;
        newTarGroup->addChild(child);


        QTableWidget * table= createEmptyTable();
        tables_.append(table);
        treeWidget_->setItemWidget(child, 0, table);



        table->setRowCount(tarArray.size());

        i =0;
        for (auto experimentInfo : tarArray){
            TableWidgetItemDate * itemDate =new TableWidgetItemDate;
            itemDate->setInFormat("dd_MM_yyyy H");
            itemDate->setOutFormat("dd.MM.yyyy");
            itemDate->setDate(experimentInfo.toObject()["date"].toString() + " "
                    + experimentInfo.toObject()["experimentNumber"].toString());
            itemDate->setTextAlignment( Qt::AlignCenter);
            itemDate->setData(Qt::UserRole, experimentInfo.toObject()["id"].toString());
            table->setItem(i,0,itemDate);

            QTableWidgetItem * itemExperimentNumber =new QTableWidgetItem(experimentInfo.toObject()["experimentNumber"].toString());
            itemExperimentNumber->setTextAlignment(Qt::AlignCenter);
            itemExperimentNumber->setData(Qt::UserRole, experimentInfo.toObject()["id"].toString());
            table->setItem(i,1,itemExperimentNumber);

            TableWidgetItemNumeric * itemTemperature =new TableWidgetItemNumeric;
            itemTemperature->setPrec(0);
            itemTemperature->setValue(experimentInfo.toObject()["thermostateTemperature"].toDouble(), QLocale::system());
            itemTemperature->setTextAlignment(Qt::AlignCenter);
            itemTemperature->setData(Qt::UserRole, experimentInfo.toObject()["id"].toString());
            table->setItem(i,2,itemTemperature);

            TableWidgetItemNumeric * itemHeaterTime =new TableWidgetItemNumeric;
            itemHeaterTime->setPrec(3);
            itemHeaterTime->setValue(experimentInfo.toObject()["heaterDurationTime"].toDouble(), QLocale::system());
            itemHeaterTime->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            itemHeaterTime->setData(Qt::UserRole, experimentInfo.toObject()["id"].toString());
            table->setItem(i,3,itemHeaterTime);

            TableWidgetItemNumeric * itemHeaterVoltage =new TableWidgetItemNumeric;
            itemHeaterVoltage->setPrec(1);
            itemHeaterVoltage->setValue(experimentInfo.toObject()["heaterVoltage"].toDouble(), QLocale::system());
            itemHeaterVoltage->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            itemHeaterVoltage->setData(Qt::UserRole, experimentInfo.toObject()["id"].toString());
            table->setItem(i,4,itemHeaterVoltage);

            TableWidgetItemNumeric * itemDeltaR =new TableWidgetItemNumeric;
            double deltaR = experimentInfo.toObject()["deltaR"].toDouble();
            if (deltaR)
                itemDeltaR->setValue(deltaR, QLocale::system());
            itemDeltaR->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            itemDeltaR->setData(Qt::UserRole, experimentInfo.toObject()["id"].toString());
            table->setItem(i,5,itemDeltaR);

            i++;
        }

        table->sortByColumn(0, Qt::AscendingOrder);
        table->setCurrentCell(i - 1, 0);
        table->clearSelection();

        QString dateFrom = table->item(0, 0)->text();
        QString dateTo = table->item(i - 1, 0)->text();
        if (dateFrom != dateTo){
            newTarGroup->setText(0, dateFrom + " - " + dateTo);
        } else {
            newTarGroup->setText(0, dateFrom);
        }

    }
}

void TarExperimentTable::experimentSelected(QTableWidgetItem * item)
{
    emit experimentNeedShow(item->data(Qt::UserRole).toString());

    clearSelectionNotActiveTable(item->tableWidget());
}

void TarExperimentTable::clearSelectionNotActiveTable(QTableWidget * activeTable)
{
    for (QTableWidget * table : tables_){
        if (table != activeTable)
            table->clearSelection();
    }
}

bool TarExperimentTable::eventFilter(QObject *obj, QEvent *event)
{
    for (QTableWidget * table : tables_){
        if (obj == table){
            if (event->type() == QEvent::KeyRelease){
                QKeyEvent * keyEvent = static_cast <QKeyEvent *>(event);
                int key = keyEvent->key();
                if (key == Qt::Key_Up || Qt::Key_Down){
                    experimentSelected(table->currentItem());
                }
            } else {
                return false;
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}
