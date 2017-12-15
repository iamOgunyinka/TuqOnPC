#include <QAction>
#include <QDialog>
#include <QJsonDocument>
#include <QJsonValueRef>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkCookieJar>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QTreeView>
#include <QTabWidget>
#include <QFileDialog>

#include "add_question_window.hpp"
#include "question_dialog.hpp"
#include "upload_status_dialog.hpp"
#include "ui_add_question_window.h"
#include "repository_lister.hpp"


AddQuestionWindow::AddQuestionWindow( ListMap &repositories, QWidget *parent ) :
    QMainWindow( parent ),
    repositories( repositories ),
    ui{ new Ui::AddQuestionWindow },
    approach{ AnswersApproach::Traditional },
    model{ new QStandardItemModel( 0, 2 ) }
{
    ui->setupUi( this );
    this->setWindowModality( Qt::ApplicationModal );

    ui->tabWidget->setCurrentIndex( 1 );

    QObject::connect( ui->tabWidget, &QTabWidget::currentChanged, [=]( int index )->void{
        ui->add_new_question_action->setEnabled( index == 0 );
    });

    QObject::connect( ui->add_new_question_action, &QAction::triggered, this,
                      &AddQuestionWindow::OnAddQuestionActionTriggered );
    QObject::connect( ui->add_from_file_action, &QAction::triggered, this,
                      &AddQuestionWindow::OnAddFromFileActionTriggered );

    QObject::connect( ui->instructions_text_edit, &QTextEdit::textChanged, [&]{
        question_root["instructions"] = ui->instructions_text_edit->toPlainText();
    });
    QObject::connect( ui->submit_method_tradional_radio, &QRadioButton::clicked, [&]
                      ( bool checked ){
        if( checked ) approach = AnswersApproach::Traditional;
    });
    QObject::connect( ui->submit_method_modern_radio, &QRadioButton::clicked, [&]
                      ( bool checked ){
        if( checked ) approach = AnswersApproach::Modern;
    });
    QObject::connect( ui->submit_method_hybrid_radio, &QRadioButton::clicked, [&]
                      ( bool checked ){
        if( checked ) approach = AnswersApproach::Hybrid;
    });

    ui->questions_tree_view->setDragDropMode( QAbstractItemView::NoDragDrop );
    ui->questions_tree_view->setContextMenuPolicy( Qt::CustomContextMenu );

    ui->local_copy_chkbox->setChecked( true );
    ui->using_icon_check->setChecked( false );
    ui->choose_icon_button->setVisible( false );
    ui->icon_preview_text->setVisible( false );
    QObject::connect( ui->choose_icon_button, &QPushButton::clicked, this,
                      &AddQuestionWindow::OnUseCustomIconButtonTriggered );

    QObject::connect( ui->using_icon_check, &QCheckBox::toggled, [=]( bool checked ){
        ui->choose_icon_button->setVisible( checked );
        ui->icon_preview_text->setVisible( checked );
    });

    QObject::connect( ui->questions_tree_view, &QTreeView::customContextMenuRequested,
                      this, &AddQuestionWindow::OnCustomMenuRequested );
    ui->expires_on_dt_edit->setEnabled( false );
    QObject::connect( this->model, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)),
                      this->ui->questions_tree_view,
                      SLOT(dataChanged(QModelIndex,QModelIndex,QVector<int>)) );

    QObject::connect( ui->add_new_button, &QPushButton::clicked, this,
                      &AddQuestionWindow::OnAddQuestionActionTriggered );
    QObject::connect( this, &AddQuestionWindow::onRepositoriesObtained, [=]{
        ui->repository_combo->addItems( this->repositories.keys() );
    });
    QObject::connect( ui->delete_question_button, &QPushButton::clicked, [=]{

    });
    QObject::connect( ui->edit_question_button, &QPushButton::clicked, [=]{
        OnEditQuestionItemClicked();
    });

    QObject::connect( ui->expires_on_checkbox, &QCheckBox::toggled, [=]( bool checked ){
        ui->expires_on_dt_edit->setEnabled( checked );
    });
    ui->submit_method_tradional_radio->setChecked( true );
    ui->randomize_false_radio->setChecked( true );

    QObject::connect( ui->submit_button, &QPushButton::clicked, this,
                      &AddQuestionWindow::OnSubmitButtonClicked );

    ui->expires_on_dt_edit->setDate( QDate::currentDate() );
    ui->held_date_dt_edit->setDate( QDate::currentDate() );
}

