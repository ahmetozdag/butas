#ifndef CONTROLS_H
#define CONTROLS_H

#include <QWidget>
#include <QAbstractButton>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QScroller>
#include <QPainter>
#include <QMouseEvent>
#include <QPointer>
#include "widgets.h"
#include "modals.h"
#include "theme.h"
#include "icons.h"

class ErrorLabel : public QWidget
{
    Q_OBJECT
public:
    explicit ErrorLabel(QWidget *parent = nullptr) : QWidget(parent)
    {
        setFixedHeight(16);
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        hide();
    }

    void setText(const QString &t)
    {
        if (m_text == t)
            return;
        m_text = t;
        setVisible(!t.isEmpty());
        update();
        if (QWidget *pw = parentWidget()) {
            if (pw->layout()) {
                pw->layout()->invalidate();
                pw->layout()->activate();
            }
        }
        for (QWidget *p = parentWidget(); p; p = p->parentWidget()) {
            if (auto *d = qobject_cast<ModalDialog *>(p)) {
                d->relayoutCard();
                break;
            }
        }
    }

    QString text() const { return m_text; }
    void clear() { setText(QString()); }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::TextAntialiasing);
        QFont f = Theme::font(12, QFont::Normal);
        p.setFont(f);
        p.setPen(QPen(Theme::red400));
        p.drawText(rect(), Qt::AlignLeft | Qt::AlignVCenter, m_text);
    }

private:
    QString m_text;
};

class DropdownOption : public QAbstractButton
{
    Q_OBJECT
public:
    DropdownOption(const QString &label, bool selected, QWidget *parent = nullptr)
        : QAbstractButton(parent), m_label(label), m_selected(selected)
    {
        setCursor(Qt::PointingHandCursor);
        setAttribute(Qt::WA_Hover, true);
        setFocusPolicy(Qt::NoFocus);
        setFixedHeight(42);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        if (m_selected || underMouse()) {
            p.setPen(Qt::NoPen);
            p.setBrush(underMouse() ? Theme::zinc800 : Theme::zinc800_80bg);
            p.drawRoundedRect(QRectF(rect()).adjusted(2, 1, -2, -1), 8, 8);
        }
        QFont f = Theme::font(14, m_selected ? QFont::Medium : QFont::Normal);
        f.setLetterSpacing(QFont::AbsoluteSpacing, Theme::trackingTight(14));
        p.setFont(f);
        p.setPen(QPen(m_selected ? Theme::white : Theme::zinc300));
        p.drawText(rect().adjusted(14, 0, -12, 0), Qt::AlignLeft | Qt::AlignVCenter, m_label);
    }

private:
    QString m_label;
    bool m_selected;
};

class DropdownPanel : public QWidget
{
    Q_OBJECT
public:
    explicit DropdownPanel(QWidget *parent = nullptr) : QWidget(parent) {}
protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        paintRoundedBox(&p, QRectF(0.5, 0.5, width() - 1, height() - 1), 12,
                        Theme::zinc900, Theme::zinc800);
    }
};

class DropdownPopup : public QWidget
{
    Q_OBJECT
public:
    explicit DropdownPopup(QWidget *parent) : QWidget(parent)
    {
        hide();
        m_panel = new DropdownPanel(this);
        QVBoxLayout *pl = new QVBoxLayout(m_panel);
        pl->setContentsMargins(6, 6, 6, 6);
        pl->setSpacing(0);

        m_scroll = new QScrollArea(m_panel);
        m_scroll->setFrameShape(QFrame::NoFrame);
        m_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_scroll->setWidgetResizable(true);
        m_scroll->setStyleSheet(QStringLiteral("background: transparent;"));
        m_scroll->viewport()->setAutoFillBackground(false);
#ifdef Q_OS_ANDROID
        QScroller::grabGesture(m_scroll->viewport(), QScroller::TouchGesture);
#endif

        m_list = new QWidget;
        m_list->setAutoFillBackground(false);
        m_listLayout = new QVBoxLayout(m_list);
        m_listLayout->setContentsMargins(0, 0, 0, 0);
        m_listLayout->setSpacing(0);
        m_listLayout->addStretch(1);
        m_scroll->setWidget(m_list);
        pl->addWidget(m_scroll);
        m_panel->hide();
    }

    void showFor(const QRect &anchor, const QStringList &options, int current)
    {
        if (parentWidget())
            setGeometry(parentWidget()->rect());

        while (m_listLayout->count() > 1) {
            QLayoutItem *it = m_listLayout->takeAt(0);
            if (it->widget())
                it->widget()->deleteLater();
            delete it;
        }

        for (int i = 0; i < options.size(); ++i) {
            auto *opt = new DropdownOption(options.at(i), i == current, m_list);
            connect(opt, &QAbstractButton::clicked, this, [this, i]() {
                dismiss();
                emit picked(i);
            });
            m_listLayout->insertWidget(m_listLayout->count() - 1, opt);
        }

        const int margin = 6;
        const int panelW = std::max(anchor.width(), 160);
        int panelH = options.size() * 42 + margin * 2;
        panelH = std::min(panelH, 320);
        int x = anchor.left();
        int y = anchor.bottom() + 6;
        if (y + panelH > height() - 12)
            y = anchor.top() - 6 - panelH;
        x = qBound(12, x, std::max(12, width() - panelW - 12));
        y = qBound(12, y, std::max(12, height() - panelH - 12));

        m_panel->setGeometry(x, y, panelW, panelH);
        m_scroll->setGeometry(margin, margin, panelW - margin * 2, panelH - margin * 2);
        m_list->adjustSize();
        m_panel->show();
        m_panel->raise();
        show();
        raise();

        const int listW = m_scroll->viewport()->width();
        const int contentH = int(options.size()) * 42;
        m_list->setMinimumSize(listW, contentH);
        m_list->resize(listW, contentH);
        m_listLayout->activate();

        int rowY = 0;
        for (int i = 0; i < m_listLayout->count(); ++i) {
            QWidget *w = m_listLayout->itemAt(i)->widget();
            if (!w)
                continue;
            w->setGeometry(0, rowY, listW, 42);
            w->show();
            rowY += 42;
        }
    }

