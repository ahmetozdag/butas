#ifndef LINEEDIT_H
#define LINEEDIT_H

#include <QLineEdit>
#include <QKeyEvent>
#include <QPainter>
#include <QPalette>
#include <functional>
#include "theme.h"

class StyledLineEdit : public QLineEdit{
    Q_OBJECT
public:
    explicit StyledLineEdit(QWidget *parent = nullptr) : QLineEdit(parent){
        setStyleSheet(
            "QLineEdit {"
            "  background-color: rgba(31,31,35,230);"
            "  border: 1px solid rgba(63,63,70,204);"
            "  border-radius: 12px;"
            "  color: #FFFFFF;"
            "  selection-background-color: #FFFFFF;"
            "  selection-color: #000000;"
            "}"
            "QLineEdit:focus {"
            "  border: 1px solid #71717A;"
            "}"
        );
        QPalette pal = palette();
        pal.setColor(QPalette::PlaceholderText, Theme::zinc500);
        setPalette(pal);
        setAttribute(Qt::WA_MacShowFocusRect, false);
        setCursor(Qt::IBeamCursor);
        setFixedHeight(50);
    }

    void makeDescriptionStyle(){
        setFixedHeight(46);
        QFont f = Theme::font(14, QFont::Normal);
        f.setLetterSpacing(QFont::AbsoluteSpacing, Theme::trackingTight(14));
        setFont(f);
    }

    void setSuffixText(const QString &s) { m_suffix = s; update(); }
    void setSuffixBold(bool b) { m_suffixBold = b; update(); }

    void applyAmountStyle(){
        QFont f = Theme::font(16, QFont::Normal);
        f.setLetterSpacing(QFont::AbsoluteSpacing, Theme::trackingTight(16));
        setFont(f);
    }

protected:
    void paintEvent(QPaintEvent *event) override{
        QLineEdit::paintEvent(event);
        if (!m_suffix.isEmpty()){
            QPainter p(this);
            p.setRenderHint(QPainter::TextAntialiasing);
            QFont f = Theme::font(16, m_suffixBold ? QFont::Bold : QFont::Normal);
            p.setFont(f);
            p.setPen(QPen(Theme::zinc400));
            p.drawText(QRect(0, 0, width() - 16, height()),
                       Qt::AlignRight | Qt::AlignVCenter, m_suffix);
        }
    }

private:
    QString m_suffix;
    bool m_suffixBold = false;
};

class ModalLineEdit : public StyledLineEdit{
    Q_OBJECT
public:
    explicit ModalLineEdit(QWidget *parent = nullptr) : StyledLineEdit(parent) {}

    std::function<void()> onEnter;
    std::function<void()> onEscape;

protected:
    void keyPressEvent(QKeyEvent *e) override{
        if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter){
            if (onEnter) { onEnter(); }
            return;
        }
        if (e->key() == Qt::Key_Escape){
            if (onEscape) { onEscape(); }
            return;
        }
        StyledLineEdit::keyPressEvent(e);
    }
};

#endif
