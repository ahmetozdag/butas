#ifndef ICONS_H
#define ICONS_H

#include <QImage>
#include <QPainter>
#include <QPainterPath>
#include "theme.h"

namespace Icons{
    inline QImage glyph(const QChar &codepoint, double sizePx, const QColor &color,
                        bool fill = false, QFont::Weight weight = QFont::Normal)
    {
        int px = int(sizePx + 0.5);
        QImage img(px * 3, px * 3, QImage::Format_ARGB32_Premultiplied);
        img.fill(Qt::transparent);
        QPainter p(&img);
        p.setRenderHint(QPainter::Antialiasing);
        p.setRenderHint(QPainter::TextAntialiasing);
        QFont f = Theme::symbolFontAt(int(sizePx * 3), weight);
        f.setPixelSize(int(sizePx * 3));
        if (fill){
        }
        p.setFont(f);
        p.setPen(QPen(color));
        p.drawText(img.rect(), Qt::AlignCenter | Qt::TextDontPrint, codepoint);
        p.end();
        return img;
    }

    inline void drawGlyph(QPainter *p, const QChar &codepoint, double sizePx,
                          const QPoint &topLeft, const QColor &color, QFont::Weight weight = QFont::Normal){
        QFont f = Theme::symbolFontAt(sizePx, weight);
        p->save();
        p->setFont(f);
        p->setPen(QPen(color));
        QRect r(topLeft.x(), topLeft.y(), int(sizePx) + 2, int(sizePx) + 4);
        p->drawText(r, Qt::AlignLeft | Qt::AlignTop, QString(codepoint));
        p->restore();
    }

    inline void drawGlyphCentered(QPainter *p, const QChar &codepoint, double sizePx,
                                  const QRect &rect, const QColor &color, QFont::Weight weight = QFont::Normal){
        QFont f = Theme::symbolFontAt(sizePx, weight);
        p->save();
        p->setFont(f);
        p->setPen(QPen(color));
        p->drawText(rect, Qt::AlignCenter, QString(codepoint));
        p->restore();
    }

    inline QPainterPath roundedRectPath(const QRectF &rect, double radius){
        QPainterPath path;
        if (radius <= 0.001){
            path.addRect(rect);
        } else{
            path.addRoundedRect(rect, radius, radius);
        }
        return path;
    }
}

#endif
