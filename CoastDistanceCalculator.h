#pragma once

#include <QObject>
#include <QGeoCoordinate>
#include <QVector>

class CoastDistanceCalculator : public QObject
{
    Q_OBJECT

public:
    static CoastDistanceCalculator* instance();
    void setCoastline(
        const QVector<QVector<QGeoCoordinate>> &coastLines);

    // Returns the minimum distance from position to coastline.
    double distanceToCoast(
        const QGeoCoordinate &position) const;

    // Returns true if the position is within maxDistanceMeters
    // of the coastline.
    bool isWithinDistance(
        const QGeoCoordinate &position,
        double maxDistanceMeters) const;

private:
    QVector<QVector<QGeoCoordinate>> m_coastLines;

    static double distanceToSegment(
        const QGeoCoordinate &position,
        const QGeoCoordinate &a,
        const QGeoCoordinate &b);

    static double distanceMeters(
        const QGeoCoordinate &a,
        const QGeoCoordinate &b);

private:
    explicit CoastDistanceCalculator();

private:
    static CoastDistanceCalculator* instance_;
};