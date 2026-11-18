#pragma once
#include <QImage>
#include <QString>
#include <QRect>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>
namespace fm {
// Stage 3 capture primitive; not connected to any transport in 0.1.
class DesktopCapture {
public:
    bool open(unsigned int outputIndex, QString& error);
    QImage nextFrame(QString& error);
    void close();
    QRect bounds() const { return bounds_; }
private:
    QRect bounds_;
    Microsoft::WRL::ComPtr<ID3D11Device> device_;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_;
    Microsoft::WRL::ComPtr<IDXGIOutputDuplication> duplication_;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> staging_;
};
}
