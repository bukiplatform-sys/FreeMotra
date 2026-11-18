#pragma once
#include <QByteArray>
#include <opus/opus.h>
#include <speex/speex_echo.h>
#include <speex/speex_preprocess.h>
#include <array>
#include <cstring>
namespace fm {
class VoiceCodec {
 OpusEncoder* encoder_=nullptr;OpusDecoder* decoder_=nullptr;
 bool playbackSeen_=false;
 SpeexEchoState* echo_=nullptr;SpeexPreprocessState* preprocess_=nullptr;
public:
 VoiceCodec(){
  int error=0;encoder_=opus_encoder_create(16000,1,OPUS_APPLICATION_VOIP,&error);decoder_=opus_decoder_create(16000,1,&error);
  if(encoder_){opus_encoder_ctl(encoder_,OPUS_SET_BITRATE(24000));opus_encoder_ctl(encoder_,OPUS_SET_COMPLEXITY(5));opus_encoder_ctl(encoder_,OPUS_SET_INBAND_FEC(1));opus_encoder_ctl(encoder_,OPUS_SET_PACKET_LOSS_PERC(5));}
  echo_=speex_echo_state_init(320,3200);int rate=16000;if(echo_)speex_echo_ctl(echo_,SPEEX_ECHO_SET_SAMPLING_RATE,&rate);
  preprocess_=speex_preprocess_state_init(320,16000);if(preprocess_){int on=1,noise=-25;speex_preprocess_ctl(preprocess_,SPEEX_PREPROCESS_SET_DENOISE,&on);speex_preprocess_ctl(preprocess_,SPEEX_PREPROCESS_SET_NOISE_SUPPRESS,&noise);if(echo_)speex_preprocess_ctl(preprocess_,SPEEX_PREPROCESS_SET_ECHO_STATE,echo_);}
 }
 ~VoiceCodec(){if(preprocess_)speex_preprocess_state_destroy(preprocess_);if(echo_)speex_echo_state_destroy(echo_);if(encoder_)opus_encoder_destroy(encoder_);if(decoder_)opus_decoder_destroy(decoder_);}
 VoiceCodec(const VoiceCodec&)=delete;VoiceCodec& operator=(const VoiceCodec&)=delete;
 QByteArray encode(const QByteArray& pcm,bool acousticEcho=false){
  if(!encoder_||pcm.size()!=640)return {};std::array<opus_int16,320> input{},clean{};std::memcpy(input.data(),pcm.constData(),640);
  if(echo_&&acousticEcho&&playbackSeen_)speex_echo_capture(echo_,input.data(),clean.data());else clean=input;
  if(preprocess_)speex_preprocess_run(preprocess_,clean.data());QByteArray packet(1275,Qt::Uninitialized);
  const int n=opus_encode(encoder_,clean.data(),320,reinterpret_cast<unsigned char*>(packet.data()),packet.size());if(n<=0)return {};packet.resize(n);return packet;
 }
 QByteArray decode(const QByteArray& packet){
  if(!decoder_||packet.isEmpty()||packet.size()>1275)return {};std::array<opus_int16,320> pcm{};
  const int n=opus_decode(decoder_,reinterpret_cast<const unsigned char*>(packet.constData()),packet.size(),pcm.data(),320,0);if(n!=320)return {};return QByteArray(reinterpret_cast<const char*>(pcm.data()),640);
 }
 void playback(const QByteArray& pcm){if(pcm.size()!=640||!echo_)return;std::array<spx_int16_t,320> frame{};std::memcpy(frame.data(),pcm.constData(),640);speex_echo_playback(echo_,frame.data());playbackSeen_=true;}
 void reset(){playbackSeen_=false;if(encoder_)opus_encoder_ctl(encoder_,OPUS_RESET_STATE);if(decoder_)opus_decoder_ctl(decoder_,OPUS_RESET_STATE);if(echo_)speex_echo_state_reset(echo_);}
};
}
