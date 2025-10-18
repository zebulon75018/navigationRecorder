// mainwindow.cpp
#include "mainwindow.h"
#include <QVBoxLayout>
#include <QToolBar>
#include <QAction>
#include <QMenuBar>
#include <QFile>
#include <QTextStream>
#include <QTimer>
#include <QDebug>
#include <QUrlQuery>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>
#include <QDir>
#include <QStatusBar>
#include <QWebEngineProfile>

// RequestInterceptor
RequestInterceptor::RequestInterceptor(std::vector<NavigationEvent>& evts, QWebEngineView* view, const QString& screenshotDir,bool isTakeScreenshot, QObject* parent)
    : QWebEngineUrlRequestInterceptor(parent), events(evts), webView(view), screenshotDir(screenshotDir), _isTakeScreenshot(isTakeScreenshot) {}

void RequestInterceptor::interceptRequest(QWebEngineUrlRequestInfo& info) {
    NavigationEvent event;
    event.url = info.requestUrl().toString();
    event.timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);
    event.method = QString::fromLatin1(info.requestMethod());
    event.isAjax = (info.navigationType() == QWebEngineUrlRequestInfo::NavigationTypeOther);

    if (event.method == "GET") {
        QUrlQuery query(info.requestUrl());
        event.params = query.toString();
    } else if (event.method == "POST") {
        event.params = "POST data not directly accessible";
    } else {
        event.params = "";
    }

    if ( _isTakeScreenshot ) {
    event.screenshot = QString("%1/%2.png").arg(screenshotDir).arg(events.size() + 1);
    QPixmap screenshot = webView->grab();
    screenshot.save(event.screenshot, "PNG");
    } else {
    event.screenshot = "";
    }

    events.push_back(event);

#ifdef DEBUG
    qDebug() << (event.isAjax ? "AJAX" : "Navigation") << ": " << event.url
             << " | Method: " << event.method << " | Params: " << event.params
             << " | Screenshot: " << event.screenshot;
#endif
}

// CustomWebEnginePage
CustomWebEnginePage::CustomWebEnginePage(std::vector<NavigationEvent>& evts, QWebEngineView* view, const QString& screenshotDir,bool isTakeScreenshot, QObject* parent)
    : QWebEnginePage(parent), events(evts), webView(view), screenshotDir(screenshotDir), _isTakeScreenshot(isTakeScreenshot)  {}

bool CustomWebEnginePage::acceptNavigationRequest(const QUrl& url, NavigationType type, bool isMainFrame) {
    if (isMainFrame) {
        NavigationEvent event;
        event.url = url.toString();
        event.timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);
        event.method = navigationTypeToString(type);
        event.isAjax = false;

        QUrlQuery query(url);
        event.params = query.toString();

    if ( _isTakeScreenshot ) {
        event.screenshot = QString("%1/%2.png").arg(screenshotDir).arg(events.size() + 1);
        QPixmap screenshot = webView->grab();
        screenshot.save(event.screenshot, "PNG");
       } else {
        event.screenshot = "";
      }

        events.push_back(event);
#ifdef DEBUG
        qDebug() << "MainFrame Navigation: " << event.url << " | Method: " << event.method
                 << " | Params: " << event.params << " | Screenshot: " << event.screenshot;
#endif
    }
    return true;
}

QString CustomWebEnginePage::navigationTypeToString(NavigationType type) {
    switch (type) {
    case QWebEnginePage::NavigationTypeLinkClicked: return "LinkClicked";
    case QWebEnginePage::NavigationTypeTyped: return "Typed";
    case QWebEnginePage::NavigationTypeFormSubmitted: return "FormSubmitted";
    case QWebEnginePage::NavigationTypeBackForward: return "BackForward";
    case QWebEnginePage::NavigationTypeReload: return "Reload";
    default: return "Other";
    }
}

