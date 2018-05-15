#include "experimentsStorage.h"
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDir>
#include <QJsonObject>
#include <QJsonDocument>
#include <QUuid>
#include <QPointF>
#include <QDataStream>
#include <QByteArray>
#include <QDebug>

json * ExperimentsStorage::settingsJson_;
QString ExperimentsStorage::pathSampleFolder_;
QString ExperimentsStorage::pathCalibrationFolder_;
QString ExperimentsStorage::pathTarFolder_;
QString ExperimentsStorage::pathMCMIXFolder_;
bool ExperimentsStorage::isInited_;
QJsonObject ExperimentsStorage::jsonStorageExperimentShortInfo_;

ExperimentsStorage::ExperimentsStorage(QObject *parent) : QObject(parent)
{

}

bool ExperimentsStorage::close()
{
    saveJsonStorageToFile();

    isInited_ = false;

    return true;
}

void ExperimentsStorage::setSettings(json *settingsJson)
{
    settingsJson_ = settingsJson;
    readSettings();
}

bool ExperimentsStorage::setPathSampleFolder(const QString &path)
{
    pathSampleFolder_ = path;
    if(settingsJson_!=nullptr)
        (*settingsJson_)["pathSampleFolder"] = path.toStdString();

    return true;
}

bool ExperimentsStorage::setPathCalibrationFolder(const QString &path)
{
    pathCalibrationFolder_ = path;
    if(settingsJson_!=nullptr)
        (*settingsJson_)["pathCalibrationFolder"] = path.toStdString();

    return true;
}

bool ExperimentsStorage::setPathTarFolder(const QString &path)
{
    pathTarFolder_ = path;
    if(settingsJson_!=nullptr)
        (*settingsJson_)["pathTarFolder"] = path.toStdString();

    return true;
}

bool ExperimentsStorage::setPathMCMIXFolder(const QString & path)
{
    pathMCMIXFolder_ = path;
    if(settingsJson_!=nullptr)
        (*settingsJson_)["pathMCMIXFolder"] = path.toStdString();

    return true;
}

void ExperimentsStorage::readSettings()
{
    std::string path;
    path = (*settingsJson_)["pathCalibrationFolder"].is_string() ? (*settingsJson_)["pathCalibrationFolder"] : "";
    pathCalibrationFolder_ = path.data();
    path = (*settingsJson_)["pathSampleFolder"].is_string() ? (*settingsJson_)["pathSampleFolder"] : "";
    pathSampleFolder_ = path.data();
    path = (*settingsJson_)["pathTarFolder"].is_string() ? (*settingsJson_)["pathTarFolder"] : "";
    pathTarFolder_ = path.data();
    path = (*settingsJson_)["pathMCMIXFolder"].is_string() ? (*settingsJson_)["pathMCMIXFolder"] : "";
    pathMCMIXFolder_ = path.data();
}

QStringList ExperimentsStorage::samples()
{
    QDir dir(pathSampleFolder_);

    return dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
}

QStringList ExperimentsStorage::calibrationAmpules()
{
    QDir dir(pathCalibrationFolder_);

    return dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
}

QJsonArray ExperimentsStorage::experiments(const QString & sampleName)
{
    if (!isInited()) return QJsonArray();
    QJsonArray dropsArray;
    for (auto drop : jsonStorageExperimentShortInfo_["ShortExperimentsInfo"].toObject()){
        QString experimentType = drop.toObject()["experimentType"].toString();
        QString sample = drop.toObject()["sample"].toString();
        if (experimentType == "experiment" && sample == sampleName){
            dropsArray.append(drop);
        }
    }

    return dropsArray;
}

QJsonArray ExperimentsStorage::calibrations(const QString & ampoule)
{
    if (!isInited()) return QJsonArray();
    QJsonArray dropsArray;
    for (auto drop : jsonStorageExperimentShortInfo_["ShortExperimentsInfo"].toObject()){
        QString experimentType = drop.toObject()["experimentType"].toString();
        QString ampoule_ = drop.toObject()["ampouleMaterial"].toString();
        if (experimentType == "calibration" && ampoule_ == ampoule){
            dropsArray.append(drop);
        }
    }

    return dropsArray;
}

