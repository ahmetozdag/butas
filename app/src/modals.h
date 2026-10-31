#ifndef MODALS_H
#define MODALS_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QImage>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QElapsedTimer>
#include <functional>
#include "widgets.h"
#include "linedit.h"
#include "blur.h"
#include "icons.h"
#include "theme.h"
#include "format.h"

class TitleLabel : public QWidget{
    Q_OBJECT
public:
    TitleLabel(QWidget *parent = nullptr) : QWidget(parent) { setFixedHeight(24); }
    void setTitle(const QString &t) { m_text = t; update(); }
protected:
    void paintEvent(QPaintEvent *) override{
        QPainter p(this);
        p.setRenderHint(QPainter::TextAntialiasing);
        QFont f = Theme::font(16, QFont::Medium);
        f.setLetterSpacing(QFont::AbsoluteSpacing, Theme::trackingTight(16));
        p.setFont(f);
        p.setPen(QPen(Theme::white));
        p.drawText(rect(), Qt::AlignLeft | Qt::AlignVCenter, m_text);
    }
private:
    QString m_text;
};

class ModalTitleBar : public QWidget{
    Q_OBJECT
public:
    ModalTitleBar(const QString &title, std::function<void()> onClose, QWidget *parent = nullptr)
        : QWidget(parent){
        setFixedHeight(24);
        QHBoxLayout *l = new QHBoxLayout(this);
        l->setContentsMargins(0, 0, 0, 0);
        l->setSpacing(0);
        m_label = new TitleLabel(this);
        m_label->setTitle(title);
        l->addWidget(m_label, 1);
        auto *closeBtn = new IconButton(20.0, Theme::zinc400, Theme::white, this);
        connect(closeBtn, &QAbstractButton::clicked, this, [onClose]() { if (onClose) onClose(); });
        l->addWidget(closeBtn);
    }
    void setTitle(const QString &t) { m_label->setTitle(t); }
private:
    TitleLabel *m_label = nullptr;
};

class TextButton : public QAbstractButton{
    Q_OBJECT
public:
    enum Role { Cancel, Confirm };
    TextButton(Role role, QWidget *parent = nullptr)
        : QAbstractButton(parent), m_role(role){
        setCursor(Qt::PointingHandCursor);
        setAttribute(Qt::WA_Hover, true);
        setFocusPolicy(Qt::NoFocus);
        setFixedHeight(40);
    }
protected:
    void paintEvent(QPaintEvent *) override{
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        QColor bg, fg;
        if (m_role == Cancel) {
            bg = underMouse() ? Theme::zinc700 : Theme::zinc800;
            fg = Theme::zinc300;
        } else {
            bg = underMouse() ? Theme::zinc200 : Theme::white;
            fg = Theme::zinc900;
        }
        p.setPen(Qt::NoPen);
        p.setBrush(bg);
        p.drawRoundedRect(rect(), 12, 12);
        QFont f = Theme::font(14, QFont::Medium);
        p.setFont(f);
        p.setPen(QPen(fg));
        p.drawText(rect(), Qt::AlignCenter, text());
    }
private:
    Role m_role;
};

class CurrencyButton : public QAbstractButton{
    Q_OBJECT
public:
    CurrencyButton(const QString &name, const QString &symbol, QWidget *parent = nullptr)
        : QAbstractButton(parent), m_name(name), m_symbol(symbol), m_selected(false){
        setCursor(Qt::PointingHandCursor);
        setAttribute(Qt::WA_Hover, true);
        setFocusPolicy(Qt::NoFocus);
        setFixedHeight(28);
        setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
    }
    QString symbol() const{ return m_symbol; }
    void setSelected(bool s) { m_selected = s; update(); }
signals:
    void picked(QString currency);
protected:
    void mouseReleaseEvent(QMouseEvent *e) override{
        QAbstractButton::mouseReleaseEvent(e);
        if (rect().contains(e->pos()))
            emit picked(m_symbol);
    }
    void paintEvent(QPaintEvent *) override{
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        if (m_selected){
            p.setPen(Qt::NoPen);
            p.setBrush(Theme::white);
            p.drawRoundedRect(rect(), 8, 8);
        }
        const QColor c = m_selected ? Theme::zinc900
                                    : (underMouse() ? Theme::white : Theme::zinc400);
        QFont fLabel = Theme::font(12, m_selected ? QFont::Bold : QFont::Medium);
        fLabel.setLetterSpacing(QFont::AbsoluteSpacing, Theme::trackingWide(12));
        QFont fSymbol = Theme::font(12, QFont::Bold);
        fSymbol.setLetterSpacing(QFont::AbsoluteSpacing, Theme::trackingWide(12));
        QFontMetricsF fmL(fLabel);
        QFontMetricsF fmS(fSymbol);
        const double gap = 4.0;
        const double totalW = fmL.horizontalAdvance(m_name) + gap + fmS.horizontalAdvance(m_symbol);
        const double x = (width() - totalW) / 2.0;
        const double y = (height() - (fmL.ascent() + fmL.descent())) / 2.0 + fmL.ascent();
        p.setFont(fLabel); p.setPen(QPen(c));
        p.drawText(QPointF(x, y), m_name);
        p.setFont(fSymbol);
        p.drawText(QPointF(x + fmL.horizontalAdvance(m_name) + gap, y), m_symbol);
    }
private:
    QString m_name, m_symbol;
    bool m_selected;
};

