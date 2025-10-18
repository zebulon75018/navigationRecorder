// main.cpp
#include "mainwindow.h"
#include <QApplication>
#include <QCommandLineParser>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    // Analyse des arguments en ligne de commande
    QCommandLineParser parser;
    parser.setApplicationDescription("WebSpy: Enregistrement et rejeu de navigation web");
    parser.addHelpOption();
    QCommandLineOption urlOption("url", "URL initiale à charger", "url");
    QCommandLineOption replayOption("replay", "Fichier JSON à rejouer", "file");
    QCommandLineOption screenshotDirOption("screenshot-dir", "Répertoire pour les captures d'écran", "dir", "screenshots");
    QCommandLineOption screenshotsOption("screenshots", "Activer les captures d'écran");
    parser.addOption(urlOption);
    parser.addOption(replayOption);
    parser.addOption(screenshotDirOption);
    parser.addOption(screenshotsOption);
    parser.process(app);

    // Vérification : au moins un argument --url, --replay ou --help
    if (!parser.isSet(urlOption) && !parser.isSet(replayOption) /*&& !parser.isSet(parser.helpOption())*/) {
        qWarning() << "Erreur : Argument manquant. Utilisez --url, --replay ou --help.";
        return 1;
    }

    QString initialUrl = parser.value(urlOption);
    QString replayFile = parser.value(replayOption);
    QString screenshotDir = parser.value(screenshotDirOption);
    bool takeScreenshots = parser.isSet(screenshotsOption);

    // Vérification : --url et --replay sont mutuellement exclusifs
    if (!replayFile.isEmpty() && !initialUrl.isEmpty()) {
        qWarning() << "Erreur : --url et --replay ne peuvent pas être utilisés ensemble.";
        return 1;
    }

    MainWindow window(initialUrl, replayFile, screenshotDir, takeScreenshots);
    window.resize(800, 600);
    window.show();
    return app.exec();
}
