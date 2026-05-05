#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QSoundEffect>
#include <QPropertyAnimation>      // 动画核心类
#include <QGraphicsOpacityEffect>  // 透明度效果类
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

    QSoundEffect *m_sndRing;    // 来电铃声
    QSoundEffect *m_sndMsgIn;   // 收到消息
    QSoundEffect *m_sndMsgOut;  // 发送消息
    QSoundEffect *m_sndSuccess; // 挑战成功
    QSoundEffect *m_sndFail;    // 挑战失败

};
#endif // MAINWINDOW_H
