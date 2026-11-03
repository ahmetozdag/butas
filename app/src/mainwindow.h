#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QScreen>
#include <QScrollArea>
#include <QScroller>
#include <QScrollerProperties>
#include <QScrollBar>
#include <QScrollEvent>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QSaveFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QTimer>
#include <QUuid>
#include <functional>
#include "widgets.h"
#include "panel.h"
#include "theme.h"
#include "modal_boxes.h"
#include "sources.h"

class RepaintScrollArea : public QScrollArea{
    Q_OBJECT
public:
    explicit RepaintScrollArea(QWidget *parent = nullptr) : QScrollArea(parent) {}

protected:
    bool viewportEvent(QEvent *event) override{
        if (event->type() == QEvent::Scroll){
            QScrollEvent *se = static_cast<QScrollEvent *>(event);
            horizontalScrollBar()->setValue(qRound(se->contentPos().x()));
            verticalScrollBar()->setValue(qRound(se->contentPos().y()));
            return true;
        }
        return QScrollArea::viewportEvent(event);
    }

    void scrollContentsBy(int dx, int dy) override{
        QScrollArea::scrollContentsBy(dx, dy);
        viewport()->update();
    }
};

class MainWindow : public QMainWindow{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr) : QMainWindow(parent){
        QPalette pal = palette();
        pal.setColor(QPalette::Window, Theme::background);
        setPalette(pal);
        setAutoFillBackground(true);

        m_scroll = new RepaintScrollArea(this);
        m_scroll->setFrameShape(QFrame::NoFrame);
        m_scroll->setWidgetResizable(true);
        m_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
#ifdef Q_OS_ANDROID
        m_scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        QScroller *scroller = QScroller::scroller(m_scroll->viewport());
        QScroller::grabGesture(m_scroll->viewport(), QScroller::TouchGesture);
        QScrollerProperties sp = scroller->scrollerProperties();
        sp.setScrollMetric(QScrollerProperties::HorizontalOvershootPolicy,
                           QScrollerProperties::OvershootAlwaysOff);
        sp.setScrollMetric(QScrollerProperties::VerticalOvershootPolicy,
                           QScrollerProperties::OvershootAlwaysOff);
        sp.setScrollMetric(QScrollerProperties::OvershootDragResistanceFactor, 0.0);
        sp.setScrollMetric(QScrollerProperties::OvershootDragDistanceFactor, 0.0);
        sp.setScrollMetric(QScrollerProperties::OvershootScrollDistanceFactor, 0.0);
        scroller->setScrollerProperties(sp);
        connect(scroller, &QScroller::stateChanged, this,
                [this](QScroller::State state){
                    if (state == QScroller::Inactive)
                        m_scroll->repaint();
                });
#endif
        m_scroll->viewport()->setAutoFillBackground(true);
        QPalette vp = m_scroll->viewport()->palette();
        vp.setColor(QPalette::Window, Theme::background);
        m_scroll->viewport()->setPalette(vp);

        QWidget *content = new QWidget;
        content->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
        content->setAutoFillBackground(true);
        QPalette cp = content->palette();
        cp.setColor(QPalette::Window, Theme::background);
        content->setPalette(cp);
        QVBoxLayout *v = new QVBoxLayout(content);
        v->setContentsMargins(28, 40, 28, 40);
        v->setSpacing(0);

        m_balance = new BalanceWidget(content);
        v->addWidget(m_balance, 0, Qt::AlignLeft);

        v->addSpacing(56);

        QWidget *actionRow = new QWidget(content);
        QHBoxLayout *ar = new QHBoxLayout(actionRow);
        ar->setContentsMargins(0, 0, 0, 0);
        ar->setSpacing(10);
        m_btnAdd      = new ActionButton(Theme::GLYPH_ADD_CIRCLE,    QLatin1String("Para Ekle"), actionRow);
        m_btnSubtract = new ActionButton(Theme::GLYPH_REMOVE_CIRCLE, QStringLiteral("Para Çıkar"), actionRow);
        connect(m_btnAdd, &QAbstractButton::clicked, this, [this]() { openActionModal(true); });
        connect(m_btnSubtract, &QAbstractButton::clicked, this, [this]() { openActionModal(false); });
        ar->addWidget(m_btnAdd, 1);
        ar->addWidget(m_btnSubtract, 1);
        v->addWidget(actionRow);

        v->addSpacing(56);

