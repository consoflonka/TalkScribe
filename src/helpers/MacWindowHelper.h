#pragma once

#include <QObject>

class QWindow;

class MacWindowHelper : public QObject
{
    Q_OBJECT

public:
    explicit MacWindowHelper(QObject* parent = nullptr);

    /// Configure a Qt Tool window as a non-hiding floating panel on macOS.
    /// Fixes the flicker caused by NSPanel's default hidesOnDeactivate=YES.
    /// Also sets NSWindowStyleMaskNonactivatingPanel so clicks don't activate the app.
    Q_INVOKABLE void configureFloatingPanel(QWindow* window);

    /// Force a non-activating panel to the front (orderFrontRegardless).
    /// Regular raise() has no effect on non-activating panels on macOS.
    Q_INVOKABLE void raiseFloatingPanel(QWindow* window);

    /// Save the PID of the currently frontmost application (not TalkLess).
    /// Call this when dictation starts so we can return to it after paste.
    Q_INVOKABLE void saveFrontmostApp();

    /// Activate the previously saved frontmost app (bring it to front).
    /// Call this before auto-pasting so Cmd+V goes to the right window.
    Q_INVOKABLE void restoreFrontmostApp();

private:
    int m_savedPid = 0;
};
