#ifndef THEME_H
#define THEME_H

#include <QString>
#include <QColor>
#include <QFont>

namespace Theme {

    inline const QColor background  = QColor(0x12, 0x12, 0x12);
    inline const QColor white       = QColor(255, 255, 255);
    inline const QColor black       = QColor(0, 0, 0);
    inline const QColor zinc300     = QColor(0xD4, 0xD4, 0xD8);
    inline const QColor zinc400     = QColor(0xA1, 0xA1, 0xAA);
    inline const QColor zinc500     = QColor(0x71, 0x71, 0x7A);
    inline const QColor zinc700     = QColor(0x3F, 0x3F, 0x46);
    inline const QColor zinc800     = QColor(0x27, 0x27, 0x2A);
    inline const QColor zinc900     = QColor(0x18, 0x18, 0x1B);
    inline const QColor zinc900_40  = QColor(24, 24, 27, 102);
    inline const QColor zinc900_90  = QColor(24, 24, 27, 230);
    inline const QColor zinc800_80  = QColor(39, 39, 42, 204);
    inline const QColor zinc700_80  = QColor(63, 63, 70, 204);
    inline const QColor zinc800_90  = QColor(31, 31, 35, 230);
    inline const QColor zinc800_80bg= QColor(31, 31, 35, 204);
    inline const QColor black75     = QColor(0, 0, 0, 191);
    inline const QColor zinc200     = QColor(0xE4, 0xE4, 0xE7);
    inline const QColor red400      = QColor(0xF8, 0x71, 0x71);

    inline const QString appFont    = QStringLiteral("Space Grotesk");
    inline const QString symbolFont = QStringLiteral("Material Symbols Outlined");

    inline double trackingTight(double fs)  { return -0.025 * fs; }
    inline double trackingWide(double fs)   { return  0.025 * fs; }
    inline double trackingWider(double fs)  { return  0.050 * fs; }

    inline const QChar GLYPH_ADD_CIRCLE    = QChar(0xE990);
    inline const QChar GLYPH_REMOVE_CIRCLE = QChar(0xF08F);
    inline const QChar GLYPH_CHEVRON_RIGHT = QChar(0xE5CC);
    inline const QChar GLYPH_CLOSE         = QChar(0xE5CD);
    inline const QChar GLYPH_DELETE        = QChar(0xE92E);
    inline const QChar GLYPH_ADD           = QChar(0xE145);
    inline const QChar GLYPH_EDIT          = QChar(0xE3C9);
    inline const QChar GLYPH_EXPAND_MORE   = QChar(0xE5CF);

    inline QFont font(double px, QFont::Weight weight) {
        QFont f(appFont);
        f.setPixelSize(int(px));
        f.setWeight(weight);
        return f;
    }

    inline QFont symbolFontAt(double px, QFont::Weight weight) {
        QFont f(symbolFont);
        f.setPixelSize(int(px));
        f.setWeight(weight);
        return f;
    }
}

#endif
