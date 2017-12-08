#include "question_dialog.hpp"
#include "ui_question_dialog.h"

#include <QInputDialog>
#include <QCloseEvent>
#include <QFileDialog>
#include <QJsonArray>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QMenu>

QuestionDialog::QuestionDialog( QWidget *parent ) :
    QDialog{ parent },
    ui{ new Ui::QuestionDialog }
{
    ui->setupUi( this );
    ui->question_line_edit->setFocus();
    QObject::connect( ui->add_to_list_button, &QPushButton::clicked, this,
                      &QuestionDialog::OnAddOptionToListClicked );
    QObject::connect( ui->options_list_widget, &QListWidget::itemDoubleClicked, this,
                      &QuestionDialog::OnOptionsItemDoubleClicked );

    ui->upload_button->setVisible( false );
    ui->upload_filename->setVisible( false );
    ui->use_none_radio->setChecked( true );

    QObject::connect( ui->use_none_radio, &QRadioButton::clicked, [=]( bool checked ){
        ui->upload_button->setVisible( !checked );
        ui->upload_filename->setVisible( !checked );
    });
    QObject::connect( ui->use_image_radio, &QRadioButton::clicked, [=]( bool checked ){
        ui->upload_button->setVisible( checked );
        ui->upload_filename->setVisible( checked );
    });
    QObject::connect( ui->use_other_radio, &QRadioButton::clicked, [=]( bool checked ){
        ui->upload_button->setVisible( checked );
        ui->upload_filename->setVisible( checked );
    });

    QObject::connect( ui->upload_button, &QPushButton::clicked, [=]{
        bool is_image_clicked = ui->use_image_radio->isChecked();
        QString caption = tr( "Open %1" ).arg( is_image_clicked ? "Image" : "File" ),
                extension = ui->use_image_radio->isChecked() ? "PNG Images(*.png)" :
                                                               "Text files(*.txt)";
        QString filename = QFileDialog::getOpenFileName( this, caption, QString(), extension );
        if( !filename.isNull() ){
            ui->upload_filename->setText( filename );
        }
    });
    QObject::connect( ui->add_button, &QPushButton::clicked, this,
                      &QuestionDialog::OnAddQuestionClicked );
    ui->options_list_widget->setContextMenuPolicy( Qt::CustomContextMenu );
    ui->options_list_widget->setSelectionMode( QAbstractItemView::SingleSelection );
    ui->options_list_widget->setSelectionBehavior( QAbstractItemView::SelectRows );
    ui->options_list_widget->setDragDropMode( QAbstractItemView::InternalMove );
    ui->options_list_widget->setEditTriggers( QAbstractItemView::DoubleClicked );
    QObject::connect( ui->options_list_widget, &QListWidget::customContextMenuRequested,
                      this, &QuestionDialog::OnCustomContextMenuRequested );
}

QuestionDialog::~QuestionDialog()
{
    delete ui;
}

void QuestionDialog::closeEvent( QCloseEvent *event )
{
    auto leaving = QMessageBox::information( this, windowTitle(), "Exit?", QMessageBox::Yes | QMessageBox::No );
    if( leaving == QMessageBox::Yes ){
        event->accept();
    } else {
        event->ignore();
    }
}

void QuestionDialog::OnCustomContextMenuRequested( QPoint const &point )
{
    QListWidgetItem* item = ui->options_list_widget->itemAt( point );
    if( item == nullptr ) return;
    QMenu custom_menu {};
    QAction *edit_item_action = new QAction( "Edit" );
    QAction *delete_item_action = new QAction( "Delete" );

    QObject::connect( edit_item_action, &QAction::triggered, [=]{
        this->OnOptionsItemDoubleClicked( ui->options_list_widget->currentItem() );
    });
    QObject::connect( delete_item_action, &QAction::triggered, [=]{
        this->OnDeleteOptionsItem( ui->options_list_widget->currentItem() );
    });

    custom_menu.addAction( edit_item_action );
    custom_menu.addAction( delete_item_action );
    custom_menu.exec( ui->options_list_widget->mapToGlobal( point ) );
}

