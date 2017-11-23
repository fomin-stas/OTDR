#ifndef REFLECTOGRAM_H
#define REFLECTOGRAM_H

#include <QObject>
#include <QDebug>
#include <QDataStream>
#include <QFile>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>

class Reflectogram : public QObject {
    Q_OBJECT
public:
    explicit Reflectogram(QObject *parent = 0);
    explicit Reflectogram(QByteArray reflectogramma, int idFiber, QObject *parent = 0);
    bool saveReflectogramm(bool isStandard);

signals:

public slots:

private:
    void parsRawReflectogram();
    unsigned int uIntFromData(QByteArray *data_to_int, int start);
    unsigned short int shIntFromData(QByteArray *data_to_int, int start);
    int id_fiber;


    QSqlDatabase db;

    /*каретка*/
    int CR;
    QByteArray data;
    unsigned int valueDS;
    /*ВРЕМЕННОЕ расстояние между отсчетами в данной рефлектограмме*/

    int TD;

    /*число накоплений*/
    unsigned int accumvalue;

    /*длина волны в нм, на которой производится измерение*/
    unsigned short int wavelength;

    /*длительность зондирующего импульса в нС*/
    unsigned short int pulseDuration;

    /*число отсчетов которые необходимо игнорировать*/
    unsigned int ignor;
};

#endif // REFLECTOGRAM_H
