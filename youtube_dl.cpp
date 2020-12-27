#include "youtube_dl.h"

YoutubeDl::YoutubeDl()
{

}

QString YoutubeDl::path = QString();

QString YoutubeDl::find(bool force) {
    if (!force && !path.isEmpty()) return path;

    // Prefer downloaded youtube-dl
    QString localPath = QStandardPaths::locate(QStandardPaths::AppDataLocation, "youtube-dl");
    QProcess* process = instance(localPath, QStringList() << "--version");
    process->start();
    process->waitForFinished();
    process->deleteLater();
    if (process->state() != QProcess::NotRunning) process->kill();
    if (process->exitCode() == QProcess::ExitStatus::NormalExit) {
        path = localPath;
        return path;
    }

    // Try system-wide youtube-dl installation
    QString globalPath = QStandardPaths::findExecutable("youtube-dl");
    process = instance(globalPath, QStringList() << "--version");
    process->start();
    process->waitForFinished();
    process->deleteLater();
    if (process->state() != QProcess::NotRunning) process->kill();
    if (process->exitCode() == QProcess::ExitStatus::NormalExit) {
        path = globalPath;
        return path;
    }

    return "";
}

QProcess* YoutubeDl::instance(QStringList arguments) {
    return instance(find(), arguments);
}

QProcess* YoutubeDl::instance(QString path, QStringList arguments) {
    QProcess *process = new QProcess();

    QString execPath = QCoreApplication::applicationDirPath();
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("PATH", execPath + ":" + env.value("PATH"));
    process->setEnvironment(env.toStringList());

    #if defined Q_OS_WIN
        process->setProgram(execPath + "/python/python.exe");
    #else
        process->setProgram(QStandardPaths::findExecutable("python"));
    #endif

    QSettings settings;
    QStringList proxyArguments;
    if (settings.value("UseProxy", false).toBool()) {
        QUrl proxyUrl;

        proxyUrl.setHost(settings.value("ProxyHost", "").toString());
        proxyUrl.setPort(settings.value("ProxyPort", "").toInt());

        if (settings.value("ProxyType", false).toInt() == 0) {
            proxyUrl.setScheme("http");
        } else {
            proxyUrl.setScheme("socks5");
        }
        if (settings.value("ProxyAuthenticationRequired", false).toBool() == true) {
            proxyUrl.setUserName(settings.value("ProxyUsername", "").toString());
            proxyUrl.setPassword(settings.value("ProxyPassword").toString());
        }

        proxyArguments << "--proxy" << proxyUrl.toString();
    }

    QStringList networkArguments;
    if (settings.value("forceIpV4", false).toBool()) {
        networkArguments << "--force-ipv4";
    }

    process->setArguments(QStringList() << path << arguments << proxyArguments << networkArguments);
    return process;
}

QString YoutubeDl::getVersion() {
    QProcess* youtubeDl = instance(QStringList("--version"));
    youtubeDl->start();
    youtubeDl->waitForFinished(10000);
    QString version = youtubeDl->readAllStandardOutput() + youtubeDl->readAllStandardError();
    youtubeDl->deleteLater();
    return version.replace("\n", "");
}

QString YoutubeDl::getPythonVersion() {
    QProcess* youtubeDl = instance(QStringList());
    youtubeDl->setArguments(QStringList("--version"));
    youtubeDl->start();
    youtubeDl->waitForFinished(10000);
    QString version = youtubeDl->readAllStandardOutput() + youtubeDl->readAllStandardError();
    youtubeDl->deleteLater();
    return version.replace("\n", "");
}

QString YoutubeDl::findPython() {
    QProcess* youtubeDl = instance(QStringList());
    QString program = youtubeDl->program();
    youtubeDl->deleteLater();
    return program;
}
