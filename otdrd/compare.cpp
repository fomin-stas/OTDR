#include "compare.h"
#include <QtSql>
#include <QVariant>
#include <QDebug>
#include <QtMath>


Compare::Compare(int id_fiber,int id_trace, QList<unsigned short int> *dots, QObject *parent) :
    QObject(parent),
    rfDots(dots),
    _id_fiber(id_fiber),
    _id_trace(id_trace),
    alfaPorog(1.0)
{
    QSqlQuery query;
    QString str =QString ("SELECT "
                          "rp.dot_value "
                          "FROM "
                          "reference_trace rt, reference_point rp "
                          "WHERE "
                          "rt.id_fiber = %1 AND "
                          "rt.id_trace = rp.id_trace").arg(id_fiber);
    if(query.exec(str)){
        if(query.size() > 0){
            while (query.next()) {
                unsigned int dot_value = query.value("dot_value").toUInt();
                unsigned short int  US_dot_value = dot_value;
                standardDots.append(US_dot_value);
            }
        }
    }
    query.clear();
}

Compare::~Compare()
{

}

int Compare::compareLSS(bool leaning)
{
    qDebug() << "Start compareLSS";
    int window;
    QSqlQuery query;
    QString str =QString ("SELECT * FROM fiber WHERE id_fiber = %1").arg(_id_fiber);
    if(query.exec(str)){
        if(query.size() > 0){
            while (query.next()) {
                window = query.value("aproximation_points").toUInt();
            }
        }
    }
    //===========Для вывода графика разности угловых коефициентов
    for(int i = 0 ; i < (standardDots.size() - window); i++){
        alfas.append(deltaAlfaKoeficient(i,i+window));
    }
    //==============================================================================
    int num_events=0;
    str =QString ("SELECT * FROM trace WHERE id_trace = %1").arg(_id_trace);
    if(query.exec(str)){
        if(query.size() > 0){
            while (query.next()) {
                ds = query.value("ds").toDouble();
            }
        }
    }else{
        qDebug() << "fail get DS parameter";
        return -1;
    }
    str =QString ("SELECT * FROM fiber_segments WHERE id_fiber = %1").arg(_id_fiber);
    if(query.exec(str)){
        if(query.size() > 0){
            while (query.next()) {
                double start = query.value("start").toDouble();
                double end = query.value("end").toDouble();
                int start_n = this->getNumberFromMeter(start);
                int end_n = this->getNumberFromMeter(end);
                double delta = deltaAlfaKoeficient(start_n, end_n);
                segmentsAlfas.insert(query.value("id_fiber_segments").toInt(),delta);
                int error_point;
                LineEvent::Status compareStatus;
                for(int i = start_n ; i < (end_n - window); i++){
                    delta = deltaAlfaKoeficient(i,i+window);
                    if (delta > qFabs(alfaPorog)){
                        double delta_start = delta;
                        int countNormalDeltas = 0;
                        int event_dB = 0;
                        int event_start = i;
                        int end_point = i;
                        while(countNormalDeltas < 5 && end_point < (end_n - window)){
                            delta = deltaAlfaKoeficient(end_point,event_start+window);
                            if (delta < qFabs(alfaPorog)){
                                countNormalDeltas++;
                            }else{
                                countNormalDeltas = 0;
                            }
                            end_point++;
                        }
                        error_point = event_start + (end_point - event_start)/2;
                        if(end_point == (end_n - window) && start_n == event_start){
                            compareStatus = LineEvent::Status::FIBER_IS_MISSING;
                        }else if(end_point == (end_n - window)){
                            compareStatus = LineEvent::Status::BREAK;
                        }else if(delta_start > 0){
                            compareStatus = LineEvent::Status::JUNCTION;
                            event_dB = deltadB(event_start-window-2,event_start-2,end_point-2,end_point+window-2);
                        }else{
                            compareStatus = LineEvent::Status::FADING;
                            event_dB = deltadB(event_start-window-2,event_start-2,end_point-2,end_point+window-2);
                        }
                        LineEvent newEvent;
                        newEvent.setEventStatus(compareStatus);
                        newEvent.setPoint(error_point);
                        newEvent.setdB(event_dB);
                        events.append(newEvent);
                        i=end_point;
                        num_events++;
                    }
                }
            }
        }
    }
    standardDots.clear();
    rfDots->clear();
    return num_events;
}

Compare::AB Compare::LSS(QList<unsigned short int> *dots){
    int n = dots->size();
    double sumx = 0;
    double sumy = 0;
    double sumx2 = 0;
    double sumxy = 0;
    for (int i = 0; i<n; i++) {
        sumx += i;
        sumy += dots->at(i);
        sumx2 += pow(i,2.);
        sumxy += (i * dots->at(i));
    }
    AB lss_ab;
    double a = ((double)n*sumxy - (sumx*sumy)) / ((double)n*sumx2 - sumx*sumx);
    double b = (sumy - a*sumx)/(double)n;
    lss_ab.a = a;
    lss_ab.b = b;
    return lss_ab;
}

double Compare::deltaAlfaKoeficient(int start, int end)
{
    QList<unsigned short int> dotsStandart;
    QList<unsigned short int> dotsRF;
    for(int wi = start; wi < end; wi++ ){
        dotsStandart.append(standardDots.at(wi));
        dotsRF.append(rfDots->at(wi));
    }
    AB ab1 = LSS(&dotsStandart);
    AB ab2 = LSS(&dotsRF);
    double delta = ab2.a - ab1.a;
    dotsStandart.clear();
    dotsRF.clear();
    return delta;
}

double Compare::deltadB(int start1, int end1, int start2, int end2)
{
    QList<unsigned short int> dots1;
    QList<unsigned short int> dots2;
    for(int wi = start1; wi < end1; wi++ ){
        dots1.append(rfDots->at(wi));
    }
    for(int wi = start2; wi < end2; wi++ ){
        dots2.append(rfDots->at(wi));
    }
    AB ab1 = LSS(&dots1);
    AB ab2 = LSS(&dots2);
    double delta = ab2.b - ab1.b;
    dots1.clear();
    dots2.clear();
    return delta;
}

int Compare::getNumberFromMeter(double x){
    double vs = 1.4681 * 1000000.0;
    double ches = (double)ds * 2.99792458;
    double constValue = ches / vs;
    return x / constValue;
}

QList<LineEvent> Compare::getEvents() const
{
    return events;
}

QMap<int, double> Compare::getSegmentsAlfas() const
{
    return segmentsAlfas;
}

QList<double> Compare::getAlfas() const{
    return alfas;
}
