#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollBar>
#include <QTimer>
#include <QVBoxLayout>
#include <QUrl>
#include <QAudioOutput> // 让视频声音能正常播放

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{

    ui->setupUi(this);

     // 初始化播放器
     m_player = new QMediaPlayer(this);
    m_videoWidget = new QVideoWidget(ui->eduContainer);
    m_player->setVideoOutput(m_videoWidget);

    m_videoWidget->setGeometry(0, 0, 370, 250);

    m_player = new QMediaPlayer(this);

    QAudioOutput *audioOutput = new QAudioOutput(this);
    m_player->setAudioOutput(audioOutput);
    audioOutput->setVolume(1.0);          // 设置音量 (0.0 到 1.0)

    m_videoWidget = new QVideoWidget(ui->eduContainer);
    m_player->setVideoOutput(m_videoWidget);
    if (!ui->chatContainer->layout()) {
        QVBoxLayout *chatLayout = new QVBoxLayout(ui->chatContainer);
        chatLayout->setContentsMargins(8, 8, 8, 8);
        chatLayout->setSpacing(8);
        chatLayout->setAlignment(Qt::AlignTop);
    }

    setFixedSize(370, 640);//固定窗口为手机大小
    setWindowTitle("反诈模拟挑战");

    ui->stackedWidget->setCurrentWidget(ui->pageHome);

    connect(ui->btnStart, &QPushButton::clicked, this, [=]()
            {
        ui->stackedWidget->setCurrentWidget(ui->pageCall);
    });

    ui->btnOption1->hide();
    ui->btnOption2->hide();

    connect(ui->btnAnswer, &QPushButton::clicked, this, [=]() {
        startChatScene();
    });
    connect(ui->btnOption1, &QPushButton::clicked, this, [=]() {
        handleOption1();
    });
    connect(ui->btnOption2, &QPushButton::clicked, this, [=]() {
        QString selectedText = ui->btnOption2->text();

        appendBubble( selectedText, true);

        ui->btnOption1->hide();
        ui->btnOption2->hide();

        if (chatStep == 6) {
            QTimer::singleShot(800, this, [=]() {
                appendBubble( "张先生，您现在打普通客服他们处理不了身份盗用案件，只会浪费您的时间。我们这里马上就要过系统保护时效了。", false);

                QTimer::singleShot(900, this, [=]() {
                    chatStep = 7;
                    ui->btnOption1->setText("好吧，验证码是836492。");
                    ui->btnOption2->setText("挂断电话，拨打银行电话询问并报警。");
                    ui->btnOption1->show();
                    ui->btnOption2->show();
                });
            });
        } else if (chatStep == 7) {
            QTimer::singleShot(800, this, [=]() {
                showResultPage(true);
            });
        }
    });

    connect(ui->btnGoEducation, &QPushButton::clicked, this, [=]() {
        ui->stackedWidget->setCurrentWidget(ui->pageEducation);

        QString videoPath = QCoreApplication::applicationDirPath() + "/video/antifraud.mp4";
        m_player->setSource(QUrl::fromLocalFile(videoPath));


        // 获取容器真实宽度
        int realWidth = ui->eduContainer->width();

        int realHeight = realWidth * 9 / 16;

        //设置视频大小
        m_videoWidget->setGeometry(0, 0, realWidth, realHeight);
        m_videoWidget->setAspectRatioMode(Qt::KeepAspectRatio); //保持宽高比
        m_videoWidget->show();

        ui->videoFrame->setGeometry(0, realHeight + 10, realWidth, 350);

        ui->btnCloseApp->setGeometry(0, 580, realWidth, 40);

        m_player->play();
    });
    connect(ui->btnCloseApp, &QPushButton::clicked, this, [=]() {
        this->close();
    });

}
MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::clearChatArea()
{
    QLayout *layout = ui->chatContainer->layout();
    if (!layout) {
        return;
    }

    while (QLayoutItem *item = layout->takeAt(0)) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
}


void MainWindow::appendBubble(const QString &text, bool isUser)
{
    QLayout *chatLayout = ui->chatContainer->layout();
    if (!chatLayout) {
        return;
    }

    QWidget *rowWidget = new QWidget;
    QHBoxLayout *rowLayout = new QHBoxLayout(rowWidget);
    rowLayout->setContentsMargins(8, 6, 8, 6);

    QLabel *avatar = new QLabel;
    avatar->setFixedSize(36, 36);
    avatar->setAlignment(Qt::AlignCenter);
    avatar->setText(isUser ? "我" : "诈");
    avatar->setStyleSheet(
        isUser
            ? "background:#4caf50; color:white; border-radius:18px; font-weight:bold;"
            : "background:#ff7043; color:white; border-radius:18px; font-weight:bold;"
        );

    QLabel *bubble = new QLabel(text);
    bubble->setWordWrap(true);
    bubble->setMaximumWidth(220);
    bubble->setStyleSheet(
        isUser
            ? "background:#dcf8c6; border-radius:12px; padding:10px;"
            : "background:white; border-radius:12px; padding:10px;"
        );

    if (isUser) {
        rowLayout->addStretch();
        rowLayout->addWidget(bubble, 0, Qt::AlignTop);
        rowLayout->addWidget(avatar, 0, Qt::AlignTop);
    } else {
        rowLayout->addWidget(avatar, 0, Qt::AlignTop);
        rowLayout->addWidget(bubble, 0, Qt::AlignTop);
        rowLayout->addStretch();
    }
    chatLayout->addWidget(rowWidget);

    QTimer::singleShot(50, this, [=]() {
        ui->scrollAreaChat->verticalScrollBar()->setValue(
            ui->scrollAreaChat->verticalScrollBar()->maximum()
            );
    });
}
void MainWindow::showSingleOption(const QString &text)
{
    ui->btnOption1->setText(text);
    ui->btnOption1->show();
    ui->btnOption2->hide();
}
void MainWindow::showSystemNotice(const QString &text)
{
    QLayout *chatLayout = ui->chatContainer->layout();
    if (!chatLayout) {
        return;
    }

    QLabel *notice = new QLabel(text);
    notice->setAlignment(Qt::AlignCenter);
    notice->setStyleSheet("color: gray; font-size: 12px; padding: 6px;");

    chatLayout->addWidget(notice);

    QTimer::singleShot(50, this, [=]() {
        ui->scrollAreaChat->verticalScrollBar()->setValue(
            ui->scrollAreaChat->verticalScrollBar()->maximum()
            );
    });
}


