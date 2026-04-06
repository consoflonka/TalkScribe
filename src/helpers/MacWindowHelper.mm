#include "MacWindowHelper.h"
#include <QWindow>
#include <QDebug>

#ifdef Q_OS_MACOS
#import <Cocoa/Cocoa.h>
#endif

MacWindowHelper::MacWindowHelper(QObject* parent)
    : QObject(parent)
{
}

void MacWindowHelper::configureFloatingPanel(QWindow* window)
{
#ifdef Q_OS_MACOS
    if (!window)
        return;

    NSView* view = reinterpret_cast<NSView*>(window->winId());
    if (!view)
        return;

    NSWindow* nsWindow = [view window];
    if (!nsWindow)
        return;

    // Prevent ghost/shadow artifacts on transparent overlay windows
    [nsWindow setOpaque:NO];
    [nsWindow setHasShadow:NO];
    [nsWindow setBackgroundColor:[NSColor clearColor]];

    if ([nsWindow isKindOfClass:[NSPanel class]]) {
        NSPanel* panel = (NSPanel*)nsWindow;
        [panel setHidesOnDeactivate:NO];
        [panel setFloatingPanel:YES];
        [panel setBecomesKeyOnlyIfNeeded:YES];
        [panel setLevel:NSFloatingWindowLevel];
        [panel setCollectionBehavior:
            NSWindowCollectionBehaviorCanJoinAllSpaces |
            NSWindowCollectionBehaviorFullScreenAuxiliary];
        [panel setStyleMask:[panel styleMask] | NSWindowStyleMaskNonactivatingPanel];
    } else {
        [nsWindow setLevel:NSFloatingWindowLevel];
        [nsWindow setCollectionBehavior:
            NSWindowCollectionBehaviorCanJoinAllSpaces |
            NSWindowCollectionBehaviorFullScreenAuxiliary];
        [nsWindow setHidesOnDeactivate:NO];
    }
#else
    Q_UNUSED(window)
#endif
}

void MacWindowHelper::raiseFloatingPanel(QWindow* window)
{
#ifdef Q_OS_MACOS
    if (!window)
        return;

    NSView* view = reinterpret_cast<NSView*>(window->winId());
    if (!view)
        return;

    NSWindow* nsWindow = [view window];
    if (!nsWindow)
        return;

    // Only use orderFrontRegardless — do NOT activate the app.
    // activateIgnoringOtherApps:YES would bring ALL app windows to front
    // (including the main window), which is wrong for floating overlays
    // like the dictation mic button.
    [nsWindow orderFrontRegardless];
#else
    Q_UNUSED(window)
    if (window)
        window->raise();
#endif
}

void MacWindowHelper::saveFrontmostApp()
{
#ifdef Q_OS_MACOS
    NSRunningApplication* frontApp = [[NSWorkspace sharedWorkspace] frontmostApplication];
    if (frontApp) {
        m_savedPid = [frontApp processIdentifier];
        qDebug() << "MacWindowHelper: Saved frontmost app PID:" << m_savedPid
                 << "name:" << QString::fromNSString([frontApp localizedName]);
    }
#endif
}

void MacWindowHelper::restoreFrontmostApp()
{
#ifdef Q_OS_MACOS
    if (m_savedPid <= 0) return;

    NSRunningApplication* app =
        [NSRunningApplication runningApplicationWithProcessIdentifier:m_savedPid];
    if (app) {
        [app activateWithOptions:0];
        qDebug() << "MacWindowHelper: Restored frontmost app PID:" << m_savedPid
                 << "name:" << QString::fromNSString([app localizedName]);
    }
    m_savedPid = 0;
#endif
}
