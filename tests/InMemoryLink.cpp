// Local protocol harness only: no sockets, TLS, SDP or actual WebRTC are exercised.
#include "LanLink.h"
#include <QHash>
#include <QPointer>
class LanServer: public QTcpServer {};
static LanLink* listener=nullptr;
static QHash<LanLink*,LanLink*> partners;
LanLink::LanLink(QObject* parent):QObject(parent),server_(std::make_unique<LanServer>()){}
LanLink::~LanLink(){stop();}
QString LanLink::listen(bool,bool){stop();host_=true;listener=this;return "in-memory-test";}
void LanLink::dial(const QString&,const QString& password,const QString& name){
 stop();host_=false;if(!listener||!listener->verifyPassword||!listener->verifyPassword(password))return;
 auto* host=listener;partners[this]=host;partners[host]=this;authenticated_=true;host->authenticated_=true;
 QTimer::singleShot(0,host,[host,name]{emit host->request(name);});
}
void LanLink::approve(bool control,bool view,bool files,bool chat,bool voice){
 auto* peer=partners.value(this);if(!peer||!host_||!authenticated_)return;
 for(auto* side:{this,peer}){side->approved_=true;side->view_=view;side->control_=view&&control;side->files_=files;side->chat_=chat;side->voice_=voice;side->active_=true;}
 emit opened();emit peer->opened();
}
void LanLink::stop(){
 if(listener==this)listener=nullptr;
 auto* peer=partners.take(this);if(peer)partners.remove(peer);
 active_=view_=control_=files_=chat_=voice_=false;authenticated_=approved_=false;
 emit ended();if(peer){peer->active_=peer->view_=peer->control_=peer->files_=peer->chat_=peer->voice_=false;emit peer->ended();}
}
void LanLink::send(const QByteArray& data){
 QPointer<LanLink> peer=partners.value(this);if(!active_||!peer)return;
 QTimer::singleShot(0,peer,[peer,data]{if(peer&&peer->active_)emit peer->message(data);});
}
quint64 LanLink::queuedBytes()const{return 0;}
QString LanLink::sessionBinding()const{return QString(64,'a');}
bool LanLink::privateAddress(const QHostAddress&){return true;}
QString LanLink::address(){return "127.0.0.1";}
