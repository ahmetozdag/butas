#ifndef WIDGETS_H
#define WIDGETS_H

#include <QWidget>
#include <QAbstractButton>
#include <QPainter>
#include <QPainterPath>
#include <QVector>
#include <QColor>
#include <QFontMetricsF>
#include <cmath>
#include <functional>
#include "theme.h"
#include "icons.h"
#include "format.h"

inline void paintRoundedBox(QPainter *p, const QRectF &r, double radius,
                            const QColor &fill, const QColor &stroke)
{
    p->setRenderHint(QPainter::Antialiasing);
    QPainterPath path;
    path.addRoundedRect(r, radius, radius);
    if (fill.alpha() > 0) {
        p->fillPath(path, fill);
    }
    if (stroke.alpha() > 0) {
        p->setPen(QPen(stroke, 1));
        p->drawPath(path);
    }
}

class IconButton : public QAbstractButton
{
    Q_OBJECT
public:
    IconButton(const QChar glyph, double iconSize, const QColor &baseColor,
               const QColor &hoverColor, QWidget *parent = nullptr)
        : QAbstractButton(parent), m_glyph(glyph), m_size(iconSize),
          m_base(baseColor), m_hover(hoverColor)
    {
        setCursor(Qt::PointingHandCursor);
        setAttribute(Qt::WA_Hover, true);
        setFixedSize(int(iconSize + 12), int(iconSize + 12));
    }

    IconButton(double iconSize, const QColor &baseColor, const QColor &hoverColor,
               QWidget *parent = nullptr)
        : QAbstractButton(parent), m_glyph(Theme::GLYPH_CLOSE), m_size(iconSize),
          m_base(baseColor), m_hover(hoverColor)
    {
        setCursor(Qt::PointingHandCursor);
        setAttribute(Qt::WA_Hover, true);
        setFixedSize(int(iconSize), int(iconSize));
    }

signals:
    void clickedIndex(int);
protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        const QColor c = underMouse() ? m_hover : m_base;
        const int ox = (width() - int(m_size)) / 2;
        const int oy = (height() - int(m_size)) / 2;
        Icons::drawGlyph(&p, m_glyph, m_size, QPoint(ox, oy), c);
    }

private:
    QChar m_glyph;
    double m_size;
    QColor m_base, m_hover;
};

class ActionButton : public QAbstractButton
{
    Q_OBJECT
public:
    ActionButton(const QChar leftGlyph, const QString &label, QWidget *parent = nullptr)
        : QAbstractButton(parent), m_glyph(leftGlyph), m_label(label)
    {
        setCursor(Qt::PointingHandCursor);
        setAttribute(Qt::WA_Hover, true);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        setFixedHeight(46);
        setText(label);
    }

    QSize sizeHint() const override { return QSize(160, 46); }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.save();
        QColor bg = Theme::zinc900;
        if (underMouse())
            bg = Theme::zinc800;
        if (isDown()) {
            p.translate(width() / 2.0, height() / 2.0);
            p.scale(0.98, 0.98);
            p.translate(-width() / 2.0, -height() / 2.0);
        }
        paintRoundedBox(&p, QRectF(0.5, 0.5, width() - 1, height() - 1), 8, bg, Theme::zinc800);

        const int iconSize = 16;
        const int padX = 14;
        const int iconY = (height() - iconSize) / 2;

        Icons::drawGlyph(&p, m_glyph, iconSize, QPoint(padX, iconY), Theme::zinc300);

        QFont f = Theme::font(12, QFont::Medium);
        f.setLetterSpacing(QFont::AbsoluteSpacing, Theme::trackingWide(12));
        p.setFont(f);
        p.setPen(QPen(Theme::white));
        const int textX = padX + iconSize + 8;
        QRect textRect(textX, 0, width() - textX - padX - iconSize - 2, height());
        p.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, m_label);

        Icons::drawGlyph(&p, Theme::GLYPH_CHEVRON_RIGHT, iconSize,
                         QPoint(width() - padX - iconSize, iconY), Theme::zinc500);
        p.restore();
    }

private:
    QChar m_glyph;
    QString m_label;
};

static inline QFont balanceFont(double px, QFont::Weight weight, double em)
{
    QFont f = Theme::font(px, weight);
    f.setLetterSpacing(QFont::AbsoluteSpacing, -0.025 * em);
    return f;
}

class BalanceWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BalanceWidget(QWidget *parent = nullptr) : QWidget(parent)
    {
        m_intPart = "0"; m_decPart = "00"; m_negative = false;
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        setFixedHeight(int(balanceHeight()));
    }

    void setBalance(double amount)
    {
        const auto parts = Fmt::balanceParts(amount);
        m_intPart = (parts.negative ? QStringLiteral("-") : QString())
                    + Fmt::groupThousands(parts.intPart);
        m_decPart = parts.decPart;
        m_negative = parts.negative;
        updateGeometry();
        update();
    }

    QSize sizeHint() const override
    {
        QFontMetricsF fb(balanceFont(52, QFont::Light, 52));
        QFontMetricsF fs(balanceFont(24, QFont::Light, 24));
        QFontMetricsF fl = QFontMetricsF(balanceFont(12, QFont::Bold, 12));
        double ls = Theme::trackingWider(12);
        double labelW = fl.horizontalAdvance(QStringLiteral("TOPLAM BÜTÇE")) + ls * 12.0;
        double valueW = balanceWidth();
        return QSize(int(std::max(valueW, labelW)), int(balanceHeight()));
    }

    double balanceHeight() const { return 52 + 6 + 16; }
    double balanceWidth() const
    {
        QFontMetricsF fb(balanceFont(52, QFont::Light, 52));
        QFontMetricsF fs(balanceFont(24, QFont::Light, 24));
        return fb.horizontalAdvance(m_intPart) +
               fs.horizontalAdvance(QLatin1String(",")) +
               fs.horizontalAdvance(m_decPart) +
               fb.horizontalAdvance(QChar(' ')) + 4 +
               fb.horizontalAdvance(QChar(0x20BA));
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.setRenderHint(QPainter::TextAntialiasing);

        QFont big = balanceFont(52, QFont::Light, 52);
        QFont small = balanceFont(24, QFont::Light, 24);
        QFontMetricsF fb(big);
        QFontMetricsF fsm(small);
        const double lineBox = 52.0;
        const double baseline = (lineBox - (fb.ascent() + fb.descent())) / 2.0 + fb.ascent();

        double x = 0;
        p.setFont(big); p.setPen(QPen(Theme::white));
        p.drawText(QPointF(x, baseline), m_intPart);
        x += fb.horizontalAdvance(m_intPart);

        p.setFont(small); p.setPen(QPen(Theme::zinc300));
        p.drawText(QPointF(x, baseline), QLatin1String(",") + m_decPart);
        x += fsm.horizontalAdvance(QLatin1String(",")) + fsm.horizontalAdvance(m_decPart);

        p.setFont(big); p.setPen(QPen(Theme::white));
        x += fb.horizontalAdvance(QChar(' ')) + 4;
        p.drawText(QPointF(x, baseline), QString(QChar(0x20BA)));

        QFont lab = balanceFont(12, QFont::Bold, 12);
        lab.setLetterSpacing(QFont::AbsoluteSpacing, Theme::trackingWider(12));
        p.setFont(lab); p.setPen(QPen(Theme::zinc400));
        p.drawText(QRect(0, int(52 + 6), width(), 16), Qt::AlignRight | Qt::AlignVCenter,
                   QStringLiteral("TOPLAM BÜTÇE"));
    }

private:
    QString m_intPart, m_decPart;
    bool m_negative;
};

struct Transaction {
    bool isAdd;
    double amount;
};

class ChipsList : public QWidget
{
    Q_OBJECT
public:
    explicit ChipsList(QWidget *parent = nullptr) : QWidget(parent)
    {
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    }

    void setTransactions(QVector<Transaction> list)
    {
        m_items = std::move(list);
        updateGeometry();
        update();
    }

    QSize sizeHint() const override
    {
        if (m_items.isEmpty())
            return QSize(0, 0);
        int totalH = chipTotalHeight();
        int maxW = 95;
        QFontMetricsF fm(chipFont());
        for (const auto &item : m_items) {
            const QString text = chipText(item);
            const double w = std::max(95.0, fm.horizontalAdvance(text) + 12 * 2 + 2 + 0);
            maxW = std::max(maxW, int(w));
        }
        return QSize(maxW, totalH);
    }

    int chipTotalHeight() const
    {
        const int n = m_items.size();
        if (n == 0) return 0;
        return n * 34 + (n - 1) * 8;
    }

    QString chipText(const Transaction &t) const
    {
        return (t.isAdd ? QStringLiteral("+") : QStringLiteral("-")) +
               Fmt::formatNumber(t.amount) + QStringLiteral(" ₺");
    }

    QFont chipFont() const
    {
        QFont f = Theme::font(12, QFont::Bold);
        f.setLetterSpacing(QFont::AbsoluteSpacing, Theme::trackingWider(12));
        return f;
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.setRenderHint(QPainter::TextAntialiasing);
        const QFont f = chipFont();
        p.setFont(f);
        int y = 0;
        for (const auto &item : m_items) {
            const QString text = chipText(item);
            const double w = std::max(95.0, p.fontMetrics().horizontalAdvance(text) + 12.0 * 2 + 2.0);
            const QColor textColor = item.isAdd ? Theme::white   : Theme::zinc500;
            const QColor borg      = item.isAdd ? Theme::zinc700 : Theme::zinc800_80;
            const QColor bfill     = item.isAdd ? Theme::zinc900_90 : Theme::zinc900_40;
            paintRoundedBox(&p, QRectF(0.5, y + 0.5, w - 1, 34 - 1), 8, bfill, borg);
            p.setFont(f);
            p.setPen(QPen(textColor));
            p.drawText(QRect(0, y, int(w), 34), Qt::AlignCenter, text);
            y += 34 + 8;
        }
    }

private:
    QVector<Transaction> m_items;
};

