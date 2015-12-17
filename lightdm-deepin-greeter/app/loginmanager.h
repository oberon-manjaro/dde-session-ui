#ifndef LoginManager_H
#define LoginManager_H

#include <QApplication>
#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QKeyEvent>

#include <QLightDM/Greeter>
#include <QLightDM/SessionsModel>

#include "util_updateui.h"
#include "userwidget.h"
#include "passwdedit.h"
#include "view/logowidget.h"
#include "view/switchframe.h"
#include "sessionwidget.h"
#include "kblayoutwidget.h"
#include "shutdownwidget.h"
#include "xkbparser.h"

#include "dbus/dbusdisplaymanager.h"
#include "dbus/dbusvariant.h"
#include "dbus/dbuslogin1manager.h"
#include "dbus/dbuslockservice.h"

#include "darrowrectangle.h"
#include "util_file.h"

class LoginManager: public QFrame {
    Q_OBJECT
public:
    LoginManager(QWidget* parent=0);
    ~LoginManager();
signals:
    /*This signals is used to change the
    widgets position in different screens*/
    void screenChanged(QRect geom);
public slots:
    /*Update the position of the widgets after finished the layout of ui*/
    void updateWidgetsPosition();
protected:
    void keyPressEvent(QKeyEvent* e) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent* e) Q_DECL_OVERRIDE;
    void leaveEvent(QEvent *) Q_DECL_OVERRIDE;
private:
    void recordPid();
    void initUI();
    void initData();
    void initConnect();

    void initDateAndUpdate();

    void expandUserWidget();

    void prompt(QString text, QLightDM::Greeter::PromptType type);
    void authenticationComplete();
    void login();

    void chooseUserMode();
    void chooseSessionMode();
    void choosedSession();

    void showShutdownFrame();
    void keyboardLayoutUI();
    void keybdLayoutWidgetPosit();
    void setCurrentKeybdLayoutList(QString keyboard_value);

    void setShutdownAction(const ShutdownWidget::Actions action);
    void leftKeyPressed();
    void rightKeyPressed();

    LogoWidget* m_logoWidget;
    SwitchFrame* m_switchFrame;
    UserWidget* m_userWidget;
    PassWdEdit* m_passWdEdit;
    SessionWidget *m_sessionWidget;
    QHBoxLayout* m_passWdEditLayout;
    QVBoxLayout* m_Layout;

    ShutdownWidget* m_requireShutdownWidget;
    KbLayoutWidget* m_keybdLayoutWidget;
    DArrowRectangle* m_keybdArrowWidget;
    XkbParser* xkbParse;

    QLightDM::Greeter *m_greeter;
    DBusLogin1Manager* m_login1ManagerInterface;
    UtilFile* m_utilFile;
};
#endif // LoginManager

