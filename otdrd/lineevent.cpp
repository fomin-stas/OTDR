#include "lineevent.h"

LineEvent::LineEvent(QObject *parent) : QObject(parent),
    eventStatus(NOTHING),
    point(-1),
    dB(0.0)
{

}

LineEvent::LineEvent(const LineEvent &other)
{
    eventStatus = other.getEventStatus();
    point = other.getPoint();
    dB = other.getdB();
}

LineEvent &LineEvent::operator=(const LineEvent &other)
{
    eventStatus = other.getEventStatus();
    point = other.getPoint();
    dB = other.getdB();
}

LineEvent::Status LineEvent::getEventStatus() const
{
    return eventStatus;
}

void LineEvent::setEventStatus(const Status &value)
{
    eventStatus = value;
}

int LineEvent::getPoint() const
{
    return point;
}

void LineEvent::setPoint(int value)
{
    point = value;
}

double LineEvent::getdB() const
{
    return dB;
}

void LineEvent::setdB(double value)
{
    dB = value;
}

