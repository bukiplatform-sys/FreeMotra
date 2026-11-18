#include "DesktopCapture.h"
namespace fm {
QVariantList DesktopCapture::monitors(){
 QVariantList list;Microsoft::WRL::ComPtr<IDXGIFactory1> factory;
 if(FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory))))return list;
 for(UINT a=0;;++a){Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;if(factory->EnumAdapters1(a,&adapter)==DXGI_ERROR_NOT_FOUND||!adapter)break;
  for(UINT o=0;;++o){Microsoft::WRL::ComPtr<IDXGIOutput> output;if(adapter->EnumOutputs(o,&output)==DXGI_ERROR_NOT_FOUND||!output)break;
   DXGI_OUTPUT_DESC desc{};if(FAILED(output->GetDesc(&desc))||!desc.AttachedToDesktop)continue;
   list.append(QVariantMap{{"name",QString::fromWCharArray(desc.DeviceName)},{"width",int(desc.DesktopCoordinates.right-desc.DesktopCoordinates.left)},{"height",int(desc.DesktopCoordinates.bottom-desc.DesktopCoordinates.top)}});
  }
 }return list;
}

bool DesktopCapture::open(unsigned int outputIndex, QString& error) {
    close(); error.clear();
    Microsoft::WRL::ComPtr<IDXGIFactory1> factory;
    if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory)))) { error = "DXGI factory failed"; return false; }
    unsigned int display = 0;
    for (UINT a = 0;; ++a) {
        Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
        if (factory->EnumAdapters1(a, &adapter) == DXGI_ERROR_NOT_FOUND) break;
        if (!adapter) break;
        for (UINT o = 0;; ++o) {
            Microsoft::WRL::ComPtr<IDXGIOutput> output;
            if (adapter->EnumOutputs(o, &output) == DXGI_ERROR_NOT_FOUND) break;
            if (!output) break;
            DXGI_OUTPUT_DESC desc{};
            output->GetDesc(&desc);
            if (!desc.AttachedToDesktop) continue;
            if (display++ != outputIndex) continue;
            if(desc.Rotation!=DXGI_MODE_ROTATION_IDENTITY && desc.Rotation!=DXGI_MODE_ROTATION_UNSPECIFIED){error="Rotated monitors are not supported in 0.2";return false;}
            D3D_FEATURE_LEVEL level;
            if (FAILED(D3D11CreateDevice(adapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT,
                nullptr, 0, D3D11_SDK_VERSION, &device_, &level, &context_))) {
                error = "D3D11 device failed"; close(); return false;
            }
            Microsoft::WRL::ComPtr<IDXGIOutput1> output1;
            if (FAILED(output.As(&output1)) || FAILED(output1->DuplicateOutput(device_.Get(), &duplication_))) {
                error = "Desktop duplication unavailable (secure desktop, session or driver)"; close(); return false;
            }
            bounds_=QRect(desc.DesktopCoordinates.left,desc.DesktopCoordinates.top,desc.DesktopCoordinates.right-desc.DesktopCoordinates.left,desc.DesktopCoordinates.bottom-desc.DesktopCoordinates.top);
            return true;
        }
    }
    error = "Monitor not found"; return false;
}
QImage DesktopCapture::nextFrame(QString& error) {
    error.clear();
    if (!duplication_) { error = "Capture not initialized"; return {}; }
    DXGI_OUTDUPL_FRAME_INFO info{};
    Microsoft::WRL::ComPtr<IDXGIResource> resource;
    HRESULT hr = duplication_->AcquireNextFrame(0, &info, &resource);
    if (hr == DXGI_ERROR_WAIT_TIMEOUT) return {};
    if (FAILED(hr)) { error = "Capture lost; reopen after display/session change"; close(); return {}; }
    struct FrameGuard { IDXGIOutputDuplication* p; ~FrameGuard() { p->ReleaseFrame(); } } guard{duplication_.Get()};
    Microsoft::WRL::ComPtr<ID3D11Texture2D> frame;
    if (FAILED(resource.As(&frame))) { error = "Unexpected capture resource"; return {}; }
    D3D11_TEXTURE2D_DESC desc{}; frame->GetDesc(&desc);
    D3D11_TEXTURE2D_DESC existing{};
    if (staging_) staging_->GetDesc(&existing);
    if (!staging_ || existing.Width != desc.Width || existing.Height != desc.Height || existing.Format != desc.Format) {
        staging_.Reset();
        desc.Usage = D3D11_USAGE_STAGING; desc.BindFlags = 0;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ; desc.MiscFlags = 0;
        if (FAILED(device_->CreateTexture2D(&desc, nullptr, &staging_))) { error = "Capture allocation failed"; return {}; }
    }
    context_->CopyResource(staging_.Get(), frame.Get());
    D3D11_MAPPED_SUBRESOURCE map{};
    if (FAILED(context_->Map(staging_.Get(), 0, D3D11_MAP_READ, 0, &map))) { error = "Capture readback failed"; return {}; }
    QImage image(static_cast<const uchar*>(map.pData), int(desc.Width), int(desc.Height), qsizetype(map.RowPitch), QImage::Format_ARGB32);
    auto owned = image.copy();
    context_->Unmap(staging_.Get(), 0);
    return owned;
}
void DesktopCapture::close() { staging_.Reset(); duplication_.Reset(); context_.Reset(); device_.Reset(); }
}
