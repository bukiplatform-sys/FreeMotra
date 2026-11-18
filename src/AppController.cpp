#include "AppController.h"
#include <QGuiApplication>
#include <QClipboard>
#include <QRegularExpression>
#include <QJsonDocument>
#include <QBuffer>
#include <QImageReader>
#include <QSysInfo>
#include <QJsonArray>
#include <QUrl>
AppController::AppController(QObject* parent) : QObject(parent), identity_(settings_) {
    link_.verifyPassword=[this](const QString& candidate){ return identity_.verifyAndConsume(candidate); };
    connect(&link_,&LanLink::status,this,[this](const QString& value){sessionStatus_=value;emit sessionChanged();});
    connect(&link_,&LanLink::request,this,[this](const QString& peer){incomingPeer_=peer;emit sessionChanged();});
    connect(&link_,&LanLink::ended,this,[this]{
        monitors_.clear();monitorIndex_=displayedMonitor_=incomingMonitor_=0;
        captureTimer_.stop();input_.releaseAll();gate_.end();capture_.close();video_.reset();peerH264_=false;captureFailures_=0;latestCapture_=QImage();forceCapture_=false;
        incomingPeer_.clear();invitation_.clear();frame_=QImage();frameSerial_=0;frameBytes_.clear();expectedBytes_=0;waitingFrame_=false;
        emit sessionChanged();emit frameChanged();
    });
    connect(&link_,&LanLink::opened,this,[this]{
        incomingPeer_.clear();
        const auto session=link_.sessionBinding();
        QTimer::singleShot(500,this,[this,session]{if(link_.active()&&link_.sessionBinding()==session)link_.send("K"+QJsonDocument(QJsonObject{{"h264",video_.decoderAvailable()}}).toJson(QJsonDocument::Compact));});
        if(link_.isHost() && link_.viewAllowed()) {
            monitors_=fm::DesktopCapture::monitors();monitorIndex_=0;
            adaptive_.reset();
            link_.send("M"+QJsonDocument(QJsonObject{{"monitors",QJsonArray::fromVariantList(monitors_)},{"selected",monitorIndex_}}).toJson(QJsonDocument::Compact));
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
    if (v != "auto" && v != "balanced" && v != "quality" && v != "speed") return;
    settings_.setValue("video/quality", v); emit settingsChanged();
}
void AppController::setNotifications(bool v) { settings_.setValue("ui/notifications", v); emit settingsChanged(); }
void AppController::copyId() { QGuiApplication::clipboard()->setText(deviceId()); }
void AppController::dismissMessage() { message_.clear(); emit messageChanged(); }

AppController::~AppController(){disconnect(&link_,nullptr,this,nullptr);link_.stop();input_.releaseAll();}
void AppController::startHosting(){refreshPassword();invitation_=link_.listen(true);emit sessionChanged();}
void AppController::startInternetHosting(){refreshPassword();invitation_=link_.listenInternet();emit sessionChanged();}
void AppController::copyInvitation(){QGuiApplication::clipboard()->setText(invitation_);}
void AppController::connectLan(const QString& invite,const QString& password){link_.dial(invite,password,QSysInfo::machineHostName());}
void AppController::approve(bool control){approvePermissions(true,control,false,false,false);}
void AppController::approvePermissions(bool view,bool control,bool files,bool chat,bool voice){
    if(incomingPeer_.isEmpty())return;
    gate_.end();
    // SessionGate is additionally bound to the already authenticated TLS session.
    gate_.authenticatedPeer(link_.sessionBinding());
    gate_.localConsent(view&&control);link_.approve(control,view,files,chat,voice);incomingPeer_.clear();emit sessionChanged();
}
void AppController::endSession(){link_.stop();sessionStatus_=language()=="tr"?"Oturum sonlandırıldı":"Session ended";emit sessionChanged();}
void AppController::captureTick(){
    if(!gate_.mayView()||!link_.viewAllowed()||!link_.active())return;
    if(waitingFrame_){if(frameWait_.elapsed()>(link_.internet()?15000:5000))endSession();return;}
    if(link_.queuedBytes()>32768)return;
    QString error;auto img=capture_.nextFrame(error);
    if(!error.isEmpty()){
        input_.releaseAll();video_.reset();latestCapture_=QImage();forceCapture_=false;
        if(++captureFailures_>3){endSession();sessionStatus_=error;emit sessionChanged();return;}
        monitors_=fm::DesktopCapture::monitors();if(monitorIndex_>=monitors_.size())monitorIndex_=0;
        QString reopen;capture_.open(unsigned(monitorIndex_),reopen);captureTimer_.setInterval(500);
        link_.send("M"+QJsonDocument(QJsonObject{{"monitors",QJsonArray::fromVariantList(monitors_)},{"selected",monitorIndex_}}).toJson(QJsonDocument::Compact));
        sessionStatus_="Ekran yeniden hazırlanıyor / Recovering display";emit sessionChanged();return;
    }
    if(img.isNull()&&forceCapture_&&!latestCapture_.isNull()){img=latestCapture_;forceCapture_=false;}
    if(img.isNull()){
        if(peerH264_&&video_.encoding()){const auto encoded=video_.poll();if(!encoded.isEmpty())sendFrame(encoded,"h264",video_.width(),video_.height());}
        return;
    }captureFailures_=0;
    const int level=quality()=="auto"?adaptive_.level():(quality()=="quality"?2:(quality()=="speed"?0:1));
    effectiveQuality_=QString("%1 · %2 px · %3 fps · ACK %4 ms").arg(quality()).arg(level==2?1600:(level==1?1280:800)).arg(peerH264_?(level==2?20:(level==1?10:6)):(level==2?10:(level==1?7:4))).arg(int(adaptive_.averageMillis()));
    captureTimer_.setInterval(peerH264_?(level==2?50:(level==1?100:166)):(level==2?100:(level==1?150:250)));
    img=img.scaled(level==2?1600:(level==1?1280:800),900,Qt::KeepAspectRatio,Qt::FastTransformation);
    latestCapture_=img;
    if(peerH264_){
        const auto bytes=video_.encode(img,level==2?3000000:(level==1?1500000:600000));
        effectiveQuality_ += video_.hardware()?" · H.264 GPU":" · H.264";
        if(video_.failed()){peerH264_=false;video_.reset();}else{if(!bytes.isEmpty())sendFrame(bytes,"h264",video_.width(),video_.height());return;}
    }
    QByteArray bytes;QBuffer out(&bytes);out.open(QIODevice::WriteOnly);
    if(!img.save(&out,"JPEG",level==0?40:(level==2?80:65)))return;
    sendFrame(bytes,"jpeg",img.width(),img.height());
}
void AppController::sendFrame(const QByteArray& bytes,const QString& codec,int width,int height){
    if(bytes.isEmpty()||bytes.size()>2000000||waitingFrame_)return;
    link_.send("F"+QJsonDocument(QJsonObject{{"size",int(bytes.size())},{"monitor",monitorIndex_},{"quality",effectiveQuality_},{"codec",codec},{"width",width},{"height",height}}).toJson(QJsonDocument::Compact));
    waitingFrame_=true;frameWait_.restart();emit sessionChanged();
    for(qsizetype i=0;i<bytes.size();i+=16000)link_.send("D"+bytes.mid(i,16000));
}

void AppController::receive(const QByteArray& bytes){
    if(bytes.isEmpty()||!link_.active())return;
    if(bytes[0]=='C'||bytes[0]=='T'||bytes[0]=='V'||bytes[0]=='O'){tools_.receive(bytes);return;}
    if(bytes.startsWith('M')&&!link_.isHost()&&link_.viewAllowed()&&bytes.size()<12000){
        const auto value=QJsonDocument::fromJson(bytes.mid(1)).object();
        const auto list=value["monitors"].toArray();if(list.size()>32)return;
        monitors_=list.toVariantList();monitorIndex_=value["selected"].toInt();emit sessionChanged();return;
    }
    if(bytes.startsWith('S')&&link_.isHost()&&link_.viewAllowed()&&bytes.size()<128){
        const auto index=QJsonDocument::fromJson(bytes.mid(1)).object()["index"].toInt(-1);
        if(index<0||index>=monitors_.size()||index==monitorIndex_)return;
        input_.releaseAll();QString error;
        if(!capture_.open(static_cast<unsigned>(index),error)){endSession();sessionStatus_=error;emit sessionChanged();return;}
        monitorIndex_=index;video_.reset();latestCapture_=QImage();forceCapture_=false;
        link_.send("M"+QJsonDocument(QJsonObject{{"monitors",QJsonArray::fromVariantList(monitors_)},{"selected",index}}).toJson(QJsonDocument::Compact));
        emit sessionChanged();return;
    }
    if(bytes.startsWith('K')&&bytes.size()<256){peerH264_=QJsonDocument::fromJson(bytes.mid(1)).object()["h264"].toBool();if(!peerH264_){video_.reset();forceCapture_=true;}return;}
    if(link_.isHost()){
        if(bytes=="A"&&waitingFrame_){
            adaptive_.observe(double(frameWait_.elapsed()));
            waitingFrame_=false;return;
        }
        if(bytes.startsWith('I')&&bytes.size()<1024)input(QJsonDocument::fromJson(bytes.mid(1)).object());
        return;
    }
    if(!link_.viewAllowed())return;
    if(bytes.startsWith('F')){
        const auto size=QJsonDocument::fromJson(bytes.mid(1)).object()["size"].toInt();
        if(expectedBytes_||size<1||size>2000000){endSession();return;}
        const auto metadata=QJsonDocument::fromJson(bytes.mid(1)).object();
        incomingCodec_=metadata["codec"].toString("jpeg");incomingWidth_=metadata["width"].toInt();incomingHeight_=metadata["height"].toInt();
        if(incomingCodec_!="jpeg"&&incomingCodec_!="h264"){endSession();return;}
        incomingMonitor_=metadata["monitor"].toInt();
        effectiveQuality_=QJsonDocument::fromJson(bytes.mid(1)).object()["quality"].toString().left(120);emit sessionChanged();
        expectedBytes_=size;frameBytes_.clear();return;
    }
    if(bytes.startsWith('D')){
        if(!expectedBytes_||frameBytes_.size()+bytes.size()-1>expectedBytes_){endSession();return;}
        frameBytes_+=bytes.mid(1);
        if(frameBytes_.size()==expectedBytes_){
            QImage img;
            if(incomingCodec_=="h264"){
                img=video_.decode(frameBytes_,incomingWidth_,incomingHeight_);
                if(img.isNull()){video_.reset();link_.send("K"+QJsonDocument(QJsonObject{{"h264",false}}).toJson(QJsonDocument::Compact));frameBytes_.clear();expectedBytes_=0;link_.send("A");return;}
            }else{
                QBuffer buf(&frameBytes_);buf.open(QIODevice::ReadOnly);QImageReader reader(&buf,"JPEG");
                const auto size=reader.size();if(size.width()<1||size.height()<1||size.width()>1920||size.height()>1080){endSession();return;}
                img=reader.read();if(img.isNull()){endSession();return;}
            }
            frame_=img;displayedMonitor_=incomingMonitor_;++frameSerial_;frameBytes_.clear();expectedBytes_=0;link_.send("A");emit frameChanged();
        }
    }
}
void AppController::remoteInput(const QString& kind,double a,double b){
    if(link_.isHost()||!link_.active()||!link_.controlAllowed())return;
    link_.send("I"+QJsonDocument(QJsonObject{{"kind",kind},{"a",a},{"b",b},{"monitor",displayedMonitor_}}).toJson(QJsonDocument::Compact));
}
void AppController::remoteText(const QString& text){
 if(link_.isHost()||!link_.active()||!link_.controlAllowed()||text.isEmpty()||text.size()>256||!text.isValidUtf16())return;
 link_.send("I"+QJsonDocument(QJsonObject{{"kind","text"},{"text",text},{"monitor",displayedMonitor_}}).toJson(QJsonDocument::Compact));
}
void AppController::releaseRemoteKeys(){remoteInput("release",0);}
void AppController::input(const QJsonObject& value){
    if(!gate_.mayControl()||!link_.viewAllowed()||value["monitor"].toInt(-1)!=monitorIndex_)return;
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
    else if(kind=="text")input_.text(value["text"].toString());
}

void AppController::selectMonitor(int index){
 if(!link_.active()||link_.isHost()||!link_.viewAllowed()||index<0||index>=monitors_.size())return;
 releaseRemoteKeys();link_.send("S"+QJsonDocument(QJsonObject{{"index",index}}).toJson(QJsonDocument::Compact));
}
void AppController::saveDevice(const QString& name,const QString& invite){
 const auto clean=invite.trimmed();
 if(!QRegularExpression("^fm2://[0-9.]{7,15}:[0-9]{1,5}/[0-9a-f]{64}$").match(clean).hasMatch()||name.trimmed().isEmpty())return;
 auto devices=savedDevices();
 for(qsizetype i=0;i<devices.size();++i){auto entry=devices[i].toMap();if(entry["name"].toString()==name.trimmed().left(80)){entry["invitation"]=clean;devices[i]=entry;settings_.setValue("devices/bookmarks",devices);emit devicesChanged();return;}}
 if(devices.size()>=100)return;
 devices.append(QVariantMap{{"name",name.trimmed().left(80)},{"invitation",clean},{"favorite",false}});
 settings_.setValue("devices/bookmarks",devices);emit devicesChanged();
}
void AppController::removeDevice(int index){auto devices=savedDevices();if(index<0||index>=devices.size())return;devices.removeAt(index);settings_.setValue("devices/bookmarks",devices);emit devicesChanged();}
void AppController::toggleFavorite(int index){auto devices=savedDevices();if(index<0||index>=devices.size())return;auto entry=devices[index].toMap();entry["favorite"]=!entry["favorite"].toBool();devices[index]=entry;settings_.setValue("devices/bookmarks",devices);emit devicesChanged();}
