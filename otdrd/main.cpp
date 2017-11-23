#include <QtCore/QCoreApplication>
#include <QtCore/QCommandLineParser>
#include <QtCore/QCommandLineOption>
#include <iostream>
#include "otdrprocessing.h"

QT_USE_NAMESPACE

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("otdrd");
    QCoreApplication::setApplicationVersion("0.1");
    QCommandLineParser parser;

    parser.setApplicationDescription("OTDR");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption portOption(QStringList() << "p");
    parser.addOption(portOption);
    QString port = QString(argv[1]);
    parser.process(app);
    QTextStream out(stdout);
    out << "Port: " << port;

    OTDRProcessing * otdr = new OTDRProcessing(port);
    return app.exec();
}
