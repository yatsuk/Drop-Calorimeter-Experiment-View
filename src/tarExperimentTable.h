#ifndef TAREXPERIMENTTABLE_H
#define TAREXPERIMENTTABLE_H

#include <QWidget>
#include <QTableWidget>
#include <QTreeWidget>
#include <QJsonArray>

class TarExperimentTable : public QWidget
{
    Q_OBJECT
public:
    explicit TarExperimentTable(QWidget *parent = 0);

signals:
    void experimentNeedShow(QString);

public slots:
    void addExperiments(QMap<int, QJsonArray> tarMap);

private slots:
    void experimentSelected(QTableWidgetItem * item);
    void clearSelectionNotActiveTable(QTableWidget * activeTable);

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    QTableWidget * createEmptyTable();

    QTreeWidget * treeWidget_;
    QVector <QTableWidget *> tables_;
};

#endif // TAREXPERIMENTTABLE_H