bool ExperimentsStorage::init()
{
    loadJsonStorageFromFile();

    QJsonArray experimentsJsonArray;
    QDir dir(pathSampleFolder_);
    QStringList allExistingExperimentsPath;
    QStringList samplesDirs = dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);

    for (auto &sample : samplesDirs){
        QJsonObject sampleJsonObject;
        dir.cd(sample);
        QStringList experimentsDirs = dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
        QJsonArray dropsJsonArray;
        for (auto &item : experimentsDirs){
            QString experimentFolder(pathSampleFolder_ + QString("/") + sample + QString("/") + item);
            allExistingExperimentsPath.append(experimentFolder);
            if(isExistExperimentInStorage(experimentFolder))continue;

            QJsonObject experimentInfo (parametersExperimentFromFileInfo(experimentFolder + QString("/info.txt")));
            experimentInfo["path"] = experimentFolder;
            QString guid = QUuid::createUuid().toString();
            experimentInfo["id"] = guid;
            QRegExp dateRegExp("(.+)_(\\d+)$");
            dateRegExp.indexIn(item);
            experimentInfo["date"] = dateRegExp.cap(1);
            experimentInfo["experimentNumber"] = dateRegExp.cap(2);

            experimentInfo["sample"] = sample;
            experimentInfo["experimentType"] = "experiment";

            if (!dateRegExp.cap(1).isEmpty() && !dateRegExp.cap(2).isEmpty()){
                dropsJsonArray.append(experimentInfo);
                saveJsonFile(experimentInfo);
                addExperimentIntoJsonStorageFile(&experimentInfo);
            }
        }
        dir.cdUp();
        sampleJsonObject[sample]=dropsJsonArray;
        experimentsJsonArray.append(sampleJsonObject);
    }

    QJsonArray calibrationJsonArray;
    dir.setPath(pathCalibrationFolder_);
    QStringList ampoulesDirs = dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);

    for (auto &ampoule : ampoulesDirs){
        QJsonObject ampouleJsonObject;
        dir.cd(ampoule);
        QStringList experimentsDirs = dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
        QJsonArray dropsJsonArray;
        for (auto &item : experimentsDirs){
            QString experimentFolder(pathCalibrationFolder_ + QString("/") + ampoule + QString("/") + item);
            allExistingExperimentsPath.append(experimentFolder);
            if(isExistExperimentInStorage(experimentFolder))continue;

            QJsonObject experimentInfo (parametersExperimentFromFileInfo(experimentFolder + QString("/info.txt")));
            experimentInfo.insert("path", experimentFolder);
            QString guid = QUuid::createUuid().toString();
            experimentInfo["id"] = guid;
            QRegExp dateRegExp("(.+)_(\\d+)");
            dateRegExp.indexIn(item);
            experimentInfo["date"] = dateRegExp.cap(1);
            experimentInfo["experimentNumber"] = dateRegExp.cap(2);
            experimentInfo["sample"] = "Пустая";
            experimentInfo["ampouleMaterial"] = ampoule;
            experimentInfo["experimentType"] = "calibration";

            if (!dateRegExp.cap(1).isEmpty() && !dateRegExp.cap(2).isEmpty()){
                dropsJsonArray.append(experimentInfo);
                saveJsonFile(experimentInfo);
                addExperimentIntoJsonStorageFile(&experimentInfo);
            }
        }
        dir.cdUp();
        ampouleJsonObject[ampoule]=dropsJsonArray;
        calibrationJsonArray.append(ampouleJsonObject);
    }

    QJsonArray tarJsonArray;
    dir.setPath(pathTarFolder_);
    QStringList tarsDirs = dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);

    for (auto &item : tarsDirs){
        QString experimentFolder(pathTarFolder_ + QString("/") + item);
        allExistingExperimentsPath.append(experimentFolder);
        if(isExistExperimentInStorage(experimentFolder))continue;

        QJsonObject experimentInfo (parametersExperimentFromFileInfo(experimentFolder + QString("/info.txt")));
        experimentInfo.insert("path", experimentFolder);
        QString guid = QUuid::createUuid().toString();
        experimentInfo["id"] = guid;
        QRegExp dateRegExp("(.+)_(\\d+)");
        dateRegExp.indexIn(item);
        experimentInfo["date"] = dateRegExp.cap(1);
        experimentInfo["experimentNumber"] = dateRegExp.cap(2);
        experimentInfo["experimentType"] = "tar";

        tarJsonArray.append(experimentInfo);

        if (!dateRegExp.cap(1).isEmpty() && !dateRegExp.cap(2).isEmpty()){
            saveJsonFile(experimentInfo);
            addExperimentIntoJsonStorageFile(&experimentInfo);
        }
    }

    isInited_ = true;


    QJsonObject shortExperimentsInfo = jsonStorageExperimentShortInfo_["ShortExperimentsInfo"].toObject();
    QJsonObject experimentsPath = jsonStorageExperimentShortInfo_["Paths"].toObject();
    for (auto path : jsonStorageExperimentShortInfo_["Paths"].toObject().keys()){
        if(!allExistingExperimentsPath.contains(path)){
            QString experimentId = experimentsPath[path].toString();
            shortExperimentsInfo.remove(experimentId);
            experimentsPath.remove(path);
            qDebug() << "remove" <<path;
        }

    }

    jsonStorageExperimentShortInfo_.remove("ShortExperimentsInfo");
    jsonStorageExperimentShortInfo_["ShortExperimentsInfo"] = shortExperimentsInfo;
    jsonStorageExperimentShortInfo_.remove("Paths");
    jsonStorageExperimentShortInfo_["Paths"] = experimentsPath;

    return true;
}

