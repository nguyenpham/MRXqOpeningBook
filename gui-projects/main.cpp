#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QtQml>

#include "openingtreemodel.h"
#include "boardadapter.h"


int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    qmlRegisterType<OpeningTreeModel>("OpeningTreeViewModel", 1, 0, "OpeningModel" );
    qmlRegisterType<BoardAdapter>("TheBoardAdapter", 1, 0, "BoardAdapter" );

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    QQuickStyle::setStyle("Fusion"); // Material, Universal, Imagine, Default

    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
