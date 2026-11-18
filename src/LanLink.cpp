#include "LanLink.h"
#include <QNetworkInterface>
#include <QJsonDocument>
#include <QDateTime>
#include <QRegularExpression>
#include <QSslConfiguration>
#include <QCryptographicHash>
#include <QPointer>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <stdexcept>

class LanServer final : public QTcpServer {
public:
 explicit LanServer(LanLink* owner):QTcpServer(owner),owner_(owner) { setMaxPendingConnections(1); }
 void incomingConnection(qintptr fd) override { owner_->incoming(fd); }
private: LanLink* owner_;
};
static void certificate(QSslCertificate& cert,QSslKey& key) {
 auto ctx=std::unique_ptr<EVP_PKEY_CTX,decltype(&EVP_PKEY_CTX_free)>(EVP_PKEY_CTX_new_id(EVP_PKEY_RSA,nullptr),EVP_PKEY_CTX_free);
 EVP_PKEY* raw=nullptr;
 if(!ctx || EVP_PKEY_keygen_init(ctx.get())<=0 || EVP_PKEY_CTX_set_rsa_keygen_bits(ctx.get(),2048)<=0 || EVP_PKEY_keygen(ctx.get(),&raw)<=0) throw std::runtime_error("TLS key generation failed");
 auto p=std::unique_ptr<EVP_PKEY,decltype(&EVP_PKEY_free)>(raw,EVP_PKEY_free);
 auto x=std::unique_ptr<X509,decltype(&X509_free)>(X509_new(),X509_free);
 if(!x) throw std::runtime_error("TLS certificate allocation failed");
 X509_set_version(x.get(),2); ASN1_INTEGER_set(X509_get_serialNumber(x.get()),1);
 X509_gmtime_adj(X509_getm_notBefore(x.get()),-60); X509_gmtime_adj(X509_getm_notAfter(x.get()),86400);
 X509_set_pubkey(x.get(),p.get()); auto name=X509_get_subject_name(x.get());
 X509_NAME_add_entry_by_txt(name,"CN",MBSTRING_ASC,reinterpret_cast<const unsigned char*>("FreeMotra LAN"),-1,-1,0);
 X509_set_issuer_name(x.get(),name);
 if(X509_sign(x.get(),p.get(),EVP_sha256())<=0) throw std::runtime_error("TLS certificate signing failed");
 auto cb=std::unique_ptr<BIO,decltype(&BIO_free)>(BIO_new(BIO_s_mem()),BIO_free);
 auto kb=std::unique_ptr<BIO,decltype(&BIO_free)>(BIO_new(BIO_s_mem()),BIO_free);
 if(!cb||!kb||!PEM_write_bio_X509(cb.get(),x.get())||!PEM_write_bio_PrivateKey(kb.get(),p.get(),nullptr,nullptr,0,nullptr,nullptr)) throw std::runtime_error("TLS serialization failed");
 char *data=nullptr; long len=BIO_get_mem_data(cb.get(),&data); cert=QSslCertificate(QByteArray(data,int(len)));
 len=BIO_get_mem_data(kb.get(),&data); key=QSslKey(QByteArray(data,int(len)),QSsl::Rsa);
 if(cert.isNull()||key.isNull()) throw std::runtime_error("TLS certificate import failed");
}
LanLink::LanLink(QObject* p):QObject(p),server_(std::make_unique<LanServer>(this)) {
 deadline_.setSingleShot(true); connect(&deadline_,&QTimer::timeout,this,[this]{fail("Oturum isteğinin süresi doldu / Request timed out");});
 watchdog_.setInterval(1000); connect(&watchdog_,&QTimer::timeout,this,[this]{
  if(socket_ && socket_->state()!=QAbstractSocket::ConnectedState) fail("Bağlantı kesildi / Disconnected");
 });
}
LanLink::~LanLink(){stop();}
bool LanLink::privateAddress(const QHostAddress& a) {
 bool ok=false; quint32 ip=a.toIPv4Address(&ok);
 return ok && ((ip>>24)==10 || (ip>>20)==0xac1 || (ip>>16)==0xc0a8 || (ip>>24)==127);
}
QString LanLink::address() {
 for(const auto& i:QNetworkInterface::allInterfaces()) {
  if(!(i.flags()&QNetworkInterface::IsUp)||i.flags()&QNetworkInterface::IsLoopBack) continue;
  for(const auto& a:i.addressEntries()) if(privateAddress(a.ip())) return a.ip().toString();
 }
 return "127.0.0.1";
}
QString LanLink::listen() {
 stop(); host_=true;
 try {certificate(cert_,key_);} catch(const std::exception&){fail("TLS sertifikası oluşturulamadı / TLS initialization failed");return {};}
 if(!QSslSocket::supportsSsl() || !server_->listen(QHostAddress::AnyIPv4,0)) {fail("LAN dinleyicisi açılamadı / Cannot listen");return {};}
 emit status("LAN daveti hazır. Yalnızca özel ağda paylaşın / LAN invitation ready");
 return QString("fm2://%1:%2/%3").arg(address()).arg(server_->serverPort()).arg(QString::fromLatin1(cert_.digest(QCryptographicHash::Sha256).toHex()));
}
void LanLink::incoming(qintptr fd) {
 auto s=new QSslSocket(this);
 if(!s->setSocketDescriptor(fd)){s->deleteLater();return;}
 if(socket_ || !privateAddress(s->peerAddress()) || QDateTime::currentMSecsSinceEpoch()<lockedUntil_) {s->abort();s->deleteLater();return;}
 auto config=QSslConfiguration::defaultConfiguration(); config.setLocalCertificate(cert_);config.setPrivateKey(key_);
 config.setPeerVerifyMode(QSslSocket::VerifyNone);config.setProtocol(QSsl::TlsV1_2OrLater);
 s->setSslConfiguration(config);attach(s);s->startServerEncryption();deadline_.start(15000);
}
void LanLink::dial(const QString& invite,const QString& password,const QString& name) {
 stop();host_=false;
 const auto match=QRegularExpression("^fm2://([0-9.]{7,15}):([0-9]{1,5})/([0-9a-f]{64})$").match(invite.trimmed());
 QHostAddress ip(match.captured(1)); bool ok=false; auto port=match.captured(2).toUShort(&ok);
 if(!match.hasMatch()||!privateAddress(ip)||!ok||port==0||password.isEmpty()||password.size()>128){fail("Geçerli LAN daveti ve şifre gerekli / Invalid LAN invitation or password");return;}
 pin_=QByteArray::fromHex(match.captured(3).toLatin1());auto s=new QSslSocket(this);attach(s);
 s->setProtocol(QSsl::TlsV1_2OrLater);s->setPeerVerifyMode(QSslSocket::VerifyPeer);
 connect(s,&QSslSocket::sslErrors,this,[this,s](const QList<QSslError>& errors){
  if(s!=socket_)return;
  if(s->peerCertificate().digest(QCryptographicHash::Sha256)!=pin_){fail("Sertifika kimliği uyuşmuyor / Certificate pin mismatch");return;}
  for(const auto& e:errors) if(e.error()!=QSslError::SelfSignedCertificate && e.error()!=QSslError::HostNameMismatch && e.error()!=QSslError::CertificateUntrusted){fail("TLS sertifikası reddedildi / TLS certificate rejected");return;}
  s->ignoreSslErrors(errors); // Only the exact out-of-band invitation certificate.
 });
 connect(s,&QSslSocket::encrypted,this,[this,s,password,name]{
  if(s!=socket_||s->peerCertificate().digest(QCryptographicHash::Sha256)!=pin_){fail("TLS kimliği doğrulanamadı / TLS pin verification failed");return;}
  signal({{"type","auth"},{"password",password},{"name",name.left(60)}});
  emit status("Karşı tarafın onayı bekleniyor / Waiting for owner approval");deadline_.start(65000);
 });
 s->connectToHostEncrypted(ip.toString(),port);deadline_.start(15000);
}
void LanLink::attach(QSslSocket* s) {
 socket_=s;s->setReadBufferSize(262144);buffer_.clear();
 connect(s,&QSslSocket::readyRead,this,&LanLink::read);
 connect(s,&QSslSocket::disconnected,this,[this,s]{if(socket_==s)fail("Oturum sonlandı / Session ended");});
 connect(s,&QSslSocket::errorOccurred,this,[this,s](QAbstractSocket::SocketError){if(socket_==s)fail("Ağ hatası / Network error");});
 watchdog_.start();
}
void LanLink::read() {
 if(!socket_||!socket_->isEncrypted())return;
 buffer_+=socket_->readAll();
 if(buffer_.size()>262144){fail("Signaling boyut sınırı / Signaling limit");return;}
 while(socket_){auto n=buffer_.indexOf('\n');if(n<0)break;
  auto line=buffer_.left(n);buffer_.remove(0,n+1);
  QJsonParseError error;auto doc=QJsonDocument::fromJson(line,&error);
  if(error.error!=QJsonParseError::NoError||!doc.isObject()){fail("Geçersiz protokol / Invalid protocol");return;}
  try {received(doc.object());}catch(const std::exception&){fail("WebRTC protokol hatası / WebRTC protocol error");return;}
 }
}
void LanLink::signal(const QJsonObject& obj) {
 if(socket_&&socket_->isEncrypted()&&socket_->bytesToWrite()<262144)socket_->write(QJsonDocument(obj).toJson(QJsonDocument::Compact)+'\n');
}
void LanLink::received(const QJsonObject& o) {
 const auto type=o["type"].toString();
 if(type=="auth" && host_ && !authenticated_ && !approved_) {
  if(!verifyPassword || !verifyPassword(o["password"].toString())) {
   if(++failures_>=5){lockedUntil_=QDateTime::currentMSecsSinceEpoch()+60000;failures_=0;}
   fail("Şifre reddedildi veya süre doldu / Password rejected or expired");return;
  }
  authenticated_=true;deadline_.start(60000);
  // The name is explicitly unverified. Approval displays the actual network address.
  emit request(o["name"].toString().left(60)+" [beyan / unverified] · "+socket_->peerAddress().toString());return;
 }
 if(type=="accepted"&&!host_&&!approved_){authenticated_=approved_=true;control_=o["control"].toBool();createPeer(true);return;}
 if(!approved_||!authenticated_) {fail("Yetkisiz signaling / Unauthorized signaling");return;}
 if(type=="sdp") {
  const auto kind=o["kind"].toString();
  if(kind!=(host_?"offer":"answer")){fail("Beklenmeyen SDP / Unexpected SDP");return;}
  if(!peer_)createPeer(false);
  peer_->setRemoteDescription(rtc::Description(o["sdp"].toString().toStdString(),kind.toStdString()));
 } else if(type=="ice"&&peer_) peer_->addRemoteCandidate(rtc::Candidate(o["candidate"].toString().toStdString(),o["mid"].toString().toStdString()));
 else if(type=="stop")stop();
 else {fail("Beklenmeyen mesaj / Unexpected message");}
}
void LanLink::approve(bool control) {
 if(!host_||!authenticated_||approved_||!socket_)return;
 approved_=true;control_=control;signal({{"type","accepted"},{"control",control}});deadline_.start(20000);
 emit status("WebRTC bağlantısı kuruluyor / Establishing WebRTC");
}
void LanLink::createPeer(bool offer) {
 rtc::Configuration config;config.disableAutoNegotiation=false; // LAN only; no third-party ICE servers.
 peer_=std::make_shared<rtc::PeerConnection>(config);
 const auto e=epoch_;QPointer<LanLink> guard(this);
 peer_->onLocalDescription([guard,e](rtc::Description d){if(guard)QMetaObject::invokeMethod(guard,[guard,e,d]{if(guard&&guard->epoch_==e)guard->signal({{"type","sdp"},{"kind",QString::fromStdString(d.typeString())},{"sdp",QString::fromStdString(std::string(d))}});},Qt::QueuedConnection);});
 peer_->onLocalCandidate([guard,e](rtc::Candidate c){if(guard)QMetaObject::invokeMethod(guard,[guard,e,c]{if(guard&&guard->epoch_==e)guard->signal({{"type","ice"},{"candidate",QString::fromStdString(std::string(c))},{"mid",QString::fromStdString(c.mid())}});},Qt::QueuedConnection);});
 peer_->onStateChange([guard,e](rtc::PeerConnection::State state){if(guard)QMetaObject::invokeMethod(guard,[guard,e,state]{if(guard&&guard->epoch_==e&&(state==rtc::PeerConnection::State::Failed||state==rtc::PeerConnection::State::Closed||state==rtc::PeerConnection::State::Disconnected))guard->fail("WebRTC bağlantısı kesildi / WebRTC disconnected");},Qt::QueuedConnection);});
 peer_->onDataChannel([guard,e](auto dc){if(guard)QMetaObject::invokeMethod(guard,[guard,e,dc]{if(guard&&guard->epoch_==e)guard->channel(dc);},Qt::QueuedConnection);});
 if(offer)channel(peer_->createDataChannel("freemotra-session-v2"));
}
void LanLink::channel(std::shared_ptr<rtc::DataChannel> dc) {
 if(channel_||dc->label()!="freemotra-session-v2"){dc->close();return;}channel_=dc;
 const auto e=epoch_;QPointer<LanLink> g(this);
 dc->onOpen([g,e]{if(g)QMetaObject::invokeMethod(g,[g,e]{if(g&&g->epoch_==e&&g->approved_){g->active_=true;g->deadline_.stop();emit g->opened();emit g->status("WebRTC DTLS bağlantısı açık / WebRTC DTLS connected");}},Qt::QueuedConnection);});
 dc->onMessage([g,e](rtc::message_variant data){if(!g)return;
  auto bin=std::get_if<rtc::binary>(&data);if(!bin||bin->size()>20000)return;
  const QByteArray bytes(reinterpret_cast<const char*>(bin->data()),qsizetype(bin->size()));
  QMetaObject::invokeMethod(g,[g,e,bytes]{if(g&&g->epoch_==e&&g->active_)emit g->message(bytes);},Qt::QueuedConnection);
 });
}
void LanLink::send(const QByteArray& data) {
 if(!active_||!channel_||!channel_->isOpen()||data.size()>20000)return;
 try{channel_->send(reinterpret_cast<const std::byte*>(data.constData()),size_t(data.size()));}catch(const std::exception&){fail("Veri kanalı hatası / Data channel error");}
}
quint64 LanLink::queuedBytes()const{return channel_?quint64(channel_->bufferedAmount()):0;}
void LanLink::fail(const QString& why){stop();emit status(why);}
void LanLink::stop() {
 if(stopping_)return;stopping_=true;++epoch_;deadline_.stop();watchdog_.stop();server_->close();
 active_=false;authenticated_=approved_=control_=false;
 if(socket_){auto s=socket_;socket_=nullptr;s->disconnect(this);s->abort();s->deleteLater();}
 if(channel_){channel_->resetCallbacks();channel_->close();channel_.reset();}
 if(peer_){peer_->resetCallbacks();peer_->close();peer_.reset();}
 buffer_.clear();pin_.clear();stopping_=false;emit ended();
}

QString LanLink::sessionBinding() const {
 return socket_ && socket_->isEncrypted() ? QString::fromLatin1(QCryptographicHash::hash(socket_->sslConfiguration().sessionTicket()+socket_->peerAddress().toString().toUtf8()+cert_.digest(QCryptographicHash::Sha256),QCryptographicHash::Sha256).toHex()):QString();
}
