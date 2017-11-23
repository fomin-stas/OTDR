#ifndef LINEEVENT_H
#define LINEEVENT_H

#include <QObject>

class LineEvent : public QObject
{
    Q_OBJECT
public:
    explicit LineEvent(QObject *parent = 0);
    LineEvent(const LineEvent &other);
    LineEvent &operator=(const LineEvent &other);
    enum Status {
        NOTHING,            //все нормально
        FADING,             //затухание
        BREAK,              //обрыв
        JUNCTION,           //пик
        FIBER_IS_MISSING    //волокно отсутствует
    };
    Status getEventStatus() const;
    void setEventStatus(const Status &value);

    int getPoint() const;
    void setPoint(int value);

    double getdB() const;
    void setdB(double value);

signals:

public slots:

private:
    Status eventStatus;
    int point;
    double dB;
};

#endif // EVENT_H
