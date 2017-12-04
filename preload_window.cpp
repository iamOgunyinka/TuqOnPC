#include "preload_window.hpp"
#include "mainwindow.hpp"
#include <QFile>
#include <QSplashScreen>
#include <QPixmap>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSplashScreen>
#include <QNetworkCookieJar>
#include <QMessageBox>
#include <QTextStream>
#include <QInputDialog>
#include <QApplication>
#include <QJsonObject>

PreloadWindow::PreloadWindow( QApplication *app, QObject *parent ) : QObject( parent ),
    main_application( app ),
    splash_screen( new QSplashScreen() ),
    network_manager( new QNetworkAccessManager )
{
    QPixmap splash_logo( ":/image/images/tuq_feature-graphic.png" );
    splash_screen->setPixmap( splash_logo.scaled( 500, 300 ) );
}

void PreloadWindow::ShowSplashScreen()
{
    splash_screen->show();
    main_application->processEvents();
    LoadStartupFile();
    splash_screen->showMessage( "Reading startup file" );
    main_application->processEvents();
    auto request = utilities::GetRequest( endpoint.main_url );
    network_manager->setCookieJar( new QNetworkCookieJar( this ) );
    QNetworkReply *reply = network_manager->get( request );
    QObject::connect( reply, &QNetworkReply::finished, this, &PreloadWindow::OnGetServerEndpoints );
    splash_screen->showMessage( "Getting endpoints from server" );
    main_application->processEvents();
}

void PreloadWindow::OnGetServerEndpoints()
{
    QJsonObject root = utilities::OnNetworkResponseReceived(qobject_cast<QNetworkReply*>(sender()));
    if( root.isEmpty() || !root.contains( "endpoints" ) ){
        std::exit( -1 );
    }
    QJsonObject server_endpoints = root.value( "endpoints" ).toObject();
    endpoint.login_to = server_endpoints.value( "login_to" ).toString();
    endpoint.post_images = server_endpoints.value( "upload_image" ).toString();
    endpoint.post_raw_files = server_endpoints.value( "upload_file" ).toString();

    endpoint.add_new_repository = server_endpoints.value( "add_repository" ).toString();
    endpoint.add_course = server_endpoints.value( "add_course" ).toString();
    endpoint.get_repositories = server_endpoints.value( "get_repositories" ).toString();
    endpoint.get_courses_via_repo = server_endpoints.value( "get_courses" ).toString();

    endpoint.delete_repository = server_endpoints.value( "delete_repo" ).toString();
    endpoint.delete_course = server_endpoints.value( "delete_course" ).toString();
    endpoint.edit_course = server_endpoints.value( "edit_course" ).toString();
    endpoint.list_partakers = server_endpoints.value( "list_partakers" ).toString();
    endpoint.delete_score = server_endpoints.value( "delete_score" ).toString();
    StartMainApplication();
}

void PreloadWindow::LoadStartupFile()
{
    QFile repo_file( file_name );
    if( !repo_file.exists() ){
        endpoint.main_url = QInputDialog::getText( nullptr, "Address",
                                                   "You need an endpoint to connect to" );
        if( endpoint.main_url.isNull() ){
            QMessageBox::critical( nullptr, "Error", "No address was entered." );
            std::exit( -1 );
        }
        utilities::WriteDataToRepositoryFile( nullptr, &repo_file, endpoint.main_url );
    } else {
        if( !repo_file.open( QIODevice::ReadOnly ) ){
            QMessageBox::critical( nullptr, "Error", "Unable to read information" );
            std::exit( -1 );
        } else {
            QTextStream text_stream{ &repo_file };
            endpoint.main_url = text_stream.readLine().trimmed();
        }
    }
    repo_file.close();
}

void PreloadWindow::StartMainApplication()
{
    MainWindow *main_window = new MainWindow( endpoint );
    main_window->SetNetworkManager( network_manager );
    splash_screen->finish( main_window );
    QObject::connect( main_window, &MainWindow::destroyed, main_window, &MainWindow::deleteLater );
    main_window->show();
}
