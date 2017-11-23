#include "otdrprocessing.h"
#include <QDateTime>
#include <QTimer>

OTDRProcessing::OTDRProcessing(QString usb_port, QObject *parent) :
    QObject(parent),
    is_processing(false),
    step(0)
{
    db = QSqlDatabase::addDatabase("QPSQL");
    db.setHostName("localhost");
    db.setDatabaseName("optic_db");
    db.setUserName("postgres");
    db.setPassword("optic111111");
    if(db.open()){
        qDebug() << "I'm?";
        otdr = new OTDR(usb_port);
    }else{
        qDebug() << "DB connection failed!" << db.lastError().text();
    }
    connect(otdr,SIGNAL(portNotOpened()),this,SLOT(deleteLater()));
    connect(otdr,SIGNAL(portNotOpened()),this,SIGNAL(oprtNotOpened()));
    connect(otdr,SIGNAL(dataIsReaded(QByteArray)),this,SLOT(goStep(QByteArray)));
    connect(otdr,SIGNAL(OTDRReady()),this,SLOT(actionSelection()));
}

bool OTDRProcessing::getIs_processing() const
{
    return is_processing;
}

void OTDRProcessing::goStep(QByteArray ba)
{
    qDebug() << "Processing " << process << ", go step " << step ;
    switch(process){
    case GETREFLECTOGRAMM:
        this->captureReflectogramm(ba);
        break;
    case SWITCHKEY:
        // do something
        break;
    case GETSTANDARTREFLECTOGRAMM:
        this->captureReflectogramm(ba);
        break;
    }
}

void OTDRProcessing::actionSelection()
{
    qDebug() << "\x1b[1;45;37m Select action \x1b[0m";
    QSqlQuery query;
    QString str = QString("SELECT ua.id_urgent_action, ua.id_fiber, ua.action FROM urgent_action ua, fiber fb WHERE fb.id_otdr = %1 AND ua.id_fiber = fb.id_fiber  LIMIT 1").arg(otdr->getId_otdr());
    qDebug() << str;
    if(query.exec(str)){
        if(query.size() > 0){
            int id_urgent_action;
            while (query.next()) {
                current_id_fiber = query.value("id_fiber").toInt();
                int action = query.value("action").toInt();
                id_urgent_action = query.value("id_urgent_action").toInt();
                qDebug() << "urgent_action on fiber - " << current_id_fiber << ", action is " << action << " " << id_urgent_action;
                switch(action){
                case 1:
                    process = GETREFLECTOGRAMM;
                    qDebug() << "\x1b[1;45;37m GETREFLECTOGRAMM \x1b[0m";
                    break;
                case 2:
                    process = SWITCHKEY;
                    qDebug() << "\x1b[1;45;37m SWITCHKEY \x1b[0m";
                    break;
                case 3:
                    process = GETSTANDARTREFLECTOGRAMM;
                    qDebug() << "\x1b[1;45;37m GETSTANDARTREFLECTOGRAMM \x1b[0m";
                    break;
                case 4:
                    process = FIBEROFF;
                    qDebug() << "\x1b[1;45;37m FIBEROFF \x1b[0m";
                    break;
                }

            }
            str = QString("DELETE FROM urgent_action WHERE id_urgent_action = %1").arg(id_urgent_action);
            qDebug() << "DELETE urgent_action";
            query.exec(str);
            qDebug() << "Returned";
            step = 0;
            QByteArray ba("0");
            goStep(ba);
            return;
        }else{
            qDebug() << "\x1b[1;45;37m No urgent action \x1b[0m";
        }
    }else{
        qDebug() << query.lastError().text();
        qDebug() << str;
    }
    str = QString("SELECT sh.id_schedule, sh.id_fiber,sh.interval, sh.last_update, sh.type_action FROM schedule sh, fiber fi WHERE fi.id_otdr = %1 AND fi.id_fiber = sh.id_fiber AND sh.enabled = TRUE").arg(otdr->getId_otdr());
    if(query.exec(str)){
        if(query.size() > 0){
            qDebug() << "\x1b[1;45;37m There is schedule action \x1b[0m";
            QMap<int,int> proSroch;
            while (query.next()) {
                QDateTime lastUpdate = query.value("last_update").toDateTime();
                int interval = query.value("interval").toInt();
                qint64 LUms = lastUpdate.toMSecsSinceEpoch();
                qint64 CTms = lastUpdate.currentDateTime().toMSecsSinceEpoch();
                current_id_fiber = query.value("id_fiber").toInt();
		if((CTms - LUms) > interval){
                    proSroch.insert(query.value("id_fiber").toInt(), (CTms - LUms));
                }
            }
            if(proSroch.size() > 0){
                int maxProSroch = 0;
                int id_fiber = 0;
                QMapIterator<int, int> i(proSroch);
                while (i.hasNext()) {
                    i.next();
                    if(i.value() > maxProSroch){
                        maxProSroch = i.value();
                        id_fiber = i.key();
                    }
                }
                //current_id_fiber = id_fiber;
                process = GETREFLECTOGRAMM;
                step = 0;
                QByteArray ba("0");
                goStep(ba);
                str = QString("UPDATE schedule SET last_update = now() WHERE id_fiber = %1").arg(id_fiber);
                if(!query.exec(str)){
                    qDebug() << query.lastError().text();
                    qDebug() << str;
                }
            }else{
                QTimer::singleShot(100, this, SLOT(actionSelection()));
            }
        }else{
            qDebug() << "\x1b[1;45;37m No schedule \x1b[0m";
            QTimer::singleShot(1000, this, SLOT(actionSelection()));
        }
    }else{
        qDebug() << query.lastError().text();
        qDebug() << str;
    }
}