AddQuestionWindow::~AddQuestionWindow()
{
    delete ui;
}

void AddQuestionWindow::OnUseCustomIconButtonTriggered()
{
    icon_filename = QFileDialog::getOpenFileName( this, "Choose custom icon", "", "PNG Files(*.png)" );
    QPixmap icon_preview{ icon_filename };
    if( icon_filename.isNull() || icon_preview.isNull() ){
        ui->using_icon_check->setChecked( false );
        return;
    }
    ui->icon_preview_text->setPixmap( icon_preview.scaled( 100, 100 ) );
}

void AddQuestionWindow::OnCustomMenuRequested( const QPoint &point )
{
    QModelIndex model_index = ui->questions_tree_view->indexAt( point );
    if( !model_index.isValid() ) return;
    while( model_index.parent().isValid() ){
        model_index = model_index.parent();
    }

    QMenu menu{ this };
    menu.setTitle( "Action" );
    if( !model_index.parent().isValid() ){
        QAction *edit_action = new QAction( "Edit" );
        QAction *delete_action = new QAction( "Delete" );
        QObject::connect( edit_action, &QAction::triggered, this,
                          &AddQuestionWindow::OnEditQuestionItemClicked );
        QObject::connect( delete_action, &QAction::triggered, this,
                          &AddQuestionWindow::OnDeleteQuestionItemClicked );
        menu.addAction( edit_action );
        menu.addAction( delete_action );
    }

    menu.exec( ui->questions_tree_view->mapToGlobal( point ) );
}

void AddQuestionWindow::OnDeleteQuestionItemClicked()
{
    auto index = ui->questions_tree_view->currentIndex();
    if( !index.isValid() ) return;

    while( index.parent().isValid() ){
        index = index.parent();
    }

    if( !index.parent().isValid() ){ // one of the parent items
        int response = QMessageBox::critical( this, "Delete row",
                                              "Are you sure you want to delete this item?",
                                              QMessageBox::Yes | QMessageBox::No );
        if( response == QMessageBox::No ) return;
        int row = index.row();
        this->model->removeRow( row );
        this->question_items.removeAt( row );
        this->answer_items.removeAt( row );
    }
}

void AddQuestionWindow::OnEditQuestionItemClicked()
{
    QModelIndex index = ui->questions_tree_view->currentIndex();
    if( !index.isValid() ) return;

    while( index.parent().isValid() ){
        index = index.parent();
    }

    int row = index.row();
    QJsonObject current_item = this->question_items[row].toObject();
    if( current_item.contains( "url" ) ){
        MBOX_CRIT( "Unfortunately, this cannot be edited" );
        return;
    }
    QuestionDialog *question_dialog = new QuestionDialog( this );
    question_dialog->SetQuestionText( current_item.value( "question" ).toString() );
    if( current_item.contains( "photo" ) ){
        question_dialog->SetUsingImage( current_item.value( "photo" ).toString() );
    } else if( current_item.contains( "raw" ) ){
        question_dialog->SetUsingRawfile( current_item.value( "raw" ).toString() );
    }
    QJsonArray option_list = current_item.value( "options" ).toObject().value( "items" )
            .toArray();
    QStringList options{};
    for( int i = 0; i != option_list.size(); ++i ) {
        options.append( option_list[i].toString() );
    }
    question_dialog->SetOptions( options );
    question_dialog->SetAnswerIndex( answer_items[row].toInt() );

    if( question_dialog->exec() ){
        auto const question_object = question_dialog->GetQuestionObject();
        question_items[row] = question_object;
        answer_items[row] = question_dialog->GetSolutionOption();
        UpdateModel( question_object, row );
        ui->questions_tree_view->setModel( model );
    }
}

