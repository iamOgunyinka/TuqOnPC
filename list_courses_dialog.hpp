#ifndef LIST_COURSES_DIALOG_HPP
#define LIST_COURSES_DIALOG_HPP

#include <QDialog>
#include <QNetworkCookie>
#include "repository_lister.hpp"

namespace Ui {
class ListCoursesDialog;
}

class ListCoursesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ListCoursesDialog( utilities::ListMap &repository_courses, QWidget *parent = 0 );
    ~ListCoursesDialog();
    void SetNetworkManager( QNetworkAccessManager * );
    void SetEndpoints( utilities::EndpointInformation const & );
    void SetCookieJar( QList<QNetworkCookie> const & cookies );
    void Start();
private:
    void OnListRepositoryCustomMenuTriggered( QPoint const &point );
    void DeleteCourse( QString const & course_name, QString const & repository_name );
    void DeleteRepository( QString const & repository_name );
    void ListCoursePartakers( long const course_id, QString const & repository_name );
    void EditCourse( long const &course_id, QString const & repository_name );
private:
    Ui::ListCoursesDialog *ui;
    utilities::ListMap      &repository_courses;
    RepositoryLister        *repo_lister;
    QNetworkAccessManager   *network_manager;
    utilities::EndpointInformation endpoint;
    QList<QNetworkCookie>   cookies;
};

#endif // LIST_COURSES_DIALOG_HPP