// MainWindow
MainWindow::MainWindow(const QString& initialUrl, const QString& replayFile, const QString& screenshotDir,bool takeScreenshots, QWidget* parent)
    : QMainWindow(parent), isRecording(false), isReplaying(false), screenshotDir(screenshotDir.isEmpty() ? "screenshots" : screenshotDir) {
    // Créer le dossier pour les captures d'écran
    QDir().mkdir(screenshotDir);

    // Configuration du profil pour intercepter les requêtes
    QWebEngineProfile* profile = new QWebEngineProfile(this);
    webView = new QWebEngineView(this);
    RequestInterceptor* interceptor = new RequestInterceptor(events, webView, screenshotDir,takeScreenshots, profile);
    profile->setRequestInterceptor(interceptor);

    // Configuration de l'interface
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(centralWidget);
    webView->setPage(new CustomWebEnginePage(events, webView, screenshotDir,takeScreenshots, profile));
    if (!replayFile.isEmpty()) {
        loadJsonFile(replayFile);
        replayNavigation();
    } else {
        webView->setUrl(QUrl(initialUrl.isEmpty() ? "https://www.example.com" : initialUrl));
    }
    layout->addWidget(webView);
    setCentralWidget(centralWidget);

    // Création de la barre de menu
    QMenuBar* menuBar = new QMenuBar(this);
    setMenuBar(menuBar);
    QMenu* navigationMenu = menuBar->addMenu("Navigation");
    QAction* startRecordAction = navigationMenu->addAction("Enregistrer la navigation");
    QAction* stopRecordAction = navigationMenu->addAction("Arrêter l'enregistrement");
    QAction* replayAction = navigationMenu->addAction("Rejouer la navigation");
    QAction* saveJsonAction = navigationMenu->addAction("Sauvegarder le fichier de navigation");
    QAction* loadJsonAction = navigationMenu->addAction("Charger un fichier de navigation");
    QAction* saveDotAction = navigationMenu->addAction("Sauvegarder le fichier .dot");
    navigationMenu->addSeparator();
    QAction* quitAction = navigationMenu->addAction("Quitter");

    // Création de la barre d'outils
    QToolBar* toolbar = addToolBar("Navigation Controls");
    QAction* recordAction = toolbar->addAction(QIcon(":/record.png"), "Record");
    QAction* playAction = toolbar->addAction(QIcon(":/play.png"), "Play");
    QAction* stopAction = toolbar->addAction(QIcon(":/stop.png"), "Stop");
    QAction* saveAction = toolbar->addAction(QIcon(":/save.png"), "Save");

    // Style pour le bouton Record (rouge lorsqu'actif)
    recordAction->setCheckable(true);
    connect(recordAction, &QAction::toggled, this, [=](bool checked) {
        if (checked) {
            recordAction->setIcon(QIcon(":/record_active.png"));
            startRecording();
        } else {
            recordAction->setIcon(QIcon(":/record.png"));
            stopRecording();
        }
    });

    // Connexions des actions de la barre d'outils
    connect(playAction, &QAction::triggered, this, &MainWindow::replayNavigation);
    connect(stopAction, &QAction::triggered, this, &MainWindow::stopAll);
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveJsonFile);

    // Connexions des actions du menu
    connect(startRecordAction, &QAction::triggered, [=]() {
        recordAction->setChecked(true);
    });
    connect(stopRecordAction, &QAction::triggered, [=]() {
        recordAction->setChecked(false);
        stopAll();
    });
    connect(replayAction, &QAction::triggered, this, &MainWindow::replayNavigation);
    connect(saveJsonAction, &QAction::triggered, this, &MainWindow::saveJsonFile);
    connect(loadJsonAction, &QAction::triggered, this, &MainWindow::loadJsonFileDialog);
    connect(saveDotAction, &QAction::triggered, this, &MainWindow::saveDotFile);
    connect(quitAction, &QAction::triggered, this, &QMainWindow::close);
}

void MainWindow::startRecording() {
    QDir(screenshotDir).removeRecursively();
    QDir().mkdir(screenshotDir);
    events.clear();
    isRecording = true;
    statusBar()->showMessage("Enregistrement démarré.");
}

void MainWindow::stopRecording() {
    isRecording = false;
    statusBar()->showMessage("Enregistrement arrêté.");
}