void AddQuestionWindow::onNetworkReply()
{
    QJsonObject server_response = utilities::OnNetworkResponseReceived( network_reply );
    if( server_response.isEmpty() ){
        this->close();
        return;
    }
    if( !server_response.value( "status" ).toInt() ){
        MBOX_CRIT( server_response.value( "detail" ).toString() );
        this->close();
        return;
    }
    repositories.clear();
    QString repo_name {};
    QJsonArray repos = server_response.value( "repositories" ).toArray();
    for( int i = 0; i != repos.size(); ++i ){
        QJsonObject object = repos.at( i ).toObject();
        repo_name = object.value( "name" ).toString();
        repositories.insert( repo_name, {} );
        QJsonArray courses = object.value( "courses" ).toArray();
        QJsonArray course_info;
        for( int x = 0; x != courses.size(); ++x ){
            course_info = courses.at( x ).toArray();
            repositories[repo_name].append( { course_info.at( 0 ).toString(),
                                              course_info.at( 1 ).toInt() } );
        }
    }
    emit onRepositoriesObtained();
}

void AddQuestionWindow::FetchRepositories( const QString &address )
{
    if( !repositories.isEmpty() ){
        emit onRepositoriesObtained();
        return;
    }
    RepositoryLister *repo = new RepositoryLister( repositories, this );
    repo->SetNetworkManager( network_manager );

    QObject::connect( repo, &RepositoryLister::ErrorOccurred, [=]( QString const & error_message ){
        MBOX_CRIT( error_message );
        this->close();
    });
    QObject::connect( repo, &RepositoryLister::ResultObtained, repo,
                      &RepositoryLister::OnValidDocumentObtained );
    QObject::connect( repo, &RepositoryLister::Finished, [=]{
        emit this->onRepositoriesObtained();
    });
    repo->GetList( address );
}

void AddQuestionWindow::SetNetwork( QNetworkAccessManager *network )
{
    network_manager = network;
}

void AddQuestionWindow::SetLoginCookie( QList<QNetworkCookie> &cookie_jar )
{
    network_manager->cookieJar()->setCookiesFromUrl( cookie_jar, raw_file_upload_address );
    network_manager->cookieJar()->setCookiesFromUrl( cookie_jar, image_upload_address );
}

void AddQuestionWindow::DisplayMessage( QString const &message )
{
    QMessageBox::information( this, this->windowTitle(), message );
}

void AddQuestionWindow::SetAddress( QString const &add )
{
    send_to_address = add;
}

void AddQuestionWindow::SetRawfileUploadAddress( QString const &address )
{
    raw_file_upload_address = address;
}

void AddQuestionWindow::SetImageUploadAddress( QString const &address )
{
    image_upload_address = address;
}

void AddQuestionWindow::OnAddQuestionActionTriggered()
{
    ui->tabWidget->setCurrentIndex( 0 );
    QuestionDialog *question_dialog = new QuestionDialog( this );
    question_dialog->setWindowModality( Qt::ApplicationModal );
    if( question_dialog->exec() == QDialog::Accepted ){
        QJsonObject const result = question_dialog->GetQuestionObject();
        question_items.append( result );
        answer_items.append( question_dialog->GetSolutionOption() );
        int const question_size = question_items.size();
        question_root["total"] = question_size;
        UpdateModel( result, question_size );

        ui->questions_tree_view->setModel( model );
    }
}

