#include "fotoView.h"
#include <QDir>
#include <QScrollArea>
#include <QPixmap>
#include <QLabel>
#include <QDebug>
#include <QEvent>
#include <QCloseEvent>

FotoView::FotoView(QWidget *parent) : QWidget(parent)
{
    fotoLayout = new QHBoxLayout;
    setLayout(fotoLayout);
    hide();
}

void FotoView::setSettings(json *settingsJson)
{
    settingsJson_ = settingsJson;
}

void FotoView::showFotoBigSize(const QPixmap *pixmap)
{
    FotoBigSizeWidget * fotoWidget = new FotoBigSizeWidget;
    fotoWidget->setSettings(settingsJson_);
    fotoWidget->setAttribute(Qt::WA_DeleteOnClose);
    fotoWidget->setPixmap(*pixmap);
    fotoWidget->show();
}

void FotoView::createLabels(int count)
{
    for (auto label : labels){
        fotoLayout->removeWidget(label);
        delete label;
    }

    labels.clear();

    for (int i = 0; i < count; ++i){
        QLabel * label = new QLabel("test");
        label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
        fotoLayout->addWidget(label);
        labels.append(label);

        label->installEventFilter(this);
    }
}

void FotoView::addFotos(const QString & experimentFolder)
{
    mapLablePixmap.clear();

    for (auto foto : fotos)
        delete foto;
    fotos.clear();


    QStringList filters;
    filters << "*.jpg" << "*.png";

    QDir dirFoto(experimentFolder);
    dirFoto.cd("foto");
    dirFoto.setNameFilters(filters);

    if (dirFoto.exists()){
        QStringList listFileFoto = dirFoto.entryList(QDir::Files);
        if (!listFileFoto.isEmpty()){
            createLabels(listFileFoto.size());
            for (auto &fileFoto : listFileFoto){
                fotos.append(new QPixmap(dirFoto.absoluteFilePath(fileFoto)));
            }

            oldSizeLabel = QSize(0, 0);
            show();
            return;
        }
    }
    hide();
}


void FotoView::insertFotoToLabel()
{
    if(!labels.isEmpty() && labels[0]->size()!=oldSizeLabel){
        for (int i = 0; i < labels.size(); ++i){
            mapLablePixmap[labels[i]]= fotos[i];
            labels[i]->setPixmap(fotos[i]->scaled(labels[i]->size(), Qt::KeepAspectRatio));
        }
        if (!labels.isEmpty())
            oldSizeLabel = labels[0]->size();
    }
}

void FotoView::paintEvent(QPaintEvent *event)
{
    insertFotoToLabel();

    QWidget::paintEvent(event);
}

bool FotoView::eventFilter(QObject *obj, QEvent *event)
{
    for (QLabel * label : labels){
        if (obj == label){
            if (event->type() == QEvent::MouseButtonDblClick){
                showFotoBigSize(mapLablePixmap[label]);
                return true;
            } else {
                return false;
            }
        }
    }

    return QWidget::eventFilter(obj, event);
}








FotoBigSizeWidget::FotoBigSizeWidget(QWidget *parent) :
    QWidget(parent)
{
    fotoLabel = new QLabel;
    fotoLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    QVBoxLayout * layout = new QVBoxLayout;
    layout->addWidget(fotoLabel);

    setLayout(layout);
}

void FotoBigSizeWidget::setSettings(json * settingsJson)
{
    settingsJson_ = settingsJson;
    readSettings();
}

void FotoBigSizeWidget::readSettings()
{
    if (settingsJson_!=nullptr){
        resize((*settingsJson_)["/BigFoto/SizeWindow/width"_json_pointer].is_number() ? static_cast<unsigned int>((*settingsJson_)["/BigFoto/SizeWindow/width"_json_pointer]) : 600,
            (*settingsJson_)["/BigFoto/SizeWindow/height"_json_pointer].is_number() ? static_cast<unsigned int>((*settingsJson_)["/BigFoto/SizeWindow/height"_json_pointer]) : 800);
    }
}

void FotoBigSizeWidget::writeSettings()
{
    if (settingsJson_!=nullptr){
        (*settingsJson_)["/BigFoto/SizeWindow/width"_json_pointer]=size().width();
        (*settingsJson_)["/BigFoto/SizeWindow/height"_json_pointer]=size().height();
    }
}

void FotoBigSizeWidget::closeEvent(QCloseEvent *event)
{
    writeSettings();
    event->accept();
}

void FotoBigSizeWidget::setPixmap(QPixmap  pixmap)
{
    pixmap_ = pixmap;
}

void FotoBigSizeWidget::insertFotoToLabel()
{
    if(fotoLabel->size()!=oldSizeLabel){
        fotoLabel->setPixmap(pixmap_.scaled(fotoLabel->size(), Qt::KeepAspectRatio));
        oldSizeLabel = fotoLabel->size();
    }
}

void FotoBigSizeWidget::paintEvent(QPaintEvent *event)
{
    insertFotoToLabel();

    QWidget::paintEvent(event);
}
