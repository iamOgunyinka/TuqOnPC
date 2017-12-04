#include <QApplication>
#include "preload_window.hpp"

int main( int argc, char *argv[] )
{
    QApplication a( argc, argv );
    PreloadWindow splash_screen_window( &a );
    splash_screen_window.ShowSplashScreen();
    return a.exec();
}
