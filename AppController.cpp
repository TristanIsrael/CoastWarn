#include "AppController.h"
#include "CoastDistanceCalculator.h"
#include "ShomCoastDownloader.h"
#include <QGeoPositionInfo>
#include <QLocationPermission>
#include <QCoreApplication>
#include <QMutex>
#include <QMutexLocker>
#include <QTimer>

constexpr int SixMilles = 6 * 1800;

AppController::AppController(QObject *parent)
    : QObject{parent}
{
    qDebug() << "Available GPS sources:";
    qDebug() << QGeoPositionInfoSource::availableSources();

    geoSource_ = QGeoPositionInfoSource::createDefaultSource(this);

    if (!geoSource_) {
        return;
    }

    geoSource_->setUpdateInterval(1000); // 1 Hz

    connect(
        geoSource_,
        &QGeoPositionInfoSource::positionUpdated,
        this,
        &AppController::onPositionUpdated
    );

    connect(
        geoSource_,
        &QGeoPositionInfoSource::errorOccurred,
        this,
        &AppController::onPositionError
    );

    connect(
        ShomCoastDownloader::instance(),
        &ShomCoastDownloader::downloadFinished,
        CoastDistanceCalculator::instance(),
        &CoastDistanceCalculator::setCoastline
    );

    connect(ShomCoastDownloader::instance(),
        &ShomCoastDownloader::downloadFinished,
        this,
        &AppController::calculateDistance
    );

}

void AppController::startPositionUpdates() const
{
    if(geoSource_ != nullptr) {
        geoSource_->startUpdates();
    }
}

void AppController::start() const
{
    QLocationPermission permission;

    permission.setAccuracy(QLocationPermission::Precise);
    permission.setAvailability(QLocationPermission::WhenInUse);

    const auto status = qApp->checkPermission(permission);

    if (status != Qt::PermissionStatus::Granted) {
        qApp->requestPermission(
            permission,
            this,
            [this](const QPermission &permission) {
                if (permission.status() == Qt::PermissionStatus::Granted) {
                    startPositionUpdates();
                } else {
                    emit error(tr("Location permission denied"));
                    qWarning() << "Location permission denied.";
                }
            });

        return;

    }

}

double AppController::distance() const
{
    return distance_;
}

static QMutex mutexPosition;
void AppController::onPositionUpdated(const QGeoPositionInfo& position)
{
    QMutexLocker lock(&mutexPosition);

    coord_ = position.coordinate();
#ifdef QT_DEBUG
    coord_ = QGeoCoordinate(47.546329, -2.922000); // 240m
    //coord_ = QGeoCoordinate(47.546329, -2.924000); // 378m
    //coord_ = QGeoCoordinate(47.546329, -2.944000); // 933m
#endif

    // First we verify whether coast data have been downloaded
    if(!ShomCoastDownloader::instance()->ready()) {
        ShomCoastDownloader::instance()->download(coord_, SixMilles);
        return;
    } else {
        calculateDistance();
    }
}

void AppController::calculateDistance() {
    auto dist = CoastDistanceCalculator::instance()->distanceToCoast(coord_);
    if(dist != INFINITY) {
        distance_ = qRound(dist);
    }

    qDebug() << "distance =" << distance_;

    if (distance_ <= 300.0) {
        // Inside the 300 m zone.
        qDebug() << "Dans la bande des 300m";
    } else if (distance_ <= 500.0) {
        // Between 300 and 500 m.
        qDebug() << "Dans la bande des 500m";
    } else {
        qDebug() << "Au-delà des 500m";
    }

    emit distanceChanged();
    emit maxSpeedChanged();
}

void AppController::onPositionError(QGeoPositionInfoSource::Error err)
{
    qWarning() << tr("An error occured while fetching the GPS coordinate:%1").arg(err);
    //emit error(tr("GPS error: %1").arg(err));
}

int AppController::maxSpeed() const
{
    if(distance_ > 300) {
        return -1;
    } else if(distance_ <= 300) {
        return 5;
    }

    return -1;
}
