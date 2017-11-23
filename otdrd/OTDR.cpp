#include <QtCore/QDebug>
#include <QtCore/QtEndian>
#include <QtNetwork/QHostAddress>
#include "OTDR.h"

OTDR::OTDR(QString pName, QObject* parent) :
    QObject(parent),
    portName(pName) {
    serial = new QSerialPort(this);
    if(!openSerialPort()){
        emit portNotOpened();
        qDebug() << "Port " << pName << " not opened!";
        return;
    }
    connect(serial, static_cast<void (QSerialPort::*)(QSerialPort::SerialPortError)> (&QSerialPort::error),
            this, &OTDR::handleError);
    connect(serial, &QSerialPort::readyRead, this, &OTDR::readData);
    qDebug() << "OTDR " << pName << " inited.";
}

bool OTDR::openSerialPort() {
    serial->setPortName(portName);
    serial->setBaudRate(static_cast<QSerialPort::BaudRate> (460800));
    serial->setDataBits(static_cast<QSerialPort::DataBits> (8));
    serial->setParity(static_cast<QSerialPort::Parity> (0));
    serial->setStopBits(static_cast<QSerialPort::StopBits> (1));
    serial->setFlowControl(static_cast<QSerialPort::FlowControl> (0));
    isTest = true;
    if (serial->open(QIODevice::ReadWrite)) {
        qDebug() << "Connection esteblished on port " << portName;
        step = 0;
        readBuferSize = serial->readBufferSize();
        if(readBuferSize == 0){
            readBuferSize = 4;
        }
        qDebug() << "Bufer size" << readBuferSize;
        QByteArray ba("*IDN?\r\n");
        QTimer::singleShot(5000, this, SLOT(notIdent()));
        writeData(ba);
        return true;
    } else {
        qDebug() << "Connection failed on port " << portName;
        return false;
    }
}

void OTDR::closeSerialPort() {
    
}

void OTDR::writeData(const QByteArray &data) {
    this->locked = true;
    //qDebug() << "Request data " << data;
    serial->write(data);
}

void OTDR::readData() {
    qint64 bytesAvailable = serial->bytesAvailable();
    //qDebug() << "bytesAvailable:" << bytesAvailable;
    QByteArray requestData = serial->read(bytesAvailable);
    while (serial->waitForReadyRead(500)) {
        bytesAvailable = serial->bytesAvailable();
        //qDebug() << "bytesAvailable2:" << bytesAvailable;
        requestData.append(serial->read(bytesAvailable));
    }
    if(!isTest){
        //qDebug() << "data is readed";
        emit dataIsReaded(requestData);
    }else{
        isTest = false;
        
        IMEI = QString(requestData);
        qDebug() << IMEI;
        initOTDR();
    }
    
    this->locked = false;
}

void OTDR::readFinished()
{
    //qDebug() << "finished";
}

void OTDR::handleError(QSerialPort::SerialPortError error) {
    if (error == QSerialPort::ResourceError) {
        qDebug() << serial->errorString();
        closeSerialPort();
    }
}

QString OTDR::getName() const
{
    return name;
}

bool OTDR::getIsActiv() const
{
    return isActiv;
}

int OTDR::getId_otdr() const
{
    return id_otdr;
}

QString OTDR::getIMEI() const
{
    return IMEI;
}

bool OTDR::getIs_busy() const
{
    return is_busy;
}



QString OTDR::getPortName() const {
    return portName;
}

void OTDR::setPortName(const QString &value) {
    portName = value;
}

bool OTDR::isLocked() const {
    return locked;
}

void OTDR::initOTDR(){
    QSqlQuery query;
    IMEI.chop(1);
    QString str = QString("SELECT * FROM otdr WHERE otdr_imei = '%1'").arg(IMEI);
    if(query.exec(str)){
        if(query.size() > 0){
            while (query.next()) {
                try{
                    name = query.value("otdr_name").toString();
                    id_otdr = query.value("id_otdr").toInt();
                    qDebug() << "find OTDR";
                }
                catch(...){
                    qDebug() << "Same wrong in OTDR.cpp select new otdr";
                }
            }
        }else{
            str = QString("INSERT INTO otdr (otdr_name,otdr_imei) "
                          "VALUES ('New otdr','%1')").arg(IMEI);
            try{
                if(!query.exec(str)){
                    qDebug() << query.lastError().databaseText();
                }else{
                    qDebug() << "Added new OTDR";
                    id_otdr = query.lastInsertId().toInt();
                }
            }
            catch(...){
                qDebug() << "Same wrong in OTDR.cpp insert new otdr";
            }
        }
        emit OTDRReady();
    }
}

void OTDR::notIdent()
{
    if(isTest){
        emit portNotOpened();
        serial->close();
        delete serial;
        deleteLater();
    }
}
