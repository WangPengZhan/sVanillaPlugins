#include "QrCodeGenerator.h"
#include <sstream>

namespace QrCodeGenerator
{

std::string generateQR(const std::string& data)
{
    constexpr auto errorCorrection = qrcodegen::QrCode::Ecc::MEDIUM;
    const auto qrCode = qrcodegen::QrCode::encodeText(data.data(), errorCorrection);
    return toSvgString(qrCode, 4);
}

std::string toSvgString(const qrcodegen::QrCode& qr, int border)
{
    std::stringstream ss;

    ss << R"(<?xml version="1.0" encoding="UTF-8"?>)" << R"(<!DOCTYPE svg PUBLIC "-//W3C//DTD SVG 1.1//EN" "http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd">)"
       << R"(<svg xmlns="http://www.w3.org/2000/svg" version="1.1" viewBox="0 0 )" << (qr.getSize() + border * 2) << " " << (qr.getSize() + border * 2)
       << R"(" stroke="none"><rect width="100%" height="100%" fill="#FFFFFF"/><path d=")";

    for (int y = 0; y < qr.getSize(); y++)
    {
        for (int x = 0; x < qr.getSize(); x++)
        {
            if (qr.getModule(x, y))
            {
                ss << (x == 0 && y == 0 ? "" : " ") << "M" << (x + border) << "," << (y + border) << "h1v1h-1z";
            }
        }
    }

    ss << R"(" fill="#000000"/></svg>)";
    return ss.str();
}

}  // namespace QrCodeGenerator