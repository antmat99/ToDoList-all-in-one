#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "mainwindow.h"
#include <QApplication>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
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
std::mutex m;
std::condition_variable cv;
std::string t_code = "";

//funzione che viene passata al thread per ottenere il token code da inserire poi nella post
void listener(){
    std::unique_lock<std::mutex> ul(m);
    std::cout<<"server on"<<std::endl;
    httplib::Server svr;
    svr.Get("/", [](const httplib::Request & req, httplib::Response &res) {
        res.set_content("Hello World!", "text/plain");
        t_code = req.get_param_value("code");
        //std::cout<<t_code<<std::endl;
        cv.notify_all();

    });
    ul.unlock();
    std::cout<<"server listening.."<<std::endl;
    svr.listen("127.0.0.1", 8080);
}

//questa funzione viene svegliata quando c'è risposta nella get fatta sopra
//e cambia il valore della var
//anzichè cambiare la variabile devo prendere la var globale e inserirgli il token code
//cosi da fare la post e poi ottenere l'access token tramite post
void change(){
    std::cout<<"qui"<<std::endl;
    std::unique_lock<std::mutex> ul(m);
    cv.wait(ul);
    std::cout<<t_code<<std::endl;
    std::cout<<"-----"<<std::endl;
    //qui devo fare la post

    // QUI USO LIBRERIA CURLPP MA DA ERRROE 404/400
    // PROBABLY PERCHE I PARAMS SONO PASSATI MALE
    /*try {
        curlpp::Cleanup cleaner;
        curlpp::Easy request;

        request.setOpt(new curlpp::options::Url("https://oauth2.googleapis.com/"));
        request.setOpt(new curlpp::options::Verbose(true));

        std::list<std::string> header;
        header.push_back("Content-Type: application/json");

        request.setOpt(new curlpp::options::HttpHeader(header));
        std::string payload = "token?code="+t_code+"&client_id=1034786735866-o2d6ot6i3mvvrvt2rck9qent1a32odnb.apps.googleusercontent.com&client_secret=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&grant_type=authorization_code&redirect_uri=http://localhost:8080&code_verifier=1111111111111111111111111111111111111111111";
        request.setOpt(new curlpp::options::PostFields(payload));
        request.setOpt(new curlpp::options::PostFieldSize(payload.length()));

        request.perform();
      }
      catch ( curlpp::LogicError & e ) {
        std::cout << e.what() << std::endl;
      }
      catch ( curlpp::RuntimeError & e ) {
        std::cout << e.what() << std::endl;
      }
      */

    // QUA PROVO A USARE LIBRERIA QNETWORK MA CRASHA
    // FORSE PERCHÈ I PARAMS SONO PASSATI MALE
    /*
    std::string txt = "token?code="+t_code+"&client_id=1034786735866-o2d6ot6i3mvvrvt2rck9qent1a32odnb.apps.googleusercontent.com&client_secret=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&grant_type=authorization_code&redirect_uri=http://localhost:8080&code_verifier=1111111111111111111111111111111111111111111";
    const char* aa = txt.c_str();
    QByteArray payload = QByteArrayLiteral(aa);
    QNetworkAccessManager man;
    QNetworkRequest req(QUrl("https://oauth2.googleapis.com/"));
    QNetworkReply* reply =man.post(req, payload);
    QObject::connect(reply,&QNetworkReply::finished,[&](){
        QByteArray read = reply->readAll(); //read contiene il contenuto della risposta
        //si usa il metodo toStdString() per convertire read a stringa
        std::cout << "Got response:\n" << read.toStdString() << std::endl;
        reply->close();
        reply->deleteLater();
    });
    */
    ul.unlock();
}


int main(int argc, char *argv[]){

    QApplication a(argc, argv);
    MainWindow w;
    //w.show();
    //return a.exec();

    //apre il browser con il link passato
    QDesktopServices::openUrl(QUrl("https://accounts.google.com/o/oauth2/v2/auth?client_id=1034786735866-o2d6ot6i3mvvrvt2rck9qent1a32odnb.apps.googleusercontent.com&response_type=code&scope=https://www.googleapis.com/auth/calendar&redirect_uri=http://localhost:8080&code_challenge=1111111111111111111111111111111111111111111&challenge_method=plain"));

    //creo un thread per stare in ascolto su localhost:8080
    std::thread t1{listener};

    //questo thread viene usato per prendere il token e metterlo nella post
    std::thread t2{change};

    t1.join();
    t2.join();

    //una volta ottenuto il token code, devo fare una post a
    //https://oauth2.googleapis.com/token?code=QUI_METTI_TOKEN_CODE&client_id=1034786735866-o2d6ot6i3mvvrvt2rck9qent1a32odnb.apps.googleusercontent.com&client_secret=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&grant_type=authorization_code&redirect_uri=http://localhost:8080&code_verifier=1111111111111111111111111111111111111111111
    //per ottenere l'access token

}
