#include "reflectogram.h"
#include "compare.h"

Reflectogram::Reflectogram(QObject *parent) : QObject(parent)
{

}

Reflectogram::Reflectogram(QByteArray rawReflectogramm,int idFiber, QObject *parent) :
    QObject(parent),
    data(rawReflectogramm),
    id_fiber(idFiber)
{
    parsRawReflectogram();

}

void Reflectogram::parsRawReflectogram(){
    CR = 0; //каретка
    //qDebug() << data.toHex() << endl;
    /*[1]Количество байт определяющих размер информационного блока*/
    CR = 1;
    int countD = data.at(CR) - 0x30;
    //qDebug() << countD << endl;
    /*[!1]*/

    /*[2]Считаем размер информационного блока*/
    int mul = 1;
    int infoBlockSize = 0;
    CR = 2;
    for (int i = countD + 1; i > 1; i--){
        /*читаем с конца, переводим в int и умнажаем на его порядок*/
        infoBlockSize += (data.at(i) - 0x30) * mul;
        mul *= 10;
    }
    //qDebug() << "Info block size=" << infoBlockSize << endl;
    /*[!2]*/

    /*[3]Считываем параметры измерения*/
    CR = countD + 2;

    valueDS = uIntFromData(&data,CR);
    CR +=4;

    //qDebug() << "valueDS =" << valueDS << endl;

    /*ВРЕМЕННОЕ расстояние между отсчетами в данной рефлектограмме*/
    TD = (valueDS / 10000) * 0.1;
    //qDebug() << "Time destination =" << TD << endl;

    /*число накоплений*/
    accumvalue =  uIntFromData(&data,CR);
    //qDebug() << "Count iterations =" << accumvalue << endl;
    CR +=4;

    /*длина волны в нм, на которой производится измерение*/
    wavelength = shIntFromData(&data,CR);
    //qDebug() << "wavelength =" << wavelength << endl;
    CR +=2;
    /*длительность зондирующего импульса в нС*/
    pulseDuration = shIntFromData(&data,CR);
    //qDebug() << "pulseDuration =" << pulseDuration << endl;
    CR +=2;

    /*здесь не используется , передается нулями*/
    CR+=4;

    /*число отсчетов которые необходимо игнорировать*/
    ignor = uIntFromData(&data,CR);
    //qDebug() << "ignore counts =" << ignor << endl;
    CR +=4;
}

unsigned int Reflectogram::uIntFromData(QByteArray *data_to_int, int start)
{
    QByteArray valueBA;
    for (int i = start; i < start+4; i++){
        valueBA.prepend(data_to_int->at(i));
    }

    QDataStream ds(valueBA);
    unsigned int value;
    ds >> value;
    return value;
}

unsigned short int Reflectogram::shIntFromData(QByteArray *data_to_int, int start)
{
    QByteArray valueBA;
    valueBA.prepend(data_to_int->at(start));
    valueBA.prepend(data_to_int->at(start+1));
    QDataStream ds(valueBA);
    unsigned short int value;
    ds >> value;
    return value;
}

