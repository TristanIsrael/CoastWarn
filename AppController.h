#pragma once

#include <QObject>
#include <QGeoPositionInfoSource>

class AppController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(double distance READ distance  NOTIFY distanceChanged FINAL)
    Q_PROPERTY(int maxSpeed READ maxSpeed NOTIFY maxSpeedChanged FINAL)

signals:
    void distanceChanged();
    void maxSpeedChanged();
    void error(const QString&) const;
    void information(const QString&) const;

public:
    explicit AppController(QObject *parent = nullptr);

    void start();

public:
    double distance() const;
    int maxSpeed() const;

private:
    void startPositionUpdates() const;
    void calculateDistance();
    void configureLocation();

private slots:
    void onPositionUpdated(const QGeoPositionInfo&);
    void onPositionError(QGeoPositionInfoSource::Error);

private:
    QGeoPositionInfoSource* geoSource_ = nullptr;
    double distance_ = 0;
    QGeoCoordinate coord_;
};
