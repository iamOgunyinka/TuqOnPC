#ifndef QUESTION_DIALOG_HPP
#define QUESTION_DIALOG_HPP

#include <QDialog>
#include <QJsonObject>
#include <QList>

namespace Ui {
class QuestionDialog;
}

class QListWidgetItem;
enum class DataToUse
{
    None,
    Image,
    Others
};

class QuestionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QuestionDialog( QWidget *parent = nullptr );
    ~QuestionDialog();

    void closeEvent( QCloseEvent * );

    QJsonObject GetQuestionObject() const;
    int  GetSolutionOption() const;
    void SetQuestionText( QString const & question_text );
    void SetUsingImage( QString const & filename );
    void SetUsingRawfile( QString const & filename );
    void SetOptions( QStringList const & options );
    void SetAnswerIndex( int index );
private:
    QStringList GetListItems() const;
    DataToUse   Data() const;
    QString     FileName() const;
private:
    Ui::QuestionDialog *ui;

    void OnAddOptionToListClicked();
    void OnCustomContextMenuRequested( QPoint const & );
    void OnOptionsItemDoubleClicked( QListWidgetItem *item );
    void OnEditOptionsItem( QListWidgetItem *item );
    void OnDeleteOptionsItem( QListWidgetItem *item );
    void OnAddQuestionClicked();
};

#endif // QUESTION_DIALOG_HPP
