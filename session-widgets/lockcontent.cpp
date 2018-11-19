#include "lockcontent.h"

#include "controlwidget.h"
#include "userinputwidget.h"
#include "sessionbasemodel.h"
#include "userframe.h"
#include "sessionwidget.h"
#include "shutdownwidget.h"

#include <QMouseEvent>

LockContent::LockContent(SessionBaseModel * const model, QWidget *parent)
    : SessionBaseWindow(parent)
    , m_model(model)
    , m_imageBlurInter(new ImageBlur("com.deepin.daemon.Accounts", "/com/deepin/daemon/ImageBlur", QDBusConnection::systemBus(), this))
{
    m_controlWidget = new ControlWidget;
    m_userInputWidget = new UserInputWidget;
    m_userFrame = new UserFrame;
    m_shutdownFrame = new ShutdownWidget;

    m_shutdownFrame->setModel(model);
    m_userFrame->setModel(model);

    restoreCenterContent();

    setRightBottomWidget(m_controlWidget);

    switch (model->currentType()) {
    case SessionBaseModel::AuthType::LightdmType:
        m_controlWidget->setSessionSwitchEnable(true);
        m_sessionFrame = new SessionWidget;
        m_sessionFrame->setModel(model);

        connect(m_sessionFrame, &SessionWidget::hideFrame, this, &LockContent::restoreCenterContent);
        connect(m_controlWidget, &ControlWidget::requestSwitchSession, this, &LockContent::pushSessionFrame);
        connect(m_model, &SessionBaseModel::onSessionKeyChanged, m_controlWidget, &ControlWidget::chooseToSession);

        break;
    case SessionBaseModel::AuthType::LockType:
        m_controlWidget->setMPRISEnable(true);
        break;
    default:
        break;
    }

    // init connect
    connect(model, &SessionBaseModel::currentUserChanged, this, &LockContent::onCurrentUserChanged);
    connect(m_userInputWidget, &UserInputWidget::requestAuthUser, this, [=] (const QString &password) {
        emit requestAuthUser(m_user, password);
    });
    connect(m_userFrame, &UserFrame::requestSwitchUser, this, &LockContent::requestSwitchToUser);
    connect(m_controlWidget, &ControlWidget::requestSwitchUser, this, &LockContent::pushUserFrame);
    connect(m_controlWidget, &ControlWidget::requestShutdown, this, &LockContent::pushShutdownFrame);
    connect(m_userFrame, &UserFrame::hideFrame, this, &LockContent::restoreCenterContent);
    connect(m_shutdownFrame, &ShutdownWidget::abortOperation, this, &LockContent::restoreCenterContent);
    connect(model, &SessionBaseModel::authFaildMessage, m_userInputWidget, &UserInputWidget::setFaildMessage);
    connect(model, &SessionBaseModel::authFaildTipsMessage, m_userInputWidget, &UserInputWidget::setFaildTipMessage);

    connect(model, &SessionBaseModel::onPowerActionChanged, this, [=] (SessionBaseModel::PowerAction poweraction) {
        switch (poweraction) {
        case SessionBaseModel::RequireNormal:
            m_userInputWidget->normalMode();
            break;
        case SessionBaseModel::RequireRestart:
            m_userInputWidget->restartMode();
            break;
        case SessionBaseModel::RequireShutdown:
            m_userInputWidget->shutdownMode();
        default:
            break;
        }
    });
}

void LockContent::onCurrentUserChanged(std::shared_ptr<User> user)
{
    m_user = user;

    connect(user.get(), &User::greeterBackgroundPathChanged, this, &LockContent::updateBackground , Qt::UniqueConnection);

    m_userInputWidget->setAvatar(user->avatarPath());
    m_userInputWidget->setIsNoPasswordGrp(user->isNoPasswdGrp());

    if (m_model->currentType() == SessionBaseModel::AuthType::LightdmType) {
        m_sessionFrame->switchToUser(user->name());
    }

    //TODO: refresh blur image
    updateBackground(user->greeterBackgroundPath());
}

void LockContent::pushUserFrame()
{
    releaseAllKeyboard();
    setCenterContent(m_userFrame);
    m_userFrame->expansion(true);
    QTimer::singleShot(300, m_userFrame, &UserFrame::grabKeyboard);
}

void LockContent::pushSessionFrame()
{
    releaseAllKeyboard();
    setCenterContent(m_sessionFrame);
    m_sessionFrame->show();
    QTimer::singleShot(300, m_sessionFrame, &SessionWidget::grabKeyboard);
}

void LockContent::pushShutdownFrame()
{
    releaseAllKeyboard();
    setCenterContent(m_shutdownFrame);
    QTimer::singleShot(300, m_shutdownFrame, &ShutdownWidget::grabKeyboard);
}

void LockContent::mouseReleaseEvent(QMouseEvent *event)
{
    restoreCenterContent();

    return SessionBaseWindow::mouseReleaseEvent(event);
}

void LockContent::releaseAllKeyboard()
{
    m_userInputWidget->releaseKeyboard();
    m_userFrame->releaseKeyboard();
    m_shutdownFrame->releaseKeyboard();

    if (m_model->currentType() == SessionBaseModel::AuthType::LightdmType) {
        m_sessionFrame->releaseKeyboard();
    }
}

void LockContent::restoreCenterContent()
{
    releaseAllKeyboard();
    setCenterContent(m_userInputWidget);
    QTimer::singleShot(300, m_userInputWidget, &UserInputWidget::grabKeyboard);
}

void LockContent::updateBackground(const QString &path)
{
    const QString &wallpaper = m_imageBlurInter->Get(path);

    emit requestBackground(wallpaper.isEmpty() ? path : wallpaper);
}

void LockContent::onBlurDone(const QString &source, const QString &blur, bool status)
{
    const QString &sourcePath = QUrl(source).isLocalFile() ? QUrl(source).toLocalFile() : source;

    if (status && m_model->currentUser()->greeterBackgroundPath() == sourcePath)
        emit requestBackground(blur);
}
