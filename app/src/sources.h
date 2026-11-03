#ifndef SOURCES_H
#define SOURCES_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QAbstractButton>
#include <QLabel>
#include <functional>
#include "widgets.h"
#include "panel.h"
#include "modals.h"
#include "controls.h"
#include "theme.h"
#include "icons.h"
#include "format.h"

class ToggleOption : public QAbstractButton
{
    Q_OBJECT
public:
    ToggleOption(const QString &label, QWidget *parent = nullptr)
        : QAbstractButton(parent), m_label(label)
    {
        setCursor(Qt::PointingHandCursor);
        setAttribute(Qt::WA_Hover, true);
        setFocusPolicy(Qt::NoFocus);
        setFixedHeight(30);
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    }

    void setSelected(bool s) { m_selected = s; update(); }

    QSize sizeHint() const override
    {
        QFontMetricsF fm(Theme::font(12, QFont::Bold));
        return QSize(int(fm.horizontalAdvance(m_label) + 24), 30);
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        if (m_selected) {
            p.setPen(Qt::NoPen);
            p.setBrush(isEnabled() ? Theme::white : Theme::zinc700);
            p.drawRoundedRect(rect(), 8, 8);
        }
        QColor c;
        if (!isEnabled())
            c = Theme::zinc700;
        else if (m_selected)
            c = Theme::zinc900;
        else
            c = underMouse() ? Theme::white : Theme::zinc400;
        QFont f = Theme::font(12, m_selected ? QFont::Bold : QFont::Medium);
        f.setLetterSpacing(QFont::AbsoluteSpacing, Theme::trackingWide(12));
        p.setFont(f);
        p.setPen(QPen(c));
        p.drawText(rect(), Qt::AlignCenter, m_label);
    }

private:
    QString m_label;
    bool m_selected = false;
};

class ChoiceToggle : public QWidget
{
    Q_OBJECT
public:
    explicit ChoiceToggle(QWidget *parent = nullptr) : QWidget(parent)
    {
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        QHBoxLayout *l = new QHBoxLayout(this);
        l->setContentsMargins(4, 4, 4, 4);
        l->setSpacing(4);
        m_yes = new ToggleOption(QStringLiteral("Evet"), this);
        m_no = new ToggleOption(QStringLiteral("Hayır"), this);
        l->addWidget(m_yes, 1);
        l->addWidget(m_no, 1);
        connect(m_yes, &QAbstractButton::clicked, this, [this]() {
            setValue(true);
            emit changed(true);
        });
        connect(m_no, &QAbstractButton::clicked, this, [this]() {
            setValue(false);
            emit changed(false);
        });
        setValue(false);
        setFixedHeight(38);
    }

    void setValue(bool v)
    {
        m_value = v;
        m_yes->setSelected(v);
        m_no->setSelected(!v);
        update();
    }

    bool value() const { return m_value; }

    void setYesEnabled(bool enabled)
    {
        m_yes->setEnabled(enabled);
        if (!enabled)
            setValue(false);
    }

signals:
    void changed(bool value);

protected:
    void paintEvent(QPaintEvent *e) override
    {
        QPainter p(this);
        paintRoundedBox(&p, QRectF(0.5, 0.5, width() - 1, height() - 1), 10,
                        Theme::zinc800_80bg, Theme::zinc700_80);
        QWidget::paintEvent(e);
    }

private:
    ToggleOption *m_yes = nullptr;
    ToggleOption *m_no = nullptr;
    bool m_value = false;
};

class SourceRow : public QWidget
{
    Q_OBJECT
public:
    SourceRow(const MoneySource &source, std::function<void()> onOpen,
              std::function<void()> onDelete, std::function<void()> onRename,
              QWidget *parent = nullptr)
        : QWidget(parent), m_source(source), m_onOpen(std::move(onOpen))
    {
        setCursor(Qt::PointingHandCursor);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        setFixedHeight(36);
        if (!isNakitSourceName(source.name)) {
            m_delete = new IconButton(Theme::GLYPH_DELETE, 13.0, Theme::zinc500,
                                      Theme::zinc300, this);
            m_delete->setToolTip(QStringLiteral("Sil"));
            connect(m_delete, &QAbstractButton::clicked, this, [onDelete]() {
                if (onDelete) onDelete();
            });
            m_edit = new IconButton(Theme::GLYPH_EDIT, 13.0, Theme::zinc500,
                                    Theme::zinc300, this);
            m_edit->setToolTip(QStringLiteral("Yeniden adlandır"));
            connect(m_edit, &QAbstractButton::clicked, this, [onRename]() {
                if (onRename) onRename();
            });
        }
    }

