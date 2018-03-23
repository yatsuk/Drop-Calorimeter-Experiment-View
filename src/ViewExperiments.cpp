#include "ViewExperiments.h"
#include "experimentsStorage.h"
#include "tableWidgetItemDate.h"
#include <QVBoxLayout>
#include <QHeaderView>
#include <QFont>
#include <QDate>
#include <QEvent>
#include <QKeyEvent>
#include <QDebug>

ViewExperiments::ViewExperiments(QWidget *parent) : QWidget(parent)
{

    tw_ = new QTreeWidget;
    tw_->setHeaderHidden(true);
    tw_->setSelectionMode(QAbstractItemView::NoSelection);
    tw_->setStyleSheet("QTreeWidget {font-weight: bold;}");
    tw_->setFrameShape(QFrame::NoFrame);


    QFont font;
    calibration_ =  new QTreeWidgetItem(tw_);
    font = calibration_->font(0);
    font.setPointSize(12);
    calibration_->setText(0, tr("Калибровка"));
    calibration_->setFont(0, font);

    sample_ =  new QTreeWidgetItem(tw_);
    sample_->setText(0, tr("Образец"));
    sample_->setFont(0, font);

    allExperimentsTable_ = new AllExperimentsTable;

    tarExperimentTable_ = new TarExperimentTable;

    tab_ = new QTabWidget;
    tab_->addTab(tw_, tr("Эксперименты"));
    tab_->addTab(allExperimentsTable_, tr("Сводная таблица"));
    tab_->addTab(tarExperimentTable_, tr("Тарировка"));

    QVBoxLayout * layout = new QVBoxLayout;
    layout->addWidget(tab_);

    setLayout(layout);


    connect(tab_, &QTabWidget::currentChanged, this, &ViewExperiments::tabBarClicked);
    connect(allExperimentsTable_, &AllExperimentsTable::experimentNeedShow,
            this, &ViewExperiments::showExperiment);
    connect(tarExperimentTable_, &TarExperimentTable::experimentNeedShow,
            this, &ViewExperiments::showExperiment);
}

void ViewExperiments::addSampleGroup(const QString & sampleName)
{
    QTreeWidgetItem * newSample =  new QTreeWidgetItem(sample_);
    newSample->setText(0, sampleName);

    QTreeWidgetItem * child = new QTreeWidgetItem;
    newSample->addChild(child);

    QTableWidget * table= createEmptyTable();
    tables.append(table);
    tw_->setItemWidget(child, 0, table);

}

void ViewExperiments::addCalibrationGroup(const QString & ampouleName)
{
    QTreeWidgetItem * newAmpoule =  new QTreeWidgetItem(calibration_);
    newAmpoule->setText(0, ampouleName);

    QTreeWidgetItem * child = new QTreeWidgetItem;
    newAmpoule->addChild(child);

    QTableWidget * table= createEmptyTable();
    tables.append(table);
    tw_->setItemWidget(child, 0, table);
}

QTableWidget * ViewExperiments::createEmptyTable()
{
    QStringList tableHeader;
    tableHeader << tr("Дата") << tr("№") << tr("Тем-ра, %1C").arg(QChar(186));

    QTableWidget * table = new QTableWidget(tw_);
    table->setColumnCount(3);
    table->setColumnWidth(1, 30);
    table->setRowCount(0);
    table->setSortingEnabled(true);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setHorizontalHeaderLabels(tableHeader);
    table->verticalHeader()->setDefaultAlignment(Qt::AlignCenter);
    table->setFrameShape(QFrame::NoFrame);
    table->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->horizontalHeader()->setHighlightSections(false);
    /*fix!!!*/table->setMaximumHeight(500);//fix!!!
    table->installEventFilter(this);

    connect(table, &QTableWidget::itemClicked,
            this, &ViewExperiments::experimentSelected);

    return table;
}

void ViewExperiments::addExperiments(const QString & sampleName,const QJsonArray & experimentsInfo)
{
    if(experimentsInfo.isEmpty()) return;
    QString experimentType = experimentsInfo.at(0).toObject()["experimentType"].toString().trimmed();
    QTreeWidgetItem * parentItem;
    if (experimentType == "experiment"){
        parentItem = sample_;
    } else if(experimentType == "calibration"){
        parentItem = calibration_;
    }


    for (int i = 0; i < parentItem->childCount(); ++i){
        if(parentItem->child(i)->text(0) == sampleName){
            QObject * obj = tw_->itemWidget(parentItem->child(i)->child(0), 0);
            if (obj){
                QTableWidget * table = qobject_cast <QTableWidget *> (obj);
                table->setRowCount(experimentsInfo.size());
                int i =0;
                for (auto experimentInfo : experimentsInfo){
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

                    QTableWidgetItem * itemTemperature =new QTableWidgetItem;
                    itemTemperature->setData(Qt::DisplayRole, experimentInfo.toObject()["temperatureFurnace"].toDouble());
                    itemTemperature->setTextAlignment(Qt::AlignCenter);
                    itemTemperature->setData(Qt::UserRole, experimentInfo.toObject()["id"].toString());
                    table->setItem(i,2,itemTemperature);

                    i++;
                }

                table->sortByColumn(0, Qt::AscendingOrder);
                table->setCurrentCell(i - 1, 0);
                table->clearSelection();
            }
        }
    }
}

void ViewExperiments::showAllExperiments()
{
    if (!ExperimentsStorage::isInited()) return;
    QJsonArray dropsArray;
    for (auto drop : ExperimentsStorage::jsonStorageExperimentShortInfo_){
        QString experimentType = drop.toObject()["experimentType"].toString();
        if (experimentType == "experiment" || experimentType == "calibration"){
            dropsArray.append(drop);
        }
    }

    allExperimentsTable_->addAllExperiments(dropsArray);
}

void ViewExperiments::showTarExperiments()
{ 
    if (!ExperimentsStorage::isInited()) return;

    QMap <int, QJsonArray> tarsMap;
    for (auto experiment : ExperimentsStorage::jsonStorageExperimentShortInfo_){
        QString experimentType = experiment.toObject()["experimentType"].toString();
        if (experimentType == "tar"){
                tarsMap[experiment.toObject()["series"].toInt()].append(experiment);
        }
    }

    tarExperimentTable_->addExperiments(tarsMap);
}

void ViewExperiments::experimentSelected(QTableWidgetItem * item)
{
    showExperiment(item->data(Qt::UserRole).toString());

    clearSelectionNotActiveTable(item->tableWidget());
}

void ViewExperiments::clearSelectionNotActiveTable(QTableWidget * activeTable)
{
    for (QTableWidget * table : tables){
        if (table != activeTable)
            table->clearSelection();
    }
}

void ViewExperiments::tabBarClicked(int index)
{
    switch (index) {
    case 0:
        emit treeExperimentsClicked();
        break;
    case 1:
        emit allExperimentsTableClicked();
        break;
    case 2:
        emit tarClicked();
        break;
    default:
        break;
    }
}

void ViewExperiments::showTab(TabName name)
{
    switch (name) {
    case TabName::allExperiment:
        tab_->setCurrentIndex(1);
        break;
    case TabName::experiment:
        tab_->setCurrentIndex(0);
        break;
    case TabName::tar:
        tab_->setCurrentIndex(2);
        break;
    }
}

bool ViewExperiments::eventFilter(QObject *obj, QEvent *event)
{
    for (QTableWidget * table : tables){
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
