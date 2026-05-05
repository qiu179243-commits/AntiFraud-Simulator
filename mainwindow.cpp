#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollBar>
#include <QTimer>
#include <QVBoxLayout>
#include <QUrl>
#include <QAudioOutput> // 让视频声音能正常播放
#include <QFile>
#include <QFontMetrics>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{

    ui->setupUi(this);

    // 拼接音频文件夹的基础路径
    QString audioDir = QCoreApplication::applicationDirPath() + "/audio/"; //

    // 1. 初始化来电铃声
    m_sndRing = new QSoundEffect(this);
    m_sndRing->setSource(QUrl::fromLocalFile(audioDir + "phone_ring.wav"));
    m_sndRing->setLoopCount(QSoundEffect::Infinite); // 铃声设为无限循环
    m_sndRing->setVolume(0.8);

    // 2. 初始化收到消息声
    m_sndMsgIn = new QSoundEffect(this);
    m_sndMsgIn->setSource(QUrl::fromLocalFile(audioDir + "msg_receive.wav"));

    // 3. 初始化发送消息/点击声
    m_sndMsgOut = new QSoundEffect(this);
    m_sndMsgOut->setSource(QUrl::fromLocalFile(audioDir + "msg_send.wav"));

    // 4. 初始化成功/失败声
    m_sndSuccess = new QSoundEffect(this);
    m_sndSuccess->setSource(QUrl::fromLocalFile(audioDir + "success.wav"));
    m_sndFail = new QSoundEffect(this);
    m_sndFail->setSource(QUrl::fromLocalFile(audioDir + "fail.wav"));

    // 加载第一个素材 (police1)
    QString police1Path = QCoreApplication::applicationDirPath() + "/image/police1.png";

    // 加载第二个素材 (police2)
    QString police2Path = QCoreApplication::applicationDirPath() + "/image/police2.png";
    if (QFile::exists(police2Path)) {
        ui->labelPolice2->setFixedSize(80, 80);           // 🛡️ 核心修复：强行锁定 80x80 大小
        ui->labelPolice2->setAlignment(Qt::AlignCenter);
        ui->labelPolice2->setPixmap(QPixmap(police2Path).scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
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
        m_sndRing->play();
        ui->stackedWidget->setCurrentWidget(ui->pageCall);
    });

    ui->btnOption1->hide();
    ui->btnOption2->hide();

    connect(ui->btnAnswer, &QPushButton::clicked, this, [=]() {
        m_sndRing->stop();   // <--- 插入这一行：接听了，赶紧把铃声掐掉[cite: 1]
        m_sndMsgIn->play();
        startChatScene();
    });
    connect(ui->btnOption1, &QPushButton::clicked, this, [=]() {
    m_sndMsgOut->play();
        handleOption1();
    });
    connect(ui->btnOption2, &QPushButton::clicked, this, [=]() {
        m_sndMsgOut->play();
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
        // 1. 切换页面
        ui->stackedWidget->setCurrentWidget(ui->pageEducation);

        // 2. 【核心修复】强行删除该页面的布局管理器，否则 move 指令无效
        if (ui->pageEducation->layout()) {
            delete ui->pageEducation->layout();
        }

        // 3. 定义画布规格 (370x640)
        int winW = 370;

        // 4. 视频定位 (y=50, 高度约210)
        int videoH = winW * 9 / 16;
        m_videoWidget->setParent(ui->pageEducation); // 确保父对象正确
        m_videoWidget->setGeometry(0, 50, winW, videoH);
        m_videoWidget->show();

        // 5. 白色文字卡片 (videoFrame) 定位
        // 必须大幅度缩减高度！设为 280 像素
        ui->videoFrame->setGeometry(15, 50 + videoH + 10, winW - 30, 280);

        // 6. 底部双按钮：绝对定位 (不再受布局控制)
        ui->btnRestartEdu->setParent(ui->pageEducation);
        ui->btnCloseApp->setParent(ui->pageEducation);

        int btnW = 155; // 按钮稍微加宽一点点
        int btnH = 50;  // 高度加到50，更有点击感
        int btnY = 570; // 距离底部留 70 像素，绝对不会撞到卡片

        ui->btnRestartEdu->setFixedSize(btnW, btnH);
        ui->btnCloseApp->setFixedSize(btnW, btnH);

        // 精准对齐：左边留20，中间空20，右边自然对齐
        ui->btnRestartEdu->move(20, btnY);
        ui->btnCloseApp->move(195, btnY);

        ui->btnRestartEdu->show();
        ui->btnCloseApp->show();

        // 7. 加载并播放视频
        QString videoPath = QCoreApplication::applicationDirPath() + "/video/antifraud.mp4";
        m_player->setSource(QUrl::fromLocalFile(videoPath));
        m_player->play();
    });
    connect(ui->btnRestartEdu, &QPushButton::clicked, this, [=]() {
        // 停掉教育页的视频
        m_player->stop();
        // 播放点击音效
        if(m_sndMsgOut) m_sndMsgOut->play();
        // 丝滑回到主页[cite: 1]
        ui->stackedWidget->setCurrentWidget(ui->pageHome);
        // 把之前的聊天记录清干净，准备下一轮[cite: 1]
        clearChatArea();
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
    avatar->setAlignment(Qt::AlignCenter);

    QString imageFileName = isUser ? "/image/user.jpg" : "/image/scammer.jpg";
    QString imagePath = QCoreApplication::applicationDirPath() + imageFileName;

    // 圆形头像切割
    if (QFile::exists(imagePath)) {
        avatar->setStyleSheet(QString(
                                  "min-width: 36px; max-width: 36px; "
                                  "min-height: 36px; max-height: 36px; "
                                  "border-radius: 18px; "
                                  "border-image: url(%1);"
                                  ).arg(imagePath));
    } else {
        avatar->setText(isUser ? "我" : "诈");
        avatar->setStyleSheet(isUser
                                  ? "min-width: 36px; max-width: 36px; min-height: 36px; max-height: 36px; border-radius:18px; background:#4caf50; color:white; font-weight:bold;"
                                  : "min-width: 36px; max-width: 36px; min-height: 36px; max-height: 36px; border-radius:18px; background:#ff7043; color:white; font-weight:bold;"
                              );
    }

    QLabel *bubble = new QLabel(text);
    bubble->setWordWrap(true);
    bubble->setMargin(10);

    // ==========================================================
    // 【杀手锏：纯手工算宽度，强行焊死！】
    // 不指望 Qt 的弱智引擎了，我们自己提前量好文字有多宽
    QFontMetrics fm(bubble->font());
    // 模拟在 230 像素宽度下换行，看看到底需要多大空间
    QRect textRect = fm.boundingRect(QRect(0, 0, 230, 9999), Qt::TextWordWrap, text);
    int realWidth = textRect.width() + 35; // 加上 20 的内边距，再多给 15 像素防吃字余量

    if (realWidth > 250) {
        realWidth = 250;
    }

    // 强行把底线拉高！弹簧再用力，也绝对无法把它挤得比 realWidth 更窄！
    bubble->setMinimumWidth(realWidth);
    bubble->setMaximumWidth(250);
    // ==========================================================

    bubble->setStyleSheet(
        isUser
            ? "background:#dcf8c6; color:#000000; border-radius:12px;"
            : "background:white; color:#000000; border-radius:12px;"
        );

    // 恢复经典的排版，这次有 realWidth 护体，再也不会变形了
    if (isUser) {
        rowLayout->addStretch();
        rowLayout->addWidget(bubble);
        rowLayout->addWidget(avatar, 0, Qt::AlignTop);
    } else {
        rowLayout->addWidget(avatar, 0, Qt::AlignTop);
        rowLayout->addWidget(bubble);
        rowLayout->addStretch();
    }

    chatLayout->addWidget(rowWidget);
    if (!isUser) {
        m_sndMsgIn->play(); // <--- 如果是骗子说话，播放“收到消息”的声音
    }

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

    ui->labelResultTitle->setAlignment(Qt::AlignCenter);
    ui->labelResultDesc->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    ui->labelResultDesc->setWordWrap(true);

    // 【防吃字绝招】：用原生的 margin 代替 QSS 的 padding！
    ui->labelResultDesc->setMargin(20);

    if (success) {
        // 第一行放 Emoji
        m_sndSuccess->play();
        ui->labelResultTitle->setText("🎉\n挑战成功\n完美识破骗局");
        ui->labelResultTitle->setStyleSheet(
            "color: #28a745; "
            "font-size: 32px; "
            "font-weight: 900; "
            "background: transparent;"
            );

        // ⚠️ 一字不差地恢复你的原版文案！
        ui->labelResultDesc->setText("\n你挂断电话后主动拨打银行官方客服并报警，确认并无欠款和身份盗用情况，成功避免损失，并向警方提供了关键线索。");

        // QSS 里绝对不再写 padding
        ui->labelResultDesc->setStyleSheet(
            "background-color: #e6f4ea; "
            "color: #1e4620; "
            "font-size: 16px; "
            "border-radius: 12px; "
            "border: 1px solid #c3e6cb;"
            );
    } else {
        m_sndFail->play();
        // 第一行放 Emoji
        ui->labelResultTitle->setText("⚠️\n挑战失败\n惨遭诈骗");
        ui->labelResultTitle->setStyleSheet(
            "color: #dc3545; "
            "font-size: 32px; "
            "font-weight: 900; "
            "background: transparent;"
            );

        // ⚠️ 一字不差地恢复你的原版文案！
        ui->labelResultDesc->setText("\n你把验证码告诉了对方，骗子立即登录你的账户并转走资金。\n\n⚠️ 此次模拟失败，请牢记：验证码、短信口令、支付密码绝不能告诉任何人。");

        // QSS 里绝对不再写 padding
        ui->labelResultDesc->setStyleSheet(
            "background-color: #fce8e6; "
            "color: #c5221f; "
            "font-size: 16px; "
            "border-radius: 12px; "
            "border: 1px solid #f5c6cb;"
            );
    }
}