    void setSource(const MoneySource &source)
    {
        m_source = source;
        update();
    }

protected:
    void resizeEvent(QResizeEvent *e) override
    {
        QWidget::resizeEvent(e);
        if (!m_edit || !m_delete)
            return;
        const int cy = (height() - m_edit->height()) / 2;
        m_edit->move(width() - m_edit->width(), cy);
        m_delete->move(width() - m_edit->width() - m_delete->width() - 4, cy);
    }

    void mousePressEvent(QMouseEvent *e) override
    {
        if (e->button() == Qt::LeftButton)
            e->accept();
        else
            QWidget::mousePressEvent(e);
    }

    void mouseReleaseEvent(QMouseEvent *e) override
    {
        if (e->button() == Qt::LeftButton && rect().contains(e->pos()) && m_onOpen)
            m_onOpen();
        QWidget::mouseReleaseEvent(e);
    }

    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::TextAntialiasing);
        QFont f = Theme::font(15, QFont::Medium);
        f.setLetterSpacing(QFont::AbsoluteSpacing, Theme::trackingTight(15));
        p.setFont(f);
        p.setPen(QPen(m_source.balance < 0 ? Theme::zinc500 : Theme::white));
        int rightPad = 0;
        if (m_edit)
            rightPad = m_edit->width() + m_delete->width() + 12;
        const QString text = p.fontMetrics().elidedText(m_source.name, Qt::ElideRight,
                                                        width() - rightPad);
        p.drawText(QRect(0, 0, width() - rightPad, height()),
                   Qt::AlignLeft | Qt::AlignVCenter, text);
    }

private:
    MoneySource m_source;
    std::function<void()> m_onOpen;
    IconButton *m_delete = nullptr;
    IconButton *m_edit = nullptr;
};

class SourcesCard : public QWidget
{
    Q_OBJECT
public:
    explicit SourcesCard(QWidget *parent = nullptr) : QWidget(parent)
    {
        QVBoxLayout *outer = new QVBoxLayout(this);
        outer->setContentsMargins(20, 20, 20, 20);
        outer->setSpacing(16);

        QWidget *header = new QWidget(this);
        QHBoxLayout *hl = new QHBoxLayout(header);
        hl->setContentsMargins(0, 0, 0, 0);
        hl->setSpacing(8);
        hl->addWidget(new UppercaseLabel(QStringLiteral("KAYNAKLAR"), Theme::zinc500, header));
        hl->addStretch(1);
        auto *add = new IconButton(Theme::GLYPH_ADD, 16.0, Theme::zinc400, Theme::white, header);
        add->setToolTip(QStringLiteral("Kaynak ekle"));
        connect(add, &QAbstractButton::clicked, this, [this]() {
            if (m_onAdd) m_onAdd();
        });
        hl->addWidget(add);
        outer->addWidget(header);

        m_rowsWidget = new QWidget(this);
        m_rowsLayout = new QVBoxLayout(m_rowsWidget);
        m_rowsLayout->setContentsMargins(0, 0, 0, 0);
        m_rowsLayout->setSpacing(4);
        outer->addWidget(m_rowsWidget);

        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    }

    void setSources(const QVector<MoneySource> &items,
                    std::function<void(const QString &)> onOpen,
                    std::function<void(const QString &)> onDelete,
                    std::function<void(const QString &)> onRename,
                    std::function<void()> onAdd)
    {
        m_onAdd = std::move(onAdd);
        while (m_rowsLayout->count() > 0) {
            QLayoutItem *it = m_rowsLayout->takeAt(0);
            if (it->widget())
                it->widget()->deleteLater();
            delete it;
        }
        m_rows.clear();
        for (const MoneySource &s : items) {
            const QString id = s.id;
            auto *row = new SourceRow(
                s,
                [onOpen, id]() { if (onOpen) onOpen(id); },
                [onDelete, id]() { if (onDelete) onDelete(id); },
                [onRename, id]() { if (onRename) onRename(id); },
                m_rowsWidget);
            m_rowsLayout->addWidget(row);
            m_rows.append(row);
        }
        m_rowsWidget->setVisible(!items.isEmpty());
        updateGeometry();
        update();
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        paintRoundedBox(&p, QRectF(0.5, 0.5, width() - 1, height() - 1), 12,
                        Theme::zinc900_40, Theme::zinc800);
    }

private:
    QWidget *m_rowsWidget = nullptr;
    QVBoxLayout *m_rowsLayout = nullptr;
    QVector<SourceRow *> m_rows;
    std::function<void()> m_onAdd;
};

