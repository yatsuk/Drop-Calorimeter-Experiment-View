#ifndef TABLEWIDGETITEMDATE_H
#define TABLEWIDGETITEMDATE_H

#include <QTableWidgetItem>
#include <QDateTime>

class TableWidgetItemDate : public QTableWidgetItem
{
public:
    QDateTime date(){return date_;}
    void setDate(const QDateTime & date);
    void setDate(const QDateTime & date, const QLocale & locale);
    void setDate(const  QString & dateStr);
    void setDate(const  QString & dateStr, const QLocale & locale);
    void setInFormat(const QString & format){inFormat_ = format;}
    void setOutFormat(const QString & format){outFormat_ = format;}

    bool operator < (const QTableWidgetItem &other) const
    {
        const TableWidgetItemDate * itemDate =  static_cast <const TableWidgetItemDate *> (&other);
        return date_ < itemDate->date_;
    }

    bool operator > (const QTableWidgetItem &other) const
    {
        const TableWidgetItemDate * itemDate =  static_cast <const TableWidgetItemDate *> (&other);
        return date_ > itemDate->date_;
    }

private:
    QDateTime date_;
    QString inFormat_ = QLocale::system().dateFormat();
    QString outFormat_ = QLocale::system().dateFormat();
};

#endif // TABLEWIDGETITEMDATE_H
