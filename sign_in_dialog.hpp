#ifndef SIGN_IN_DIALOG_HPP
#define SIGN_IN_DIALOG_HPP

#include <QDialog>

namespace Ui {
class SignInDialog;
}

class QNetworkAccessManager;
class QNetworkReply;
class SignInDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SignInDialog( QNetworkAccessManager *network, QWidget *parent = 0);
    ~SignInDialog();
    void SetLoginAddress( QString const & address );
    QNetworkReply* NetworkReply();
private:
    Ui::SignInDialog *ui;
    QNetworkAccessManager *network_manager;
    QNetworkReply *new_reply;
    QString login_address;
    void OnLoginButtonClicked();
};

#endif // SIGN_IN_DIALOG_HPP