class SourceModalDialog : public ModalDialog
{
    Q_OBJECT
public:
    explicit SourceModalDialog(QWidget *parent = nullptr) : ModalDialog(parent)
    {
        m_titleBar = addTitleBar(QStringLiteral("Kaynak Ekle"));

        QWidget *stack = new QWidget(card());
        QVBoxLayout *l = new QVBoxLayout(stack);
        l->setContentsMargins(0, 0, 0, 0);
        l->setSpacing(8);

        m_name = new ModalLineEdit(stack);
        m_name->makeDescriptionStyle();
        m_name->setTextMargins(16, 0, 16, 0);
        m_name->setPlaceholderText(QStringLiteral("Kaynak adı"));
        m_name->onEnter = [this]() { tryConfirm(); };
        m_name->onEscape = [this]() { closeDialog(); };
        l->addWidget(m_name);

        m_nameError = new ErrorLabel(stack);
        l->addWidget(m_nameError);

        m_flagWidget = new QWidget(stack);
        QVBoxLayout *fl = new QVBoxLayout(m_flagWidget);
        fl->setContentsMargins(0, 8, 0, 0);
        fl->setSpacing(8);
        QLabel *flagLabel = new QLabel(QStringLiteral("Eksiye düşebilir mi?"), m_flagWidget);
        flagLabel->setFont(Theme::font(12, QFont::Normal));
        flagLabel->setStyleSheet(QStringLiteral("color: #A1A1AA;"));
        fl->addWidget(flagLabel);
        m_toggle = new ChoiceToggle(m_flagWidget);
        fl->addWidget(m_toggle, 0, Qt::AlignLeft);
        l->addWidget(m_flagWidget);

        cardLayout()->addWidget(stack);
        addButtonsRow([this]() { tryConfirm(); });
    }

    void openCreate(const QStringList &existingNames)
    {
        m_createMode = true;
        m_existing = existingNames;
        m_titleBar->setTitle(QStringLiteral("Kaynak Ekle"));
        m_name->clear();
        m_nameError->clear();
        m_toggle->setValue(false);
        m_flagWidget->show();
        refreshCardLayout();
        openDialog();
    }

    void openRename(const MoneySource &source, const QStringList &existingNames)
    {
        m_createMode = false;
        m_existing = existingNames;
        m_titleBar->setTitle(QStringLiteral("Yeniden Adlandır"));
        m_name->setText(source.name);
        m_nameError->clear();
        m_flagWidget->hide();
        refreshCardLayout();
        openDialog();
    }

    bool isCreateMode() const { return m_createMode; }

    void tryCreate(const QString &name, bool canGoNegative)
    {
        m_name->setText(name);
        m_toggle->setValue(canGoNegative);
        tryConfirm();
    }

    void tryRename(const QString &name)
    {
        m_name->setText(name);
        tryConfirm();
    }

    void tryConfirm()
    {
        const QString name = m_name->text().trimmed();
        if (name.isEmpty()) {
            m_nameError->setText(QStringLiteral("doldurmanız zorunlu."));
            m_name->setFocus();
            return;
        }
        if (isNakitSourceName(name)) {
            m_nameError->setText(QStringLiteral("Nakit adı kullanılamaz."));
            m_name->setFocus();
            return;
        }
        const QString norm = normalizedSourceName(name);
        for (const QString &existing : std::as_const(m_existing)) {
            if (normalizedSourceName(existing) == norm) {
                m_nameError->setText(QStringLiteral("Bu ad kullanılıyor."));
                m_name->setFocus();
                return;
            }
        }
        emit submitted(name, m_createMode ? m_toggle->value() : false);
        closeDialog();
    }

signals:
    void submitted(QString name, bool canGoNegative);

protected:
    void onOpened() override { m_name->setFocus(); }

private:
    void refreshCardLayout()
    {
        for (QWidget *w = m_name; w && w != card(); w = w->parentWidget()) {
            if (QLayout *l = w->layout()) {
                l->invalidate();
                l->activate();
            }
        }
    }