        QWidget *middle = new QWidget(content);
        QHBoxLayout *mr = new QHBoxLayout(middle);
        mr->setContentsMargins(0, 0, 0, 0);
        mr->setSpacing(16);
        m_chips = new ChipsList(middle);
        m_panel = new PanelCard(middle);
        m_panel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        mr->addWidget(m_chips);
        mr->addWidget(m_panel, 1);
        v->addWidget(middle);

        QWidget *secondaryRow = new QWidget(content);
        QHBoxLayout *sr = new QHBoxLayout(secondaryRow);
        sr->setContentsMargins(0, 0, 0, 0);
        sr->setSpacing(10);
        m_btnIncome  = new ActionButton(Theme::GLYPH_ADD_CIRCLE,    QLatin1String("Gelir Ekle"), secondaryRow);
        m_btnExpense = new ActionButton(Theme::GLYPH_REMOVE_CIRCLE, QLatin1String("Gider Ekle"), secondaryRow);
        connect(m_btnIncome, &QAbstractButton::clicked, this, [this]() { openRecurringModal(true); });
        connect(m_btnExpense, &QAbstractButton::clicked, this, [this]() { openRecurringModal(false); });
        sr->addWidget(m_btnIncome, 1);
        sr->addWidget(m_btnExpense, 1);
        v->addSpacing(16);
        v->addWidget(secondaryRow);

        v->addSpacing(16);
        m_sourcesCard = new SourcesCard(content);
        v->addWidget(m_sourcesCard);

        m_scroll->setWidget(content);
        setCentralWidget(m_scroll);

        m_actionModal = new ActionModalDialog(this);
        connect(m_actionModal, &ActionModalDialog::confirmed, this, &MainWindow::handleBalanceModalConfirm);
        connect(m_actionModal, &ActionModalDialog::sourceAddRequested, this, [this]() { openSourceCreate(true); });
        m_recurringModal = new RecurringModalDialog(this);
        connect(m_recurringModal, &RecurringModalDialog::confirmed, this, &MainWindow::handleRecurringConfirm);
        m_deleteModal = new DeleteModalDialog(this);
        connect(m_deleteModal, &DeleteModalDialog::confirmed, this, &MainWindow::handleDeleteConfirm);
        m_sourceModal = new SourceModalDialog(this);
        connect(m_sourceModal, &SourceModalDialog::submitted, this, &MainWindow::handleSourceSubmitted);
        m_sourceInfo = new SourceInfoDialog(this);
        connect(m_sourceInfo, &SourceInfoDialog::canGoNegativeChanged, this, &MainWindow::handleSourceFlagChanged);

        loadState();
        refreshBalance();
        renderTransactions();
        renderRecurringItems();
        renderSources();
        updateViewportFonts();
    }

