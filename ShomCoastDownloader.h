#pragma once

#include <QObject>
#include <QGeoCoordinate>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QVector>

class ShomCoastDownloader : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool ready READ ready NOTIFY readyChanged FINAL)

signals:
    void downloadFinished(const QVector<QVector<QGeoCoordinate>> &coastLines);
    void error(const QString &error);
    void downloading();
    void readyChanged();

public:
    static ShomCoastDownloader* instance();

    // Downloads coastline data around center within radiusMeters.
    void download(const QGeoCoordinate &center, double radiusMeters);
    bool ready() const;

private slots:
    void onReplyFinished();

private:
    explicit ShomCoastDownloader();

    static QString buildUrl(const QGeoCoordinate &center, double radiusMeters);

    static double metersToLatitudeDegrees(double meters);
    static double metersToLongitudeDegrees(double meters, double latitudeDegrees);

    static QVector<QVector<QGeoCoordinate>> parseGeoJson(const QByteArray &data);

private:
    static ShomCoastDownloader* instance_;
    QNetworkAccessManager m_networkManager;
    QNetworkReply* m_reply = nullptr;
    bool ready_ = false;
    bool working_ = false;
};
