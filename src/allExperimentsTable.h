#ifndef ALLEXPERIMENTSTABLE_H
#define ALLEXPERIMENTSTABLE_H

#include <QWidget>
#include <QTableWidget>

class AllExperimentsTable : public QWidget
{
    Q_OBJECT
public:
    explicit AllExperimentsTable(QWidget *parent = 0);

signals:
    void experimentNeedShow(QString);

public slots:
    void addAllExperiments(const QJsonArray &dropsArray);

private slots:
    void experimentSelected(QTableWidgetItem * item);

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    QTableWidget * experimentsTable_;
};

#endif // ALLEXPERIMENTSTABLE_H
