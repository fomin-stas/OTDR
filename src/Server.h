#pragma once

#include <QtCore>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QtNetwork/QTcpServer>
#include <QtSerialPort/QSerialPortInfo>
#include <QTextStream>
#include <QList>

class QThread;

class Server: public QTcpServer
{
Q_OBJECT

public:

    Server(QObject * parent = nullptr);
    ~Server();

protected:

private slots:
    void initOTDRs();
    void processError(QProcess::ProcessError error);
private:
    QList<QProcess *> otdrProcesses;
    QSqlDatabase db;
    size_t m_threadCount;
    QList<QProcess *> *otdrs;
    QVector<QThread*> m_threads;
    size_t m_rrcounter;
    void readSTDIN();
};

