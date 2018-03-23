#ifndef EXPERIMENTSBROWSER_H
#define EXPERIMENTSBROWSER_H

#include <QObject>

class ExperimentsBrowser : public QObject
{
    Q_OBJECT
public:
    explicit ExperimentsBrowser(QObject *parent = 0);

signals:

public slots:
};

#endif // EXPERIMENTSBROWSER_H