void ExperimentsStorage::addExperimentIntoJsonStorageFile(QJsonObject *experiment)
{
    qDebug() << "new" << (*experiment)["path"].toString();

    QJsonObject files;
    QFileInfo fileInfo;
    QDir experimentDir((*experiment)["path"].toString());
    QStringList filesList = experimentDir.entryList(QDir::Files);
    for (auto &file : filesList){
        fileInfo.setFile((*experiment)["path"].toString() + "/" + file);
        files[file] = fileInfo.lastModified().toMSecsSinceEpoch()/1000;
    }

    QJsonObject shortExperimentInfo;
    shortExperimentInfo["id"] = (*experiment)["id"];
    shortExperimentInfo["date"] = (*experiment)["date"];
    shortExperimentInfo["experimentNumber"] = (*experiment)["experimentNumber"];
    shortExperimentInfo["path"] = (*experiment)["path"];
    shortExperimentInfo["experimentType"] = (*experiment)["experimentType"];
    shortExperimentInfo["files"] = files;

    QString experimentType = shortExperimentInfo["experimentType"].toString();
    if (experimentType == "experiment" || experimentType == "calibration"){
        shortExperimentInfo["sample"] = (*experiment)["sample"];
        shortExperimentInfo["temperatureFurnace"] = (*experiment)["temperatureFurnace"];

        shortExperimentInfo["ampouleMaterial"] = (*experiment)["ampouleMaterial"];
        shortExperimentInfo["ampouleDemountableType"] = (*experiment)["ampouleDemountableType"];
        shortExperimentInfo["ampouleType"] = (*experiment)["ampouleType"];
        shortExperimentInfo["ampouleBatch"] = (*experiment)["ampouleBatch"];
        shortExperimentInfo["ampouleNumber"] = (*experiment)["ampouleNumber"];
    } else if (experimentType == "tar") {
        shortExperimentInfo["series"] = (*experiment)["series"];
        shortExperimentInfo["thermostateTemperature"] = (*experiment)["thermostateTemperature"];
        shortExperimentInfo["heaterDurationTime"] = (*experiment)["heaterDurationTime"];
        shortExperimentInfo["heaterVoltage"] = (*experiment)["heaterVoltage"];
        shortExperimentInfo["deltaR"] = (*experiment)["deltaR"];
    }

    QJsonObject shortExperimentsInfo = jsonStorageExperimentShortInfo_["ShortExperimentsInfo"].toObject();
    shortExperimentsInfo[(*experiment)["id"].toString()] = shortExperimentInfo;
    jsonStorageExperimentShortInfo_.remove("ShortExperimentsInfo");
    jsonStorageExperimentShortInfo_["ShortExperimentsInfo"] = shortExperimentsInfo;

    QJsonObject paths = jsonStorageExperimentShortInfo_["Paths"].toObject();
    paths[(*experiment)["path"].toString()] = (*experiment)["id"].toString();
    jsonStorageExperimentShortInfo_.remove("Paths");
    jsonStorageExperimentShortInfo_["Paths"] = paths;
}