void MainWindow::stopAll() {
    isRecording = false;
    isReplaying = false;
    statusBar()->showMessage("Toutes les actions arrêtées.");
}

void MainWindow::saveJsonFile() {
    QString fileName = QFileDialog::getSaveFileName(this, "Sauvegarder le fichier JSON",
                                                    "", "JSON Files (*.json)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QJsonArray jsonArray;
        for (const auto& event : events) {
            QJsonObject jsonEvent;
            jsonEvent["url"] = event.url;
            jsonEvent["timestamp"] = event.timestamp;
            jsonEvent["method"] = event.method;
            jsonEvent["params"] = event.params;
            jsonEvent["isAjax"] = event.isAjax;
            jsonEvent["screenshot"] = event.screenshot;
            jsonArray.append(jsonEvent);
        }
        QJsonDocument doc(jsonArray);
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
        statusBar()->showMessage("Fichier JSON sauvegardé.");
    } else {
        QMessageBox::warning(this, "Erreur", "Impossible de sauvegarder le fichier JSON.");
    }
}

void MainWindow::loadJsonFileDialog() {
    QString fileName = QFileDialog::getOpenFileName(this, "Charger un fichier JSON",
                                                    "", "JSON Files (*.json)");
    if (fileName.isEmpty()) return;
    loadJsonFile(fileName);
}

void MainWindow::loadJsonFile(const QString& fileName) {
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        file.close();
        if (!doc.isArray()) {
            QMessageBox::warning(this, "Erreur", "Le fichier JSON n'est pas valide.");
            return;
        }

        events.clear();
        QJsonArray jsonArray = doc.array();
        for (const QJsonValue& value : jsonArray) {
            QJsonObject obj = value.toObject();
            NavigationEvent event;
            event.url = obj["url"].toString();
            event.timestamp = obj["timestamp"].toString();
            event.method = obj["method"].toString();
            event.params = obj["params"].toString();
            event.isAjax = obj["isAjax"].toBool();
            event.screenshot = obj["screenshot"].toString();
            events.push_back(event);
        }
        statusBar()->showMessage("Fichier JSON chargé.");
    } else {
        QMessageBox::warning(this, "Erreur", "Impossible de charger le fichier JSON.");
    }
}

void MainWindow::saveDotFile() {
    QString fileName = QFileDialog::getSaveFileName(this, "Sauvegarder le fichier .dot",
                                                    "", "DOT Files (*.dot)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << "digraph NavigationGraph {\n";
        out << "  rankdir=LR;\n";
        out << "  node [shape=box];\n";
        for (size_t i = 0; i < events.size(); ++i) {
            QString node = QString("Node%1 [image=\"%2\", label=\"\"];\n")
                              .arg(i)
                              .arg(events[i].screenshot);
            out << node;
            if (i > 0) {
                QString edgeStyle = events[i].isAjax ? "[style=dashed]" : "";
                out << QString("Node%1 -> Node%2 [label=\"%3\" %4];\n")
                           .arg(i - 1)
                           .arg(i)
                           .arg(events[i].method)
                           .arg(edgeStyle);
            }
        }
        out << "}\n";
        file.close();
        statusBar()->showMessage("Fichier .dot sauvegardé.");
    } else {
        QMessageBox::warning(this, "Erreur", "Impossible de sauvegarder le fichier .dot.");
    }
}

void MainWindow::replayNavigation() {
    if (events.empty()) {
        QMessageBox::warning(this, "Erreur", "Aucune navigation à rejouer.");
        return;
    }
    isReplaying = true;
    currentReplayIndex = 0;
    QTimer::singleShot(1000, this, &MainWindow::replayNext);
    statusBar()->showMessage("Rejeu de la navigation démarré.");
}

void MainWindow::replayNext() {
    if (!isReplaying || currentReplayIndex >= events.size()) {
        isReplaying = false;
        statusBar()->showMessage("Rejeu de la navigation terminé.");
        return;
    }
    if (!events[currentReplayIndex].isAjax) {
        webView->setUrl(QUrl(events[currentReplayIndex].url));
    }
    currentReplayIndex++;
    QTimer::singleShot(1000, this, &MainWindow::replayNext);
}
