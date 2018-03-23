#ifndef TABLEWIDGETITEMNUMERIC_H
#define TABLEWIDGETITEMNUMERIC_H

#include <QTableWidgetItem>

class TableWidgetItemNumeric : public QTableWidgetItem
{

public:
    double value(){return value_;}
    void setValue(double value);
    void setValue(double value, const QLocale & locale);
    void setValue(const  QString & valueStr);
    void setValue(const  QString & valueStr, const QLocale & locale);
    void setPrec(int prec){prec_ = prec;}
    void setFormat(QChar format){format_ = format;}
    void setFieldWidth(int fieldWidth){fieldWidth_ = fieldWidth;}
    void setFillChar(QChar fillChar){fillChar_ = fillChar;}

    bool operator < (const QTableWidgetItem &other) const
    {
        const TableWidgetItemNumeric * itemNumeric =  static_cast <const TableWidgetItemNumeric *> (&other);
        return value_ < itemNumeric->value_;
    }

    bool operator > (const QTableWidgetItem &other) const
    {
        const TableWidgetItemNumeric * itemNumeric =  static_cast <const TableWidgetItemNumeric *> (&other);
        return value_ > itemNumeric->value_;
    }

private:
    double value_;
    int prec_ = 2;
    QChar format_ = 'f';
    int fieldWidth_ = 0;
    QChar fillChar_ = ' ';
};

#endif // TABLEWIDGETITEMNUMERIC_H