bool ExperimentsStorage::isExistExperimentInStorage(const QString & path)
{
    QFileInfo infoFileInfo;

    QJsonObject experimentsPath = jsonStorageExperimentShortInfo_["Paths"].toObject();
    QJsonObject shortExperimentsInfo = jsonStorageExperimentShortInfo_["ShortExperimentsInfo"].toObject();
    if (experimentsPath.contains(path)){
        QString experimentId = experimentsPath[path].toString();
        QJsonObject filesObject = shortExperimentsInfo[experimentId].toObject()["files"].toObject();
        QDir experimentDir(path);
        for (auto fileName : experimentDir.entryList(QDir::Files)){
            if(!filesObject.contains(fileName)){
                shortExperimentsInfo.remove(experimentId);
                jsonStorageExperimentShortInfo_.remove("ShortExperimentsInfo");
                jsonStorageExperimentShortInfo_["ShortExperimentsInfo"] = shortExperimentsInfo;
                return false;
            }
        }

        for (auto fileName : filesObject.keys()){
            infoFileInfo.setFile(path + "/" + fileName);
            if(filesObject[fileName].toInt() != (infoFileInfo.exists() ? infoFileInfo.lastModified().toMSecsSinceEpoch()/1000 : -1)){
                shortExperimentsInfo.remove(experimentId);
                jsonStorageExperimentShortInfo_.remove("ShortExperimentsInfo");
                jsonStorageExperimentShortInfo_["ShortExperimentsInfo"] = shortExperimentsInfo;
                return false;
            }
        }
        return true;
    }

    return false;
}

bool ExperimentsStorage::saveJsonStorageToFile()
{
    QDir storageDir(QDir::currentPath());
    if(!storageDir.exists("storage"))
        storageDir.mkdir("storage");

    QFile jsonStorageFile("storage/MCMIX.json");
    jsonStorageFile.open(QIODevice::WriteOnly);
    QJsonDocument jsonDoc(jsonStorageExperimentShortInfo_);
    jsonStorageFile.write(jsonDoc.toJson());

    jsonStorageFile.close();

    return true;
}

bool ExperimentsStorage::loadJsonStorageFromFile()
{
    QFile jsonStorageFile("storage/MCMIX.json");
    if (!jsonStorageFile.exists()){
        return false;
    }

    jsonStorageFile.open(QIODevice::ReadOnly);

    jsonStorageExperimentShortInfo_ = QJsonDocument::fromJson(jsonStorageFile.readAll()).object();

    jsonStorageFile.close();

    return true;
}