void OTDRProcessing::captureReflectogramm(QByteArray requestData)
{
    switch (step){
        case 0:{
            QByteArray ba20("*MONITOR\r\n");
            otdr->writeData(ba20);
            step = 1;
            break;
        }
        case 1:{
            QString answer(requestData);
            //qDebug() << answer;
            if(answer == "OK\r"){
                qDebug() << "\n";
            }else if(answer == "1550\r"){
                qDebug() << "\n";
            }else if(answer == "STOP_OK\r"){
                QByteArray ba2(";rmt:get:graph 2\r\n");
                otdr->writeData(ba2);
                step = 3;
            }else{
                QByteArray ba21(";rmt:otdr:run\r\n");
                otdr->writeData(ba21);
            }
            break;
        }
        case 3:{
            Reflectogram *rf = new Reflectogram(requestData, current_id_fiber);
            if(process == GETSTANDARTREFLECTOGRAMM){
                rf->saveReflectogramm(true);
            }else{
                rf->saveReflectogramm(false);
            }
            delete rf;
            process = NOTHING;
            db.close();
            db.open();
            qDebug() << "NOTHING";
            actionSelection();
            break;
        }
    }
}

void OTDRProcessing::switchKey(QByteArray requestData)
{
    switch (step){
        case 0:{
            QByteArray ba20("*MONITOR\r\n");
            otdr->writeData(ba20);
            step = 1;
            break;
        }
        case 1:{
            QString answer(requestData);
            if(answer == "OK\r"){
//                qDebug() << "\n";
            }else if(answer == "1550\r"){
//                qDebug() << "\n";
            }else if(answer == "STOP_OK\r"){
//                QByteArray ba2(";rmt:get:graph 2\r\n");
//                otdr->writeData(ba2);
//                step = 3;
            }else{
//                QByteArray ba21(";rmt:otdr:run\r\n");
//                otdr->writeData(ba21);
            }
            break;
        }
        case 3:{
            Reflectogram *rf = new Reflectogram(requestData, current_id_fiber);
            if(process == GETSTANDARTREFLECTOGRAMM){
                rf->saveReflectogramm(true);
            }else{
                rf->saveReflectogramm(false);
            }
            delete rf;
            process = NOTHING;
            db.close();
            db.open();
            qDebug() << "NOTHING";
            actionSelection();
            break;
        }
    }
}

void OTDRProcessing::getOTDRParameters()
{

}

void OTDRProcessing::getLineParameters()
{

}

void OTDRProcessing::setOTDRParametrs()
{

}

void OTDRProcessing::setLineParameters()
{

}

int OTDRProcessing::getCurrent_id_fiber() const
{
    return current_id_fiber;
}

void OTDRProcessing::setLineNumber(int value)
{
    lineNumber = value;
}

int OTDRProcessing::getLineNumber() const
{
    return lineNumber;
}
