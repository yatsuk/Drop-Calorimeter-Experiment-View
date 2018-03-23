#ifndef ExperimentsStorage_H
#define ExperimentsStorage_H

#include <QObject>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QMap>
#include <QDate>
#include "include/externals/nlohmann/json/json.hpp"

using json = nlohmann::json;

class ExperimentsStorage : public QObject
{
    Q_OBJECT
public:
    explicit ExperimentsStorage(QObject *parent = 0);
    void setSettings(json * settingsJson);
    bool setPathSampleFolder(const QString & path);
    bool setPathCalibrationFolder(const QString & path);
    bool setPathTarFolder(const QString & path);
    bool setPathMCMIXFolder(const QString & path);
    QString pathSampleFolder(){return pathSampleFolder_;}
    QString pathCalibrationFolder(){return pathCalibrationFolder_;}
    QString pathTarFolder(){return pathTarFolder_;}
    QString pathMCMIXFolder(){return pathMCMIXFolder_;}
    static bool isInited(){return isInited_;}

    static QJsonObject jsonStorageExperimentShortInfo_;

signals:

public slots:
    QStringList samples();
    QStringList calibrationAmpules();
    QJsonArray experiments(const QString & sampleName);
    QJsonArray calibrations(const QString & ampoule);
    bool init();
    bool close();
    QString getFileInfoDirById(const QString & id);

private:
    QJsonObject parametersExperimentFromFileInfo(const QString & fileName);
    void addExperimentIntoJsonStorageFile(QJsonObject *experiment);
    void readSettings();
    bool validateFolder(const QString & infoFilePath, const QString & magicString);
    bool saveJsonFile(const QJsonObject &experimentInfo);
    bool saveJsonStorageToFile();
    bool loadJsonStorageFromFile();
    bool isExistExperimentInStorage(const QString & path);
    bool addJsonObjectExperimentParametr(QJsonObject * parameters, const QString & paramName, QJsonValue value, const QString & description, const QString & unit = "", double multiplier = 1);


    static json * settingsJson_;
    static QString pathSampleFolder_;
    static QString pathCalibrationFolder_;
    static QString pathTarFolder_;
    static QString pathMCMIXFolder_;

    static bool isInited_;
};

class ExperimentDataJson
{
public:
    enum class Axis{x ,y};

    void setAxisInfo(Axis axis, const QString & unit, const QString & description, double multiplier);
    void setExperimentFile(const QString & fileName, QChar separator, int colNumberX, int colNumberY);
    QJsonObject getExperimentDataJson(){return experimentDataJson_;}

private:
    QVector <QPointF> getPointArrayFromFile(const QString & fileName, QChar separator, int colNumberX, int colNumberY);

    QJsonObject experimentDataJson_;
};

class ParserExpr;
class ParserInfoFile
{
public:
    void setFileName(const QString & fileName);
    void addParseExprs(QString regExp, const QList <ParserExpr> & parseExprs);
    void run();
    QJsonObject experimentParameters(){return experimentParameters_;}

private:
    bool parse(const QString &line);
    bool parseLine(const QString &line, QString regExpStr, QList<ParserExpr> &parseExprs);

    QMap <QString, QList <ParserExpr> > parserExprs_;
    QString fileName_;
    QJsonObject experimentParameters_;
};

class ParserExpr
{
public:
    enum class TypeParametr{type_int, type_double, type_string};

    ParserExpr(int regExpCatchNumber, const QString & name, TypeParametr typeParametr)
    {
        regExpCatchNumber_ = regExpCatchNumber;
        typeParametr_ = typeParametr;
        name_ = name;
        ok_ = false;
    }
    int regExpCatchNumber_;
    TypeParametr typeParametr_;
    QString  name_;
    bool ok_;
};

#endif // ExperimentsStorage_H
