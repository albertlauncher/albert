#ifndef XHOTKEYMANAGER_H
#define XHOTKEYMANAGER_H

#include <QThread>
#include <QDebug>
#include <X11/Xlib.h>

class XHotKeyManager : public QThread
{
    Q_OBJECT

    typedef struct {
        unsigned int numlock;
        unsigned int capslock;
        unsigned int scrolllock;
    } xhkLockmasks;

public:
    static XHotKeyManager * getInstance(){
        if (instance == nullptr)
            instance = new XHotKeyManager();
        return instance;
    }
    void run() Q_DECL_OVERRIDE ;

signals:
    void hotKeyPressed();

private:
    XHotKeyManager();
    ~XHotKeyManager();

    static XHotKeyManager*  instance;
    Display*                _display;
    Window                  _root;
    xhkLockmasks            _lmasks;
    bool                    _continue;

    void grab_key(int keycode, unsigned int modifiers);
    void ungrab_key(int keycode, unsigned int modifiers);
    void getOffendingModifiers();
};

#endif // XHOTKEYMANAGER_H
