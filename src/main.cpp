#include <QtCore/QCoreApplication>
#include <QtCore/QCommandLineParser>
#include <QtCore/QCommandLineOption>

#include "Server.h"

constexpr size_t DEFAULT_TREADS = 2;

constexpr size_t DEFAULT_PORT = 30014;

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("OTDRServer");
    QCoreApplication::setApplicationVersion("0.1");

    qRegisterMetaType<qintptr>("qintptr");

    QCommandLineParser parser;
    parser.setApplicationDescription("OTDR Server");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption portOption(QStringList() << "p" << "port", "port",
            "port");
    parser.addOption(portOption);
    QCommandLineOption threadOption(QStringList() << "t" << "threads",
            "The number of worker OTDRs", "count");
    parser.addOption(threadOption);
    parser.process(app);

    size_t port = parser.value(portOption).toUInt();
    if (port == 0)
    {
        port = DEFAULT_PORT;
    }

    Server * server = new Server();

    if (!server->listen(QHostAddress::LocalHost, port)){
        qCritical() << server->errorString();
        exit(1);
    }
    qDebug() << "Run server on" << port;
    return app.exec();
}
