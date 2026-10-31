#ifndef BLUR_H
#define BLUR_H

#include <QImage>
#include <QColor>
#include <vector>

namespace BlurEffect{
    inline void boxBlurPassHLine(QRgb *row, const int w, const int radius){
        const int window = radius * 2 + 1;
        int bars[4] = {0, 0, 0, 0};
        for (int dx = -radius; dx <= radius; ++dx){
            const QRgb p = row[qBound(0, dx, w - 1)];
            bars[0] += qRed(p); bars[1] += qGreen(p);
            bars[2] += qBlue(p); bars[3] += qAlpha(p);
        }
        std::vector<int> out(w * 4);
        for (int x = 0; x < w; ++x){
            out[4 * x + 0] = bars[0] / window;
            out[4 * x + 1] = bars[1] / window;
            out[4 * x + 2] = bars[2] / window;
            out[4 * x + 3] = bars[3] / window;
            const QRgb pa = row[qBound(0, x - radius, w - 1)];
            const QRgb pb = row[qBound(0, x + radius, w - 1)];
            bars[0] += qRed(pb) - qRed(pa);
            bars[1] += qGreen(pb) - qGreen(pa);
            bars[2] += qBlue(pb) - qBlue(pa);
            bars[3] += qAlpha(pb) - qAlpha(pa);
        }
        for (int x = 0; x < w; ++x)
            row[x] = qRgba(out[4 * x], out[4 * x + 1], out[4 * x + 2], out[4 * x + 3]);
    }

    inline void boxBlurPass(QImage &img, const int radius){
        if (img.isNull() || radius < 1)
            return;
        const int w = img.width();
        const int h = img.height();
        QRgb *pixels = reinterpret_cast<QRgb *>(img.bits());
        const int stride = img.bytesPerLine() / 4;

        for (int y = 0; y < h; ++y)
            boxBlurPassHLine(pixels + y * stride, w, radius);

        const int window = radius * 2 + 1;
        std::vector<QRgb> col(h);
        std::vector<int> out(h * 4);
        for (int x = 0; x < w; ++x){
            for (int y = 0; y < h; ++y)
                col[y] = pixels[y * stride + x];
            int bars[4] = {0, 0, 0, 0};
            for (int dy = -radius; dy <= radius; ++dy){
                const QRgb p = col[qBound(0, dy, h - 1)];
                bars[0] += qRed(p); bars[1] += qGreen(p);
                bars[2] += qBlue(p); bars[3] += qAlpha(p);
            }
            for (int y = 0; y < h; ++y){
                out[4 * y + 0] = bars[0] / window;
                out[4 * y + 1] = bars[1] / window;
                out[4 * y + 2] = bars[2] / window;
                out[4 * y + 3] = bars[3] / window;
                const QRgb pa = col[qBound(0, y - radius, h - 1)];
                const QRgb pb = col[qBound(0, y + radius, h - 1)];
                bars[0] += qRed(pb) - qRed(pa);
                bars[1] += qGreen(pb) - qGreen(pa);
                bars[2] += qBlue(pb) - qBlue(pa);
                bars[3] += qAlpha(pb) - qAlpha(pa);
            }
            for (int y = 0; y < h; ++y)
                pixels[y * stride + x] =
                    qRgba(out[4 * y], out[4 * y + 1], out[4 * y + 2], out[4 * y + 3]);
        }
    }

    inline void stackBlur(QImage &image, int radius){
        if (image.isNull() || radius < 1)
            return;
        image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
        boxBlurPass(image, radius);
        boxBlurPass(image, radius);
        boxBlurPass(image, radius);
    }
}

#endif