class CurrencySelector : public QWidget{
    Q_OBJECT
public:
    explicit CurrencySelector(QWidget *parent = nullptr) : QWidget(parent){
        setFixedHeight(38);
        QHBoxLayout *l = new QHBoxLayout(this);
        l->setContentsMargins(5, 5, 5, 5);
        l->setSpacing(4);
        const struct { QString n, s; } opts[3] = {
            { QLatin1String("TL"),    QStringLiteral("₺") },
            { QLatin1String("Dolar"), QLatin1String("$") },
            { QLatin1String("Euro"),  QStringLiteral("€") },
        };
        for (auto &o : opts){
            auto *b = new CurrencyButton(o.n, o.s, this);
            b->setMinimumWidth(1);
            l->addWidget(b, 1);
            m_buttons.append(b);
            connect(b, &CurrencyButton::picked, this, [this](const QString &c) {
                selectCurrency(c);
            });
        }
        selectCurrency(QStringLiteral("₺"));
    }

    void selectCurrency(const QString &c){
        m_selected = c;
        for (CurrencyButton *b : std::as_const(m_buttons))
            b->setSelected(b->symbol() == c);
        emit selected(c);
    }
    QString currentCurrency() const { return m_selected; }

protected:
    void paintEvent(QPaintEvent *e) override{
        QPainter p(this);
        paintRoundedBox(&p, QRectF(0.5, 0.5, width() - 1.0, height() - 1.0), 12,
                        Theme::zinc800_80bg, Theme::zinc700_80);
        QWidget::paintEvent(e);
    }

signals:
    void selected(const QString &currency);

private:
    QVector<CurrencyButton *> m_buttons;
    QString m_selected;
};

class RoundedCard : public QWidget{
    Q_OBJECT
public:
    explicit RoundedCard(QWidget *parent = nullptr) : QWidget(parent) {}
protected:
    void paintEvent(QPaintEvent *) override{
        QPainter p(this);
        paintRoundedBox(&p, QRectF(0.5, 0.5, width() - 1.0, height() - 1.0), 16,
                        Theme::zinc900, Theme::zinc800);
    }
};

class ModalDialog : public QWidget{
    Q_OBJECT
public:
    bool isOpen() const { return isVisible(); }

    void openDialog(){
        snapshotBackdrop();
        layoutCard();
        m_value = 0.0;
        m_card->hide();
        reposition();
        show();
        raise();
        startAnim(true);
    }

    void closeDialog(){
        if (!isVisible())
            return;
        startAnim(false);
    }

    void onParentResized(){
        if (!isVisible()) return;
        reposition();
        layoutCard();
        if (m_card->isVisible())
            m_card->update();
    }

    explicit ModalDialog(QWidget *parent = nullptr) : QWidget(parent){
        m_card = new RoundedCard(this);
        m_cardLayout = new QVBoxLayout(m_card);
        m_cardLayout->setContentsMargins(24, 24, 24, 24);
        m_cardLayout->setSpacing(20);
        m_card->hide();
        hide();
        setMouseTracking(true);
    }

    ModalTitleBar *addTitleBar(const QString &title){
        auto *tb = new ModalTitleBar(title, [this]() { closeDialog(); }, m_card);
        m_cardLayout->addWidget(tb);
        m_titleBar = tb;
        return tb;
    }

    void addButtonsRow(std::function<void()> onConfirm){
        QWidget *row = new QWidget(m_card);
        QHBoxLayout *l = new QHBoxLayout(row);
        l->setContentsMargins(0, 0, 0, 0);
        l->setSpacing(12);
        auto *cancel = new TextButton(TextButton::Cancel, row);
        cancel->setText(QStringLiteral("İptal"));
        auto *confirm = new TextButton(TextButton::Confirm, row);
        confirm->setText(QLatin1String("Onayla"));
        connect(cancel, &QAbstractButton::clicked, this, &ModalDialog::closeDialog);
        connect(confirm, &QAbstractButton::clicked, this, [this, onConfirm]() {
            if (onConfirm) onConfirm();
        });
        l->addWidget(cancel, 1);
        l->addWidget(confirm, 1);
        m_cardLayout->addWidget(row);
    }

    QWidget *card() const { return m_card; }
    QVBoxLayout *cardLayout() const { return m_cardLayout; }
    ModalTitleBar *titleBar() const { return m_titleBar; }

    virtual void onOpened() = 0;