void AddQuestionWindow::UpdateModel( QJsonObject const & object, int const index )
{
    bool const editing_item = index < this->question_items.size();
    int const row_count = editing_item ? index + 1 : index;

    QStandardItem * const item = new QStandardItem( QString( "Question %1" ).arg( row_count ) );
    QStandardItem *const question_item = new QStandardItem( "Question" );
    question_item->appendRow( new QStandardItem( object.value( "question" ).toString() ) );
    item->appendRow( question_item );

    QStandardItem * const option_items = new QStandardItem( "Options" );
    QJsonArray const question_options = object.value( "options" ).toObject().value( "items" ).toArray();
    for( int column = 0; column != question_options.size(); ++column ){
        option_items->appendRow( new QStandardItem( question_options.at( column ).toString() ) );
    }
    item->appendRow( option_items );

    question_item->setEditable( false );
    option_items->setEditable( false );
    item->setEditable( false );

    QStandardItem *const correct_option = new QStandardItem( "Correct option" );
    int const answer_index = editing_item ? index : index - 1;
    QString const option_str = tr( "Option %1" ).arg( answer_items[answer_index].toInt() + 1 );
    correct_option->appendRow( new QStandardItem( option_str ) );
    correct_option->setEditable( false );
    item->appendRow( correct_option );

    QStandardItem * const url_item = new QStandardItem( "No file needed" );

    if( object.contains( "raw" ) ){
        url_item->setText( "Filename: " + object.value( "raw" ).toString() );
    } else if( object.contains( "photo" ) ){
        url_item->setText( "Filename: " + object.value( "photo" ).toString() );
    } else if( object.contains( "url" ) ){
        url_item->setText( "URL: " + object.value( "url" ).toString() );
    }
    url_item->setEditable( false );

    if( editing_item ){ // when an edited question is used
        model->removeRow( index );
        model->insertRow( index, { item, url_item } );
    } else {
        model->appendRow( { item, url_item } );
    }
}

QJsonObject AddQuestionWindow::ReadFile()
{
    QString const filename = QFileDialog::getOpenFileName( this, "Question file", "",
                                                     "JSON Files( *.json )" );
    if( filename.isNull() ) return {};
    QFile file{ filename };
    if( !file.open( QIODevice::ReadOnly )){
        MBOX_CRIT( file.errorString() );
        return {};
    }
    QJsonObject root_element = QJsonDocument::fromJson( file.readAll() ).object();
    file.close();
    return root_element;
}

void AddQuestionWindow::OnAddFromFileActionTriggered()
{
    QJsonObject const data{ ReadFile() };
    if( data.isEmpty() ) return;
    AddFromObject( data );
}

