#pragma once
#include "LanLink.h"
#include "VoiceCodec.h"
#include <QAudioSource>
#include <QAudioSink>
#include <QFile>
#include <QSettings>
#include <QUrl>
#include <QVariantList>
#include <QTimer>
#include <memory>

// Session capabilities are enforced here for both incoming and outgoing traffic.
class SessionTools final : public QObject {
 Q_OBJECT
 Q_PROPERTY(QVariantList messages READ messages NOTIFY changed)
 Q_PROPERTY(QString pendingFile READ pendingFile NOTIFY changed)
 Q_PROPERTY(QString transferStatus READ transferStatus NOTIFY changed)
 Q_PROPERTY(double progress READ progress NOTIFY changed)
 Q_PROPERTY(bool microphone READ microphone NOTIFY changed)
 Q_PROPERTY(bool listening READ listening NOTIFY changed)
 Q_PROPERTY(QString audioStatus READ audioStatus NOTIFY changed)
 Q_PROPERTY(bool filesAllowed READ filesAllowed NOTIFY changed)
 Q_PROPERTY(bool chatAllowed READ chatAllowed NOTIFY changed)
 Q_PROPERTY(bool voiceAllowed READ voiceAllowed NOTIFY changed)
public:
 explicit SessionTools(LanLink& link,QObject* parent=nullptr);
 ~SessionTools() override;
 QVariantList messages() const { return messages_; }
 QString pendingFile() const { return incomingName_; }
 QString transferStatus() const { return transferStatus_; }
 double progress() const { return progress_; }
 bool microphone() const { return source_ != nullptr; }
 bool listening() const { return sink_ != nullptr; }
 QString audioStatus() const { return audioStatus_; }
 bool filesAllowed() const { return link_.active() && link_.filesAllowed(); }
 bool chatAllowed() const { return link_.active() && link_.chatAllowed(); }
 bool voiceAllowed() const { return link_.active() && link_.voiceAllowed(); }
 Q_INVOKABLE void sendChat(const QString& text);
 Q_INVOKABLE void sendFile(const QUrl& url);
 Q_INVOKABLE void acceptFile(const QUrl& directory);
 Q_INVOKABLE void rejectFile();
 Q_INVOKABLE void cancelTransfer();
 Q_INVOKABLE void setMicrophone(bool enabled);
 Q_INVOKABLE void setListening(bool enabled);
 void receive(const QByteArray& data);
signals:
 void changed();
private:
 void sendJson(const QJsonObject& value);
 void nextChunk(qint64 offset);
 void reset();
 void addChat(const QString& text,bool local);
 void stopAudio();
 fm::VoiceCodec codec_;
 LanLink& link_;
 QVariantList messages_;
 QFile outgoing_,incoming_;
 QString outgoingId_,incomingId_,incomingName_,incomingHash_,finalPath_,transferStatus_,audioStatus_;
 qint64 outgoingSize_=0,incomingSize_=0,ackOffset_=0;
 bool awaitingAck_=false, verifying_=false;
 double progress_=0;
 QTimer fileDeadline_,audioTimer_;
 std::unique_ptr<QAudioSource> source_;
 std::unique_ptr<QAudioSink> sink_;
 QIODevice *audioInput_=nullptr,*audioOutput_=nullptr;
 QByteArray pcm_;
};