struct RecurringItem {
    QString amount;
    QString currency;
    QString label;
    RecurringItem() = default;
    RecurringItem(const QString &a, const QString &c, const QString &l)
        : amount(a), currency(c), label(l) {}
};

class RecurringRow : public QWidget
{
    Q_OBJECT
public:
    RecurringRow(const RecurringItem &item, std::function<void()> onDelete,
                 QWidget *parent = nullptr)
        : QWidget(parent), m_item(item), m_onDelete(std::move(onDelete)), m_fontPx(27)
    {
        setAttribute(Qt::WA_Hover, false);
        auto *btn = new IconButton(Theme::GLYPH_DELETE, 13.0, Theme::zinc500, Theme::zinc300, this);
        btn->setToolTip(QLatin1String("Sil"));
        connect(btn, &QAbstractButton::clicked, this, [this]() { if (m_onDelete) m_onDelete(); });
        m_deleteBtn = btn;
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        updateSizing();
    }

    void setFontPx(double px)
    {
        if (px != m_fontPx) {
            m_fontPx = px;
            updateSizing();
            update();
        }
    }

    static double clampFontSize(int viewportW)
    {
        return std::min(36.0, std::max(24.0, 0.07 * viewportW));
    }

protected:
    void resizeEvent(QResizeEvent *e) override
    {
        QWidget::resizeEvent(e);
        m_deleteBtn->move(width() - m_deleteBtn->width(), (height() - m_deleteBtn->height()) / 2);
    }

    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.setRenderHint(QPainter::TextAntialiasing);

        QFont big = Theme::font(int(m_fontPx), QFont::Light);
        big.setLetterSpacing(QFont::AbsoluteSpacing, -0.025 * m_fontPx);
        QFontMetricsF fb(big);
        QFontMetricsF fcur(big);
        const QString amount = m_item.amount;
        const QString currency = m_item.currency;
        const double lineBox = m_fontPx;
        const double baseline = (lineBox - (fb.ascent() + fb.descent())) / 2.0 + fb.ascent();

        double x = 0;
        p.setFont(big); p.setPen(QPen(Theme::white));
        p.drawText(QPointF(x, baseline), amount);
        x += fb.horizontalAdvance(amount);
        x += fb.horizontalAdvance(QChar(' '));
        p.setPen(QPen(Theme::zinc300));
        p.drawText(QPointF(x, baseline), currency);

        QFont lab = Theme::font(10, QFont::Bold);
        lab.setLetterSpacing(QFont::AbsoluteSpacing, Theme::trackingWider(10));
        QFontMetricsF fl(lab);
        const double labelYTop = m_fontPx + 4;
        const double textBlockW = std::max(textWidth(), QFontMetricsF(lab).horizontalAdvance(m_item.label));
        p.setFont(lab); p.setPen(QPen(Theme::zinc400));
        p.drawText(QRectF(0, labelYTop, textBlockW, 15),
                   Qt::AlignRight | Qt::AlignVCenter, m_item.label);
        Q_UNUSED(fcur);
    }

    double textWidth() const
    {
        QFont big = Theme::font(int(m_fontPx), QFont::Light);
        big.setLetterSpacing(QFont::AbsoluteSpacing, -0.025 * m_fontPx);
        QFontMetricsF fb(big);
        return fb.horizontalAdvance(m_item.amount) + fb.horizontalAdvance(QChar(' ')) +
               fb.horizontalAdvance(m_item.currency);
    }

    QSize sizeHint() const override
    {
        QFont lab = Theme::font(10, QFont::Bold);
        lab.setLetterSpacing(QFont::AbsoluteSpacing, Theme::trackingWider(10));
        QFontMetricsF fl(lab);
        const double textBlockW = std::max(textWidth(), fl.horizontalAdvance(m_item.label));
        const double h = m_fontPx + 4 + 15;
        return QSize(int(textBlockW + m_deleteBtn->width()), int(std::max(h, 25.0)));
    }

private:
    void updateSizing()
    {
        setFixedHeight(int(std::max(m_fontPx + 4 + 15, 25.0)));
    }

    RecurringItem m_item;
    std::function<void()> m_onDelete;
    IconButton *m_deleteBtn;
    double m_fontPx;
};

#endif