public:
    Q_INVOKABLE void debugOpenActionModal(bool add) { openActionModal(add); }
    Q_INVOKABLE void debugOpenRecurringModal(bool income) { openRecurringModal(income); }
    Q_INVOKABLE void debugOpenDeleteModal(int idx) { openDeleteModal(Income, idx); }
    Q_INVOKABLE void debugOpenSourceCreate() { openSourceCreate(false); }
    Q_INVOKABLE void debugOpenSourceInfo() { openSourceInfo(sourceIdByName(QStringLiteral("Nakit"))); }
    Q_INVOKABLE void debugOpenSourceInfoNamed(const QString &name) { openSourceInfo(sourceIdByName(name)); }
    Q_INVOKABLE void debugOpenSourceDeleteNamed(const QString &name) { openSourceDelete(sourceIdByName(name)); }
    Q_INVOKABLE void debugOpenSourceMenu() { m_actionModal->debugOpenSourceMenu(); }
    Q_INVOKABLE void debugActionError(int which){
        if (which == 1){
            openActionModal(false);
            m_actionModal->selectSource(sourceIdByName(QStringLiteral("Nakit")));
            m_actionModal->tryWithText(QStringLiteral("5000"));
        } else{
            openActionModal(true);
            m_actionModal->tryWithText(QStringLiteral("100"));
        }
    }
    Q_INVOKABLE void debugSourceError(int which){
        openSourceCreate(false);
        m_sourceModal->tryCreate(which == 1 ? QString() : QStringLiteral("nakit"), false);
    }

    void debugRunScript(){
        debugDump(QStringLiteral("initial"));
        openSourceCreate(false);
        m_sourceModal->tryCreate(QStringLiteral("Ziraat"), true);
        debugDump(QStringLiteral("after create source Ziraat"));
        openSourceCreate(false);
        m_sourceModal->tryCreate(QStringLiteral("nakit"), false);
        debugDump(QStringLiteral("after duplicate nakit attempt"));
        openSourceCreate(false);
        m_sourceModal->tryCreate(QString(), false);
        debugDump(QStringLiteral("after empty name attempt"));
        m_sourceModal->closeDialog();

        openActionModal(true);
        m_actionModal->tryWithText(QStringLiteral("1250,75"));
        debugDump(QStringLiteral("after add without source"));
        m_actionModal->selectSource(sourceIdByName(QStringLiteral("Nakit")));
        m_actionModal->tryWithText(QStringLiteral("1250,75"));
        debugDump(QStringLiteral("after add 1250,75 nakit"));
        openActionModal(true);
        m_actionModal->debugRequestAddSource();
        m_sourceModal->tryCreate(QStringLiteral("Enpara"), false);
        debugDump(QStringLiteral("after create from action modal"));
        m_actionModal->tryWithText(QStringLiteral("200"));
        debugDump(QStringLiteral("after add 200 enpara"));
        openActionModal(false);
        m_actionModal->selectSource(sourceIdByName(QStringLiteral("Nakit")));
        m_actionModal->tryWithText(QStringLiteral("5000"));
        debugDump(QStringLiteral("after insufficient nakit"));
        m_actionModal->closeDialog();

        openActionModal(false);
        m_actionModal->selectSource(sourceIdByName(QStringLiteral("Ziraat")));
        m_actionModal->tryWithText(QStringLiteral("85,50"));
        debugDump(QStringLiteral("after subtract 85,50 ziraat"));

        m_recurringModal->tryWith(true, QStringLiteral("4500,25"), QStringLiteral("kira"),
                                  QStringLiteral("€"));
        debugDump(QStringLiteral("after income kira"));
        m_recurringModal->tryWith(false, QStringLiteral("19,99"), QString(),
                                  QStringLiteral("$"));
        debugDump(QStringLiteral("after expense default label"));
        m_deleteTargetType = Income;
        m_deleteTargetIndex = 0;
        handleDeleteConfirm();
        debugDump(QStringLiteral("after delete income 0"));

        openSourceRename(sourceIdByName(QStringLiteral("Ziraat")));
        m_sourceModal->tryRename(QStringLiteral("Garanti"));
        debugDump(QStringLiteral("after rename Ziraat -> Garanti"));
        handleSourceFlagChanged(sourceIdByName(QStringLiteral("Garanti")), false);
        debugDump(QStringLiteral("after flag change Garanti -> H"));
        openSourceDelete(sourceIdByName(QStringLiteral("Garanti")));
        handleDeleteConfirm();
        debugDump(QStringLiteral("after delete Garanti"));
    }

    void debugDump(const QString &when) const{
        QString chips;
        for (const auto &t : std::as_const(m_transactions))
            chips += (t.isAdd ? QStringLiteral("+") : QStringLiteral("-")) +
                     Fmt::formatNumber(t.amount) + QStringLiteral(" ");
        QStringList inc;
        for (const auto &i : std::as_const(m_incomes))
            inc << i.amount << i.currency << i.label;
        QStringList exp;
        for (const auto &e : std::as_const(m_expenses))
            exp << e.amount << e.currency << e.label;
        QStringList src;
        for (const auto &s : std::as_const(m_sources))
            src << s.name + QStringLiteral(":") + Fmt::formatNumber(s.balance) +
                       (s.canGoNegative ? QStringLiteral(":E") : QStringLiteral(":H"));
        QString out;
        QTextStream ts(&out);
        ts << "--DUMP(" << when << ")--\n";
        ts << "total=" << Fmt::formatNumber(totalBalance()) << "\n";
        ts << "sources=[" << src.join(QStringLiteral("|")) << "]\n";
        ts << "transactions=[" << chips.trimmed() << "] count=" << m_transactions.size() << "\n";
        ts << "incomes=[" << inc.join(QStringLiteral("|")) << "]\n";
        ts << "expenses=[" << exp.join(QStringLiteral("|")) << "]\n";
        QString path = qEnvironmentVariable("BUTAS_DUMP");
        if (path.isEmpty())
            fprintf(stdout, "%s", qPrintable(out));
        else{
            QFile f(path);
            if (f.open(QIODevice::WriteOnly | QIODevice::Append))
                f.write(out.toUtf8());
        }
    }

