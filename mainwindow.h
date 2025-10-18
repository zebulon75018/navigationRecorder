// mainwindow.h
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWebEngineView>
#include <QWebEnginePage>
#include <QWebEngineUrlRequestInterceptor>
#include <vector>

struct NavigationEvent {
    QString url;
    QString timestamp;
    QString method;
    QString params;
    bool isAjax;
    QString screenshot;
};

class RequestInterceptor : public QWebEngineUrlRequestInterceptor {
    Q_OBJECT
public:
    RequestInterceptor(std::vector<NavigationEvent>& evts, QWebEngineView* view, const QString& screenshotDir,bool isTakeScreenshot, QObject* parent = nullptr);

    void interceptRequest(QWebEngineUrlRequestInfo& info) override;

private:
    std::vector<NavigationEvent>& events;
    QWebEngineView* webView;
    QString screenshotDir;
    bool _isTakeScreenshot;
};

class CustomWebEnginePage : public QWebEnginePage {
    Q_OBJECT
public:
    CustomWebEnginePage(std::vector<NavigationEvent>& evts, QWebEngineView* view, const QString& screenshotDir,bool isTakeScreenshot, QObject* parent = nullptr);

protected:
    bool acceptNavigationRequest(const QUrl& url, NavigationType type, bool isMainFrame) override;

private:
    std::vector<NavigationEvent>& events;
    QWebEngineView* webView;
    QString screenshotDir;
    bool _isTakeScreenshot;

    QString navigationTypeToString(NavigationType type);
};

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(const QString& initialUrl, const QString& replayFile, const QString& screenshotDir,bool takeScreenshots,QWidget* parent = nullptr);

private slots:
    void startRecording();
    void stopRecording();
    void stopAll();
    void saveJsonFile();
    void loadJsonFileDialog();
    void loadJsonFile(const QString& fileName);
    void saveDotFile();
    void replayNavigation();
    void replayNext();

private:
    QWebEngineView* webView;
    std::vector<NavigationEvent> events;
    bool isRecording;
    bool isReplaying;
    size_t currentReplayIndex;
    QString screenshotDir;
};

#endif // MAINWINDOW_H
