#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QMdiArea>
#include <QNetworkCookie>
#include <QNetworkRequest>
#include <QJsonArray>
#include <QPoint>

#include "repository_lister.hpp"

// forward declarations
class QAction;
class QNetworkAccessManager;
class QFile;
class QNetworkReply;
class QTreeView;
using utilities::EndpointInformation;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow( EndpointInformation & ep, QWidget *parent = nullptr );
    void SetNetworkManager( QNetworkAccessManager * );
    ~MainWindow();

protected:
    void closeEvent( QCloseEvent *event ) override;
private:
    void InitializeActions();
    void InitializeMenus();
    void onCreateCourseActionTriggered();
    void onUpdateCourseActionTriggered();
    void onEditRepositoryActionTriggered();
    void onAddRepositoryTriggered();
    void onListRepoTriggered();
    void SignUserIn();
private:
    QMdiArea *main_ui_workspace;
    QAction  *create_course_action;
    QAction  *update_course_action;
    QAction  *edit_repository_action;
    QAction  *login_action;
    QAction  *add_repository_action;
    QAction  *list_repositories_action;
    QNetworkAccessManager *network_manager;
    QNetworkReply         *reply;
    bool                  is_logged_in;
    QList<QNetworkCookie> login_cookies;
    utilities::ListMap    repository_courses;
    EndpointInformation   &endpoint;
    QString const         file_name { "repo_address.rep" };
};

#endif // MAINWINDOW_HPP
