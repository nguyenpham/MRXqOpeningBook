#include "openingbuilderadapter.h"

OpeningBuilderAdapter::OpeningBuilderAdapter(QObject *parent) : QObject(parent)
{

}


void OpeningBuilderAdapter::create(QString folder, QString openingPath, QString info, int sides, int minGameLength, int addToLength, int repeatCnt)
{
    std::map<std::string, std::string> paramMap;

    paramMap["folder"] = folder.toStdString();
    paramMap["out"] = openingPath.isEmpty() ? "./opening.xop" : openingPath.toStdString();
    paramMap["info"] = info.toStdString();

    paramMap["maxply"] = QString(addToLength).toStdString();
    paramMap["minply"] = QString(minGameLength).toStdString();
    paramMap["mingame"] = QString(repeatCnt).toStdString();

    if (sides != 3) {
        if (sides == 1) {
            paramMap["-only-black"] = "1";
        } else
        if (sides == 2) {
            paramMap["-only-white"] = "1";
        }
    }

    m_builder.create(paramMap);
}
