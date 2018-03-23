#ifndef FOTOVIEW_H
#define FOTOVIEW_H

#include <QWidget>
#include <QLabel>
#include <QVector>
#include <QPixmap>
#include <QHBoxLayout>
#include <QMap>
#include "include/externals/nlohmann/json/json.hpp"

using json = nlohmann::json;

class FotoView : public QWidget
{
    Q_OBJECT
public:
    explicit FotoView(QWidget *parent = 0);
    void setSettings(json * settingsJson);

public slots:
    void addFotos(const QString & experimentFolder);

protected:
    void paintEvent(QPaintEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void showFotoBigSize(const QPixmap * pixmap);

private:
    void createLabels(int count);
    void insertFotoToLabel();

    QVector <QLabel *> labels;
    QHBoxLayout * fotoLayout;
    QVector <QPixmap * > fotos;
    QMap <QLabel *, QPixmap *> mapLablePixmap;
    QSize oldSizeLabel;

    json * settingsJson_;
};

class FotoBigSizeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FotoBigSizeWidget(QWidget *parent = 0);

public slots:
    void setPixmap(QPixmap pixmap);
    void setSettings(json *settingsJson);

protected:
    void paintEvent(QPaintEvent *event);
    void closeEvent(QCloseEvent *event);

private:
    void readSettings();
    void writeSettings();
    void insertFotoToLabel();

    QLabel * fotoLabel;
    QPixmap  pixmap_;
    QSize oldSizeLabel;

    json * settingsJson_;
};

#endif // FOTOVIEW_H