    void layoutCard(){
        const double bw = std::min(320.0, double(parentWidget()->width() - 48));
        m_card->setFixedSize(int(bw), m_card->sizeHint().height());
        QImage snap(m_card->width(), m_card->height(), QImage::Format_ARGB32_Premultiplied);
        snap.fill(Qt::transparent);
        QPainter sp(&snap);
        m_card->render(&sp, QPoint(0, 0), QRegion(),
                       QWidget::DrawWindowBackground | QWidget::DrawChildren);
        sp.end();
        m_snapshot = snap;
        m_shadow = QImage();
    }

private slots:
    void onAnimTick(){
        if (m_animActive){
            const qint64 ms = m_elapsed.elapsed();
            if (ms >= 200) {
                m_value = 1.0;
                killTimer(m_timerId);
                m_timerId = 0;
                m_animActive = false;
                if (m_opening){
                    m_card->show();
                    onOpened();
                } else{
                    hide();
                }
            } else{
                m_value = double(ms) / 200.0;
            }
            update();
        }
    }

    void timerEvent(QTimerEvent *e) override{
        if (e->timerId() == m_timerId)
            onAnimTick();
        QWidget::timerEvent(e);
    }

private:
    void startAnim(bool opening){
        m_opening = opening;
        if (m_timerId){
            killTimer(m_timerId);
            m_timerId = 0;
        }
        m_elapsed.start();
        m_timerId = startTimer(16, Qt::PreciseTimer);
        m_animActive = true;
        if (m_opening)
            update();
    }

private:
    void snapshotBackdrop(){
        QWidget *pw = parentWidget();
        if (!pw) return;
        QImage img = pw->grab().toImage();
        const QSize logical(pw->width(), pw->height());
        if (!img.isNull())
            img = img.scaled(logical.width(), logical.height(), Qt::IgnoreAspectRatio,
                             Qt::SmoothTransformation);
        BlurEffect::stackBlur(img, 4);
        m_backdrop = img;
    }

    void reposition(){
        if (parentWidget() && geometry() != parentWidget()->rect())
            setGeometry(parentWidget()->rect());
        if (m_card)
            m_card->move((width() - m_card->width()) / 2, (height() - m_card->height()) / 2);
    }

    void paintEvent(QPaintEvent *) override{
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        if (!m_backdrop.isNull()){
            p.drawImage(rect(), m_backdrop);
        }
        p.fillRect(rect(), Theme::black75);

        if (m_card && !m_card->isVisible()){
            const QRectF cr = QRectF(m_card->geometry());
            if (m_shadow.isNull())
                m_shadow = makeShadow(QSizeF(cr.width(), cr.height()));
            p.drawImage(QPointF(cr.x() - 90, cr.y() - 50 + 25), m_shadow);
        }
        if (!m_opening){
            p.save();
            const double cx = width() / 2.0, cy = height() / 2.0;
            const double s = 1.0 - 0.05 * m_value;
            p.translate(cx, cy);
            p.scale(s, s);
            p.setOpacity(1.0 - m_value);
            p.translate(-m_snapshot.width() / 2.0, -m_snapshot.height() / 2.0);
            p.drawImage(QPointF(0, 0), m_snapshot);
            p.restore();
        } else if(m_card->isVisible()){
        } else{
            p.save();
            const double cx = width() / 2.0, cy = height() / 2.0;
            const double s = 0.95 + 0.05 * m_value;
            p.translate(cx, cy);
            p.scale(s, s);
            p.setOpacity(m_value);
            p.translate(-m_snapshot.width() / 2.0, -m_snapshot.height() / 2.0);
            p.drawImage(QPointF(0, 0), m_snapshot);
            p.restore();
        }
    }

    static QImage makeShadow(const QSizeF &boxSize){
        const int pad = 90;
        const double w = boxSize.width() - 24.0;
        const double h = boxSize.height() - 24.0;
        QImage img(int(boxSize.width()) + pad * 2, int(boxSize.height()) + pad * 2,
                   QImage::Format_ARGB32_Premultiplied);
        img.fill(Qt::transparent);
        QPainter pp(&img);
        pp.setRenderHint(QPainter::Antialiasing);
        pp.setPen(Qt::NoPen);
        pp.setBrush(QColor(0, 0, 0, 64));
        pp.drawRoundedRect(QRectF(pad + 12, pad + 12, w, h), 16, 16);
        pp.end();
        BlurEffect::stackBlur(img, 25);
        return img;
    }

    void mousePressEvent(QMouseEvent *e) override{
        if (m_card && !m_card->geometry().contains(e->pos())){
            closeDialog();
        }
    }

    RoundedCard *m_card = nullptr;
    QVBoxLayout *m_cardLayout = nullptr;
    ModalTitleBar *m_titleBar = nullptr;
    QImage m_backdrop;
    QImage m_snapshot;
    QImage m_shadow;
    double m_value = 1.0;
    bool m_opening = true;
    bool m_animActive = false;
    int m_timerId = 0;
    QElapsedTimer m_elapsed;
};

#endif
