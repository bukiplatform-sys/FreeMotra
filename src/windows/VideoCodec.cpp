#include "VideoCodec.h"
#include <wmcodecdsp.h>
#include <codecapi.h>
#include <mferror.h>
#include <algorithm>
#include <cstring>
namespace fm {
using Microsoft::WRL::ComPtr;
namespace {
ComPtr<IMFMediaType> type(GUID subtype,int width,int height){
 ComPtr<IMFMediaType> t;if(FAILED(MFCreateMediaType(&t)))return {};
 t->SetGUID(MF_MT_MAJOR_TYPE,MFMediaType_Video);t->SetGUID(MF_MT_SUBTYPE,subtype);t->SetUINT32(MF_MT_INTERLACE_MODE,MFVideoInterlace_Progressive);
 MFSetAttributeSize(t.Get(),MF_MT_FRAME_SIZE,UINT32(width),UINT32(height));MFSetAttributeRatio(t.Get(),MF_MT_FRAME_RATE,20,1);MFSetAttributeRatio(t.Get(),MF_MT_PIXEL_ASPECT_RATIO,1,1);return t;
}
ComPtr<IMFSample> sample(const QByteArray& bytes,LONGLONG time){
 ComPtr<IMFSample> s;ComPtr<IMFMediaBuffer> b;
 if(FAILED(MFCreateSample(&s))||FAILED(MFCreateMemoryBuffer(DWORD(bytes.size()),&b)))return {};
 BYTE* p=nullptr;if(FAILED(b->Lock(&p,nullptr,nullptr)))return {};std::memcpy(p,bytes.constData(),size_t(bytes.size()));b->Unlock();b->SetCurrentLength(DWORD(bytes.size()));s->AddBuffer(b.Get());s->SetSampleTime(time);s->SetSampleDuration(500000);return s;
}
HRESULT output(IMFTransform* transform,DWORD id,DWORD minimum,QByteArray& bytes){
 MFT_OUTPUT_STREAM_INFO info{};auto hr=transform->GetOutputStreamInfo(id,&info);if(FAILED(hr))return hr;
 ComPtr<IMFSample> supplied;ComPtr<IMFMediaBuffer> buffer;
 if(!(info.dwFlags&MFT_OUTPUT_STREAM_PROVIDES_SAMPLES)){
  const auto capacity=std::max(info.cbSize,minimum);if(capacity>8*1024*1024)return E_FAIL;
  if(FAILED(MFCreateSample(&supplied))||FAILED(MFCreateAlignedMemoryBuffer(capacity,info.cbAlignment,&buffer)))return E_FAIL;
  supplied->AddBuffer(buffer.Get());
 }
 MFT_OUTPUT_DATA_BUFFER data{};data.dwStreamID=id;data.pSample=supplied.Get();DWORD status=0;
 hr=transform->ProcessOutput(0,1,&data,&status);if(data.pEvents)data.pEvents->Release();
 ComPtr<IMFSample> owned;if(data.pSample&&data.pSample!=supplied.Get())owned.Attach(data.pSample);
 if(FAILED(hr)||!data.pSample)return hr;
 ComPtr<IMFMediaBuffer> contiguous;if(FAILED(data.pSample->ConvertToContiguousBuffer(&contiguous)))return E_FAIL;
 BYTE* p=nullptr;DWORD n=0;if(FAILED(contiguous->Lock(&p,nullptr,&n)))return E_FAIL;
 if(n>8*1024*1024){contiguous->Unlock();return E_FAIL;}bytes=QByteArray(reinterpret_cast<const char*>(p),n);contiguous->Unlock();return S_OK;
}
QByteArray nv12(const QImage& source){
 const auto image=source.convertToFormat(QImage::Format_ARGB32);const int w=image.width(),h=image.height();QByteArray out(w*h*3/2,Qt::Uninitialized);
 auto* y=reinterpret_cast<unsigned char*>(out.data());auto* uv=y+w*h;
 for(int row=0;row<h;++row){const auto* pixels=reinterpret_cast<const QRgb*>(image.constScanLine(row));for(int col=0;col<w;++col){const auto p=pixels[col];y[row*w+col]=static_cast<unsigned char>(std::clamp(((66*qRed(p)+129*qGreen(p)+25*qBlue(p)+128)>>8)+16,0,255));}}
 for(int row=0;row<h;row+=2)for(int col=0;col<w;col+=2){int r=0,g=0,b=0;for(int dy=0;dy<2;++dy){const auto* p=reinterpret_cast<const QRgb*>(image.constScanLine(row+dy));for(int dx=0;dx<2;++dx){r+=qRed(p[col+dx]);g+=qGreen(p[col+dx]);b+=qBlue(p[col+dx]);}}r/=4;g/=4;b/=4;const int i=(row/2)*w+col;uv[i]=static_cast<unsigned char>(std::clamp(((-38*r-74*g+112*b+128)>>8)+128,0,255));uv[i+1]=static_cast<unsigned char>(std::clamp(((112*r-94*g-18*b+128)>>8)+128,0,255));}
 return out;
}
}
VideoCodec::VideoCodec(){const auto hr=CoInitializeEx(nullptr,COINIT_MULTITHREADED);com_=SUCCEEDED(hr);mf_=SUCCEEDED(MFStartup(MF_VERSION,MFSTARTUP_LITE));}
VideoCodec::~VideoCodec(){reset();if(mf_)MFShutdown();if(com_)CoUninitialize();}
void VideoCodec::reset(){if(encoder_)encoder_->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH,0);if(decoder_)decoder_->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH,0);events_.Reset();encoder_.Reset();decoder_.Reset();async_=hardware_=failed_=false;needInput_=false;outputs_=width_=height_=decodeWidth_=decodeHeight_=0;time_=decodeTime_=0;pending_=QImage();}
bool VideoCodec::decoderAvailable()const{ComPtr<IMFTransform> d;return mf_&&SUCCEEDED(CoCreateInstance(CLSID_CMSH264DecoderMFT,nullptr,CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&d)));}
void VideoCodec::events(){
 if(!events_)return;for(int i=0;i<32;++i){ComPtr<IMFMediaEvent> event;if(FAILED(events_->GetEvent(MF_EVENT_FLAG_NO_WAIT,&event)))break;MediaEventType kind=MEUnknown;HRESULT status=S_OK;event->GetType(&kind);event->GetStatus(&status);if(FAILED(status)){failed_=true;return;}if(kind==METransformNeedInput)needInput_=true;if(kind==METransformHaveOutput)++outputs_;}
}
bool VideoCodec::encoder(int w,int h,int bitrate){
 encoder_.Reset();events_.Reset();width_=w;height_=h;time_=0;async_=hardware_=false;outputs_=0;needInput_=false;encoderInput_=encoderOutput_=0;
 auto configure=[&](IMFTransform* e){
  ComPtr<IMFAttributes> attrs;if(SUCCEEDED(e->GetAttributes(&attrs))){UINT32 a=0;attrs->GetUINT32(MF_TRANSFORM_ASYNC,&a);async_=a!=0;if(async_&&FAILED(attrs->SetUINT32(MF_TRANSFORM_ASYNC_UNLOCK,TRUE)))return false;attrs->SetUINT32(MF_LOW_LATENCY,TRUE);}
  ComPtr<ICodecAPI> codec;if(SUCCEEDED(e->QueryInterface(IID_PPV_ARGS(&codec)))){VARIANT value;VariantInit(&value);value.vt=VT_BOOL;value.boolVal=VARIANT_TRUE;codec->SetValue(&CODECAPI_AVLowLatencyMode,&value);value.vt=VT_UI4;value.ulVal=0;codec->SetValue(&CODECAPI_AVEncMPVDefaultBPictureCount,&value);value.ulVal=20;codec->SetValue(&CODECAPI_AVEncMPVGOPSize,&value);VariantClear(&value);}
  auto compressed=type(MFVideoFormat_H264,w,h),raw=type(MFVideoFormat_NV12,w,h);if(!compressed||!raw)return false;compressed->SetUINT32(MF_MT_AVG_BITRATE,UINT32(bitrate));compressed->SetUINT32(MF_MT_MPEG2_PROFILE,eAVEncH264VProfile_Base);
  raw->SetUINT32(MF_MT_DEFAULT_STRIDE,UINT32(w));raw->SetUINT32(MF_MT_SAMPLE_SIZE,UINT32(w*h*3/2));
  e->GetStreamIDs(1,&encoderInput_,1,&encoderOutput_);
  if(FAILED(e->SetOutputType(encoderOutput_,compressed.Get(),0))||FAILED(e->SetInputType(encoderInput_,raw.Get(),0)))return false;
  if(async_&&FAILED(e->QueryInterface(IID_PPV_ARGS(&events_))))return false;
  e->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING,0);e->ProcessMessage(MFT_MESSAGE_NOTIFY_START_OF_STREAM,0);return true;
 };
 MFT_REGISTER_TYPE_INFO input{MFMediaType_Video,MFVideoFormat_NV12},out{MFMediaType_Video,MFVideoFormat_H264};IMFActivate** activations=nullptr;UINT32 count=0;
 if(SUCCEEDED(MFTEnumEx(MFT_CATEGORY_VIDEO_ENCODER,MFT_ENUM_FLAG_HARDWARE|MFT_ENUM_FLAG_SORTANDFILTER,&input,&out,&activations,&count))){
  for(UINT32 i=0;i<count;++i){if(!encoder_){ComPtr<IMFTransform> candidate;if(SUCCEEDED(activations[i]->ActivateObject(IID_PPV_ARGS(&candidate)))&&configure(candidate.Get())){encoder_=candidate;hardware_=true;}else events_.Reset();}activations[i]->Release();}CoTaskMemFree(activations);
 }
 if(!encoder_){async_=false;events_.Reset();if(FAILED(CoCreateInstance(CLSID_CMSH264EncoderMFT,nullptr,CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&encoder_)))||!configure(encoder_.Get())){encoder_.Reset();return false;}}
 return true;
}
QByteArray VideoCodec::poll(){
 if(!encoder_||failed_)return {};events();QByteArray bytes;
 auto pull=[&]{if(async_&&outputs_<=0)return;const auto hr=output(encoder_.Get(),encoderOutput_,DWORD(std::max(width_*height_,262144)),bytes);if(async_&&outputs_>0)--outputs_;if(FAILED(hr)&&hr!=MF_E_TRANSFORM_NEED_MORE_INPUT)failed_=true;};
 pull();if(failed_)return {};
 if(!pending_.isNull()&&(!async_||needInput_)){
  const auto raw=nv12(pending_);auto s=sample(raw,time_);if(!s){failed_=true;return {};}
  const auto hr=encoder_->ProcessInput(encoderInput_,s.Get(),0);if(SUCCEEDED(hr)){time_+=500000;needInput_=false;pending_=QImage();}else if(hr!=MF_E_NOTACCEPTING){failed_=true;return {};}
 }
 if(bytes.isEmpty()){events();pull();}return bytes;
}
QByteArray VideoCodec::encode(const QImage& source,int bitrate){
 if(!mf_||source.isNull()||source.width()<2||source.height()<2||source.width()>1920||source.height()>1080)return {};
 const auto image=source.scaled(source.width()&~1,source.height()&~1,Qt::IgnoreAspectRatio,Qt::FastTransformation);
 if(!encoder_||image.width()!=width_||image.height()!=height_){if(!encoder(image.width(),image.height(),bitrate)){failed_=true;return {};}}
 pending_=image;return poll();
}
QImage VideoCodec::decode(const QByteArray& bytes,int w,int h){
 if(!mf_||bytes.isEmpty()||bytes.size()>2000000||w<2||h<2||w>1920||h>1080||(w&1)||(h&1))return {};
 if(!decoder_||decodeWidth_!=w||decodeHeight_!=h){decoder_.Reset();if(FAILED(CoCreateInstance(CLSID_CMSH264DecoderMFT,nullptr,CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&decoder_))))return {};ComPtr<IMFAttributes> attrs;if(SUCCEEDED(decoder_->GetAttributes(&attrs)))attrs->SetUINT32(MF_LOW_LATENCY,TRUE);auto input=type(MFVideoFormat_H264,w,h);if(!input||FAILED(decoder_->SetInputType(0,input.Get(),0))){decoder_.Reset();return {};}auto raw=type(MFVideoFormat_NV12,w,h);decoder_->SetOutputType(0,raw.Get(),0);decoder_->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING,0);decoder_->ProcessMessage(MFT_MESSAGE_NOTIFY_START_OF_STREAM,0);decodeWidth_=w;decodeHeight_=h;decodeTime_=0;}
 auto s=sample(bytes,decodeTime_);decodeTime_+=500000;if(!s||FAILED(decoder_->ProcessInput(0,s.Get(),0)))return {};QByteArray raw;auto hr=output(decoder_.Get(),0,DWORD(w*h*3/2),raw);
 if(hr==MF_E_TRANSFORM_STREAM_CHANGE){
  bool selected=false;for(DWORD i=0;i<32;++i){ComPtr<IMFMediaType> format;if(FAILED(decoder_->GetOutputAvailableType(0,i,&format)))break;GUID subtype{};format->GetGUID(MF_MT_SUBTYPE,&subtype);if(subtype==MFVideoFormat_NV12&&SUCCEEDED(decoder_->SetOutputType(0,format.Get(),0))){selected=true;break;}}
  if(!selected)return {};hr=output(decoder_.Get(),0,DWORD(w*h*3/2),raw);
 }
 if(FAILED(hr))return {};ComPtr<IMFMediaType> current;UINT32 stride=UINT32(w),planeHeight=UINT32(h);if(SUCCEEDED(decoder_->GetOutputCurrentType(0,&current))){UINT32 actual=0;if(SUCCEEDED(current->GetUINT32(MF_MT_DEFAULT_STRIDE,&actual)))stride=actual;UINT32 actualWidth=0,actualHeight=0;if(SUCCEEDED(MFGetAttributeSize(current.Get(),MF_MT_FRAME_SIZE,&actualWidth,&actualHeight))&&actualHeight>=UINT32(h)&&actualHeight<=UINT32(h+32))planeHeight=actualHeight;}
 if(stride<UINT32(w)||stride>UINT32(w*4)||raw.size()<qint64(stride)*planeHeight*3/2)return {};
 QImage image(w,h,QImage::Format_RGB32);const auto* y=reinterpret_cast<const unsigned char*>(raw.constData());const auto* uv=y+stride*planeHeight;
 for(int row=0;row<h;++row){auto* out=reinterpret_cast<QRgb*>(image.scanLine(row));for(int col=0;col<w;++col){const int yy=int(y[row*stride+col])-16,u=int(uv[(row/2)*stride+(col&~1)])-128,v=int(uv[(row/2)*stride+(col&~1)+1])-128;out[col]=qRgb(std::clamp((298*yy+409*v+128)>>8,0,255),std::clamp((298*yy-100*u-208*v+128)>>8,0,255),std::clamp((298*yy+516*u+128)>>8,0,255));}}
 return image;
}
}