QJsonObject ExperimentsStorage::parametersExperimentFromFileInfo(const QString & fileName)
{

    ParserInfoFile parserInfoFile;
    parserInfoFile.setFileName(fileName);

    QList <ParserExpr> parserExpr;

    parserExpr << ParserExpr(1, "ampouleMaterial", ParserExpr::TypeParametr::type_string)
               << ParserExpr(2, "ampouleDemountableType", ParserExpr::TypeParametr::type_string)
               << ParserExpr(3, "ampouleType", ParserExpr::TypeParametr::type_int)
               << ParserExpr(4, "ampouleBatch", ParserExpr::TypeParametr::type_int)
               << ParserExpr(5, "ampouleNumber", ParserExpr::TypeParametr::type_int);
    parserInfoFile.addParseExprs("Ампула:\\s*(\\w*),\\s*(\\w.*),"
                                 "\\s*тип\\s+(\\d+),"
                                 "\\s*партия\\s+(\\d+),"
                                 "\\s*№(\\d+)\\w*$", parserExpr);
    parserExpr.clear();

    parserExpr << ParserExpr(1, "temperatureFurnace", ParserExpr::TypeParametr::type_double);
    parserInfoFile.addParseExprs("Температура в печи:\\s*(\\d+([.,]\\d*)?).*", parserExpr);
    parserExpr.clear();

    parserExpr << ParserExpr(1, "weightAmpoule", ParserExpr::TypeParametr::type_double);
    parserInfoFile.addParseExprs("Масса ампулы:\\s*(\\d+([.,]\\d*)?).*", parserExpr);
    parserExpr.clear();

    parserExpr << ParserExpr(1, "weightBefore", ParserExpr::TypeParametr::type_double);
    parserInfoFile.addParseExprs("Масса до эксперимента:\\s*(\\d+([.,]\\d*)?).*", parserExpr);
    parserExpr.clear();

    parserExpr << ParserExpr(1, "weightAfter", ParserExpr::TypeParametr::type_double);
    parserInfoFile.addParseExprs("Масса после эксперимента:\\s*(\\d+([.,]\\d*)?).*", parserExpr);
    parserExpr.clear();

    parserExpr << ParserExpr(1, "pressureThermostate", ParserExpr::TypeParametr::type_double);
    parserInfoFile.addParseExprs("Давление в термостате:\\s*(\\d+([.,]\\d*([Ee][+-]\\d+)?)?).*", parserExpr);
    parserExpr.clear();

    parserExpr << ParserExpr(1, "thermostateTemperature", ParserExpr::TypeParametr::type_double);
    parserInfoFile.addParseExprs("Температура в термостате:\\s*(\\d+([.,]\\d*)?).*", parserExpr);
    parserExpr.clear();

    parserExpr << ParserExpr(1, "thermostateColdTemperature", ParserExpr::TypeParametr::type_double);
    parserInfoFile.addParseExprs("Температура в охл. термостате:\\s*(\\d+([.,]\\d*)?).*", parserExpr);
    parserExpr.clear();

    parserExpr << ParserExpr(1, "pressureFurnace", ParserExpr::TypeParametr::type_double);
    parserInfoFile.addParseExprs("Давление в печи:\\s*(\\d+([.,]\\d*)?).*", parserExpr);
    parserExpr.clear();

    parserExpr << ParserExpr(1, "dropTime", ParserExpr::TypeParametr::type_double)
               << ParserExpr(3, "dropDuration", ParserExpr::TypeParametr::type_double);
    parserInfoFile.addParseExprs("Время пролета\\s*\\((\\d+([.,]\\d*)?).*\\)\\s*:\\s*(\\d+([.,]\\d*)?).*", parserExpr);
    parserExpr.clear();

    parserExpr << ParserExpr(1, "openedCoversDuration", ParserExpr::TypeParametr::type_double);
    parserInfoFile.addParseExprs("Время засветки:\\s*(\\d+([.,]\\d*)?).*", parserExpr);
    parserExpr.clear();

    parserExpr << ParserExpr(1, "temperatureRoom", ParserExpr::TypeParametr::type_double);
    parserInfoFile.addParseExprs("Температура в комнате:\\s*(\\d+([.,]\\d*)?).*", parserExpr);
    parserExpr.clear();

    parserExpr << ParserExpr(1, "temperatureColdThermocoupleAmpoule", ParserExpr::TypeParametr::type_double);
    parserInfoFile.addParseExprs("Температура хол. спаев термопары ампулы:\\s*(\\d+([.,]\\d*)?).*", parserExpr);
    parserExpr.clear();

    parserExpr << ParserExpr(1, "temperatureColdThermocoupleFurnace", ParserExpr::TypeParametr::type_double);
    parserInfoFile.addParseExprs("Температура хол. спаев регулировочных термопар:\\s*(\\d+([.,]\\d*)?).*", parserExpr);
    parserExpr.clear();

    parserExpr << ParserExpr(1, "temperatureColdWater", ParserExpr::TypeParametr::type_double);
    parserInfoFile.addParseExprs("Температура хол. воды:\\s*(\\d+([.,]\\d*)?).*", parserExpr);
    parserExpr.clear();

    parserExpr << ParserExpr(1, "series", ParserExpr::TypeParametr::type_int);
    parserInfoFile.addParseExprs("Серия:\\s*(\\d+).*", parserExpr);
    parserExpr.clear();

    parserExpr << ParserExpr(1, "heaterDurationTime", ParserExpr::TypeParametr::type_double);
    parserInfoFile.addParseExprs("Время работы нагревателя:\\s*(\\d+([,.]\\d+)?).*", parserExpr);
    parserExpr.clear();

    parserExpr << ParserExpr(1, "heaterVoltage", ParserExpr::TypeParametr::type_double);
    parserInfoFile.addParseExprs("Напряжение на ИП калибровочного нагревателя:\\s*(\\d+([,.]\\d+)?).*", parserExpr);
    parserExpr.clear();

    parserExpr << ParserExpr(1, "pressureGate", ParserExpr::TypeParametr::type_double);
    parserInfoFile.addParseExprs("Давление в шлюзе:\\s*(\\d+([.,]\\d*)?).*", parserExpr);
    parserExpr.clear();

    parserInfoFile.run();


    QJsonObject parameters = parserInfoFile.experimentParameters();

    if (parameters.contains("heaterVoltage") && parameters.contains("heaterDurationTime")){
        double heaterVoltage = parameters["heaterVoltage"].toDouble();
        double heaterDurationTime = parameters["heaterDurationTime"].toDouble();
        parameters["deltaR"] = (heaterVoltage*heaterVoltage/2500)*1.17/1000.0*heaterDurationTime;
    }

    return parameters;
}

