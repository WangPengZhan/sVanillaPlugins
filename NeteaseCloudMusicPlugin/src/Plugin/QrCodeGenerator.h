#pragma once
#include "Qrcodegen.h"

namespace QrCodeGenerator
{

std::string generateQR(const std::string& data);
std::string toSvgString(const qrcodegen::QrCode& qr, int border);

};  // namespace QrCodeGenerator
