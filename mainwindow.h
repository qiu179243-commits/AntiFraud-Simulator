#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMediaPlayer>
#include <QVideoWidget>
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    Ui::MainWindow *ui;
    QMediaPlayer *m_player = nullptr;
    QVideoWidget *m_videoWidget = nullptr;

    int chatStep = 0;

    void appendBubble(const QString &text, bool isUser);
    void clearChatArea();
    void startChatScene();
    void handleOption1();
    void showSingleOption(const QString &text);
    void showSystemNotice(const QString &text);
    void advanceChat();
    void showResultPage(bool success);



};
#endif // MAINWINDOW_H