void AddQuestionWindow::AddFromObject( QJsonObject const & object )
{
    if( object.isEmpty() ){
        MBOX_CRIT( "Error parsing the question document" );
        return;
    }
    QJsonObject const root_element = object.value( "detail" ).toObject();
    if( root_element.contains( "course_code" ) ){
        ui->course_code_line_edit->setText( root_element.value( "course_code" ).toString() );
        ui->course_code_line_edit->setEnabled( false );
    }
    QString const icon_location = root_element.value( "icon" ).toString();
    if( icon_location != "default" ){
        ui->using_icon_check->setChecked( true );
        ui->using_icon_check->setEnabled( false );
        ui->choose_icon_button->setVisible( false );
        ui->icon_preview_text->setText( "Custom image used." );
        icon_filename = icon_location;
    }
    ui->course_name_line_edit->setText( root_element.value( "name" ).toString() );
    ui->instructor_name_line_edit->setText( root_element.value( "administrator_name" ).toString() );
    QJsonObject const question_object = root_element.value( "question" ).toObject();

    ui->instructions_text_edit->setText( question_object.value( "instructions" ).toString() );
    ui->repository_combo->addItem( root_element.value( "repository_name" ).toString() );
    if( root_element.value( "randomize" ).toBool() ){
        ui->randomize_true_radio->setChecked( true );
    }
    switch( root_element.value( "approach" ).toInt() ){
    case utilities::AnswersApproach::Modern:
        ui->submit_method_modern_radio->setChecked( true );
        break;
    case utilities::AnswersApproach::Hybrid:
        ui->submit_method_hybrid_radio->setChecked( true );
        break;
    case utilities::AnswersApproach::Traditional: default:
        ui->submit_method_tradional_radio->setChecked( true );
        break;
    }
    ui->use_time_line_edit->setText( QString::number( root_element.value( "duration" ).toInt() ));
    ui->use_time_line_edit->setEnabled( true );

    QString const hearing_date = root_element.value( "date_to_be_held" ).toString();
    QString const expiry_date = root_element.value( "expires_on" ).toString();
    ui->held_date_dt_edit->setDate( QDate::fromString( hearing_date, "yyyy-MM-dd") );
    if( expiry_date != "None" ){
        ui->expires_on_checkbox->setChecked( true );
        ui->expires_on_dt_edit->setEnabled( true );
        ui->expires_on_dt_edit->setDate( QDate::fromString( expiry_date, "yyyy-MM-dd" ));
    }
    QJsonArray department_list = root_element.value( "departments" ).toArray();
    int const dept_size = department_list.size();
    for( int i = 0; i != dept_size; ++i ){
        ui->departments_text_edit->append( department_list.at( i ).toString() +
                                           ( i != dept_size ? "\n" : "" ) );
    }

    QJsonArray question_list = question_object.value( "items" ).toArray(),
            answer_list = root_element.value( "answers" ).toArray();

    for( int index = 0; index != question_list.size(); ++index ){
        QJsonObject item = question_list.at( index ).toObject();
        QJsonArray item_options = item.value( "options" ).toObject().value( "items" ).toArray();
        if( !item.contains( "question" ) || ( item_options.isEmpty() || item_options.size() < 3 )){
            MBOX_CRIT( "The question file contain some invalid data" );
            return;
        }

        question_items.append( item );
        answer_items.append( answer_list.at( index ).toInt() );
        UpdateModel( item, question_items.size() );
    }
    question_root["total"] = question_items.size();
    ui->questions_tree_view->setModel( model );
}

void AddQuestionWindow::OnSubmitButtonClicked()
{
    ui->tabWidget->setCurrentIndex( 1 );
    ui->tabWidget->setEnabled( false );
    if( ui->course_code_line_edit->text().trimmed().isEmpty() ){
        MBOX_CRIT( "Course code has not been set" );
        ui->course_code_line_edit->setFocus();
        ui->tabWidget->setEnabled( true );
        return;
    }
    if( ui->course_name_line_edit->text().trimmed().isEmpty() ){
        MBOX_CRIT( "Course name has not been set" );
        ui->course_name_line_edit->setFocus();
        ui->tabWidget->setEnabled( true );
        return;
    }
    if( ui->use_time_line_edit->text().trimmed().isEmpty() ){
        MBOX_CRIT( "You cannot leave this column empty\n" );
        ui->use_time_line_edit->setFocus();
        ui->tabWidget->setEnabled( true );
        return;
    }
    bool is_ok = false;
    duration = ui->use_time_line_edit->text().toLongLong( &is_ok );
    if( !is_ok ){
        MBOX_CRIT( "Invalid number" );
        ui->use_time_line_edit->clear();
        ui->use_time_line_edit->setFocus();
        ui->tabWidget->setEnabled( true );
        return;
    }
    if( ui->expires_on_checkbox->isChecked() ){
        if( ui->expires_on_dt_edit->date() < QDate::currentDate() ){
            MBOX_CRIT( "Please specify a correct expiry date" );
            ui->tabWidget->setEnabled( true );
            return;
        }
    }
    if( ui->instructor_name_line_edit->text().trimmed().isEmpty() ){
        MBOX_CRIT( "You haven't specified an instructor's name" );
        ui->instructor_name_line_edit->setFocus();
        ui->tabWidget->setEnabled( true );
        return;
    }

    for( int i = 0; i != answer_items.size(); ++i ){
        if( answer_items[i] == -1 ){
            ui->tabWidget->setCurrentIndex( 0 );
            QMessageBox::critical( this, windowTitle(), "We have a question with unmarked solution");
            ui->questions_tree_view->setCurrentIndex( model->index( i, 0 ) );
            ui->tabWidget->setEnabled( true );
            return;
        }
    }

    QObject::connect( ui->preview_window_action, &QAction::triggered, [&]{
        question_root["items"] = question_items;
    });

    if( ui->local_copy_chkbox->isChecked() ){

        QString const filename = QFileDialog::getSaveFileName( this, this->windowTitle(), "",
                                                               "JSON Files( *.json )"  );
        if( !filename.isEmpty() ){
            QFile file( filename );
            if( !file.open( QIODevice::WriteOnly ) ){
                MBOX_CRIT( file.errorString() );
            } else {
                QJsonObject const file_data{ { "detail", GetQuestionJson() } };
                QTextStream file_stream( &file );
                file_stream << QJsonDocument( file_data ).toJson();
            }
        }
    }
    FillDataToUpload();
    if( upload_metadata.size() == 0 ){ // no files to upload?
        CompleteSubmission();
        ui->tabWidget->setEnabled( true );
    } else {
        ui->tabWidget->setCurrentIndex( 0 );
        PerformUpload();
    }
}