bool Reflectogram::saveReflectogramm(bool isStandard){
    QString traceTable;
    QString dotsTable;
    QSqlQuery query;
    if(isStandard){
        qDebug() << "Standart";
        traceTable = "reference_trace";
        dotsTable = "reference_point";
        QString str = QString("DELETE FROM reference_trace WHERE id_fiber = %1").arg(id_fiber);
        if(!query.exec(str)){
            qDebug() << "Fails delete standart reflectogramm from DB!  " << query.lastError().text();
            qDebug() << str;
        }
    }else{
        traceTable = "trace";
        dotsTable = "dots";
    }
    qDebug() << "Id fiber = " << id_fiber;
    QString queryStr = QString("INSERT INTO %5 (id_fiber,id_user,trace_date, ds, accumvalue, wave, pulse) "
                               "VALUES (%6,1,now(),%1,%2,%3,%4)")
            .arg(valueDS)
            .arg(accumvalue)
            .arg(wavelength)
            .arg(pulseDuration)
            .arg(traceTable)
            .arg(id_fiber);
    if(!query.exec(queryStr)){
        qDebug() << "Fails insert new trace into DB!  " << query.lastError().text();
        qDebug() << queryStr;
        return false;
    }
    int id_trace = query.lastInsertId().toInt();
    //qDebug() << "Id_trace = " << id_trace;
    /*[!3]*/
    CR = CR + ignor*2;

    double n = 1;
    //    double vs = 1.4681 * 1000000.0;
    //    double LIGHTSPEED = (double)valueDS * 2.99792458;
    //    double constValue = LIGHTSPEED/vs;
    QList<unsigned short int> dots;
    queryStr = QString("INSERT INTO %1 (id_trace, dot_value) VALUES ").arg(dotsTable);
    for (int i = CR; i < data.size(); i+=2){
        unsigned short int Y = shIntFromData(&data,i);
        n++;
        queryStr += QString("(%1, %2), ").arg(id_trace).arg(Y);
        dots << Y;
    }
    queryStr.remove(queryStr.size() - 2 , 1);
    if(!query.exec(queryStr)){
        qDebug() << "Fails insert new trace into DB!  " << query.lastError().text();
        return false;
    }
    qDebug() << "Total dots whith ignored dots = " << n;
    if(!isStandard){
        qDebug() << "Start compare";
        Compare * rfCompare = new Compare(id_fiber,id_trace,&dots);
        bool is_learnin = true; //нужна на время экспериментов, потом перевести в режим false
        int resultCompare = rfCompare->compareLSS(is_learnin);
        if(is_learnin){
            QList<double> alfas = rfCompare->getAlfas();
            queryStr = QString("INSERT INTO alfa_compare (id_trace, alfa) VALUES ");
            for (int i = 0; i < alfas.size(); i++){
                double alfa = alfas.at(i);
                queryStr += QString("(%1, %2), ").arg(id_trace).arg(alfa);
            }
            queryStr.remove(queryStr.size() - 2 , 1);
            if(!query.exec(queryStr)){
                qDebug() << "Fails insert new alfas into DB!  " << query.lastError().text();
            }
            query.clear();
            QMap<int,double> segmentsAlfas = rfCompare->getSegmentsAlfas();
            queryStr = QString("INSERT INTO segments_alfas (id_fiber_segments, alfa) VALUES ");
            QMapIterator<int, double> i(segmentsAlfas);
            while (i.hasNext()) {
                i.next();
                double alfa = i.value();
                queryStr += QString("(%1, %2), ").arg(i.key()).arg(alfa);
            }
            queryStr.remove(queryStr.size() - 2 , 1);
            if(!query.exec(queryStr)){
                qDebug() << "Fails insert new alfas into DB!  " << query.lastError().text();
            }
            query.clear();
        }
        qDebug() << "Count of events: " << resultCompare;
        if(resultCompare > 0){
            QList<LineEvent> events = rfCompare->getEvents();
            for (int i = 0; i < events.size(); i++){
                double dB = events.at(i).getdB();
                queryStr = QString("select count(*) FROM alarm_levels WHERE \"dB\" < %1").arg(dB);
                int alarm_level;
                if(query.exec(queryStr)){
                    if(query.size() > 0){
                        while (query.next()) {
                            alarm_level = query.value("count").toInt();
                        }
                    }
                }
                queryStr = QString("INSERT INTO alarm (id_user,id_fiber, id_trace, id_alarm_levels, point, \"dB\", alarm_status, alarm_type, ackstatus, fiber_status, alarm_start, statuschanged) "
                                   "VALUES (0,%1,%2,%3,%4, %5, 1, %6, FALSE, 1, NOW(), NOW())")
                        .arg(id_fiber)
                        .arg(id_trace)
                        .arg(alarm_level+1)
                        .arg(events.at(i).getPoint())
                        .arg(dB)
                        .arg(events.at(i).getEventStatus());
                if(!query.exec(queryStr)){
                    qDebug() << "Fails insert new alarm! -- " << query.lastError().text();
                }
            }
        }
        delete rfCompare;
    }
    dots.clear();
    return true;
}
