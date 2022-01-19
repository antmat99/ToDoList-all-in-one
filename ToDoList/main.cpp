#define CPPHTTPLIB_OPENSSL_SUPPORT

#include "calendar.h"
#include "event.h"
#include "api.h"
#include "mainwindow.h"
#include <QApplication>
#include <QDesktopServices>
#include <thread>
#include <cpp-httplib-master/httplib.h>

std::mutex m;
std::condition_variable cv;
std::string t_code = "";
bool pass=false;

//funzione che viene passata al thread per ottenere il token code da inserire poi nella post
void listener(){
    httplib::Server svr;
    svr.Get("/", [](const httplib::Request & req, httplib::Response &res) {
        std::unique_lock<std::mutex> ul(m);
        res.set_content("Puoi chiudere questa pagina !", "text/plain");
        t_code = req.get_param_value("code");
        pass=true;
        cv.notify_all();
    });
    svr.listen("127.0.0.1", 8080);
}

int main(int argc, char *argv[]){
    std::unique_lock<std::mutex> ul(m);
    QApplication a(argc, argv);
    //apre il browser con il link passato
    QDesktopServices::openUrl(QUrl("https://accounts.google.com/o/oauth2/v2/auth?client_id=1034786735866-o2d6ot6i3mvvrvt2rck9qent1a32odnb.apps.googleusercontent.com&response_type=code&scope=https://www.googleapis.com/auth/calendar&redirect_uri=http://localhost:8080&code_challenge=1111111111111111111111111111111111111111111&challenge_method=plain"));
    //creo un thread per stare in ascolto su localhost:8080
    std::thread t1{listener};
    cv.wait(ul,[](){return pass;});
    API* api = API::getInstance(t_code);
    MainWindow w;
    w.show();
    return a.exec();

}
