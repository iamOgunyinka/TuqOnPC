#ifndef ADD_QUESTION_WINDOW_HPP
#define ADD_QUESTION_WINDOW_HPP

#include <QJsonObject>
#include <QJsonArray>
#include <QMainWindow>
#include <QNetworkCookie>
#include <QPoint>
#include <QUrl>
#include "resources.hpp"

#define MBOX_INFO(message) (QMessageBox::information( this, windowTitle(), message))
#define MBOX_CRIT(message) (QMessageBox::critical( this, windowTitle(), message))

namespace Ui {
class AddQuestionWindow;
}

class QNetworkReply;
class QNetworkAccessManager;
class QStandardItemModel;

using namespace utilities;

class AddQuestionWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit AddQuestionWindow( ListMap & repositories, QWidget *parent = nullptr );
    ~AddQuestionWindow();
    void SetNetwork( QNetworkAccessManager *network );
    void FetchRepositories( QString const & address );
    void SetAddress( QString const & add );
    void SetImageUploadAddress( QString const & address );
    void SetRawfileUploadAddress( QString const & address );
    void SetLoginCookie( QList<QNetworkCookie> & cookie_jar );
    void DisplayMessage( QString const & message );
    void AddFromObject( QJsonObject const & root_element );
signals:
    void onRepositoriesObtained();
private slots:
    void onNetworkReply();
private:
    Ui::AddQuestionWindow *ui;
    QNetworkAccessManager *network_manager;
    QNetworkReply         *network_reply;
    QStandardItemModel    *model;
    int            approach;
    qlonglong      duration = 0LL;
    QJsonObject    question_root;
    QJsonArray     question_items;
    QJsonArray     answer_items;
    QString        send_to_address;
    QString        image_upload_address;
    QString        raw_file_upload_address;
    ListMap        &repositories;
    UploadDataList upload_metadata;
    UploadDataList failed_uploads;

private:
    void OnCustomMenuRequested( QPoint const & point );
    void OnAddQuestionActionTriggered();
    void OnAddFromFileActionTriggered();
    void OnSubmitButtonClicked();
    void OnEditQuestionItemClicked();
    void OnDeleteQuestionItemClicked();
    void UpdateModel( QJsonObject const &object , int const index );
    void CompleteSubmission();
    void PerformUpload();
    void FillDataToUpload();
    MetaData GetUploadMetaData( unsigned int const i, QJsonObject const & );
    QJsonObject ReadFile();
};

class UploadTask: public QObject
{
    Q_OBJECT
private:
    UploadDataList        &upload_metadata;
    UploadDataList        &failed_upload_metadata;
    QNetworkAccessManager *network_manager;
    QUrl                  rawfile_upload_address;
    QUrl                  image_upload_address;
    QList<int>            index_list;
protected:
    QByteArray FileToData( QString const & filename , UploadType type );
public:
    UploadTask( UploadDataList & metadata, UploadDataList & erro_list, QObject *parent = nullptr );
    void SetNetworkManager( QNetworkAccessManager *manager );
    void SetImageUploadAddress( QString const & address );
    void SetRawfileUploadAddress( QString const & address );
    void SetRepositoryName( QString const & name );
    void loop();
    void cancel();
signals:
    void completed();
    void status( QString const & message );
    void successful( int index, QString const & upload_url ); // when an upload is completed
    void errorOccured( int index, QString message );
private slots:
    void OnNetworkReplied();
};

#endif // ADD_QUESTION_WINDOW_HPP