void AddQuestionWindow::FillDataToUpload()
{
    upload_metadata.clear();
    for( unsigned int i = 0; i != question_items.size(); ++i ){
        QJsonObject const item = question_items[i].toObject();
        // do we have an item that has *not* already been uploaded? Upload it.
        if( item.contains( "photo" ) || item.contains( "raw" ) ){
            upload_metadata.push_back( GetUploadMetaData( i, item ) );
        }
    }
    if( ui->using_icon_check->isChecked() && ui->using_icon_check->isEnabled() ){
        upload_metadata.push_back( GetUploadMetaData( -1, QJsonObject{ { "photo", icon_filename } } ) );
    }
    failed_uploads.clear();
}

void AddQuestionWindow::PerformUpload()
{
    UploadTask *upload_task{ new UploadTask( upload_metadata, failed_uploads, this ) };
    upload_task->SetImageUploadAddress( image_upload_address );
    upload_task->SetRawfileUploadAddress( raw_file_upload_address );
    upload_task->SetNetworkManager( network_manager );
    upload_task->SetRepositoryName( ui->repository_combo->currentText() );

    UploadStatusDialog *upload_status_dialog = new UploadStatusDialog( this );
    upload_status_dialog->setMessage( "Please wait while we upload your images" );

    QObject::connect( upload_status_dialog, &UploadStatusDialog::accepted, [=]{
        upload_task->cancel();
    });
    QObject::connect( upload_task, &UploadTask::completed, [=, this]{
        if( this->failed_uploads.isEmpty() ){
            upload_status_dialog->accept();
            this->CompleteSubmission();
        } else {
            auto response = QMessageBox::critical( this, "Error",
                                   "Some files could not be uploaded to the server. Retry?",
                                   QMessageBox::Yes | QMessageBox::No );
            if( response == QMessageBox::No ){
                ui->tabWidget->setEnabled( true );
                return;
            }
            this->FillDataToUpload();
            upload_task->loop();
        }
    });
    QObject::connect( upload_task, &UploadTask::status, upload_status_dialog,
                      &UploadStatusDialog::setMessage );
    QObject::connect( upload_task, &UploadTask::successful, [=,this]( int index, QString url ) mutable {
        if( index == -1 ){ // this is the icon for the image, not regular images for questions
            ui->icon_preview_text->setText( "New logo uploaded" );
            this->icon_filename = "URL: " + url;
            ui->using_icon_check->setEnabled( false );
            return;
        }
        QJsonValueRef data_ref = this->question_items[index];
        QJsonObject result = data_ref.toObject();
        result["url"] = url;
        bool const is_image_file = result.contains( "photo" );
        QString const file_data = is_image_file ? "photo" : "raw";
        upload_status_dialog->setMessage( result.value( file_data ).toString() + " uploaded." );
        result.remove( file_data );
        data_ref = result;
        auto model_index = this->model->index( index, 1 );
        model->setData( model_index, "URL: " + url );
    });

    QObject::connect( upload_task, &UploadTask::errorOccured,
                      [this, upload_status_dialog]( int index, QString msg ) mutable {
        auto item = this->question_items.at( index ).toObject();
        this->failed_uploads.push_back( GetUploadMetaData( index, item ) );
        upload_status_dialog->setMessage( tr( "Error on item %1: %2" ).arg(index + 1).arg( msg ) );
    });

    QObject::connect( upload_status_dialog, &UploadStatusDialog::cancelled, [=]{
        upload_task->cancel();
    });
    upload_status_dialog->show();
    upload_task->loop();
}

