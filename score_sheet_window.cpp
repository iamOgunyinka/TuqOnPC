#include "score_sheet_window.hpp"
#include "ui_score_sheet_window.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QUrl>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardItemModel>
#include <QStandardItem>

#include "resources.hpp"

ScoreSheetWindow::ScoreSheetWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ScoreSheetWindow)
{
    ui->setupUi( this );
    setWindowTitle( "Score listing" );
}

void ScoreSheetWindow::SetNetworkManager( QNetworkAccessManager *const network )
{
    network_manager = network;
}

void ScoreSheetWindow::SetDeleteScoreAddress( QString const &address )
{
    delete_score_address = QUrl::fromUserInput( address );
}

void ScoreSheetWindow::SetWebAddress(QString const &address, QPair<QString, long> const &query_info )
{
    score_listing_address = QUrl::fromUserInput( address);
    QUrlQuery extra_query{};
    extra_query.addQueryItem( "repository", query_info.first );
    extra_query.addQueryItem( "course_id", QString::number( query_info.second ) );
    score_listing_address.setQuery( extra_query );
}

ScoreSheetWindow::~ScoreSheetWindow()
{
    delete ui;
}

void ScoreSheetWindow::StartFetchingData()
{
    QNetworkRequest const request = utilities::GetRequest( score_listing_address.toString() );
    auto reply = network_manager->get( request );
    QObject::connect( reply, &QNetworkReply::finished, [=]{
        auto const response = utilities::OnNetworkResponseReceived( reply );
        if( response.isEmpty() || response.value( "status" ).toInt() == 0 ){
            if( !response.isEmpty() ){
                QMessageBox::critical( this, this->windowTitle(),
                                       response.value( "detail" ).toString() );
            }
            this->close();
            return;
        }
        qDebug() << response;
        DisplayData( response.value( "detail" ).toArray() );
    });
    QMessageBox::information( this, windowTitle(), "Please wait a few seconds" );
}

void ScoreSheetWindow::DisplayData( QJsonArray const &data )
{
    if( data.size() == 0 ){
        QMessageBox::information( this, windowTitle(), "This examination has not been taken yet." );
        this->close();
        return;
    }
    int const rows = data.size();
    QStandardItemModel *model = new QStandardItemModel;
    QJsonObject data_item;
    for( int index = 0; index != rows; ++index )
    {
        data_item = data.at( index ).toObject();
        auto full_name_item = new QStandardItem( data_item.value( "fullname" ).toString() );
        auto username_item = new QStandardItem( data_item.value( "username" ).toString() );
        auto score_item = new QStandardItem( QString::number( data_item.value( "score" ).toInt()) );
        auto date_time_item = new QStandardItem( data_item.value( "date_time" ).toString() );
        auto total_score_item = new QStandardItem( QString::number(
                                                       data_item.value( "total" ).toInt() ));
        full_name_item->setEditable( false );
        username_item->setEditable( false );
        score_item->setEditable( false );
        date_time_item->setEditable( false );
        total_score_item->setEditable( false );

        model->appendRow( { full_name_item, username_item, score_item,
                            total_score_item, date_time_item } );
        score_reference_ids.append( data_item.value( "reference_id" ).toString() );
    }

    model->setHorizontalHeaderLabels( { "Full name", "Username", "Score", "Total", "Date and Time" });
    model->setParent( ui->tableView );
    ui->tableView->setModel( model );
    ui->tableView->setContextMenuPolicy( Qt::CustomContextMenu );
    QObject::connect( ui->tableView, &QTableView::customContextMenuRequested, this,
                      &ScoreSheetWindow::OnCustomContextMenuReceived );
}

void ScoreSheetWindow::OnCustomContextMenuReceived( QPoint const &point )
{
    QModelIndex item_index = ui->tableView->indexAt( point );
    if ( !item_index.isValid() ) return;
    QMenu context_menu;
    context_menu.setTitle( "Actions" );
    QAction *delete_action = new QAction( "Delete" );
    QObject::connect( delete_action, &QAction::triggered, [=]{
        auto response = QMessageBox::information( this, windowTitle(),
                                                  "Are you sure?",
                                                  QMessageBox::No | QMessageBox::Yes );
        if( response == QMessageBox::No ) return;
        DeleteScoreWithReference( item_index.row() );
    });
    context_menu.addAction( delete_action );
    context_menu.exec( ui->tableView->mapToGlobal( point ) );
}

void ScoreSheetWindow::DeleteScoreWithReference( int const row )
{
    QUrlQuery extra_query{};
    extra_query.addQueryItem( "ref_id", score_reference_ids.at( row ) );
    delete_score_address.setQuery( extra_query );
    auto request = utilities::GetRequest( delete_score_address.toString() );
    auto network_reply = network_manager->get( request );
    QObject::connect( network_reply, &QNetworkReply::finished, [=]{
        auto result = utilities::OnNetworkResponseReceived( network_reply );
        if( result.isEmpty() ) return;
        ui->tableView->model()->removeRow( row );
        this->score_reference_ids.removeAt( row );
    });
}