protected:
    void resizeEvent(QResizeEvent *e) override{
        QMainWindow::resizeEvent(e);
        updateViewportFonts();
        m_actionModal->onParentResized();
        m_recurringModal->onParentResized();
        m_deleteModal->onParentResized();
        m_sourceModal->onParentResized();
        m_sourceInfo->onParentResized();
    }

private slots:
    void handleBalanceModalConfirm(double signedAmount, const QString &sourceId){
        const int idx = sourceIndexById(sourceId);
        if (idx < 0)
            return;
        m_sources[idx].balance += signedAmount;
        m_transactions.prepend({signedAmount >= 0, std::abs(signedAmount), sourceId});
        if (m_transactions.length() > 10)
            m_transactions.resize(10);
        refreshBalance();
        renderTransactions();
        renderSources();
        saveState();
    }

    void handleSourceSubmitted(const QString &name, bool canGoNegative){
        if (m_sourceModal->isCreateMode()){
            MoneySource source;
            source.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
            source.name = name;
            source.balance = 0.0;
            source.canGoNegative = canGoNegative;
            m_sources.append(source);
            renderSources();
            saveState();
            if (m_sourceCreateFromActionModal){
                m_sourceCreateFromActionModal = false;
                m_actionModal->setSources(m_sources);
                m_actionModal->selectSource(source.id);
            }
        } else{
            const int idx = sourceIndexById(m_sourceRenameId);
            m_sourceRenameId.clear();
            if (idx < 0)
                return;
            m_sources[idx].name = name;
            renderSources();
            saveState();
        }
    }

    void handleSourceFlagChanged(const QString &id, bool value){
        const int idx = sourceIndexById(id);
        if (idx < 0)
            return;
        if (isNakitSourceName(m_sources.at(idx).name))
            return;
        m_sources[idx].canGoNegative = value;
        renderSources();
        saveState();
    }

    void handleRecurringConfirm(const QString &amount, const QString &currency, const QString &label){
        const RecurringItem item{amount, currency, label};
        if (m_recurringModal->isIncomeMode())
            m_incomes.append(item);
        else
            m_expenses.append(item);
        renderRecurringItems();
        saveState();
    }

    void handleDeleteConfirm(){
        if (m_deleteTargetType == Income && m_deleteTargetIndex >= 0
            && m_deleteTargetIndex < m_incomes.size())
            m_incomes.removeAt(m_deleteTargetIndex);
        else if (m_deleteTargetType == Expense && m_deleteTargetIndex >= 0
                 && m_deleteTargetIndex < m_expenses.size())
            m_expenses.removeAt(m_deleteTargetIndex);
        else if (m_deleteTargetType == Source){
            const int idx = sourceIndexById(m_deleteSourceId);
            if (idx >= 0 && !isNakitSourceName(m_sources.at(idx).name)){
                m_sources.removeAt(idx);
                refreshBalance();
                renderSources();
            }
        }
        renderRecurringItems();
        m_deleteTargetType = None;
        m_deleteTargetIndex = -1;
        m_deleteSourceId.clear();
        saveState();
    }