MetaData AddQuestionWindow::GetUploadMetaData( unsigned int const index, QJsonObject const & item )
{
    bool is_raw_file = item.contains( "raw" );
    if( is_raw_file ){
        return MetaData{ item.value( "raw" ).toString(), UploadType::Rawfile, index };
    }
    return MetaData { item.value( "photo" ).toString(), UploadType::Image, index };
}

QJsonObject AddQuestionWindow::GetQuestionJson()
{
    QJsonObject question {};
    question["name"] = ui->course_name_line_edit->text();
    question["course_code"] = ui->course_code_line_edit->text();
    question["date_to_be_held"] = GetDate( ui->held_date_dt_edit->date() );
    question["duration"] = duration;
    question["administrator_name"] = ui->instructor_name_line_edit->text();

    question_root["instructor"] = ui->instructor_name_line_edit->text();
    question_root["total"] = question_items.size();
    question_root["items"] = question_items;
    question["answers"] = answer_items;
    question["question"] = question_root;
    question["approach"] = approach;
    question["randomize"] = ui->randomize_true_radio->isChecked();
    question["departments"] = QJsonArray::fromStringList(
                ui->departments_text_edit->toPlainText().split( "\n" ) );
    question["repository_name"] = ui->repository_combo->currentText().trimmed();
    if( ui->using_icon_check->isChecked() && icon_filename.startsWith( "URL:" ) ){
        question["icon"] = icon_filename;
    } else {
        question["icon"] = "default";
    }

    if( ui->expires_on_checkbox->isChecked() ){
        question["expires_on"] = GetDate( ui->expires_on_dt_edit->date() );
    }
    return question;
}

void AddQuestionWindow::CompleteSubmission()
{
    QDialog *sending_dialog = new QDialog( this );
    QVBoxLayout *layout = new QVBoxLayout;
    QLineEdit *message_box = new QLineEdit( "Please wait while we send your question over" );
    layout->addWidget( message_box );
    sending_dialog->setLayout( layout );

    auto const question = GetQuestionJson();
    QByteArray data = QJsonDocument( question ).toJson();
    auto request = utilities::GetPostNetworkRequestFrom( send_to_address, data.size() );
    request.setHeader( QNetworkRequest::ContentTypeHeader, "application/json" );
    request.setHeader( QNetworkRequest::ContentLengthHeader, data.size() );
    QNetworkReply *network_reply = network_manager->post( request, data );

    QObject::connect( network_reply, &QNetworkReply::finished, [=]{
        QJsonObject result = utilities::OnNetworkResponseReceived( network_reply );
        if( result.isEmpty() ){
            return;
        }
        bool is_success = result.value( "status" ).toInt() == 1;
        if( !is_success ){
            MBOX_CRIT( result.value( "detail" ).toString() );
            return;
        }
        MBOX_INFO( result.value( "detail" ).toString() );
        this->close();
    });
}

UploadTask::UploadTask( UploadDataList &metadata, UploadDataList &error_list, QObject *parent )
    : QObject( parent ), upload_metadata( metadata ), failed_upload_metadata( error_list )
{
}

