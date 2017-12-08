#include <QAction>
#include <QCloseEvent>
#include <QFile>
#include <QGridLayout>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QNetworkCookieJar>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QProgressBar>
#include <QPushButton>
#include <QStandardItemModel>
#include <QStatusBar>
#include <QTreeView>
#include <QUrlQuery>
#include <QVBoxLayout>

#include "mainwindow.hpp"
#include "add_question_window.hpp"
#include "list_courses_dialog.hpp"
#include "repository_lister.hpp"
#include "sign_in_dialog.hpp"

enum ServerResponse {
    Error = 0,
    Success
};


MainWindow::MainWindow( utilities::EndpointInformation &ep, QWidget *parent ) : QMainWindow{ parent },
    main_ui_workspace{ new QMdiArea }, network_manager{ nullptr },
    endpoint( ep ), reply{ nullptr }, is_logged_in{ false }
{
    setWindowIcon( QIcon{ ":/image/images/logo.png" } );
    setWindowTitle( "TuqOnThePC" );
    setCentralWidget( main_ui_workspace );
    setWindowState( Qt::WindowMaximized );

    InitializeActions();
    InitializeMenus();
    statusBar()->showMessage( "Done" );
}


void MainWindow::SetNetworkManager( QNetworkAccessManager * network )
{
    network_manager = network;
}

void MainWindow::SignUserIn()
{
    if( is_logged_in ) return;
    SignInDialog *login_dialog = new SignInDialog( network_manager, this );
    login_dialog->SetLoginAddress( endpoint.login_to );
    if( login_dialog->exec() == QDialog::Accepted ){
        auto reply = login_dialog->NetworkReply();
        login_cookies = qvariant_cast<
                QList<QNetworkCookie>>( reply->header( QNetworkRequest::SetCookieHeader ));
        login_action->setEnabled( false );
        is_logged_in = true;
    }
}

void MainWindow::InitializeActions()
{
    create_course_action = new QAction( "Create new course" );
    create_course_action->setShortcut( tr("Ctrl+N") );
    QObject::connect( create_course_action, &QAction::triggered, this,
                      &MainWindow::onCreateCourseActionTriggered );

    update_course_action = new QAction( "Update course" );
    update_course_action->setShortcut( tr("Ctrl+E") );
    QObject::connect( update_course_action, &QAction::triggered, this,
                      &MainWindow::onUpdateCourseActionTriggered );

    edit_repository_action = new QAction( "Change server address" );
    QObject::connect( edit_repository_action, &QAction::triggered, this,
                      &MainWindow::onEditRepositoryActionTriggered );

    login_action = new QAction( "Login" );
    login_action->setShortcut( tr( "Ctrl+L" ) );
    QObject::connect( login_action, &QAction::triggered, this, &MainWindow::SignUserIn );

    add_repository_action = new QAction( "Create repository" );
    add_repository_action->setShortcut( tr( "Ctrl+R" ) );
    QObject::connect( add_repository_action,&QAction::triggered, this,
                      &MainWindow::onAddRepositoryTriggered );

    list_repositories_action = new QAction( "List my courses" );
    list_repositories_action->setShortcut( tr( "Ctrl+A" ) );
    QObject::connect( list_repositories_action, &QAction::triggered, this,
                      &MainWindow::onListRepoTriggered );
}

void MainWindow::InitializeMenus()
{
    QMenu *main_menu = menuBar()->addMenu( "Menu" );
    main_menu->addAction( login_action );

    QMenu *repository_menu = new QMenu( "Repository" );
    repository_menu->addAction( add_repository_action );
    repository_menu->addAction( list_repositories_action );
    repository_menu->addAction( edit_repository_action );
    main_menu->addMenu( repository_menu );

    QAction *exit_action = new QAction( "Exit" );
    exit_action->setShortcut( tr( "Ctrl+Q" ) );
    QObject::connect( exit_action, &QAction::triggered, [=]{ this->close(); });
    main_menu->addAction( exit_action );

    QMenu *question_menu = menuBar()->addMenu( "Questions" );
    question_menu->addAction( create_course_action );
    question_menu->addAction( update_course_action );
}

void MainWindow::closeEvent( QCloseEvent *event )
{
    auto response = QMessageBox::information( this, windowTitle(),
                                             "Are you sure you want to leave?",
                                             QMessageBox::Yes | QMessageBox::No );
    if( response == QMessageBox::Yes ){
        event->accept();
    } else {
        event->ignore();
    }
}

void MainWindow::onAddRepositoryTriggered()
{
    QString repository_name = QInputDialog::getText( this, "Repository", "Enter a repository name" );
    if( repository_name.isNull() ) return;

    QByteArray data = QJsonDocument( QJsonObject{ { "repository_name", repository_name } }).toJson();
    auto request = utilities::GetPostNetworkRequestFrom( endpoint.add_new_repository, data.size() );
    network_manager->cookieJar()->setCookiesFromUrl( login_cookies,
                                                     QUrl::fromUserInput( endpoint.add_new_repository));
    QDialog *wait_dialog = new QDialog(this);
    wait_dialog->setWindowModality( Qt::ApplicationModal );
    QVBoxLayout *layout = new QVBoxLayout;
    auto label = new QLabel( "Please wait while we send your request to the server" );
    layout->addWidget( label );
    wait_dialog->setLayout( layout );

    QNetworkReply *reply = network_manager->post( request, data );
    QObject::connect( reply, &QNetworkReply::finished, [=]{
        auto response = utilities::OnNetworkResponseReceived( reply );
        if( response.isEmpty() ) {
            wait_dialog->accept();
            return;
        }
        label->setText( response.value( "detail" ).toString() );
    });
    wait_dialog->exec();
}

void MainWindow::onListRepoTriggered()
{
    ListCoursesDialog *list_dialog = new ListCoursesDialog( repository_courses, this );
    list_dialog->SetEndpoints( endpoint );
    list_dialog->SetNetworkManager( network_manager );
    list_dialog->SetCookieJar( login_cookies );
    list_dialog->Start();
    list_dialog->exec();
}

void MainWindow::onUpdateCourseActionTriggered()
{
    onListRepoTriggered();
}

void MainWindow::onCreateCourseActionTriggered()
{
    SignUserIn();
    if( !is_logged_in ) return;

    AddQuestionWindow *question_window = new AddQuestionWindow( repository_courses, this );
    question_window->SetNetwork( network_manager );
    question_window->SetAddress( endpoint.add_course );
    question_window->FetchRepositories( endpoint.get_repositories );
    question_window->SetImageUploadAddress( endpoint.post_images );
    question_window->SetRawfileUploadAddress( endpoint.post_raw_files );
    question_window->SetLoginCookie( login_cookies );
    question_window->setWindowModality( Qt::ApplicationModal );
    question_window->show();
}

void MainWindow::onEditRepositoryActionTriggered()
{
    QString const input { QInputDialog::getText( this, windowTitle(), "Enter new address", QLineEdit::Normal,
                                                 endpoint.main_url ) };
    if( input.isNull() ) return;
    endpoint.main_url = input;
    QFile file( file_name );
    login_cookies.clear();
    is_logged_in = false;
    login_action->setEnabled( true );
    utilities::WriteDataToRepositoryFile( this, &file, endpoint.main_url );
    MBOX_INFO( "You have been logged out, please sign in again" );
}

MainWindow::~MainWindow()
{
}

#undef MBOX_CRIT
#undef MBOX_INFO