    ModalTitleBar *m_titleBar = nullptr;
    ModalLineEdit *m_name = nullptr;
    ErrorLabel *m_nameError = nullptr;
    QWidget *m_flagWidget = nullptr;
    ChoiceToggle *m_toggle = nullptr;
    QStringList m_existing;
    bool m_createMode = true;
};

class SourceInfoDialog : public ModalDialog
{
    Q_OBJECT
public:
    explicit SourceInfoDialog(QWidget *parent = nullptr) : ModalDialog(parent)
    {
        addTitleBar(QString());

        m_name = new QLabel(card());
        m_name->setAlignment(Qt::AlignCenter);
        m_name->setWordWrap(true);
        m_name->setFont(Theme::font(16, QFont::Medium));
        m_name->setStyleSheet(QStringLiteral("color: #FFFFFF;"));
        cardLayout()->addWidget(m_name);

        m_balance = new QLabel(card());
        m_balance->setAlignment(Qt::AlignCenter);
        QFont bf = Theme::font(26, QFont::Light);
        bf.setLetterSpacing(QFont::AbsoluteSpacing, Theme::trackingTight(26));
        m_balance->setFont(bf);
        m_balance->setStyleSheet(QStringLiteral("color: #E4E4E7;"));
        cardLayout()->addWidget(m_balance);

        QWidget *body = new QWidget(card());
        QHBoxLayout *bl = new QHBoxLayout(body);
        bl->setContentsMargins(0, 0, 0, 0);
        bl->setSpacing(12);

        QWidget *left = new QWidget(body);
        QVBoxLayout *ll = new QVBoxLayout(left);
        ll->setContentsMargins(0, 0, 0, 0);
        ll->setSpacing(8);
        ll->addWidget(new UppercaseLabel(QStringLiteral("SON İŞLEMLER"), Theme::zinc500, left));
        m_chips = new ChipsList(left);
        ll->addWidget(m_chips);
        m_emptyLabel = new QLabel(QStringLiteral("İşlem yok"), left);
        m_emptyLabel->setFont(Theme::font(12, QFont::Normal));
        m_emptyLabel->setStyleSheet(QStringLiteral("color: #71717A;"));
        m_emptyLabel->setFixedHeight(20);
        ll->addWidget(m_emptyLabel);
        ll->addStretch(1);
        bl->addWidget(left, 1);

        QWidget *right = new QWidget(body);
        QVBoxLayout *rl = new QVBoxLayout(right);
        rl->setContentsMargins(0, 0, 0, 0);
        rl->setSpacing(8);
        QLabel *flagLabel = new QLabel(QStringLiteral("Eksiye düşebilir mi?"), right);
        QFont lf = Theme::font(12, QFont::Normal);
        flagLabel->setFont(lf);
        flagLabel->setWordWrap(true);
        flagLabel->setStyleSheet(QStringLiteral("color: #A1A1AA;"));
        rl->addWidget(flagLabel);
        m_toggle = new ChoiceToggle(right);
        rl->addWidget(m_toggle, 0, Qt::AlignLeft);
        rl->addStretch(1);
        bl->addWidget(right, 0, Qt::AlignTop);

        cardLayout()->addWidget(body);

        connect(m_toggle, &ChoiceToggle::changed, this, [this](bool value) {
            emit canGoNegativeChanged(m_sourceId, value);
        });
    }

    void openFor(const MoneySource &source, const QVector<Transaction> &transactions)
    {
        m_sourceId = source.id;
        const bool locked = isNakitSourceName(source.name);
        m_name->setText(source.name);
        m_balance->setText(Fmt::formatNumber(source.balance) + QStringLiteral(" ₺"));
        m_toggle->setYesEnabled(!locked);
        m_toggle->setValue(!locked && source.canGoNegative);
        m_chips->setTransactions(transactions);
        m_chips->setVisible(!transactions.isEmpty());
        m_emptyLabel->setVisible(transactions.isEmpty());
        for (QWidget *w = m_chips->parentWidget(); w && w != card(); w = w->parentWidget()) {
            if (QLayout *l = w->layout()) {
                l->invalidate();
                l->activate();
            }
        }
        openDialog();
    }

    double cardMaxWidth() const override { return 336.0; }

signals:
    void canGoNegativeChanged(QString id, bool value);

protected:
    void onOpened() override {}

private:
    QLabel *m_name = nullptr;
    QLabel *m_balance = nullptr;
    ChipsList *m_chips = nullptr;
    QLabel *m_emptyLabel = nullptr;
    ChoiceToggle *m_toggle = nullptr;
    QString m_sourceId;
};

#endif