    void dismiss()
    {
        if (m_panel)
            m_panel->hide();
        hide();
    }

signals:
    void picked(int index);

protected:
    void mousePressEvent(QMouseEvent *e) override
    {
        if (!m_panel->geometry().contains(e->pos()))
            dismiss();
    }

private:
    DropdownPanel *m_panel = nullptr;
    QScrollArea *m_scroll = nullptr;
    QWidget *m_list = nullptr;
    QVBoxLayout *m_listLayout = nullptr;
};

class SourceSelectBox : public QAbstractButton
{
    Q_OBJECT
public:
    explicit SourceSelectBox(QWidget *parent = nullptr) : QAbstractButton(parent)
    {
        setCursor(Qt::PointingHandCursor);
        setAttribute(Qt::WA_Hover, true);
        setFocusPolicy(Qt::NoFocus);
        setFixedHeight(50);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    }

    void setOptions(const QVector<MoneySource> &sources)
    {
        m_sources = sources;
        if (!m_selectedId.isEmpty() && indexOf(m_selectedId) < 0)
            m_selectedId.clear();
        update();
    }

    void setSelectedId(const QString &id)
    {
        m_selectedId = id;
        update();
    }

    QString selectedId() const { return m_selectedId; }

    QString selectedName() const
    {
        const int idx = indexOf(m_selectedId);
        return idx >= 0 ? m_sources.at(idx).name : QString();
    }

    void clearSelection()
    {
        m_selectedId.clear();
        update();
    }

    void dismiss()
    {
        if (m_popup)
            m_popup->dismiss();
    }

    void debugOpenMenu() { openPopup(); }

signals:
    void addSourceRequested();
    void selectionChanged();

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.setRenderHint(QPainter::TextAntialiasing);
        paintRoundedBox(&p, QRectF(0.5, 0.5, width() - 1, height() - 1), 12,
                        Theme::zinc800_80bg, Theme::zinc700_80);
        const QString name = selectedName();
        QFont f = Theme::font(14, QFont::Normal);
        f.setLetterSpacing(QFont::AbsoluteSpacing, Theme::trackingTight(14));
        p.setFont(f);
        p.setPen(QPen(name.isEmpty() ? Theme::zinc500 : Theme::white));
        p.drawText(QRect(16, 0, width() - 56, height()),
                   Qt::AlignLeft | Qt::AlignVCenter,
                   name.isEmpty() ? QStringLiteral("Kaynak seçin") : name);
        Icons::drawGlyphCentered(&p, Theme::GLYPH_EXPAND_MORE, 18,
                                 QRect(width() - 42, 0, 28, height()), Theme::zinc400);
    }

    void mouseReleaseEvent(QMouseEvent *e) override
    {
        QAbstractButton::mouseReleaseEvent(e);
        if (rect().contains(e->pos()))
            openPopup();
    }

private:
    int indexOf(const QString &id) const
    {
        if (id.isEmpty())
            return -1;
        for (int i = 0; i < m_sources.size(); ++i)
            if (m_sources.at(i).id == id)
                return i;
        return -1;
    }

    QWidget *hostDialog() const
    {
        for (QWidget *p = parentWidget(); p; p = p->parentWidget())
            if (qobject_cast<ModalDialog *>(p))
                return p;
        return window();
    }

    void openPopup()
    {
        QWidget *host = hostDialog();
        if (!host)
            return;
        if (!m_popup) {
            auto *popup = new DropdownPopup(host);
            connect(popup, &DropdownPopup::picked, this, [this](int idx) {
                if (idx == 0) {
                    emit addSourceRequested();
                    return;
                }
                const int si = idx - 1;
                if (si >= 0 && si < m_sources.size()) {
                    m_selectedId = m_sources.at(si).id;
                    update();
                    emit selectionChanged();
                }
            });
            m_popup = popup;
        }
        QStringList options;
        options << QStringLiteral("Kaynak Ekle");
        int current = -1;
        for (int i = 0; i < m_sources.size(); ++i) {
            options << m_sources.at(i).name;
            if (m_sources.at(i).id == m_selectedId)
                current = i + 1;
        }
        const QRect anchor(mapTo(host, QPoint(0, 0)), size());
        m_popup->showFor(anchor, options, current);
    }

    QPointer<DropdownPopup> m_popup;
    QVector<MoneySource> m_sources;
    QString m_selectedId;
};

#endif
