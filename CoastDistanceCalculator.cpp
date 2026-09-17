#include "CoastDistanceCalculator.h"

#include <QtMath>
#include <QCoreApplication>
#include <algorithm>
#include <cmath>
#include <limits>

namespace
{
constexpr double EarthRadiusMeters = 6371008.8;

struct LocalPoint
{
    double x;
    double y;
};

LocalPoint toLocal(
    const QGeoCoordinate &origin,
    const QGeoCoordinate &point)
{
    const double lat0 =
        qDegreesToRadians(origin.latitude());

    const double dLat =
        qDegreesToRadians(
            point.latitude() - origin.latitude());

    const double dLon =
        qDegreesToRadians(
            point.longitude() - origin.longitude());

    return {
        EarthRadiusMeters * dLon * std::cos(lat0),
        EarthRadiusMeters * dLat
    };
}
}

CoastDistanceCalculator::CoastDistanceCalculator()
    : QObject{qApp}
{
}

CoastDistanceCalculator* CoastDistanceCalculator::instance_ = nullptr;
CoastDistanceCalculator* CoastDistanceCalculator::instance()
{
    if(instance_ == nullptr) {
        instance_ = new CoastDistanceCalculator;
    }

    return instance_;
}

void CoastDistanceCalculator::setCoastline(
    const QVector<QVector<QGeoCoordinate>> &coastLines)
{
    m_coastLines = coastLines;
}

double CoastDistanceCalculator::distanceToCoast(
    const QGeoCoordinate &position) const
{
    if (!position.isValid() || m_coastLines.isEmpty())
        return std::numeric_limits<double>::infinity();

    double minimumDistance =
        std::numeric_limits<double>::infinity();

    for (const auto &line : m_coastLines)
    {
        for (int i = 0; i + 1 < line.size(); ++i)
        {
            const double distance =
                distanceToSegment(
                    position,
                    line.at(i),
                    line.at(i + 1));

            minimumDistance =
                std::min(minimumDistance, distance);
        }
    }

    return minimumDistance;
}

bool CoastDistanceCalculator::isWithinDistance(
    const QGeoCoordinate &position,
    double maxDistanceMeters) const
{
    if (maxDistanceMeters < 0.0)
        return false;

    return distanceToCoast(position)
           <= maxDistanceMeters;
}

double CoastDistanceCalculator::distanceToSegment(
    const QGeoCoordinate &position,
    const QGeoCoordinate &a,
    const QGeoCoordinate &b)
{
    const LocalPoint pa = toLocal(position, a);
    const LocalPoint pb = toLocal(position, b);

    const double dx = pb.x - pa.x;
    const double dy = pb.y - pa.y;

    const double lengthSquared =
        dx * dx + dy * dy;

    if (lengthSquared < 1e-12)
    {
        return std::hypot(pa.x, pa.y);
    }

    // Projection of the origin onto the segment.
    double t =
        -(pa.x * dx + pa.y * dy)
        / lengthSquared;

    t = std::clamp(t, 0.0, 1.0);

    const double x =
        pa.x + t * dx;

    const double y =
        pa.y + t * dy;

    return std::hypot(x, y);
}

double CoastDistanceCalculator::distanceMeters(
    const QGeoCoordinate &a,
    const QGeoCoordinate &b)
{
    return a.distanceTo(b);
}