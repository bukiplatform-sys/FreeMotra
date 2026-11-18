#include "AppController.h"
#include <QGuiApplication>
#include <QClipboard>
#include <QRegularExpression>
#include <QJsonDocument>
#include <QBuffer>
#include <QImageReader>
#include <QSysInfo>
AppController::AppController(QObject* parent) : QObject(parent), identity_(settings_) {
    link_.verifyPassword=[this](const QString& candidate){ return identity_.verifyAndConsume(candidate); };
    connect(&link_,&LanLink::status,this,[this](const QString& value){sessionStatus_=value;emit sessionChanged();});
    connect(&link_,&LanLink::request,this,[this](const QString& peer){incomingPeer_=peer;emit sessionChanged();});
    connect(&link_,&LanLink::ended,this,[this]{
        captureTimer_.stop();input_.releaseAll();gate_.end();capture_.close();
        incomingPeer_.clear();invitation_.clear();frame_=QImage();frameBytes_.clear();expectedBytes_=0;waitingFrame_=false;
        emit sessionChanged();emit frameChanged();
    });
    connect(&link_,&LanLink::opened,this,[this]{
        incomingPeer_.clear();
        if(link_.isHost()) {
            QString error;
            if(!capture_.open(0,error)){endSession();sessionStatus_=error;emit sessionChanged();return;}
            captureTimer_.start(150);
        }
        emit sessionChanged();
    });
    connect(&link_,&LanLink::message,this,&AppController::receive);
    connect(&captureTimer_,&QTimer::timeout,this,&AppController::captureTick);
    refreshPassword();
    expiryTimer_.setInterval(300000);
    connect(&expiryTimer_, &QTimer::timeout, this, &AppController::refreshPassword);
    expiryTimer_.start();
}
void AppController::refreshPassword() {
    password_.fill(QChar(0));
    password_ = identity_.refreshPassword();
    emit passwordChanged();
    if (expiryTimer_.isActive()) expiryTimer_.start();
}
void AppController::setLanguage(const QString& v) {
    if (v != "tr" && v != "en") return;
    settings_.setValue("ui/language", v); emit settingsChanged();
}
void AppController::setDark(bool v) { settings_.setValue("ui/dark", v); emit settingsChanged(); }
void AppController::setQuality(const QString& v) {
    if (v != "balanced" && v != "quality" && v != "speed") return;
    settings_.setValue("video/quality", v); emit settingsChanged();
}
void AppController::setNotifications(bool v) { settings_.setValue("ui/notifications", v); emit settingsChanged(); }
void AppController::copyId() { QGuiApplication::clipboard()->setText(deviceId()); }
void AppController::requestConnection(const QString& remote) {
    const bool tr = language() == "tr";
    if (!QRegularExpression("^FM-[0-9A-Fa-f]{32}$").match(remote.trimmed()).hasMatch())
        message_ = tr ? "Geçerli bir FM- cihaz kimliği girin." : "Enter a valid FM- device identity.";
    else if (remote.trimmed().compare(deviceId(), Qt::CaseInsensitive) == 0)
        message_ = tr ? "Bu cihazın kendisine bağlanamazsınız." : "You cannot connect to this device itself.";
    else
        message_ = tr ? "Bağlantı motoru bu sürümde henüz hazır değil. Ağ bağlantısı başlatılmadı." : "The connection engine is not available in this version. No network connection was started.";
    emit messageChanged();
}
void AppController::dismissMessage() { message_.clear(); emit messageChanged(); }

