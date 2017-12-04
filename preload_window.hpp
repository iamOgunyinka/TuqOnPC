#ifndef PRELOAD_WINDOW_HPP
#define PRELOAD_WINDOW_HPP

#include <QObject>
#include "resources.hpp"

class QSplashScreen;
class QNetworkAccessManager;
class QApplication;

class PreloadWindow : public QObject
{
    Q_OBJECT
public:
    explicit PreloadWindow( QApplication *app, QObject *parent = 0 );
    void ShowSplashScreen();
private:
    void LoadStartupFile();
    void OnGetServerEndpoints();
    void StartMainApplication();
private:
    QApplication *main_application;
    QSplashScreen *splash_screen;
    QNetworkAccessManager *network_manager;
    utilities::EndpointInformation   endpoint;
    QString const file_name { "repo_address.rep" };
};

#endif // PRELOAD_WINDOW_HPP
