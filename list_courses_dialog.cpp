#include "list_courses_dialog.hpp"
#include "ui_list_courses_dialog.h"
#include <QMessageBox>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QAction>
#include <QMenu>
#include <QUrl>
#include <QUrlQuery>
#include <QNetworkCookieJar>

#include "add_question_window.hpp"
#include "score_sheet_window.hpp"

ListCoursesDialog::ListCoursesDialog( utilities::ListMap & repo_info, QWidget *parent) :
    QDialog(parent),
    ui( new Ui::ListCoursesDialog ),
    repository_courses( repo_info ),
    repo_lister( new RepositoryLister( repository_courses ) )
{
    ui->setupUi(this);
    setWindowTitle( "Repositories" );
    setWindowModality( Qt::ApplicationModal );
    setMinimumSize( QSize( 300, 300 ));
}

ListCoursesDialog::~ListCoursesDialog()
{
    delete ui;
}

void ListCoursesDialog::SetNetworkManager(QNetworkAccessManager * network )
{
    network_manager = network;
}

void ListCoursesDialog::SetCookieJar( QList<QNetworkCookie> const & cookies )
{
    this->cookies = cookies;
}

void ListCoursesDialog::SetEndpoints( utilities::EndpointInformation const & ep )
{
    endpoint = ep;
}

void ListCoursesDialog::Start()
{
    QUrl url { QUrl::fromUserInput( endpoint.get_repositories ) };
    network_manager->cookieJar()->setCookiesFromUrl( cookies, url );
    repo_lister->SetNetworkManager( network_manager );
    repo_lister->GetList( endpoint.get_repositories );
    QObject::connect( repo_lister, &RepositoryLister::ErrorOccurred, [=]( QString const & error ){
        QMessageBox::information( this, windowTitle(), error );
        this->accept();
    });
    QObject::connect( repo_lister, &RepositoryLister::ResultObtained,
                      [=]( QJsonObject const & server_response ){
        if( server_response.value( "status" ).toInt() != 1 ){
            QMessageBox::critical( this, "Error", server_response.value( "detail" ).toString() );
            this->accept();
            return;
        }
        this->repository_courses.clear();

        QJsonArray repos = server_response.value( "repositories" ).toArray();
        if( repos.size() == 0 ){
            QMessageBox::information( this, "List repository", "No repository has been added yet");
            this->accept();
            return;
        }
        QStandardItemModel *model( new QStandardItemModel( repos.size(), 2 ) );

        for( int i = 0; i != repos.size(); ++i ){
            QJsonObject object = repos.at( i ).toObject();
            QString const repo_name = object.value( "name").toString();
            repository_courses.insert( repo_name, {} );

            QStandardItem *name_item = new QStandardItem( repo_name );
            name_item->setEditable( false );
            QStandardItem *url_item = new QStandardItem( object.value( "url" ).toString() );
            QJsonArray courses = object.value( "courses" ).toArray();
            for( int column = 0; column != courses.size(); ++column ){
                QJsonArray const course_name_id = courses.at( column ).toArray();
                QString const course_name = course_name_id.at( 0 ).toString();
                long course_id = course_name_id.at( 1 ).toInt();
                auto subject_item = new QStandardItem( course_name );
                repository_courses[repo_name].append( { course_name, course_id } );
                subject_item->setEditable( false );
                name_item->appendRow( subject_item );
            }
            model->setItem( i, 0, name_item );
            model->setItem( i, 1, url_item );
        }
        model->setHorizontalHeaderLabels( { "Repositories", "Links" } );
        ui->repo_treeview->setModel( model );
        ui->repo_treeview->setSelectionMode( QAbstractItemView::SingleSelection );
        ui->repo_treeview->setVisible( true );
        ui->repo_treeview->setContextMenuPolicy( Qt::CustomContextMenu );
        QObject::connect( ui->repo_treeview, &QTreeView::customContextMenuRequested, [=]( QPoint const &point ){
            OnListRepositoryCustomMenuTriggered( point );
        });
    });
}

void ListCoursesDialog::OnListRepositoryCustomMenuTriggered( QPoint const &point )
{
    QModelIndex model_index = ui->repo_treeview->indexAt( point );
    if( !model_index.isValid() ) return;
    QMenu menu{ this };
    menu.setTitle( "Action");

    if( !model_index.parent().isValid() ){ // this is a repository
        QAction * const delete_repository_action = new QAction( "Delete repository" );
        QObject::connect( delete_repository_action, &QAction::triggered, [=]{
            auto response = QMessageBox::information( this, "Delete repository",
                                   "Deleting this repository means you lose every course "
                                   "existing under it, are you sure you want to continue?",
                                   QMessageBox::Yes | QMessageBox::No );
            if( response == QMessageBox::Yes ){
                QModelIndex model = ui->repo_treeview->model()->index( model_index.row(), 0 );
                QString repository_name = ui->repo_treeview->model()->data( model ).toString();
                this->DeleteRepository( repository_name );
                ui->repo_treeview->model()->removeRow( model_index.row() );
            }
        });
        menu.addAction( delete_repository_action );
    } else {
        QAction *delete_course_action = new QAction( "Delete course" );
        QAction *edit_course_action = new QAction( "Edit course" );
        QAction *list_partaker_action = new QAction( "List partakers" );

        auto course_prelim_action = [=]( QString const & msgbox_title )-> utilities::StringPair {
            auto response = QMessageBox::information( this, msgbox_title,
                                      "Are you sure?", QMessageBox::Yes | QMessageBox::No );
            if( response == QMessageBox::Yes ){
                QModelIndex parent_model = ui->repo_treeview->model()->
                        index( model_index.parent().row(), 0 );
                QString const course_name = ui->repo_treeview->model()->
                        data( model_index ).toString();
                QString const repository_name = ui->repo_treeview->model()->
                        data( parent_model ).toString();
                return { course_name, repository_name };
            }
            return {};
        };
        QObject::connect( list_partaker_action, &QAction::triggered, [=]{
            QModelIndex parent_model = ui->repo_treeview->model()->
                    index( model_index.parent().row(), 0 );
            QString const repository_name = ui->repo_treeview->model()->
                    data( parent_model ).toString();
            long const course_id = repository_courses.value( repository_name )
                    .at( parent_model.row() ).second;
            this->ListCoursePartakers( course_id, repository_name );
        });

        QObject::connect( delete_course_action, &QAction::triggered, [=]{
            utilities::StringPair course_repo_pair = course_prelim_action( "Delete course" );
            if( course_repo_pair.first.isEmpty() ) return;
            this->DeleteCourse( course_repo_pair.first, course_repo_pair.second );
            ui->repo_treeview->model()->removeRow( model_index.row(),
                                                   model_index.parent() );
        } );

        QObject::connect( edit_course_action, &QAction::triggered, [=]{
            utilities::StringPair course_repository_pair = course_prelim_action( "Edit course" );
            if( course_repository_pair.first.isEmpty() ) return;
            this->EditCourse( course_repository_pair.first, course_repository_pair.second );
        });
        menu.addAction( list_partaker_action );
        menu.addAction( edit_course_action );
        menu.addAction( delete_course_action );
    }
    menu.exec( ui->repo_treeview->mapToGlobal( point ) );
}