private:
    enum TargetType { None, Income, Expense, Source };

    static bool persistenceEnabled(){
        if (qEnvironmentVariable("BUTAS_PERSIST") == "1")
            return true;
        return qEnvironmentVariable("BUTAS_TEST") != "1";
    }

    QString stateFilePath() const{
        const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        return dir + QStringLiteral("/state.json");
    }

    void loadState(){
        m_sources.clear();
        if (persistenceEnabled()){
            QFile f(stateFilePath());
            if (f.open(QIODevice::ReadOnly)){
                const QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
                if (doc.isObject()){
                    const QJsonObject root = doc.object();
                    const int version = root.value(QStringLiteral("version")).toInt(1);

                    if (version >= 2 && root.value(QStringLiteral("sources")).isArray()){
                        const QJsonArray arr = root.value(QStringLiteral("sources")).toArray();
                        for (const QJsonValue &v : arr){
                            if (!v.isObject())
                                continue;
                            const QJsonObject o = v.toObject();
                            MoneySource s;
                            s.id = o.value(QStringLiteral("id")).toString();
                            s.name = o.value(QStringLiteral("name")).toString().trimmed();
                            s.balance = o.value(QStringLiteral("balance")).toDouble();
                            s.canGoNegative = o.value(QStringLiteral("canGoNegative")).toBool(false);
                            if (s.id.isEmpty() || s.name.isEmpty())
                                continue;
                            if (isNakitSourceName(s.name))
                                s.canGoNegative = false;
                            m_sources.append(s);
                        }
                    }

                    if (root.value(QStringLiteral("transactions")).isArray()){
                        QVector<Transaction> list;
                        const QJsonArray arr = root.value(QStringLiteral("transactions")).toArray();
                        for (const QJsonValue &v : arr){
                            if (!v.isObject())
                                continue;
                            const QJsonObject t = v.toObject();
                            if (!t.contains(QStringLiteral("amount")))
                                continue;
                            const double amount = t.value(QStringLiteral("amount")).toDouble();
                            list.append({ t.value(QStringLiteral("add")).toBool(true),
                                          std::abs(amount),
                                          t.value(QStringLiteral("sourceId")).toString() });
                        }
                        m_transactions = list;
                    }

                    const auto readItems = [](const QJsonValue &v){
                        QVector<RecurringItem> list;
                        for (const QJsonValue &iv : v.toArray()){
                            if (!iv.isObject())
                                continue;
                            const QJsonObject r = iv.toObject();
                            list.append(RecurringItem(r.value(QStringLiteral("amount")).toString(),
                                                      r.value(QStringLiteral("currency")).toString(),
                                                      r.value(QStringLiteral("label")).toString()));
                        }
                        return list;
                    };
                    if (root.value(QStringLiteral("incomes")).isArray())
                        m_incomes = readItems(root.value(QStringLiteral("incomes")));
                    if (root.value(QStringLiteral("expenses")).isArray())
                        m_expenses = readItems(root.value(QStringLiteral("expenses")));
                }
            }
        }
        if (!hasNakitSource())
            m_sources.prepend(makeNakitSource());
    }

    void saveState() const{
        if (!persistenceEnabled())
            return;

        QJsonArray sources;
        for (const MoneySource &s : std::as_const(m_sources)){
            QJsonObject o;
            o.insert(QStringLiteral("id"), s.id);
            o.insert(QStringLiteral("name"), s.name);
            o.insert(QStringLiteral("balance"), s.balance);
            o.insert(QStringLiteral("canGoNegative"), s.canGoNegative);
            sources.append(o);
        }

        QJsonArray transactions;
        for (const Transaction &t : std::as_const(m_transactions)){
            QJsonObject o;
            o.insert(QStringLiteral("add"), t.isAdd);
            o.insert(QStringLiteral("amount"), t.amount);
            if (!t.sourceId.isEmpty())
                o.insert(QStringLiteral("sourceId"), t.sourceId);
            transactions.append(o);
        }
        const auto itemsToJson = [](const QVector<RecurringItem> &items){
            QJsonArray a;
            for (const RecurringItem &i : items){
                QJsonObject o;
                o.insert(QStringLiteral("amount"), i.amount);
                o.insert(QStringLiteral("currency"), i.currency);
                o.insert(QStringLiteral("label"), i.label);
                a.append(o);
            }
            return a;
        };

        QJsonObject root;
        root.insert(QStringLiteral("version"), 2);
        root.insert(QStringLiteral("sources"), sources);
        root.insert(QStringLiteral("transactions"), transactions);
        root.insert(QStringLiteral("incomes"), itemsToJson(m_incomes));
        root.insert(QStringLiteral("expenses"), itemsToJson(m_expenses));

        const QString path = stateFilePath();
        QDir().mkpath(QFileInfo(path).absolutePath());
        QSaveFile f(path);
        if (!f.open(QIODevice::WriteOnly))
            return;
        f.write(QJsonDocument(root).toJson(QJsonDocument::Compact));
        f.commit();
    }

    void openActionModal(bool add){
        m_actionModal->setSources(m_sources);
        m_actionModal->openModal(add);
    }

    void openRecurringModal(bool income){
        m_recurringModal->openModal(income);
    }

    void openDeleteModal(TargetType type, int index){
        m_deleteTargetType = type;
        m_deleteTargetIndex = index;
        m_deleteSourceId.clear();
        m_deleteModal->setDescription(QStringLiteral("Silmek istediğinize emin misiniz?"));
        m_deleteModal->openModal();
    }

    void openSourceCreate(bool fromActionModal){
        m_sourceCreateFromActionModal = fromActionModal;
        QStringList names;
        for (const MoneySource &s : std::as_const(m_sources))
            names << s.name;
        m_sourceModal->openCreate(names);
    }

    void openSourceRename(const QString &id){
        const int idx = sourceIndexById(id);
        if (idx < 0 || isNakitSourceName(m_sources.at(idx).name))
            return;
        QStringList names;
        for (int i = 0; i < m_sources.size(); ++i)
            if (i != idx)
                names << m_sources.at(i).name;
        m_sourceRenameId = id;
        m_sourceModal->openRename(m_sources.at(idx), names);
    }

    void openSourceDelete(const QString &id){
        const int idx = sourceIndexById(id);
        if (idx < 0 || isNakitSourceName(m_sources.at(idx).name))
            return;
        m_deleteTargetType = Source;
        m_deleteTargetIndex = -1;
        m_deleteSourceId = id;
        m_deleteModal->setDescription(
            QStringLiteral("Kaynak ve içindeki para bütçeden silinir. Onaylıyor musunuz?"));
        m_deleteModal->openModal();
    }

    void openSourceInfo(const QString &id){
        const int idx = sourceIndexById(id);
        if (idx < 0)
            return;
        QVector<Transaction> last;
        for (const Transaction &t : std::as_const(m_transactions)){
            if (t.sourceId != id)
                continue;
            last.append(t);
            if (last.size() == 5)
                break;
        }
        m_sourceInfo->openFor(m_sources.at(idx), last);
    }

    void renderTransactions(){
        QVector<Transaction> renderList;
        for (int i = 0; i < m_transactions.size() && i < 10; ++i)
            renderList.append(m_transactions.at(i));
        m_chips->setTransactions(renderList);
    }

    void renderRecurringItems(){
        m_panel->setIncomeItems(m_incomes, [this](int idx) { openDeleteModal(Income, idx); });
        m_panel->setExpenseItems(m_expenses, [this](int idx) { openDeleteModal(Expense, idx); });
    }

    void renderSources(){
        m_sourcesCard->setSources(
            m_sources,
            [this](const QString &id) { openSourceInfo(id); },
            [this](const QString &id) { openSourceDelete(id); },
            [this](const QString &id) { openSourceRename(id); },
            [this]() { openSourceCreate(false); });
    }

    int sourceIndexById(const QString &id) const{
        for (int i = 0; i < m_sources.size(); ++i)
            if (m_sources.at(i).id == id)
                return i;
        return -1;
    }

    QString sourceIdByName(const QString &name) const{
        const QString norm = normalizedSourceName(name);
        for (const MoneySource &s : std::as_const(m_sources))
            if (normalizedSourceName(s.name) == norm)
                return s.id;
        return QString();
    }

    bool hasNakitSource() const{
        for (const MoneySource &s : std::as_const(m_sources))
            if (isNakitSourceName(s.name))
                return true;
        return false;
    }

    static MoneySource makeNakitSource(){
        MoneySource s;
        s.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        s.name = QStringLiteral("Nakit");
        s.balance = 0.0;
        s.canGoNegative = false;
        return s;
    }

    double totalBalance() const{
        double total = 0.0;
        for (const MoneySource &s : std::as_const(m_sources))
            total += s.balance;
        return total;
    }

    void refreshBalance(){
        m_balance->setBalance(totalBalance());
    }

    void updateViewportFonts(){
        const int vw = width();
        m_panel->setViewportWidth(vw);
    }

    QScrollArea *m_scroll = nullptr;
    BalanceWidget *m_balance = nullptr;
    ChipsList *m_chips = nullptr;
    PanelCard *m_panel = nullptr;
    SourcesCard *m_sourcesCard = nullptr;
    ActionButton *m_btnAdd = nullptr, *m_btnSubtract = nullptr;
    ActionButton *m_btnIncome = nullptr, *m_btnExpense = nullptr;

    QVector<MoneySource> m_sources;
    QVector<Transaction> m_transactions;
    QVector<RecurringItem> m_incomes, m_expenses;
    int m_deleteTargetIndex = -1;
    TargetType m_deleteTargetType = None;
    QString m_deleteSourceId;
    QString m_sourceRenameId;
    bool m_sourceCreateFromActionModal = false;

    ActionModalDialog *m_actionModal = nullptr;
    RecurringModalDialog *m_recurringModal = nullptr;
    DeleteModalDialog *m_deleteModal = nullptr;
    SourceModalDialog *m_sourceModal = nullptr;
    SourceInfoDialog *m_sourceInfo = nullptr;
};

#endif
