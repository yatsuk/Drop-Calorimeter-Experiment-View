#include "tableWidgetItemNumeric.h"

void TableWidgetItemNumeric::setValue(double value)
{
    value_ = value;
    setText(QString("%1").arg(value, fieldWidth_, format_.toLatin1(), prec_, fillChar_));
}

void TableWidgetItemNumeric::setValue(double value, const QLocale & locale){
    value_ = value;
    setText(locale.toString(value, format_.toLatin1(), prec_));
}

void TableWidgetItemNumeric::setValue(const  QString & valueStr)
{
    bool ok;
    double value = valueStr.toDouble(&ok);
    if (ok)
        setValue(value);
}

void TableWidgetItemNumeric::setValue(const  QString & valueStr, const QLocale & locale)
{
    bool ok;
    double value = valueStr.toDouble(&ok);
    if (ok)
        setValue(value, locale);
}
