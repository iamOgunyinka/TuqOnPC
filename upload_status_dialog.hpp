#ifndef UPLOAD_STATUS_DIALOG_HPP
#define UPLOAD_STATUS_DIALOG_HPP

#include <QtWidgets>
#include <QDialog>

class UploadStatusDialog : public QDialog
{
    Q_OBJECT
public:
    UploadStatusDialog( QWidget *parent = nullptr );
public slots:
    void setMessage( QString const & message );
signals:
    void cancelled();
protected:
    void closeEvent( QCloseEvent * );
private:
    QPushButton *cancel_button;
    QTextEdit   *status_label;
};

#endif // UPLOAD_STATUS_DIALOG_HPP