bool ExperimentsStorage::saveJsonFile(const QJsonObject &experimentInfo)
{
    QJsonObject main;
    QJsonObject mainOld;
    QFile dataJsonFile(experimentInfo["path"].toString() + "/data.json");
    if (dataJsonFile.exists()){
        if (!dataJsonFile.open(QIODevice::ReadOnly)) {
            qWarning("Couldn't open save file.");
            return false;
        }
        main = QJsonDocument::fromJson(dataJsonFile.readAll()).object();
        dataJsonFile.close();
    }

    mainOld = main;
    QJsonObject experiment;
    {
        QJsonObject parameters;
        parameters["type"] = experimentInfo["experimentType"];
        parameters["path"] = experimentInfo["path"];
        parameters["date"] = experimentInfo["data"];
        parameters["dropNumber"] = experimentInfo["experimentNumber"];
        if (parameters["type"]=="experiment" || parameters["type"]=="calibration"){
            QJsonObject ampouleInfo;
            ampouleInfo["material"] = experimentInfo["ampouleMaterial"];
            ampouleInfo["demountableType"] = experimentInfo["ampouleDemountableType"];
            ampouleInfo["type"] = experimentInfo["ampouleType"];
            ampouleInfo["batch"] = experimentInfo["ampouleBatch"];
            ampouleInfo["number"] = experimentInfo["ampouleNumber"];
            parameters["ampouleInfo"] = ampouleInfo;
        }
        addJsonObjectExperimentParametr(&parameters, "temperatureFurnace" ,experimentInfo["temperatureFurnace"], tr("Уставка температуры в печи калориметра"), tr("%1").arg(QChar(0x2103)), 1);
        addJsonObjectExperimentParametr(&parameters, "weightAmpoule", experimentInfo["weightAmpoule"], tr("Масса ампулы"), tr("г"), 1e-3);
        addJsonObjectExperimentParametr(&parameters, "weightBefore", experimentInfo["weightBefore"], tr("Масса до эксперимента"), tr("г"), 1e-3);
        addJsonObjectExperimentParametr(&parameters, "weightAfter", experimentInfo["weightAfter"], tr("Масса после эксперимента"), tr("г"), 1e-3);
        addJsonObjectExperimentParametr(&parameters, "pressureThermostate", experimentInfo["pressureThermostate"], tr("Давление в калориметрическом блоке"), tr("мБар"), 1e-3);
        addJsonObjectExperimentParametr(&parameters, "thermostateTemperature", experimentInfo["thermostateTemperature"], tr("Температура в термостате"), tr("%1").arg(QChar(0x2103)), 1);
        addJsonObjectExperimentParametr(&parameters, "thermostateColdTemperature", experimentInfo["thermostateColdTemperature"], tr("Температура в охл. термостате"), tr("%1").arg(QChar(0x2103)), 1);
        addJsonObjectExperimentParametr(&parameters, "pressureFurnace", experimentInfo["pressureFurnace"], tr("Давление в печи"), tr("ати"), 1);
        addJsonObjectExperimentParametr(&parameters, "dropTime", experimentInfo["dropTime"], tr("Время сброса ампулы после запуска программы"), tr("с"), 1);
        addJsonObjectExperimentParametr(&parameters, "dropDuration", experimentInfo["dropDuration"], tr("Время падения ампулы"), tr("мс"), 1e-3);
        addJsonObjectExperimentParametr(&parameters, "openedCoversDuration", experimentInfo["openedCoversDuration"], tr("Время открытых крышек блока калориметра в течение сброса ампулы"), tr("мс"), 1e-3);
        addJsonObjectExperimentParametr(&parameters, "temperatureRoom", experimentInfo["temperatureRoom"], tr("Температура в комнате"), tr("%1").arg(QChar(0x2103)), 1);
        addJsonObjectExperimentParametr(&parameters, "temperatureColdWater", experimentInfo["temperatureColdWater"], tr("Температура хол. воды"), tr("%1").arg(QChar(0x2103)), 1);
        addJsonObjectExperimentParametr(&parameters, "temperatureColdThermocoupleAmpoule", experimentInfo["temperatureColdThermocoupleAmpoule"], tr("Температура хол. спаев термопары ампулы"), tr("%1").arg(QChar(0x2103)), 1);
        addJsonObjectExperimentParametr(&parameters, "temperatureColdThermocoupleFurnace", experimentInfo["temperatureColdThermocoupleFurnace"], tr("Температура хол. спаев регулировочных термопар"), tr("%1").arg(QChar(0x2103)), 1);

        addJsonObjectExperimentParametr(&parameters, "series", experimentInfo["series"], tr("Серия калибровочного эксперимента"));
        addJsonObjectExperimentParametr(&parameters, "heaterDurationTime", experimentInfo["heaterDurationTime"], tr("Время работы нагревателя"), tr("с"), 1);
        addJsonObjectExperimentParametr(&parameters, "heaterVoltage", experimentInfo["heaterVoltage"], tr("Напряжение ИП калибровочного нагревателя"), tr("В"), 1);
        addJsonObjectExperimentParametr(&parameters, "pressureGate", experimentInfo["pressureGate"], tr("Давление в шлюзе"), tr("ати"), 1);
        addJsonObjectExperimentParametr(&parameters, "deltaR", experimentInfo["deltaR"], tr("Изменение сопротивления МТС калор. блока"), tr("Ом"), 1);

        experiment["parameters"] = parameters;
    }

    QJsonObject settings;
    settings["codepage"] = "UTF-8";
    main["settings"] = settings;

    QJsonObject data;
    {
        ExperimentDataJson sampleThermocoupleData;
        sampleThermocoupleData.setAxisInfo(ExperimentDataJson::Axis::x, tr("с"), tr("Время"), 1);
        sampleThermocoupleData.setAxisInfo(ExperimentDataJson::Axis::y, tr("мВ"), tr("Температура термопары образца"), 1e-3);
        sampleThermocoupleData.setExperimentFile(experimentInfo["path"].toString() + "/TSample.dat", '\t', 1, 2);
        data["sampleThermocouple"] = sampleThermocoupleData.getExperimentDataJson();

        ExperimentDataJson thermostateTemperatureData;
        thermostateTemperatureData.setAxisInfo(ExperimentDataJson::Axis::x, tr("с"), tr("Время"), 1);
        thermostateTemperatureData.setAxisInfo(ExperimentDataJson::Axis::y, tr("%1").arg(QChar(0x2103)), tr("Температура термостата калориметрического блока"), 1);
        thermostateTemperatureData.setExperimentFile(experimentInfo["path"].toString() + "/Diff.dat", '\t', 1, 3);
        data["thermostateTemperature"] = thermostateTemperatureData.getExperimentDataJson();

        ExperimentDataJson thermostateDiffTemperatureData;
        thermostateDiffTemperatureData.setAxisInfo(ExperimentDataJson::Axis::x, tr("с"), tr("Время"), 1);
        thermostateDiffTemperatureData.setAxisInfo(ExperimentDataJson::Axis::y, tr("мВ"), tr("Дифф. температура калориметрического блока"), 1e-3);
        thermostateDiffTemperatureData.setExperimentFile(experimentInfo["path"].toString() + "/Diff.dat", '\t', 1, 2);
        data["thermostateDiffTemperature"] = thermostateDiffTemperatureData.getExperimentDataJson();

        ExperimentDataJson resistanceBlockData;
        resistanceBlockData.setAxisInfo(ExperimentDataJson::Axis::x, tr("с"), tr("Время"), 1);
        resistanceBlockData.setAxisInfo(ExperimentDataJson::Axis::y, tr("Ом"), tr("Сопротивление МТС калориметрического блока"), 1);
        resistanceBlockData.setExperimentFile(experimentInfo["path"].toString() + "/RBlok.dat", '\t', 1, 2);
        data["resistanceBlock"] = resistanceBlockData.getExperimentDataJson();
    }
    experiment["data"] = data;

    main["experiment"] = experiment;

    if (main != mainOld){
        QJsonDocument jsonDoc(main);
        if (!dataJsonFile.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text)) {
            qWarning("Couldn't open save file.");
            return false;
        }
        dataJsonFile.write(jsonDoc.toJson());
        dataJsonFile.close();
    }

    return true;
}

