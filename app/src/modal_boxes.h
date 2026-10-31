#ifndef MODAL_BOXES_H
#define MODAL_BOXES_H

#include "modals.h"
#include <QLabel>

class ActionModalDialog : public ModalDialog{
    Q_OBJECT
public:
    explicit ActionModalDialog(QWidget *parent = nullptr) : ModalDialog(parent){
        m_titleBar = addTitleBar(QLatin1String("Para Ekle"));
        m_titleBar->setTitle(QLatin1String("Para Ekle"));

        QWidget *inputRow = new QWidget(card());
        QHBoxLayout *l = new QHBoxLayout(inputRow);
        l->setContentsMargins(0, 0, 0, 0);
        l->setSpacing(0);
        m_input = new ModalLineEdit(inputRow);
        m_input->applyAmountStyle();
        m_input->setSuffixText(QStringLiteral("₺"));
        m_input->setTextMargins(16, 0, 40, 0);
        m_input->setPlaceholderText(QLatin1String("0,00"));
        m_input->setInputMethodHints(Qt::ImhPreferNumbers | Qt::ImhNoPredictiveText);
        m_input->onEnter  = [this]() { tryConfirm(); };
        m_input->onEscape = [this]() { closeDialog(); };
        l->addWidget(m_input);
        cardLayout()->addWidget(inputRow);

        addButtonsRow([this]() { tryConfirm(); });
    }

    void openModal(bool add){
        m_modeIsAdd = add;
        m_titleBar->setTitle(add ? QLatin1String("Para Ekle") : QStringLiteral("Para Çıkar"));
        m_input->clear();
        openDialog();
    }

    void tryConfirm(){
        bool ok = false;
        const double parsed = Fmt::parseJsFloat(m_input->text(), ok);
        if (ok && parsed > 0){
            emit confirmed(m_modeIsAdd ? parsed : -parsed);
            closeDialog();
        } else{
            m_input->setFocus();
        }
    }

    void tryWithText(const QString &text){
        m_input->setText(text);
        tryConfirm();
    }

signals:
    void confirmed(double signedAmount);

protected:
    void onOpened() override { m_input->setFocus(); }

private:
    ModalTitleBar *m_titleBar = nullptr;
    ModalLineEdit *m_input = nullptr;
    bool m_modeIsAdd = true;
};

class RecurringModalDialog : public ModalDialog{
    Q_OBJECT
public:
    explicit RecurringModalDialog(QWidget *parent = nullptr) : ModalDialog(parent){
        m_titleBar = addTitleBar(QString());
        m_titleBar->setTitle(QLatin1String("Gelir Ekle"));

        QWidget *stack = new QWidget(card());
        QVBoxLayout *l = new QVBoxLayout(stack);
        l->setContentsMargins(0, 0, 0, 0);
        l->setSpacing(12);

        m_selector = new CurrencySelector(stack);
        connect(m_selector, &CurrencySelector::selected, this,
                &RecurringModalDialog::onCurrencyChanged);
        l->addWidget(m_selector);

        QWidget *amountRow = new QWidget(stack);
        QHBoxLayout *al = new QHBoxLayout(amountRow);
        al->setContentsMargins(0, 0, 0, 0);
        al->setSpacing(0);
        m_amount = new ModalLineEdit(amountRow);
        m_amount->applyAmountStyle();
        m_amount->setTextMargins(16, 0, 40, 0);
        m_amount->setPlaceholderText(QLatin1String("Miktar"));
        m_amount->setInputMethodHints(Qt::ImhPreferNumbers | Qt::ImhNoPredictiveText);
        m_amount->onEnter  = [this]() { m_desc->setFocus(); };
        m_amount->onEscape = [this]() { closeDialog(); };
        al->addWidget(m_amount);
        l->addWidget(amountRow);

        m_desc = new ModalLineEdit(stack);
        m_desc->makeDescriptionStyle();
        m_desc->setTextMargins(16, 0, 16, 0);
        m_desc->onEnter  = [this]() { tryConfirm(); };
        m_desc->onEscape = [this]() { closeDialog(); };
        l->addWidget(m_desc);

        cardLayout()->addWidget(stack);
        addButtonsRow([this]() { tryConfirm(); });
    }

    void openModal(bool income){
        m_modeIncome = income;
        m_titleBar->setTitle(income ? QLatin1String("Gelir Ekle") : QLatin1String("Gider Ekle"));
        m_desc->setPlaceholderText(income ? QStringLiteral("Açıklama (örn. BURS)")
                                          : QStringLiteral("Açıklama (örn. OPENCODE GO)"));
        m_selector->selectCurrency(income ? QStringLiteral("₺") : QLatin1String("$"));
        m_amount->clear();
        m_desc->clear();
        openDialog();
    }

    void tryConfirm(){
        QString rawAmount;
        for (const QChar ch : m_amount->text()){
            if (ch.isSpace())
                continue;
            rawAmount += ch;
        }
        const int comma = rawAmount.indexOf(QLatin1Char(','));
        if (comma >= 0)
            rawAmount = rawAmount.left(comma) + QChar('.') + rawAmount.mid(comma + 1);
        rawAmount = rawAmount.trimmed();

        if (rawAmount.isEmpty()){
            m_amount->setFocus();
            return;
        }

        bool ok = false;
        const double parsed = Fmt::parseJsFloat(rawAmount, ok);
        const QString amountText = ok ? Fmt::formatTrLocale(parsed) : rawAmount;

        const QString desc = m_desc->text().trimmed();
        const QString defaultLabel = m_modeIncome ? QStringLiteral("GELİR") : QStringLiteral("GİDER");
        const QString labelText = desc.isEmpty() ? defaultLabel : desc.toUpper();
        const QString currency = m_selector->currentCurrency();

        emit confirmed(amountText, currency, labelText);
        closeDialog();
    }

    void tryWith(bool income, const QString &amount, const QString &desc,
                 const QString &currency){
        openModal(income);
        m_amount->setText(amount);
        m_desc->setText(desc);
        m_selector->selectCurrency(currency);
        tryConfirm();
    }

    bool isIncomeMode() const { return m_modeIncome; }

signals:
    void confirmed(QString amount, QString currency, QString label);

protected:
    void onOpened() override{
        m_amount->setFocus();
    }

private:
    void onCurrencyChanged(const QString &c){
        m_amount->setSuffixText(c);
    }

    ModalTitleBar *m_titleBar = nullptr;
    CurrencySelector *m_selector = nullptr;
    ModalLineEdit *m_amount = nullptr;
    ModalLineEdit *m_desc = nullptr;
    bool m_modeIncome = true;
};

class DeleteModalDialog : public ModalDialog{
    Q_OBJECT
public:
    explicit DeleteModalDialog(QWidget *parent = nullptr) : ModalDialog(parent){
        addTitleBar(QStringLiteral("Silme İşlemi"));

        QLabel *desc = new QLabel(QStringLiteral("Silmek istediğinize emin misiniz?"), card());
        QFont f = Theme::font(14, QFont::Normal);
        desc->setFont(f);
        desc->setStyleSheet("color: #A1A1AA;");
        desc->setFixedHeight(20);
        cardLayout()->addWidget(desc);

        addButtonsRow([this]() {
            emit confirmed();
            closeDialog();
        });
    }

    void openModal() { openDialog(); }

signals:
    void confirmed();

protected:
    void onOpened() override {}
};

#endif
