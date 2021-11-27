#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "mainwindow.h"
#include <QApplication>
#include <stdio.h>
#include <stdlib.h>
#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <cpp-httplib-master/httplib.h>
#include <QDesktopServices>
#include <QUrl>

using namespace curlpp::options;

int main(int argc, char *argv[]){

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    //return a.exec();

    //apre il browser con il link passato
    QDesktopServices::openUrl(QUrl("https://accounts.google.com/o/oauth2/v2/auth?client_id=1034786735866-o2d6ot6i3mvvrvt2rck9qent1a32odnb.apps.googleusercontent.com&response_type=code&scope=https://www.googleapis.com/auth/calendar&redirect_uri=http://localhost:8080&code_challenge=1111111111111111111111111111111111111111111&challenge_method=plain"));

    //creo un server su localhost:8080
    httplib::Server svr;

    //faccio una get quando qualcuno contatta localhost:8080 e prelevo il valore 'code'
    //che contiene il codice da allegare alla post per ottenere l'auth code
    svr.Get("/", [](const httplib::Request & req, httplib::Response &res) {
        res.set_content("Hello World!", "text/plain");
        std::cout<< req.get_param_value("code") <<std::endl;
    });

    //mi metto in ascolto su localhost 8080
    svr.listen("127.0.0.1", 8080);
}