void QuestionDialog::OnAddQuestionClicked()
{
    if( ui->question_line_edit->text().trimmed().size() == 0 ){
        QMessageBox::critical( this, windowTitle(), "You may not have an empty question" );
        ui->question_line_edit->setFocus();
        return;
    }
    if( ui->options_list_widget->count() < 3 ){
        QMessageBox::critical( this, windowTitle(), "Options must be at least 3" );
        ui->options_line_edit->setFocus();
        return;
    }
    if( ui->answer_combo->currentText().isNull() ){
        QMessageBox::critical( this, windowTitle(), "The solution segment cannot be left empty." );
        ui->answer_combo->setFocus();
        return;
    }
    accept();
}

QStringList QuestionDialog::GetListItems() const
{
    QStringList items {};
    for( int i = 0; i != ui->options_list_widget->count(); ++i ){
        items << ui->options_list_widget->item( i )->text().trimmed();
    }
    return items;
}

QJsonObject QuestionDialog::GetQuestionObject() const
{
    QJsonObject question_object {};
    question_object["question"] = ui->question_line_edit->text();

    QJsonObject options {};
    options["items"] = QJsonArray::fromStringList( GetListItems() );
    options["arity"] = ui->options_list_widget->count();
    question_object["options"] = options;

    switch( Data() ){
    case DataToUse::Image:
        question_object["photo"] = FileName();
        break;
    case DataToUse::Others:
        question_object["raw"] = FileName();
        break;
    case DataToUse::None:;
    }

    return question_object;
}

int QuestionDialog::GetSolutionOption() const {
    return ui->answer_combo->currentIndex();
}

DataToUse QuestionDialog::Data() const {
    return ui->use_none_radio->isChecked() ?
                DataToUse::None :
                ui->use_image_radio->isChecked() ?
                    DataToUse::Image : DataToUse::Others;
}

QString QuestionDialog::FileName() const {
    return ui->upload_filename->text();
}

void QuestionDialog::OnAddOptionToListClicked()
{
    QString const new_option = ui->options_line_edit->text().trimmed();
    if( new_option.isEmpty() ) return;

    ui->options_list_widget->addItem( new_option );
    ui->answer_combo->addItem( new_option );
    ui->options_line_edit->clear();
    ui->options_line_edit->setFocus();
}

void QuestionDialog::OnDeleteOptionsItem( QListWidgetItem *item )
{
    if( item ){
        ui->answer_combo->removeItem( ui->answer_combo->findText( item->text() ));
        delete item;
    }
}

void QuestionDialog::OnOptionsItemDoubleClicked( QListWidgetItem *item )
{
    if( !item ) return;

    bool ok_flag = false;
    QString text = QInputDialog::getText( this, windowTitle(), "Editing this item changes the original"
                                                               "\nClear it to remove it from the list",
                                          QLineEdit::Normal, item->text(), &ok_flag );
    text = text.trimmed();
    if( ok_flag ){
        int index_of_current_text = ui->answer_combo->findText( item->text() );
        if( text.isEmpty() ){
            if( index_of_current_text != -1 ){
                ui->answer_combo->removeItem( index_of_current_text );
            }
            delete item;
        } else {
            if( index_of_current_text != - 1 ){
                ui->answer_combo->setItemText( index_of_current_text, text );
            }
            item->setText( text );
        }
    }
}

void QuestionDialog::SetQuestionText( QString const &question_text )
{
    ui->question_line_edit->setText( question_text );
    ui->question_line_edit->setFocus();
}

void QuestionDialog::SetAnswerIndex( int index )
{
    if( index != -1 || index >= ui->answer_combo->count() ){
        ui->answer_combo->setCurrentIndex( index );
    }
}

void QuestionDialog::SetOptions( QStringList const &options )
{
    ui->options_list_widget->clear();
    ui->answer_combo->clear();

    ui->options_list_widget->addItems( options );
    ui->answer_combo->addItems( options);
    ui->options_line_edit->clear();
    ui->options_line_edit->setFocus();
}

void QuestionDialog::SetUsingRawfile( QString const &filename )
{
    ui->upload_button->setVisible( true );
    ui->use_other_radio->setChecked( true );
    ui->upload_filename->setText( filename );
}

void QuestionDialog::SetUsingImage( QString const &filename )
{
    ui->use_image_radio->setChecked( true );
    ui->upload_button->setVisible( true );
    ui->upload_filename->setText( filename );
}
