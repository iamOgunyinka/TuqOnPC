#ifndef REPOSITORY_LISTER_HPP
#define REPOSITORY_LISTER_HPP

#include <QJsonObject>
#include <QObject>
#include <QList>
#include <QMap>
#include "resources.hpp"

class QNetworkAccessManager;

class RepositoryLister: public QObject
{
    Q_OBJECT

public:
    RepositoryLister( utilities::ListMap & repository,QObject *parent = nullptr );
    void SetNetworkManager( QNetworkAccessManager *network_manager );
    void GetList( QString const &address );

public slots:
    void OnNetworkResultObtained();
    void OnValidDocumentObtained( QJsonObject const & document );
signals:
    void ResultObtained( QJsonObject );
    void ErrorOccurred( QString const & );
    void Finished();
private:
    utilities::ListMap &repository;
    QNetworkAccessManager *network_manager;
};

#endif // REPOSITORY_LISTER_HPP
