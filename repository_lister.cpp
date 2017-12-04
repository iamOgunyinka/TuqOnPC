#include "repository_lister.hpp"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonArray>

RepositoryLister::RepositoryLister( utilities:: ListMap & repos , QObject *parent ):
    QObject( parent ), repository{ repos }
{
}

void RepositoryLister::SetNetworkManager( QNetworkAccessManager *manager )
{
    network_manager = manager;
}

void RepositoryLister::GetList( QString const & address )
{
    auto request = utilities::GetRequest( address );
    QNetworkReply *network_reply = network_manager->get( request );
    QObject::connect( network_reply, &QNetworkReply::finished, this,
                      &RepositoryLister::OnNetworkResultObtained );
}

void RepositoryLister::OnNetworkResultObtained()
{
    QNetworkReply *response = qobject_cast<QNetworkReply*>( sender() );
    Q_ASSERT( response != nullptr );

    if( response->error() != QNetworkReply::NoError ){
        emit ErrorOccurred( response->errorString() );
        return;
    }
    QJsonDocument json_document{ QJsonDocument::fromJson( response->readAll() ) };
    if( json_document.isNull() ){ // do we *not* have a valid JSON document?
        emit ErrorOccurred( "We got an invalid response from the server." );
        return;
    }
    emit ResultObtained( json_document.object() );
}

void RepositoryLister::OnValidDocumentObtained( QJsonObject const & document )
{
    if( !document.value( "status" ).toInt() ){
        emit ErrorOccurred( document.value( "detail" ).toString() );
        return;
    }
    repository.clear();
    QString repo_name {};
    QJsonArray repos = document.value( "repositories" ).toArray();
    for( int i = 0; i != repos.size(); ++i ){
        QJsonObject object = repos.at( i ).toObject();
        repo_name = object.value( "name" ).toString();
        repository.insert( repo_name, {} );
        QJsonArray courses = object.value( "courses" ).toArray();
        QJsonArray course_info {};
        for( int x = 0; x != courses.size(); ++x ){
            course_info = courses.at( x ).toArray();
            repository[repo_name].append( { course_info.at( 0 ).toString(),
                                            course_info.at( 1 ).toInt() } );
        }
    }
    emit Finished();
}