bool ExperimentsStorage::addJsonObjectExperimentParametr(QJsonObject * parameters, const QString & paramName, QJsonValue value, const QString & description, const QString & unit, double multiplier)
{
    if (value.isUndefined()) return false;

    QJsonObject parameter;
    parameter["value"] = value;
    parameter["unit"] = unit;
    parameter["multiplier"] = multiplier;
    parameter["description"] = description;

    (*parameters)[paramName] = parameter;

    return true;
}

QString ExperimentsStorage::getFileInfoDirById(const QString & id)
{
    QJsonObject shortExperimentsInfo = jsonStorageExperimentShortInfo_["ShortExperimentsInfo"].toObject();
    if (shortExperimentsInfo.contains(id)){
        return shortExperimentsInfo[id].toObject()["path"].toString();
    }
    return "";
}



void ExperimentDataJson::setAxisInfo(Axis axis, const QString & unit, const QString & description, double multiplier)
{
    QJsonObject axisInfo;
    axisInfo["description"] = description;
    axisInfo["unit"] = unit;
    axisInfo["multiplier"] = multiplier;

    switch(axis)
    {
    case Axis::x:{
        experimentDataJson_["X"] = axisInfo; break;
    }
    case Axis::y:{
        experimentDataJson_["Y"] = axisInfo; break;
    }
    }
}

