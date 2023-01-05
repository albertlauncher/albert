// Copyright (c) 2022 Manuel Schneider
#include "macos.h"
#include <Cocoa/Cocoa.h>

void setActivationPolicyAccessory()
{
    // Agent app
    [NSApp setActivationPolicy:NSApplicationActivationPolicyAccessory];

    // Always dark mode 😎
//    [NSApp setAppearance:[NSAppearance appearanceNamed:NSAppearanceNameDarkAqua]];

//    https://developer.apple.com/documentation/appkit/nsvisualeffectview?language=objc
//    https://stackoverflow.com/questions/24414483/how-can-i-use-nsvisualeffectview-in-windows-title-bar
//    http://eon.codes/blog/2016/01/23/Chromeless-window/

//    int NSWindowCollectionBehaviorCanJoinAllSpaces = 1 << 0;
//    int NSWindowCollectionBehaviorMoveToActiveSpace = 1 << 1;     // floats in spaces, hidden by exposé.  Default behavior if windowLevel != NSNormalWindowLevel
//    int NSWindowCollectionBehaviorTransient = 1 << 3;
//    int NSWindowCollectionBehaviorStationary = 1 << 4;
//    int NSWindowCollectionBehaviorFullScreenAuxiliary = 1 << 8;
//    int NSWindowCollectionBehaviorFullScreenNone = 1 << 9;
//    WId window_id = this->winId();
//    objc_object * ns_view_object = reinterpret_cast<objc_object *>(window_id);
//    objc_object * ns_window = ((objc_object* (*)(id, SEL))objc_msgSend)(ns_view_object, sel_registerName("window"));
//    ((objc_object* (*)(id, SEL, int))objc_msgSend)(ns_window, sel_registerName("setCollectionBehavior:"),
//                                                   NSWindowCollectionBehaviorCanJoinAllSpaces
//                                                   | NSWindowCollectionBehaviorTransient
//                                                   //                                                      | NSWindowCollectionBehaviorFullScreenAuxiliary
//                                                   //                                                   NSWindowCollectionBehaviorMoveToActiveSpace
//                                                   | NSWindowCollectionBehaviorStationary);
    //        | NSWindowCollectionBehaviorFullScreenNone
//    NSView *nsview = reinterpret_cast<NSView *>(wid);
//    NSWindow * nswindow = [nsview window];
//
//    [nswindow setCollectionBehavior:NSWindowCollectionBehaviorCanJoinAllSpaces
//                                    | NSWindowCollectionBehaviorTransient
//                                    | NSWindowCollectionBehaviorStationary ];
//    [nswindow setAnimationBehavior: NSWindowAnimationBehaviorNone];
//    qDebug() << "isKeyWindow" << [nswindow isKeyWindow];
//    qDebug() << "NSPanel" << [nswindow isKindOfClass: [NSPanel class]];
//    qDebug() << "canBecomeKeyWindow" << [nswindow canBecomeKeyWindow];
//    qDebug() << "[nswindow makeKeyWindow]";
//    [nswindow makeKeyWindow];
//    qDebug() << "[nswindow makeKeyAndOrderFront:nil];";
//    [nswindow makeKeyAndOrderFront:nil];
//    nswindow.styleMask |= NSWindowStyleMaskNonactivatingPanel;
//    qDebug() << "nswindow.styleMask" << nswindow.styleMask;

//    [nswindow setStyleMask: NSWindowStyleMaskNonactivatingPanel];

//    nswindow.styleMask |= NSWindowStyleMaskNonactivatingPanel;
//    qDebug() << "nswindow.styleMask" << nswindow.styleMask;
//    [nswindow toggleFullScreen:nil];

// seee also https://github.com/Andre-Gl/electron-panel/blob/master/functions_mac.cc

}

