#include <QApplication>
#include <QFontDatabase>
#include <QFile>
#include <QScreen>
#include <QTimer>
#include "mainwindow.h"

int main(int argc, char *argv[]){
    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName(QStringLiteral("BUTAS"));
    QCoreApplication::setApplicationName(QStringLiteral("BUTAS"));

    const QStringList fonts = {
        QStringLiteral(":/fonts/SpaceGrotesk.ttf"),
        QStringLiteral(":/fonts/MaterialSymbolsOutlined.ttf"),
    };
    for (const QString &path : fonts){
        QFile f(path);
        if (f.open(QIODevice::ReadOnly)){
            if (QFontDatabase::addApplicationFontFromData(f.readAll()) < 0)
                qWarning("Font yuklenemedi: %s", qPrintable(path));
        } else{
            qWarning("Font dosyasi acilamadi: %s", qPrintable(path));
        }
    }

    MainWindow w;
#ifdef Q_OS_ANDROID
    if (QScreen *scr = QGuiApplication::primaryScreen()){
        const QRect avail = scr->availableGeometry();
        w.resize(qMin(390, avail.width()), qMin(854, avail.height()));
    } else{
        w.resize(390, 854);
    }
#else
    w.resize(390, 854);
#endif
    w.show();

    const QByteArray shotPath = qgetenv("BUTAS_SHOT");
    const bool isTest = qEnvironmentVariable("BUTAS_TEST") == "1";
    if (isTest){
        QTimer::singleShot(800, &w, [&w]() { w.debugRunScript(); });
        QTimer::singleShot(1600, &w, []() { QCoreApplication::quit(); });
    } else if (!shotPath.isEmpty()){
        const QByteArray modalEnv = qgetenv("BUTAS_MODAL");
        if (modalEnv.isEmpty()){
            QTimer::singleShot(800, &w, [&w, shotPath](){
                w.grab().save(QString::fromUtf8(shotPath));
                QCoreApplication::quit();
            });
        } else{
            QTimer::singleShot(800, &w, [&w, modalEnv](){
                if (modalEnv == "1") w.debugOpenActionModal(true);
                else if (modalEnv == "2") w.debugOpenActionModal(false);
                else if (modalEnv == "3") w.debugOpenRecurringModal(true);
                else if (modalEnv == "4") w.debugOpenRecurringModal(false);
                else if (modalEnv == "5") w.debugOpenDeleteModal(0);
                else if (modalEnv == "6") w.debugOpenSourceCreate();
                else if (modalEnv == "7"){
                    const QString name = qEnvironmentVariable("BUTAS_SOURCE");
                    if (name.isEmpty()) w.debugOpenSourceInfo();
                    else w.debugOpenSourceInfoNamed(name);
                }
                else if (modalEnv == "8"){
                    w.debugOpenActionModal(true);
                    QTimer::singleShot(400, &w, [&w]() { w.debugOpenSourceMenu(); });
                }
                else if (modalEnv == "9") w.debugActionError(1);
                else if (modalEnv == "10") w.debugActionError(2);
                else if (modalEnv == "11") w.debugSourceError(1);
                else if (modalEnv == "12") w.debugSourceError(2);
                else if (modalEnv == "13") w.debugOpenSourceDeleteNamed(QStringLiteral("Ziraat"));
            });
            const int shotDelay = modalEnv == "8" ? 1800 : 1400;
            QTimer::singleShot(shotDelay, &w, [&w, shotPath](){
                w.grab().save(QString::fromUtf8(shotPath));
                QCoreApplication::quit();
            });
        }
    }

    return app.exec();
}