void ExperimentDataJson::setExperimentFile(const QString & fileName, QChar separator, int colNumberX, int colNumberY)
{
    QByteArray ba;
    QDataStream dataStream (&ba, QIODevice::WriteOnly);
    dataStream << getPointArrayFromFile(fileName, separator, colNumberX, colNumberY);
    experimentDataJson_["values"] = ba.toHex().data();
}

QVector<QPointF> ExperimentDataJson::getPointArrayFromFile(const QString & fileName, QChar separator, int colNumberX, int colNumberY)
{
    QVector<QPointF> points;

    QFile file (fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        return points;
    }

    QTextStream in(&file);
    while(!in.atEnd()){
        QString line = in.readLine();
        QStringList listValues =  line.split(separator);
        bool xValueOk;
        bool yValueOk;
        double x;
        double y;

        if(listValues.size()>=colNumberX && listValues.size()>=colNumberY){
            x = listValues.at(colNumberX - 1).toDouble(&xValueOk);
            y = listValues.at(colNumberY - 1).toDouble(&yValueOk);
            if (xValueOk && yValueOk){
                QPointF point;
                point.setX(x);
                point.setY(y);
                points.append(point);
            }
        }
    }

    file.close();

    return points;
}




void ParserInfoFile::setFileName(const QString & fileName)
{
    fileName_ = fileName;
}

void ParserInfoFile::run()
{    
    QFile infoFile (fileName_);
    if (!infoFile.open(QIODevice::ReadOnly | QIODevice::Text)){
        return;
    }

    QTextStream in(&infoFile);
    in.setCodec("CP1251");

    while (!in.atEnd()){
        QString line = in.readLine();
        parse(line);

    }

    for(auto it = parserExprs_.begin(); it != parserExprs_.end(); ++it){
        it.value().clear();
    }
    parserExprs_.clear();

    infoFile.close();
}

void ParserInfoFile::addParseExprs(QString regExp, const QList <ParserExpr> & parseExprs)
{
    parserExprs_[regExp] = parseExprs;
}

bool ParserInfoFile::parse(const QString & line)
{
    for(auto it = parserExprs_.begin(); it != parserExprs_.end(); ++it){
        if (parseLine(line, it.key(), it.value())){
            parserExprs_.remove(it.key());
            break;
        }
    }

    return true;
}

bool ParserInfoFile::parseLine(const QString &line, QString regExpStr,  QList <ParserExpr> & parseExprs)
{
    QRegExp regExp(regExpStr);
    regExp.indexIn(line);

    for (auto & parseExpr : parseExprs){
        if(!regExp.cap(parseExpr.regExpCatchNumber_).isEmpty()){
            if (parseExpr.typeParametr_ == ParserExpr::TypeParametr::type_string){
                parseExpr.ok_ = true;
                experimentParameters_[parseExpr.name_] = regExp.cap(parseExpr.regExpCatchNumber_);
            } else if (parseExpr.typeParametr_ == ParserExpr::TypeParametr::type_int){
                int value  = regExp.cap(parseExpr.regExpCatchNumber_).toInt(&parseExpr.ok_);
                if (parseExpr.ok_)
                    experimentParameters_[parseExpr.name_] = value;
            } else if (parseExpr.typeParametr_ == ParserExpr::TypeParametr::type_double){
                double value = regExp.cap(parseExpr.regExpCatchNumber_).replace(",", ".").toDouble(&parseExpr.ok_);
                if (parseExpr.ok_)
                    experimentParameters_[parseExpr.name_] = value;
            }
        }
    }

    for (auto parseExpr : parseExprs){
        if(!parseExpr.ok_)
            return false;
    }

    return true;
}
