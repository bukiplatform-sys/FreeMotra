#pragma once
#include <QImage>
#include <QByteArray>
#include <QString>
#include <mfapi.h>
#include <mfidl.h>
#include <mftransform.h>
#include <wrl/client.h>
namespace fm {
class VideoCodec {
 using Transform=Microsoft::WRL::ComPtr<IMFTransform>;
 Transform encoder_,decoder_;
 Microsoft::WRL::ComPtr<IMFMediaEventGenerator> events_;
 HMODULE mfModule_=nullptr;
 bool mf_=false,com_=false,async_=false,needInput_=false,hardware_=false,failed_=false;
 int outputs_=0,width_=0,height_=0,decodeWidth_=0,decodeHeight_=0;
 DWORD encoderInput_=0,encoderOutput_=0;
 LONGLONG time_=0,decodeTime_=0;QImage pending_;
 void events();bool encoder(int width,int height,int bitrate);
public:
 VideoCodec();~VideoCodec();
 VideoCodec(const VideoCodec&)=delete;VideoCodec& operator=(const VideoCodec&)=delete;
 bool decoderAvailable()const;
 QByteArray encode(const QImage& image,int bitrate);
 QByteArray poll();
 QImage decode(const QByteArray& bytes,int width,int height);
 void reset();
 bool failed()const{return failed_;}
 bool hardware()const{return hardware_;}
 bool encoding()const{return encoder_.Get()!=nullptr;}
 int width()const{return width_;}int height()const{return height_;}
};
}
