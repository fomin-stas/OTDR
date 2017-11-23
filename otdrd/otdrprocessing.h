#ifndef OTDRPROCESSING_H
#define OTDRPROCESSING_H

#include <QObject>
#include <QtCore>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QtNetwork/QTcpServer>
#include <QtSerialPort/QSerialPortInfo>
#include <QTextStream>
#include "reflectogram.h"
#include "OTDR.h"

class OTDRProcessing : public QObject
{
    Q_OBJECT
public:
    explicit OTDRProcessing(QString usb_port, QObject *parent = 0);

    bool getIs_processing() const;
    int getLineNumber() const;
    void setLineNumber(int value);
    int getCurrent_id_fiber() const;

signals:
    void oprtNotOpened();
public slots:

private slots:
    void goStep(QByteArray);
    void actionSelection();
    
private:
    void getIDN();
    void captureReflectogramm(QByteArray requestData);
    void switchKey(QByteArray requestData);
    void getOTDRParameters();
    void getLineParameters();
    void setOTDRParametrs();
    void setLineParameters();

    OTDR *otdr;
    bool is_processing;
    int step;
    int current_id_fiber;
    enum OTDRprocess {
        GETREFLECTOGRAMM,
        SWITCHKEY,
        NOTHING,
        GETSTANDARTREFLECTOGRAMM,
        FIBEROFF
    };
    OTDRprocess process;
    QSqlDatabase db;
    int lineNumber;

};

#endif // OTDRPROCESSING_H
