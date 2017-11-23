#ifndef COMPARE_H
#define COMPARE_H

#include <QObject>
#include <QMap>
#include <QList>
#include "lineevent.h"
#define PI 3.14159265
class Compare : public QObject
{
    Q_OBJECT
public:
    explicit Compare(int id_fiber, int id_trace, QList<unsigned short int> *dots, QObject *parent = 0);
    int compareLSS(bool leaning);
    ~Compare();

    QList<double> getAlfas() const;

    QMap<int, double> getSegmentsAlfas() const;
    struct AB{
        double a;
        double b;
    };

    QList<LineEvent> getEvents() const;

signals:

public slots:

private:
    AB LSS(QList<unsigned short int> *dots);
    double deltaAlfaKoeficient(int start, int end);
    double deltadB(int start1, int end1, int start2, int end2);
    int getNumberFromMeter(double);
    QList<unsigned short int> *rfDots;
    QList<unsigned short int> standardDots;
    QList<double> alfas;
    QMap<int, double> segmentsAlfas;
    double alfaPorog;
    int _id_fiber;
    int _id_trace;
    int ds;
    QList<LineEvent> events;

};

#endif // COMPARE_H