AppController::~AppController(){link_.stop();}
void AppController::startHosting(){invitation_=link_.listen();emit sessionChanged();}
void AppController::copyInvitation(){QGuiApplication::clipboard()->setText(invitation_);}
void AppController::connectLan(const QString& invite,const QString& password){link_.dial(invite,password,QSysInfo::machineHostName());}
void AppController::approve(bool control){
    if(incomingPeer_.isEmpty())return;
    gate_.end();
    // SessionGate is additionally bound to the already authenticated TLS session.
    gate_.authenticatedPeer(link_.sessionBinding());
    gate_.localConsent(control);link_.approve(control);incomingPeer_.clear();emit sessionChanged();
}
void AppController::endSession(){link_.stop();sessionStatus_=language()=="tr"?"Oturum sonlandırıldı":"Session ended";emit sessionChanged();}
void AppController::captureTick(){
    if(!gate_.mayView()||!link_.active())return;
    if(waitingFrame_){if(frameWait_.elapsed()>5000)endSession();return;}
    if(link_.queuedBytes()>32768)return;
    QString error;auto img=capture_.nextFrame(error);
    if(!error.isEmpty()){endSession();sessionStatus_=error;emit sessionChanged();return;}
    if(img.isNull())return;
    img=img.scaled(quality()=="quality"?1600:1280,900,Qt::KeepAspectRatio,Qt::FastTransformation);
    QByteArray bytes;QBuffer out(&bytes);out.open(QIODevice::WriteOnly);
    if(!img.save(&out,"JPEG",quality()=="speed"?45:70)||bytes.size()>2000000)return;
    link_.send("F"+QJsonDocument(QJsonObject{{"size",int(bytes.size())}}).toJson(QJsonDocument::Compact));
    waitingFrame_=true;frameWait_.restart();
    for(qsizetype i=0;i<bytes.size();i+=16000)link_.send("D"+bytes.mid(i,16000));
}
void AppController::receive(const QByteArray& bytes){
    if(bytes.isEmpty())return;
    if(link_.isHost()){
        if(bytes=="A"){waitingFrame_=false;return;}
        if(bytes.startsWith('I')&&bytes.size()<1024)input(QJsonDocument::fromJson(bytes.mid(1)).object());
        return;
    }
    if(bytes.startsWith('F')){
        const auto size=QJsonDocument::fromJson(bytes.mid(1)).object()["size"].toInt();
        if(expectedBytes_||size<1||size>2000000){endSession();return;}
        expectedBytes_=size;frameBytes_.clear();return;
    }
    if(bytes.startsWith('D')){
        if(!expectedBytes_||frameBytes_.size()+bytes.size()-1>expectedBytes_){endSession();return;}
        frameBytes_+=bytes.mid(1);
        if(frameBytes_.size()==expectedBytes_){
            QBuffer buf(&frameBytes_);buf.open(QIODevice::ReadOnly);QImageReader reader(&buf,"JPEG");
            auto size=reader.size();if(size.width()<1||size.height()<1||size.width()>1920||size.height()>1080){endSession();return;}
            auto img=reader.read();if(img.isNull()){endSession();return;}
            frame_=img;++frameSerial_;frameBytes_.clear();expectedBytes_=0;link_.send("A");emit frameChanged();
        }
    }
}
void AppController::remoteInput(const QString& kind,double a,double b){
    if(link_.isHost()||!link_.active()||!link_.controlAllowed())return;
    link_.send("I"+QJsonDocument(QJsonObject{{"kind",kind},{"a",a},{"b",b}}).toJson(QJsonDocument::Compact));
}
void AppController::releaseRemoteKeys(){remoteInput("release",0);}
void AppController::input(const QJsonObject& value){
    if(!gate_.mayControl())return;
    const auto kind=value["kind"].toString();double a=value["a"].toDouble(),b=value["b"].toDouble();
    if(kind=="move") {
        const auto rect=capture_.bounds();
        const double vx=GetSystemMetrics(SM_XVIRTUALSCREEN),vy=GetSystemMetrics(SM_YVIRTUALSCREEN);
        const double vw=GetSystemMetrics(SM_CXVIRTUALSCREEN),vh=GetSystemMetrics(SM_CYVIRTUALSCREEN);
        if(vw>1 && vh>1 && a>=0 && a<=1 && b>=0 && b<=1)input_.move((rect.x()+a*(rect.width()-1)-vx)/(vw-1),(rect.y()+b*(rect.height()-1)-vy)/(vh-1));
    }
    else if(kind=="button"&&(a==0||a==1)&&(b==0||b==1))input_.button(a==1,b==1);
    else if(kind=="key"&&a>=8&&a<=254&&(b==0||b==1))input_.key(static_cast<unsigned short>(a),b==1);
    else if(kind=="wheel"&&a>=-1200&&a<=1200)input_.wheel(int(a));
    else if(kind=="release")input_.releaseAll();
}
