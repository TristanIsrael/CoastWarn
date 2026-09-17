#include "ShomCoastDownloader.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
#include <QUrlQuery>
#include <QtMath>
#include <QCoreApplication>
#include <QMutex>
#include <QMutexLocker>

namespace
{
constexpr double EarthRadiusMeters = 6371008.8;

constexpr char WfsUrl[] =
    "https://services.data.shom.fr/INSPIRE/wfs";

constexpr char CoastLayer[] =
    "LIMTM_2154_WFS:limite_terre_mer_france_metropolitaine_ligne";
}

ShomCoastDownloader* ShomCoastDownloader::instance_ = nullptr;
ShomCoastDownloader* ShomCoastDownloader::instance() {
    if(instance_ == nullptr) {
        instance_ = new ShomCoastDownloader;
    }

    return instance_;
}

ShomCoastDownloader::ShomCoastDownloader()
    : QObject(qApp)
{
}

static QMutex mutexDownload;
void ShomCoastDownloader::download(const QGeoCoordinate& center, double radiusMeters)
{
    if(working_) {
        qDebug() << "Already working...";
        return;
    }

    if (!center.isValid() || radiusMeters <= 0.0) {
        qWarning() << "Invalid center coordinate or radius.";
        emit error(tr("Invalid coordinates"));
        return;
    }

    QMutexLocker lock(&mutexDownload);
    working_ = true;

    qDebug() << center;

    emit downloading();

    if (m_reply != nullptr) {
        m_reply->abort();
        m_reply->deleteLater();
        m_reply = nullptr;
    }

    const QUrl url(buildUrl(center, radiusMeters));

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("CoastDistanceApp/1.0"));

    m_reply = m_networkManager.get(request);

    connect(
        m_reply,
        &QNetworkReply::finished,
        this,
        &ShomCoastDownloader::onReplyFinished);
}

bool ShomCoastDownloader::ready() const
{
    return ready_;
}

void ShomCoastDownloader::onReplyFinished()
{
    QNetworkReply* reply = m_reply;
    m_reply = nullptr;

    if (reply == nullptr) {
        working_ = false;
        return;
    }

    const auto data = reply->readAll();

    if (reply->error() != QNetworkReply::NoError)
    {
        qWarning() << reply->errorString();
        reply->deleteLater();
        working_ = false;
        return;
    }

    const auto coastLines = parseGeoJson(data);

    if (coastLines.isEmpty())
    {
        qWarning() << "No coastline geometry found in WFS response.";
        emit error(tr("No coastline data found"));
    }
    else
    {
        ready_ = true;
        emit readyChanged();
        qDebug() << "Coastline downloaded successfully";
        emit downloadFinished(coastLines);
    }

    reply->deleteLater();
    working_ = false;
}

QString ShomCoastDownloader::buildUrl(
    const QGeoCoordinate &center,
    double radiusMeters)
{
    const double lat = center.latitude();
    const double lon = center.longitude();

    const double latDelta =
        metersToLatitudeDegrees(radiusMeters);

    const double lonDelta =
        metersToLongitudeDegrees(radiusMeters, lat);

    const double minLon = lon - lonDelta;
    const double minLat = lat - latDelta;
    const double maxLon = lon + lonDelta;
    const double maxLat = lat + latDelta;

    QUrl url(QString::fromLatin1(WfsUrl));

    QUrlQuery query;

    query.addQueryItem(QStringLiteral("service"),
                       QStringLiteral("WFS"));

    query.addQueryItem(QStringLiteral("version"),
                       QStringLiteral("2.0.0"));

    query.addQueryItem(QStringLiteral("request"),
                       QStringLiteral("GetFeature"));

    query.addQueryItem(QStringLiteral("typeNames"),
                       QString::fromLatin1(CoastLayer));

    query.addQueryItem(QStringLiteral("outputFormat"),
                       QStringLiteral("application/json"));

    query.addQueryItem(QStringLiteral("srsName"),
                       QStringLiteral("EPSG:4326"));

    // WFS 2.0 BBOX:
    // minLon,minLat,maxLon,maxLat,CRS
    const QString bbox =
        QStringLiteral("%1,%2,%3,%4,EPSG:4326")
            .arg(minLon, 0, 'f', 8)
            .arg(minLat, 0, 'f', 8)
            .arg(maxLon, 0, 'f', 8)
            .arg(maxLat, 0, 'f', 8);

    query.addQueryItem(
        QStringLiteral("bbox"),
        bbox);

    url.setQuery(query);

    return url.toString();
}

double ShomCoastDownloader::metersToLatitudeDegrees(
    double meters)
{
    return meters / 111320.0;
}

double ShomCoastDownloader::metersToLongitudeDegrees(
    double meters,
    double latitudeDegrees)
{
    const double latitudeRadians =
        qDegreesToRadians(latitudeDegrees);

    const double metersPerDegree =
        111320.0 * std::cos(latitudeRadians);

    if (std::abs(metersPerDegree) < 1e-9)
        return 180.0;

    return meters / metersPerDegree;
}

QVector<QVector<QGeoCoordinate>>
ShomCoastDownloader::parseGeoJson(const QByteArray &data)
{
    QVector<QVector<QGeoCoordinate>> result;

    QJsonParseError parseError;
    const QJsonDocument document =
        QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError)
        return result;

    if (!document.isObject())
        return result;

    const QJsonObject root = document.object();

    const QJsonArray features =
        root.value(QStringLiteral("features"))
            .toArray();

    for (const QJsonValue &featureValue : features)
    {
        const QJsonObject feature =
            featureValue.toObject();

        const QJsonObject geometry =
            feature.value(QStringLiteral("geometry"))
                .toObject();

        const QString type =
            geometry.value(QStringLiteral("type"))
                .toString();

        const QJsonArray coordinates =
            geometry.value(QStringLiteral("coordinates"))
                .toArray();

        if (type == QStringLiteral("LineString"))
        {
            QVector<QGeoCoordinate> line;

            for (const QJsonValue &pointValue : coordinates)
            {
                const QJsonArray point =
                    pointValue.toArray();

                if (point.size() < 2)
                    continue;

                const double lon = point.at(0).toDouble();
                const double lat = point.at(1).toDouble();

                line.append(QGeoCoordinate(lat, lon));
            }

            if (line.size() >= 2)
                result.append(line);
        }
        else if (type == QStringLiteral("MultiLineString"))
        {
            for (const QJsonValue &lineValue : coordinates)
            {
                const QJsonArray lineCoordinates =
                    lineValue.toArray();

                QVector<QGeoCoordinate> line;

                for (const QJsonValue &pointValue :
                     lineCoordinates)
                {
                    const QJsonArray point =
                        pointValue.toArray();

                    if (point.size() < 2)
                        continue;

                    const double lon =
                        point.at(0).toDouble();

                    const double lat =
                        point.at(1).toDouble();

                    line.append(
                        QGeoCoordinate(lat, lon));
                }

                if (line.size() >= 2)
                    result.append(line);
            }
        }
    }

    return result;
}