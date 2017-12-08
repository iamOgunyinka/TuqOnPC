#ifndef SCORE_SHEET_WINDOW_HPP
#define SCORE_SHEET_WINDOW_HPP

#include <QMainWindow>
#include <QPoint>
#include <QUrl>

class QNetworkAccessManager;

namespace Ui {
class ScoreSheetWindow;
}

struct TableData
{
    QString fullname;
    QString username;
    QString date_time;
    int     score;
    int     total_score;
};

class ScoreSheetWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ScoreSheetWindow( QWidget *parent = 0 );
    ~ScoreSheetWindow();
    void SetWebAddress( QString const & address , QPair<QString, long> const &query_info );
    void SetDeleteScoreAddress( QString const & address );
    void SetNetworkManager( QNetworkAccessManager * const network );
    void StartFetchingData();
private:
    void DisplayData( QJsonArray const &data );
    void OnCustomContextMenuReceived( QPoint const & point );
    void DeleteScoreWithReference( int const row );
    void OnCSVPrintTriggered();
    void OnPDFPrintTriggered();
private:
    Ui::ScoreSheetWindow    *ui;
    QNetworkAccessManager   *network_manager;
    QUrl                    score_listing_address;
    QUrl                    delete_score_address;
    QList<QString>          score_reference_ids;
    QList<TableData>        table_data_list;
};

#endif // SCORE_SHEET_WINDOW_HPP
