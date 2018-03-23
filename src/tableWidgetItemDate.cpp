#include "tableWidgetItemDate.h"
#include <QDebug>

void TableWidgetItemDate::setDate(const QDateTime & date)
{
    date_ = date;
    setText(date.toString(outFormat_));
}

void TableWidgetItemDate::setDate(const QDateTime &date, const QLocale & locale)
{
    date_ = date;
    setText(date.toString(locale.dateFormat(QLocale::ShortFormat)));
}

void TableWidgetItemDate::setDate(const  QString & dateStr)
{
    QDateTime date(QDateTime::fromString(dateStr, inFormat_));

    if (date.isValid())
        setDate(date);

}

void TableWidgetItemDate::setDate(const  QString & dateStr, const QLocale & locale)
{
    QDateTime date(QDateTime::fromString(dateStr, inFormat_));

    if (date.isValid())
        setDate(date, locale);
}