void ListCoursesDialog::DeleteCourse( QString const &course_name, QString const &repository_name )
{
    QJsonObject const json_object{ { "course_name", course_name },
                                   { "repository_name", repository_name } };
    QByteArray delete_data = QJsonDocument{ json_object }.toJson();
    auto const network_request = utilities::GetPostNetworkRequestFrom( endpoint.delete_course,
                                                            delete_data.length());
    network_manager->cookieJar()->setCookiesFromUrl( cookies, QUrl::fromUserInput( endpoint.delete_course ));
    auto network_reply = network_manager->post( network_request, delete_data );
    QObject::connect( network_reply, &QNetworkReply::finished, [=]{
        auto const response = utilities::OnNetworkResponseReceived( network_reply );
        if( response.isEmpty() ) return;
        QMessageBox::information( this, windowTitle(), response.value( "detail" ).toString() );
    });
    QMessageBox::information( this, windowTitle(), "Please wait while this deletion request is sent to the server" );
}

void ListCoursesDialog::DeleteRepository( QString const &repository_name )
{
    QJsonObject const json_object { {"repository_name", repository_name } };
    QByteArray const data = QJsonDocument( json_object ).toJson();
    QNetworkRequest const request{ utilities::GetPostNetworkRequestFrom( endpoint.delete_repository,
                                                              data.length() ) };
    network_manager->cookieJar()
            ->setCookiesFromUrl( cookies, QUrl::fromUserInput( endpoint.delete_repository ) );
    QNetworkReply * const reply = network_manager->post( request, data );
    QObject::connect( reply, &QNetworkReply::finished, [=]{
        QJsonObject const response = utilities::OnNetworkResponseReceived( reply );
        if( response.isEmpty() ){
            return;
        }
        QMessageBox::information( this, windowTitle(), response.value( "detail" ).toString() );
    });
    QMessageBox::information( this, windowTitle(), "Please wait while we send request to server" );
}

void ListCoursesDialog::ListCoursePartakers( long const course_id, QString const &repository_name )
{
    ScoreSheetWindow *score_window = new ScoreSheetWindow( this );
    network_manager->cookieJar()
            ->setCookiesFromUrl( cookies, QUrl::fromUserInput( endpoint.list_partakers ) );
    network_manager->cookieJar()->setCookiesFromUrl( cookies, QUrl::fromUserInput( endpoint.delete_score ));
    score_window->SetNetworkManager( network_manager );
    score_window->SetWebAddress( endpoint.list_partakers, { repository_name, course_id } );
    score_window->SetDeleteScoreAddress( endpoint.delete_score );
    score_window->StartFetchingData();
    score_window->show();
}

void ListCoursesDialog::EditCourse( QString const &course_name, QString const &repository_name )
{
    AddQuestionWindow *question_window = new AddQuestionWindow( repository_courses, this );
    question_window->SetNetwork( network_manager );
    question_window->SetAddress( endpoint.edit_course );
    question_window->SetImageUploadAddress( endpoint.post_images );
    question_window->SetRawfileUploadAddress( endpoint.post_raw_files );
    question_window->SetLoginCookie( cookies );
    question_window->DisplayMessage( "Please wait while we get the necessary information" );
    question_window->setWindowModality( Qt::ApplicationModal );
    question_window->setEnabled( false );

    auto url = QUrl::fromUserInput( endpoint.edit_course );
    QUrlQuery extra_query{};
    extra_query.addQueryItem( "repository_name", repository_name );
    extra_query.addQueryItem( "course_name", course_name );
    url.setQuery( extra_query );
    auto request = utilities::GetRequest( url.toString() );
    network_manager->cookieJar()
            ->setCookiesFromUrl( cookies, QUrl::fromUserInput( endpoint.edit_course ) );
    auto reply = network_manager->get( request );
    QObject::connect( reply, &QNetworkReply::finished, [=]{
        QJsonObject const response = utilities::OnNetworkResponseReceived( reply );
        if( response.isEmpty() ){
            question_window->close();
            return;
        }
        question_window->AddFromObject( response );
        question_window->setEnabled( true );
    });
    question_window->show();
}