void UploadTask::SetNetworkManager( QNetworkAccessManager *manager )
{
    network_manager = manager;
}

void UploadTask::SetRepositoryName( QString const &name )
{
    QUrlQuery url_query{};
    url_query.addQueryItem( "repository", name );

    image_upload_address.setQuery( url_query );
    rawfile_upload_address.setQuery( url_query );
}

void UploadTask::SetImageUploadAddress( QString const &address )
{
    image_upload_address = QUrl::fromUserInput( address );
}

void UploadTask::SetRawfileUploadAddress( QString const &address )
{
    rawfile_upload_address = QUrl::fromUserInput( address );
}

QByteArray UploadTask::FileToData( QString const &filename, UploadType upload_type )
{
    QFile file( filename );
    if( !file.exists() ) return QByteArray();

    bool is_image = upload_type == UploadType::Image;
    QString const mime_type = is_image ? "image/png" : "text/plain";
    QString const boundary = "data_boundary";

    QByteArray data{ QString( "--" + boundary + "\r\n" ).toUtf8() };
    data.append( QString( "Content-Disposition: form-data; filename=\"%1\"; "
                          "name=\"%2\"\r\n" ).arg( QFileInfo( filename ).fileName() )
                 .arg( is_image ? "photo": "raw" ));
    data.append( QString( "Content-Type: " + mime_type + "\r\n\r\n" ).toUtf8() );

    if( !file.open( QIODevice::ReadOnly )){
        return QByteArray();
    }
    data.append( file.readAll() );
    data.append( "\r\n" );
    data.append( QString( "--" + boundary + "--\r\n" ).toUtf8() );
    file.close();
    return data;
}

void UploadTask::loop()
{
    QString boundary{ "data_boundary" };
    while( !upload_metadata.isEmpty() ){
        MetaData front_item { upload_metadata.dequeue() };
        QUrl const address = front_item.type == UploadType::Rawfile ? rawfile_upload_address :
                                                                      image_upload_address;
        QNetworkRequest request = utilities::GetRequest( address.toString() );
        QByteArray data = FileToData( front_item.filename, front_item.type );
        if( data.isNull() ){
            emit errorOccured( front_item.data_index, tr( "Unable to read file" ) );
            continue;
        }
        request.setHeader( QNetworkRequest::ContentTypeHeader,
                           "multipart/form-data; boundary=" + boundary );
        request.setHeader( QNetworkRequest::ContentLengthHeader, data.length() );
        request.setRawHeader( "FileIndex", QByteArray::number( front_item.data_index ) );
        emit status( tr( "Uploading %1" ).arg( front_item.filename ) );

        QNetworkReply *reply = network_manager->post( request, data );
        index_list.push_back( front_item.data_index );
        QObject::connect( reply, &QNetworkReply::finished, this, &UploadTask::OnNetworkReplied );
    }
}

void UploadTask::OnNetworkReplied()
{
    QNetworkReply *network_reply = qobject_cast<QNetworkReply*>( sender() );
    Q_ASSERT( network_reply != nullptr );

    int index = network_reply->request().rawHeader( "FileIndex" ).toInt();

    index_list.removeAll( index );

    if( network_reply->error() != QNetworkReply::NoError ){
        emit errorOccured( index, network_reply->errorString() );
        if( index_list.isEmpty() ) emit completed();
        return;
    }
    QJsonDocument json_document{ QJsonDocument::fromJson( network_reply->readAll() ) };
    if( json_document.isNull() ){ // do we *not* have a valid JSON document?
        emit errorOccured( index, "Invalid response from the server" );
        if( index_list.isEmpty() ) emit completed();
        return;
    }
    QJsonObject response = json_document.object();
    if( response.value( "status" ).toInt() == 0 ){
        emit errorOccured( index, response.value( "detail" ).toString() );
        return;
    }
    emit successful( index, response.value( "url" ).toString() );
    if( index_list.isEmpty() ){
        emit completed();
    }
}

void UploadTask::cancel()
{

}
