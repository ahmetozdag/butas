#ifndef PANEL_H
#define PANEL_H

#include <QWidget>
#include <QVBoxLayout>
#include <functional>
#include "widgets.h"

class UppercaseLabel : public QWidget
{
    Q_OBJECT
public:
    UppercaseLabel(const QString &text, const QColor &color, QWidget *parent = nullptr)
        : QWidget(parent), m_text(text), m_color(color)
    {
        QFont f = Theme::font(10, QFont::Bold);
        f.setLetterSpacing(QFont::AbsoluteSpacing, Theme::trackingWider(10));
        QFontMetrics fm(f);
        setFixedSize(fm.horizontalAdvance(text), 15);
    }
protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::TextAntialiasing);
        QFont f = Theme::font(10, QFont::Bold);
        f.setLetterSpacing(QFont::AbsoluteSpacing, Theme::trackingWider(10));
        p.setFont(f);
        p.setPen(QPen(m_color));
        p.drawText(rect(), Qt::AlignLeft | Qt::AlignVCenter, m_text);
    }
private:
    QString m_text;
    QColor m_color;
};

class Divider : public QWidget
{
    Q_OBJECT
public:
    explicit Divider(QWidget *parent = nullptr) : QWidget(parent) { setFixedHeight(1); }
protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.fillRect(rect(), Theme::zinc800_80);
    }
};

class PanelCard : public QWidget
{
    Q_OBJECT
public:
    explicit PanelCard(QWidget *parent = nullptr) : QWidget(parent)
    {
        QVBoxLayout *outer = new QVBoxLayout(this);
        outer->setContentsMargins(20, 20, 20, 20);
        outer->setSpacing(24);

        m_incomeLabel = new UppercaseLabel(QStringLiteral("AYLIK DÜZENLİ GELİR"), Theme::zinc500, this);
        m_incomeRowsWidget = new QWidget(this);
        m_incomeRowsLayout = toVBox(m_incomeRowsWidget);

        m_expenseLabel = new UppercaseLabel(QStringLiteral("AYLIK DÜZENLİ GİDER"), Theme::zinc500, this);
        m_expenseRowsWidget = new QWidget(this);
        m_expenseRowsLayout = toVBox(m_expenseRowsWidget);

        outer->addWidget(sectionWidget(m_incomeLabel, m_incomeRowsWidget));

        m_divider = new Divider(this);
        outer->addWidget(m_divider);

        outer->addWidget(sectionWidget(m_expenseLabel, m_expenseRowsWidget));

        outer->addStretch(1);

        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }

    void setIncomeItems(const QVector<RecurringItem> &items, std::function<void(int)> onDelete)
    {
        rebuild(m_incomeRowsLayout, m_incomeRows, items, std::move(onDelete));
    }

    void setExpenseItems(const QVector<RecurringItem> &items, std::function<void(int)> onDelete)
    {
        rebuild(m_expenseRowsLayout, m_expenseRows, items, std::move(onDelete));
    }

    void setViewportWidth(int vw)
    {
        m_viewportW = vw;
        const double px = RecurringRow::clampFontSize(vw);
        for (RecurringRow *r : std::as_const(m_incomeRows))
            r->setFontPx(px);
        for (RecurringRow *r : std::as_const(m_expenseRows))
            r->setFontPx(px);
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        paintRoundedBox(&p, QRectF(0.5, 0.5, width() - 1, height() - 1), 12,
                        Theme::zinc900_40, Theme::zinc800);
    }

private:
    static QVBoxLayout *toVBox(QWidget *w)
    {
        QVBoxLayout *l = new QVBoxLayout(w);
        l->setContentsMargins(0, 0, 0, 0);
        l->setSpacing(12);
        return l;
    }

    QWidget *sectionWidget(UppercaseLabel *label, QWidget *rowsWidget) const
    {
        QWidget *sec = new QWidget;
        QVBoxLayout *sl = new QVBoxLayout(sec);
        sl->setContentsMargins(0, 0, 0, 0);
        sl->setSpacing(8);
        sl->addWidget(label);
        sl->addWidget(rowsWidget);
        return sec;
    }

    void rebuild(QVBoxLayout *rowsLayout, QVector<RecurringRow *> &registry,
                 const QVector<RecurringItem> &items, std::function<void(int)> onDelete)
    {
        Q_ASSERT(rowsLayout != nullptr);
        while (rowsLayout->count() > 0) {
            QLayoutItem *it = rowsLayout->takeAt(0);
            if (it->widget())
                it->widget()->deleteLater();
            delete it;
        }
        registry.clear();
        for (int i = 0; i < items.size(); ++i) {
            const RecurringItem &item = items.at(i);
            const int idx = i;
            auto *row = new RecurringRow(item, [onDelete, idx]() { if (onDelete) onDelete(idx); }, this);
            row->setFontPx(RecurringRow::clampFontSize(m_viewportW));
            rowsLayout->addWidget(row);
            registry.append(row);
        }
        rowsLayout->update();
    }

    QVBoxLayout *m_incomeRowsLayout = nullptr;
    QVBoxLayout *m_expenseRowsLayout = nullptr;
    QWidget *m_incomeRowsWidget = nullptr;
    QWidget *m_expenseRowsWidget = nullptr;
    UppercaseLabel *m_incomeLabel = nullptr;
    UppercaseLabel *m_expenseLabel = nullptr;
    Divider *m_divider = nullptr;
    QVector<RecurringRow *> m_incomeRows;
    QVector<RecurringRow *> m_expenseRows;
    int m_viewportW = 390;
};

#endif
