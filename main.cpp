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
#include <thread>

using namespace curlpp::options;


//funzione che viene passata al thread per essere eseguita
void listener(){
    std::cout<<"server on"<<std::endl;
    httplib::Server svr;
    svr.Get("/", [](const httplib::Request & req, httplib::Response &res) {
        res.set_content("Hello World!", "text/plain");
        std::cout<< req.get_param_value("code") <<std::endl;
    });
    std::cout<<"server listening.."<<std::endl;
    svr.listen("127.0.0.1", 8080);
}


int main(int argc, char *argv[]){

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    //return a.exec();

    //apre il browser con il link passato
    QDesktopServices::openUrl(QUrl("https://accounts.google.com/o/oauth2/v2/auth?client_id=1034786735866-o2d6ot6i3mvvrvt2rck9qent1a32odnb.apps.googleusercontent.com&response_type=code&scope=https://www.googleapis.com/auth/calendar&redirect_uri=http://localhost:8080&code_challenge=1111111111111111111111111111111111111111111&challenge_method=plain"));

    std::thread t1{listener};
    t1.join();


}
