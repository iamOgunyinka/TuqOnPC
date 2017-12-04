#include <QMessageBox>

#include "upload_status_dialog.hpp"

UploadStatusDialog::UploadStatusDialog( QWidget *parent ): QDialog( parent ),
    cancel_button( new QPushButton( "Cancel uploads") ), status_label( new QTextEdit( "" ))
{
    QGridLayout *layout = new QGridLayout;
    layout->addWidget( status_label );
    layout->addWidget( cancel_button, 1, 1 );

    this->setWindowTitle( "Upload Status" );
    this->setLayout( layout );
    QObject::connect( cancel_button, &QPushButton::clicked, this, &UploadStatusDialog::close );
}

void UploadStatusDialog::closeEvent( QCloseEvent * event )
{
    auto choice = QMessageBox::critical( this, "Cancel uploads", "Closing this window indicates "
                                                                 "you want to cancel the uploads, "
                                                                 "are you sure?",
                           QMessageBox::Yes | QMessageBox::No );
    if( choice == QMessageBox::Yes ){
        emit cancelled();
        event->accept();
    } else {
        event->ignore();
    }
}

void UploadStatusDialog::setMessage( QString const & message )
{
    status_label->insertPlainText( message + "\n\n" );
}
