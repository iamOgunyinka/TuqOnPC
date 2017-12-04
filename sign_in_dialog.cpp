#include "sign_in_dialog.hpp"
#include "ui_sign_in_dialog.h"
#include "resources.hpp"

#include <QNetworkAccessManager>

SignInDialog::SignInDialog( QNetworkAccessManager *network, QWidget *parent ) :
    QDialog( parent ), network_manager( network ),
    ui( new Ui::SignInDialog )
{
    ui->setupUi( this );
    setWindowTitle( "Sign in" );
    setMaximumSize( QSize{ 286, 100 } );
    QObject::connect( ui->ok_button, &QPushButton::clicked,this, &SignInDialog::OnLoginButtonClicked);
}

void SignInDialog::SetLoginAddress( QString const &address )
{
    login_address = address;
}

void SignInDialog::OnLoginButtonClicked()
{
    QString const username = ui->username_edit->text();
    QString const password = ui->password_edit->text();

    if( username.isEmpty() || password.isEmpty() ){
        this->reject();
        return;
    }
    QJsonObject doc( { { "username", username }, { "password", password } } );
    auto data = QJsonDocument( doc ).toJson();
    ui->ok_button->setEnabled( false );

    auto request = utilities::GetPostNetworkRequestFrom( login_address, data.size() );
    new_reply = network_manager->post( request, data );
    QObject::connect( new_reply, &QNetworkReply::finished, [=]{
        QJsonObject result = utilities::OnNetworkResponseReceived( new_reply );
        if( result.isEmpty() ){
            this->reject();
            return;
        }
        QString const message = result.value( "detail" ).toString();
        if( result.value( "status" ).toInt() == 0 ){
            QMessageBox::critical( this, windowTitle(), message );
            this->reject();
            return;
        }
        QMessageBox::information( this, windowTitle(), "Welcome, " + message );
        this->accept();
    });
}

QNetworkReply* SignInDialog::NetworkReply()
{
    return new_reply;
}

SignInDialog::~SignInDialog()
{
    delete ui;
}
