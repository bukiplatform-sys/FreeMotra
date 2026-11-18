#include "SessionTools.h"
#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QMediaDevices>
#include <QAudioDevice>
#include <QUuid>
#include <QRegularExpression>
#include <QFutureWatcher>
#include <QtConcurrentRun>
namespace {
QString digest(QFile& file) {
 const auto pos=file.pos();file.seek(0);
 QCryptographicHash hash(QCryptographicHash::Sha256);
 const bool ok=hash.addData(&file);file.seek(pos);
 return ok?QString::fromLatin1(hash.result().toHex()):QString();
}
QAudioFormat voiceFormat() {
 QAudioFormat f;f.setSampleRate(16000);f.setChannelCount(1);f.setSampleFormat(QAudioFormat::Int16);return f;
}
}
SessionTools::SessionTools(LanLink& link,QObject* parent):QObject(parent),link_(link) {
 fileDeadline_.setSingleShot(true);fileDeadline_.setInterval(30000);
 connect(&fileDeadline_,&QTimer::timeout,this,[this]{cancelTransfer();transferStatus_="Aktarım zaman aşımı; aynı dosyayı yeniden seçerek devam edin / Transfer timed out; select the same file to resume";emit changed();});
 connect(&link_,&LanLink::ended,this,&SessionTools::reset);
 connect(&link_,&LanLink::opened,this,[this]{messages_.clear();emit changed();});
 audioTimer_.setInterval(20);
 connect(&audioTimer_,&QTimer::timeout,this,[this]{
  if(!audioInput_||!voiceAllowed())return;
  pcm_+=audioInput_->readAll();
  if(pcm_.size()>6400)pcm_=pcm_.right(640); // Drop stale audio rather than accumulating latency.
  while(pcm_.size()>=640){const auto packet=pcm_.first(640);pcm_.remove(0,640);if(link_.voiceQueuedBytes()<4000){const auto encoded=codec_.encode(packet);if(!encoded.isEmpty())link_.send("O"+encoded);}}
  if(source_&&source_->error()!=QAudio::NoError){setMicrophone(false);audioStatus_="Mikrofon hatası / Microphone error";emit changed();}
 });
}
SessionTools::~SessionTools(){stopAudio();}
void SessionTools::sendJson(const QJsonObject& o){link_.send("T"+QJsonDocument(o).toJson(QJsonDocument::Compact));}
void SessionTools::addChat(const QString& text,bool local){
 messages_.append(QVariantMap{{"text",text},{"local",local},{"time",QDateTime::currentDateTime().toString("HH:mm")}});
 if(messages_.size()>200)messages_.removeFirst();emit changed();
}
void SessionTools::sendChat(const QString& text){
 const auto clean=text.trimmed();if(!chatAllowed()||clean.isEmpty()||clean.size()>2000)return;
 link_.send("C"+clean.toUtf8());addChat(clean,true);
}
void SessionTools::sendFile(const QUrl& url){
 if(!filesAllowed()||!outgoingId_.isEmpty())return;
 const auto path=url.toLocalFile();QFileInfo info(path);
 if(!info.isFile()||info.isSymLink()){transferStatus_="Normal bir dosya seçin / Select a regular file";emit changed();return;}
 outgoing_.setFileName(path);
 if(!outgoing_.open(QIODevice::ReadOnly)){transferStatus_=outgoing_.errorString();emit changed();return;}
 outgoingSize_=outgoing_.size();
 // Hash on a worker so large files do not stall video, audio or the heartbeat.
 outgoingId_=QUuid::createUuid().toString(QUuid::WithoutBraces);progress_=0;awaitingAck_=false;
 const auto id=outgoingId_,name=info.fileName();
 transferStatus_="Dosya hazırlanıyor / Preparing file";emit changed();
 auto* watcher=new QFutureWatcher<QString>(this);
 connect(watcher,&QFutureWatcher<QString>::finished,this,[this,watcher,id,name]{
  const auto hash=watcher->result();watcher->deleteLater();
  if(id!=outgoingId_||!filesAllowed())return;
  if(hash.isEmpty()){outgoing_.close();outgoingId_.clear();transferStatus_="Dosya okunamadı / Cannot read file";emit changed();return;}
  sendJson({{"op","offer"},{"id",id},{"name",name},{"size",QString::number(outgoingSize_)},{"hash",hash}});
  transferStatus_="Dosya kabulü bekleniyor / Waiting for file acceptance";fileDeadline_.start(30000);emit changed();
 });
 watcher->setFuture(QtConcurrent::run([path]{QFile file(path);return file.open(QIODevice::ReadOnly)?digest(file):QString();}));
}
void SessionTools::acceptFile(const QUrl& directory){
 if(!filesAllowed()||incomingId_.isEmpty()||incoming_.isOpen()||verifying_||!directory.isLocalFile())return;
 QDir dir(directory.toLocalFile());if(!dir.exists())return;
 // Receiver chooses a directory; remote names can never escape it. Never overwrite an existing file.
 finalPath_=dir.absoluteFilePath(incomingName_);
 const auto partial=dir.absoluteFilePath(".freemotra-"+incomingHash_+".part");
 if(QFileInfo::exists(finalPath_)||QFileInfo(partial).isSymLink()){
  transferStatus_="Hedef dosya mevcut veya güvenli değil; başka klasör seçin / Destination exists or is unsafe; choose another folder";emit changed();return;
 }
 incoming_.setFileName(partial);
 if(!incoming_.open(QIODevice::ReadWrite)){transferStatus_=incoming_.errorString();emit changed();return;}
 if(incoming_.size()>incomingSize_)incoming_.resize(0);
 incoming_.seek(incoming_.size());
 transferStatus_="Dosya alınıyor / Receiving file";
 sendJson({{"op","ready"},{"id",incomingId_},{"offset",QString::number(incoming_.pos())}});fileDeadline_.start(30000);emit changed();
}
void SessionTools::rejectFile(){
 if(!incomingId_.isEmpty())sendJson({{"op","cancel"},{"id",incomingId_}});
 incoming_.close();verifying_=false;incomingId_.clear();incomingName_.clear();if(outgoingId_.isEmpty())fileDeadline_.stop();emit changed();
}
void SessionTools::cancelTransfer(){
 rejectFile();if(!outgoingId_.isEmpty())sendJson({{"op","cancel"},{"id",outgoingId_}});
 outgoing_.close();outgoingId_.clear();awaitingAck_=false;fileDeadline_.stop();
 transferStatus_="Aktarım durduruldu; kısmi dosya devam etmek için saklandı / Transfer stopped; partial file kept for resume";emit changed();
}
void SessionTools::nextChunk(qint64 offset){
 if(!outgoing_.isOpen()||offset<0||offset>outgoingSize_)return;
 outgoing_.seek(offset);
 if(offset==outgoingSize_){sendJson({{"op","finish"},{"id",outgoingId_}});awaitingAck_=false;fileDeadline_.start(30000);return;}
 const auto bytes=outgoing_.read(12000);
 if(bytes.isEmpty()){cancelTransfer();return;}
 ackOffset_=offset+bytes.size();awaitingAck_=true;
 sendJson({{"op","chunk"},{"id",outgoingId_},{"offset",QString::number(offset)},{"data",QString::fromLatin1(bytes.toBase64())}});
 progress_=outgoingSize_?double(offset)/double(outgoingSize_):1;
 transferStatus_="Dosya gönderiliyor / Sending file";fileDeadline_.start(30000);emit changed();
}
void SessionTools::receive(const QByteArray& data){
 if(!link_.active()||data.isEmpty())return;
 if(data[0]=='C'&&chatAllowed()&&data.size()<=8001){auto text=QString::fromUtf8(data.mid(1));if(text.size()<=2000)addChat(text,false);return;}
 if(data[0]=='O'&&voiceAllowed()){
  if(!sink_||!audioOutput_)return;const auto pcm=codec_.decode(data.mid(1));
  if(pcm.size()==640&&sink_->bytesFree()>=640){if(audioOutput_->write(pcm)==640)codec_.playback(pcm);}return;
 }
 if(data[0]=='V'&&voiceAllowed()){
  if(data.size()!=641||!sink_||!audioOutput_)return;
  if(sink_->bytesFree()>=640)audioOutput_->write(data.constData()+1,640);return;
 }
 if(data[0]!='T'||!filesAllowed()||data.size()>18000)return;
 const auto o=QJsonDocument::fromJson(data.mid(1)).object();const auto op=o["op"].toString(),id=o["id"].toString();
 if(!QRegularExpression("^[0-9a-f-]{36}$").match(id).hasMatch())return;
 if(op=="offer"){
  if(!incomingId_.isEmpty()){sendJson({{"op","cancel"},{"id",id}});return;}
  const auto name=o["name"].toString(),hash=o["hash"].toString();bool ok=false;auto size=o["size"].toString().toLongLong(&ok);
  // Conservative portable names also exclude NTFS ADS, reserved names, trailing dots and path traversal.
  const auto stem=name.section('.',0,0).toUpper();
  if(!ok||size<0||size>qint64(1024)*1024*1024*1024||name.isEmpty()||name.size()>180||name=="."||name==".."||name.endsWith('.')||name.endsWith(' ')||
   name.contains(QRegularExpression("[\\\\/:*?\"<>|\\x00-\\x1f]"))||QRegularExpression("^(CON|PRN|AUX|NUL|COM[0-9]|LPT[0-9])$").match(stem).hasMatch()||!QRegularExpression("^[0-9a-f]{64}$").match(hash).hasMatch()){
   sendJson({{"op","cancel"},{"id",id}});return;
  }
  incomingId_=id;incomingName_=name;incomingSize_=size;incomingHash_=hash;transferStatus_="Gelen dosya için klasör seçin / Choose a folder for the incoming file";emit changed();return;
 }
 if(op=="cancel"){
  if(id==incomingId_){incoming_.close();verifying_=false;incomingId_.clear();incomingName_.clear();}
  if(id==outgoingId_){outgoing_.close();outgoingId_.clear();awaitingAck_=false;}
  if(incomingId_.isEmpty()&&outgoingId_.isEmpty())fileDeadline_.stop();transferStatus_="Karşı taraf aktarımı durdurdu / Peer cancelled transfer";emit changed();return;
 }
 bool validOffset=false;const auto offset=o["offset"].toString().toLongLong(&validOffset);
 if(op=="ready"&&id==outgoingId_&&outgoing_.isOpen()&&!awaitingAck_){if(validOffset&&offset>=0&&offset<=outgoingSize_)nextChunk(offset);return;}
 if(op=="ack"&&id==outgoingId_&&awaitingAck_){if(validOffset&&offset==ackOffset_)nextChunk(offset);return;}
 if(op=="verifying"&&id==outgoingId_&&!awaitingAck_&&outgoing_.isOpen()&&outgoing_.pos()==outgoingSize_){fileDeadline_.start(600000);return;}
 if(op=="complete"&&id==outgoingId_&&!awaitingAck_&&outgoing_.isOpen()&&outgoing_.pos()==outgoingSize_){outgoing_.close();outgoingId_.clear();if(incomingId_.isEmpty())fileDeadline_.stop();progress_=1;transferStatus_="Dosya karşı tarafta doğrulandı / File verified by receiver";emit changed();return;}
 if(id!=incomingId_||!incoming_.isOpen())return;
 if(op=="chunk"){
  const auto chunk=QByteArray::fromBase64(o["data"].toString().toLatin1(),QByteArray::AbortOnBase64DecodingErrors);
  if(!validOffset||offset!=incoming_.pos()||chunk.isEmpty()||chunk.size()>12000||offset+chunk.size()>incomingSize_) {rejectFile();return;}
  if(incoming_.write(chunk)!=chunk.size()||!incoming_.flush()){rejectFile();return;}
  sendJson({{"op","ack"},{"id",id},{"offset",QString::number(incoming_.pos())}});
  progress_=incomingSize_?double(incoming_.pos())/double(incomingSize_):1;fileDeadline_.start(30000);emit changed();return;
 }
 if(op=="finish"){
  if(incoming_.size()!=incomingSize_){rejectFile();return;}
  incoming_.flush();incoming_.close();verifying_=true;
  const auto path=incoming_.fileName();
  sendJson({{"op","verifying"},{"id",id}});fileDeadline_.start(600000);
  transferStatus_="Dosya bütünlüğü doğrulanıyor / Verifying file integrity";emit changed();
  auto* watcher=new QFutureWatcher<QString>(this);
  connect(watcher,&QFutureWatcher<QString>::finished,this,[this,watcher,id]{
   const auto hash=watcher->result();watcher->deleteLater();
   if(id!=incomingId_||!filesAllowed())return;
   verifying_=false;
   if(hash!=incomingHash_){incoming_.remove();rejectFile();transferStatus_="Dosya doğrulaması başarısız; yeniden gönderin / File verification failed; resend";emit changed();return;}
   if(!incoming_.rename(finalPath_)){rejectFile();transferStatus_="Dosya kaydedilemedi / Cannot publish received file";emit changed();return;}
   sendJson({{"op","complete"},{"id",id}});incomingId_.clear();incomingName_.clear();if(outgoingId_.isEmpty())fileDeadline_.stop();progress_=1;transferStatus_="Dosya alındı ve doğrulandı / File received and verified";emit changed();
  });
  watcher->setFuture(QtConcurrent::run([path]{QFile file(path);return file.open(QIODevice::ReadOnly)?digest(file):QString();}));
 }
}
void SessionTools::setMicrophone(bool enabled){
 if(!enabled){audioTimer_.stop();audioInput_=nullptr;if(source_)source_->stop();source_.reset();pcm_.clear();emit changed();return;}
 if(!voiceAllowed()||source_)return;
 auto device=QMediaDevices::defaultAudioInput();auto format=voiceFormat();
 if(device.isNull()||!device.isFormatSupported(format)){audioStatus_="16 kHz mono mikrofon desteklenmiyor / 16 kHz mono microphone unavailable";emit changed();return;}
 source_=std::make_unique<QAudioSource>(device,format);source_->setBufferSize(1280);audioInput_=source_->start();
 if(!audioInput_||source_->error()!=QAudio::NoError){source_.reset();audioInput_=nullptr;audioStatus_="Mikrofon açılamadı / Cannot open microphone";emit changed();return;}
 audioStatus_="Mikrofon açık; kulaklık önerilir / Microphone on; headphones recommended";audioTimer_.start();emit changed();
}
void SessionTools::setListening(bool enabled){
 if(!enabled){audioOutput_=nullptr;if(sink_)sink_->stop();sink_.reset();emit changed();return;}
 if(!voiceAllowed()||sink_)return;
 auto device=QMediaDevices::defaultAudioOutput();auto format=voiceFormat();
 if(device.isNull()||!device.isFormatSupported(format)){audioStatus_="16 kHz ses çıkışı desteklenmiyor / 16 kHz audio output unavailable";emit changed();return;}
 sink_=std::make_unique<QAudioSink>(device,format);sink_->setBufferSize(1280);audioOutput_=sink_->start();
 if(!audioOutput_||sink_->error()!=QAudio::NoError){sink_.reset();audioOutput_=nullptr;audioStatus_="Ses çıkışı açılamadı / Cannot open audio output";}
 emit changed();
}
void SessionTools::stopAudio(){setMicrophone(false);setListening(false);}
void SessionTools::reset(){
 stopAudio();fileDeadline_.stop();outgoing_.close();incoming_.close();outgoingId_.clear();incomingId_.clear();incomingName_.clear();awaitingAck_=verifying_=false;messages_.clear();codec_.reset();progress_=0;
 transferStatus_="Bağlantı kapalı; yarım dosya yeniden kabul edilince devam eder / Disconnected; accept the same file again to resume";audioStatus_.clear();emit changed();
}