void MainWindow::startChatScene()
{
    ui->stackedWidget->setCurrentWidget(ui->pageChat);

    clearChatArea();

    ui->btnOption1->hide();
    ui->btnOption2->hide();

    chatStep = 0;

    appendBubble("您好，请问是张伟先生吗？我这里是××银行信用卡中心风险管理部，工号0521。", false);

    QTimer::singleShot(800, this, [=]() {
        showSingleOption("我是，有什么事？");
    });
}



void MainWindow::handleOption1()
{
    QString selectedText = ui->btnOption1->text();

    appendBubble( selectedText, true);

    ui->btnOption1->hide();
    ui->btnOption2->hide();

    if (chatStep == 6) {
        QTimer::singleShot(800, this, [=]() {
            showResultPage(false);
        });
        return;
    }

    if (chatStep == 7) {
        QTimer::singleShot(800, this, [=]() {
            showResultPage(false);
        });
        return;
    }


    chatStep++;
    QTimer::singleShot(800, this, [=]() {
        advanceChat();
    });
}


void MainWindow::advanceChat()
{
    if (chatStep == 1) {
        appendBubble( "张先生，系统显示您名下尾号8832的信用卡，有一笔分期账单逾期了，现在欠款累计到了一万两千元，这个情况您清楚吗？", false);

        QTimer::singleShot(800, this, [=]() {
            showSingleOption("我没办过你们银行的信用卡，你肯定搞错了。");
        });
    }
    else if (chatStep == 2) {
        appendBubble( "张先生，您确定吗？但这张卡的开卡记录是三个月前，在××市一个网点激活的，绑定的就是您现在这个手机号。如果您本人真的没办过，那这个情况比欠款要严重得多。", false);

        QTimer::singleShot(800, this, [=]() {
            showSingleOption("什么意思？");
        });
    }
    else if (chatStep == 3) {
        appendBubble( "这种情况，极有可能是有人冒用了您的身份证信息，盗办了信用卡在恶意透支。这已经不止是征信问题了，是涉嫌金融诈骗。您现在名下所有银行账户都可能已经被盯上了。", false);

        QTimer::singleShot(800, this, [=]() {
            showSingleOption("那我该怎么办？");
        });
    }
    else if (chatStep == 4) {
        appendBubble( "我马上把电话转接到我们金融安全中心，您先别挂。稍后会有专员指导您做紧急保护。", false);

        QTimer::singleShot(800, this, [=]() {
            showSingleOption("确认转接");
        });
    }
    else if (chatStep == 5) {
        showSystemNotice("已转接金融安全中心专员");

        QTimer::singleShot(1000, this, [=]() {
            appendBubble( "您好，这里是金融安全中心，我是李主任，工号0623。张先生，银行那边已经把您的情况报过来了，您名下疑似发生身份盗用办卡案件。现在首要任务是把您现有的合法账户保护起来。", false);

            QTimer::singleShot(900, this, [=]() {
                showSingleOption("怎么验证？");
            });
        });
    }
    else if (chatStep == 6) {
        appendBubble( "我现在会给您的手机发送一条验证短信，这是用于登录我们金融安全保护系统、生成一个账户防盗用数字证书的。您收到后把验证码报给我就行。", false);

        QTimer::singleShot(900, this, [=]() {
            ui->btnOption1->setText("验证码是836492。");
            ui->btnOption2->setText("不行，我要自己打银行客服核实。");
            ui->btnOption1->show();
            ui->btnOption2->show();
        });
    }
}
void MainWindow::showResultPage(bool success)
{
    ui->stackedWidget->setCurrentWidget(ui->pageResult);

    // 自动换行，并水平居中、顶部对齐
    ui->labelResultDesc->setWordWrap(true);
    ui->labelResultDesc->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    if (success) {
        ui->labelResultTitle->setText("挑战成功\n完美识破骗局");
        ui->labelResultDesc->setText("\n你挂断电话后主动拨打银行官方客服并报警，确认并无欠款和身份盗用情况，成功避免损失，并向警方提供了关键线索。");

        ui->labelResultTitle->setStyleSheet("color: #34c759; font-size: 32px; font-weight: 900; background: transparent;");
        ui->labelResultDesc->setStyleSheet("color: #666666; font-size: 16px; background: transparent; line-height: 24px;");
    } else {
        ui->labelResultTitle->setText("挑战失败\n惨遭诈骗");
        ui->labelResultDesc->setText("\n你把验证码告诉了对方，骗子立即登录你的账户并转走资金。\n\n⚠️ 此次模拟失败，请牢记：验证码、短信口令、支付密码绝不能告诉任何人。");

        ui->labelResultTitle->setStyleSheet("color: #ff3b30; font-size: 32px; font-weight: 900; background: transparent;");
        ui->labelResultDesc->setStyleSheet("color: #666666; font-size: 16px; background: transparent; line-height: 24px;");
    }
}