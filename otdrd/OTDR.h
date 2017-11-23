#pragma once

#include <QtCore/QObject>
#include <QTimer>
#include <QtSerialPort/QSerialPort>
#include "reflectogram.h"

class OTDR: public QObject
{
    Q_OBJECT

public:
    OTDR(QString pName,QObject* parent = 0);

    QString getPortName() const;
    bool isLocked() const;
    void setPortName(const QString &value);
    bool getIs_busy() const;
    
    QString getIMEI() const;

    int getId_otdr() const;

    bool getIsActiv() const;

    QString getName() const;

public slots:
    bool openSerialPort();
    void closeSerialPort();
    void writeData(const QByteArray &data);
    void readData();
    void readFinished();

private slots:
    void handleError(QSerialPort::SerialPortError error);
    void notIdent();

signals:
    void dataIsReaded(QByteArray data);
    void portNotOpened();
    void OTDRReady();

private:
    QSerialPort *serial;
    QString portName;
    qint64 readBuferSize;
    QString IMEI;
    QString name;
    bool isActiv;
    int id_otdr;

    bool locked;
    bool isTest;
    int step;
    bool is_busy;
    void initOTDR();
};

