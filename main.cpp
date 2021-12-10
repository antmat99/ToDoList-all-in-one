#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "mainwindow.h"
#include "mynetwork.h"
#include <QApplication>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <cpp-httplib-master/httplib.h>
#include <QDesktopServices>
#include <QUrl>
#include <QUrlQuery>
#include <thread>
#include <vector>

using namespace curlpp::options;
std::mutex m;
std::condition_variable cv;
std::string t_code = "";
std::string a_code = "";
std::string r_token ="";
std::string exp_in = "";
std::ostringstream body;

// for string delimiter
std::vector<std::string> split (std::string s, std::string delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<std::string> res;

    while ((pos_end = s.find (delimiter, pos_start)) != std::string::npos) {
        token = s.substr (pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back (token);
    }

    res.push_back (s.substr (pos_start));
    return res;
}

//funzione che viene passata al thread per ottenere il token code da inserire poi nella post
void listener(){
    std::unique_lock<std::mutex> ul(m);
    std::cout<<"server on"<<std::endl;
    httplib::Server svr;
    svr.Get("/", [](const httplib::Request & req, httplib::Response &res) {
        res.set_content("Puoi chiudere questa pagina !", "text/plain");
        t_code = req.get_param_value("code");
        //std::cout<<t_code<<std::endl;
        cv.notify_all();

    });
    ul.unlock();
    std::cout<<"server listening.."<<std::endl;
    svr.listen("127.0.0.1", 8080);
}

//questa funzione viene svegliata quando c'è risposta nella get fatta sopra
//e cambia il valore di t_code
//cosicchè posso fare una post con tutti i parametri che mi servono
void change(){
    std::unique_lock<std::mutex> ul(m);
    cv.wait(ul);
    //se mi trovo qui, mi sono svegliato
    //std::cout<<t_code<<std::endl;
    std::cout<<"-----"<<std::endl;
    //qui devo fare la post, uso libreria curlpp

    try {
        curlpp::Cleanup cleaner;
        curlpp::Easy request;

        //imposto i parametri
        request.setOpt(new curlpp::options::Url(std::string("https://oauth2.googleapis.com/token?code="+t_code+"&client_id=1034786735866-o2d6ot6i3mvvrvt2rck9qent1a32odnb.apps.googleusercontent.com&client_secret=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&grant_type=authorization_code&redirect_uri=http://localhost:8080&code_verifier=1111111111111111111111111111111111111111111").c_str()));
        request.setOpt(new curlpp::options::Verbose(true));

        //TODO: dubbio sul content-type: text plain o application json?
        std::list<std::string> header;
        header.push_back("Content-Type: text/plain");
        //imposto header
        request.setOpt(new curlpp::options::HttpHeader(header));
        //specifico che il body è vuoto
        request.setOpt(new curlpp::options::PostFields("\r"));
        request.setOpt(new curlpp::options::PostFieldSize(0));

        //metto da parte la risposta che otterrò
        std::ostringstream response;
        //il body della risposta alla post viene salvato in response
        request.setOpt(new curlpp::options::WriteStream(&body));

        //eseguo post
        request.perform();
        std::cout<<"-----------"<<std::endl;
        //stampa del body
        std::cout<<body.str()<<std::endl;

        //splitto per prendere i parametri che mi servono
        std::vector<std::string> v = split(body.str(), " ");

        //salvo le var che userò per inserirle nelle varie richieste
        a_code = v[3];      //access_token
        exp_in = v[6];      //expires_in
        r_token = v[9];     //refresh_token

        //elimino "" e , siccome ho splittato il testo da json
        a_code.erase(std::remove(a_code.begin(), a_code.end(), '\"'), a_code.end());
        a_code.erase(std::remove(a_code.begin(), a_code.end(), ','), a_code.end());
        exp_in.erase(std::remove(exp_in.begin(), exp_in.end(), ','), exp_in.end());
        r_token.erase(std::remove(r_token.begin(), r_token.end(), '\"'), r_token.end());
        r_token.erase(std::remove(r_token.begin(), r_token.end(), ','), r_token.end());


    }
    catch ( curlpp::LogicError & e ) {
        std::cout << e.what() << std::endl;
    }
    catch ( curlpp::RuntimeError & e ) {
        std::cout << e.what() << std::endl;
    }

    ul.unlock();
    cv.notify_all();
}

void getConQ(){
    std::unique_lock<std::mutex> ul(m);
    cv.wait(ul);
    //se mi trovo qui, vuol dire che dopo la post posso fare la get
    std::cout<<"---get---"<<std::endl;
    MyNetwork network;
    network.makeGetRequest("https://www.polito.it");
    //std::string tmp = network.getMiao();
    //std::cout<<"GET request done. Result is: \n"+tmp<<std::endl;
    cv.notify_all();
}


int main(int argc, char *argv[]){

    QApplication a(argc, argv);
    MainWindow w;
    //w.show();


    //apre il browser con il link passato
    QDesktopServices::openUrl(QUrl("https://accounts.google.com/o/oauth2/v2/auth?client_id=1034786735866-o2d6ot6i3mvvrvt2rck9qent1a32odnb.apps.googleusercontent.com&response_type=code&scope=https://www.googleapis.com/auth/calendar&redirect_uri=http://localhost:8080&code_challenge=1111111111111111111111111111111111111111111&challenge_method=plain"));

    //creo un thread per stare in ascolto su localhost:8080
    std::thread t1{listener};

    //questo thread viene usato per prendere il token e metterlo nella post
    std::thread t2{change};

    //creo un thread che fa get con qnetwork
    std::thread t3{getConQ};

    t1.join();
    t2.join();
    t3.join();

    return a.exec();
}
