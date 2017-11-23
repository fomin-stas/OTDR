#include <QtCore/QThread>


#include "Server.h"

Server::Server(QObject * parent) :
    QTcpServer(parent),
    m_rrcounter(0)
{
    db = QSqlDatabase::addDatabase("QPSQL");
    db.setHostName("localhost");
    db.setDatabaseName("optic_db");
    db.setUserName("postgres");
    db.setPassword("optic111111");
    qDebug() << "I'm?";
    if(db.open()){
        initOTDRs();
    }else{
        qDebug() << "DB connection failed!" << db.lastError().text();
    }
}

Server::~Server()
{

}

void Server::initOTDRs()
{
    QTextStream out(stdout);
    const auto serialPortInfos = QSerialPortInfo::availablePorts();

    out << QObject::tr("Total number of ports available: ") << serialPortInfos.count() << endl;

    const QString blankString = QObject::tr("N/A");
    QString description;
    QString manufacturer;
    QString serialNumber;

    for (const QSerialPortInfo &serialPortInfo : serialPortInfos) {
        description = serialPortInfo.description();
        manufacturer = serialPortInfo.manufacturer();
        serialNumber = serialPortInfo.serialNumber();
        out << endl
            << QObject::tr("Port: ") << serialPortInfo.portName() << endl
            << QObject::tr("Location: ") << serialPortInfo.systemLocation() << endl
            << QObject::tr("Description: ") << (!description.isEmpty() ? description : blankString) << endl
            << QObject::tr("Manufacturer: ") << (!manufacturer.isEmpty() ? manufacturer : blankString) << endl
            << QObject::tr("Serial number: ") << (!serialNumber.isEmpty() ? serialNumber : blankString) << endl
            << QObject::tr("Vendor Identifier: ") << (serialPortInfo.hasVendorIdentifier() ? QByteArray::number(serialPortInfo.vendorIdentifier(), 16) : blankString) << endl
            << QObject::tr("Product Identifier: ") << (serialPortInfo.hasProductIdentifier() ? QByteArray::number(serialPortInfo.productIdentifier(), 16) : blankString) << endl
            << QObject::tr("Busy: ") << (serialPortInfo.isBusy() ? QObject::tr("Yes") : QObject::tr("No")) << endl;
        QStringList arguments;
        arguments  << serialPortInfo.systemLocation();
        QProcess *otdr = new QProcess();
        otdr->setProcessChannelMode(QProcess::ForwardedChannels);
        connect(otdr,SIGNAL(error(QProcess::ProcessError)),this,SLOT(processError(QProcess::ProcessError)));
        otdr->start("./otdrd", arguments);
        otdrProcesses.append(otdr);
    }
}

void Server::processError(QProcess::ProcessError error)
{
    qDebug() << "Process error - " << error ;
}

void Server::readSTDIN(){
    QTextStream stream(stdin);
    QString line;
    while (stream.readLineInto(&line)) {
        qDebug() << line;
    }
}
