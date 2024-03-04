// Copyright (c) 2022-2024 Manuel Schneider

#include "albert/albert.h"
#include "albert/frontend/frontend.h"
#include "albert/logging.h"
#include "platform/platform.h"
#include <Cocoa/Cocoa.h>
#include <QGuiApplication>
#include <QMessageBox>
#include <QTimer>
#include <UserNotifications/UserNotifications.h>
//#include <Foundation/Foundation.h>

using namespace albert;

static void requestAccessibilityPermissions()
{
    if (!AXIsProcessTrusted())
    {
        NSDictionary *options = @{(id)kAXTrustedCheckOptionPrompt: @YES};
        AXIsProcessTrustedWithOptions((CFDictionaryRef)options);
    }
}

static void requestNotificationPermissions()
{
//    // https://developer.apple.com/documentation/usernotifications/asking_permission_to_use_notifications?language=objc

//    UNUserNotificationCenter *center = [UNUserNotificationCenter currentNotificationCenter];


//    [center requestAuthorizationWithOptions:(UNAuthorizationOptionAlert + UNAuthorizationOptionSound)
//                          completionHandler:^(BOOL granted, NSError * _Nullable error) {

//                              if (error) {
//                                  WARN << QString::fromNSString(error.localizedDescription);
//                              }

//                              if (granted) {
//                                  WARN << "granted";
//                              } else {
//                                  WARN << "denied";
//                                  CRIT << "Notification permissions are mandatory.";
//                              }
//                          }];

//    [center getNotificationSettingsWithCompletionHandler:^(UNNotificationSettings * _Nonnull settings) {
//        switch (settings.authorizationStatus) {

//        case UNAuthorizationStatusNotDetermined:{
//            WARN << "UNAuthorizationStatusNotDetermined";


//            break;
//            }
//        case UNAuthorizationStatusDenied:
//            WARN << "UNAuthorizationStatusDenied";
//            break;
//        case UNAuthorizationStatusAuthorized:
//            WARN << "UNAuthorizationStatusAuthorized";
//            break;
//        case UNAuthorizationStatusProvisional:
//            WARN << "UNAuthorizationStatusProvisional";
//            break;
//        default:
//            break;
//        }
//    }];
}

void platform::initPlatform()
{
    requestAccessibilityPermissions();
    requestNotificationPermissions();
}

void platform::initNativeWindow(unsigned long long wid)
{
    NSWindow *ns_window = [reinterpret_cast<id>(wid) window];

    /*
     * @const NSWindowAnimationBehaviorDefault  Let AppKit infer animation behavior for this window.
     * @const NSWindowAnimationBehaviorNone     Suppress inferred animations (don't animate).
     * @const NSWindowAnimationBehaviorDocumentWindow
     * @const NSWindowAnimationBehaviorUtilityWindow
     * @const NSWindowAnimationBehaviorAlertPanel
     */

    [ns_window setAnimationBehavior: NSWindowAnimationBehaviorNone]; // no fancy fade or sth


    /*
     * @const NSWindowCollectionBehaviorPrimary Marks a window as primary. This collection behavior should commonly be used for document or viewer windows.
     * @const NSWindowCollectionBehaviorAuxiliary Marks a window as auxiliary. This collection behavior should commonly be used for About or Settings windows, as well as utility panes.
     * @const NSWindowCollectionBehaviorCanJoinAllApplications Marks a window as able to join all applications, allowing it to join other apps' sets and full screen spaces when eligible. This collection behavior should commonly be used for floating windows and system overlays.
     *
     * @discussion You may specify at most one of @c NSWindowCollectionBehaviorPrimary, @c NSWindowCollectionBehaviorAuxiliary, or @c NSWindowCollectionBehaviorCanJoinAllApplications. If unspecified, the window gets the default treatment determined by its other collection behaviors.
     *
     * @const NSWindowCollectionBehaviorDefault
     * @const NSWindowCollectionBehaviorCanJoinAllSpaces
     * @const NSWindowCollectionBehaviorMoveToActiveSpace
     *
     * @discussion You may specify at most one of \c NSWindowCollectionBehaviorManaged, \c NSWindowCollectionBehaviorTransient, or \c NSWindowCollectionBehaviorStationary.  If neither is specified, the window gets the default behavior determined by its window level.
     *
     * @const NSWindowCollectionBehaviorManaged Participates in spaces, exposé.  Default behavior if `windowLevel == NSNormalWindowLevel`.
     * @const NSWindowCollectionBehaviorTransient Floats in spaces, hidden by exposé.  Default behavior if `windowLevel != NSNormalWindowLevel`.
     * @const NSWindowCollectionBehaviorStationary Unaffected by exposé.  Stays visible and stationary, like desktop window.
     *
     * @discussion You may specify at most one of \c NSWindowCollectionBehaviorParticipatesInCycle or \c NSWindowCollectionBehaviorIgnoresCycle.  If unspecified, the window gets the default behavior determined by its window level.
     *
     * @const NSWindowCollectionBehaviorParticipatesInCycle Default behavior if `windowLevel != NSNormalWindowLevel`.
     * @const NSWindowCollectionBehaviorIgnoresCycle Default behavior if `windowLevel != NSNormalWindowLevel`.
     *
     * @discussion You may specify at most one of \c NSWindowCollectionBehaviorFullScreenPrimary, \c NSWindowCollectionBehaviorFullScreenAuxiliary, or \c NSWindowCollectionBehaviorFullScreenNone.
     *
     * @const NSWindowCollectionBehaviorFullScreenPrimary The frontmost window with this collection behavior will be the fullscreen window.
     * @const NSWindowCollectionBehaviorFullScreenAuxiliary Windows with this collection behavior can be shown with the fullscreen window.
     * @const NSWindowCollectionBehaviorFullScreenNone The window can not be made fullscreen when this bit is set.
     *
     * @discussion You may specify at most one of \c NSWindowCollectionBehaviorFullScreenAllowsTiling or \c NSWindowCollectionBehaviorFullScreenDisallowsTiling, or an assertion will be raised.
     *
     * The default behavior is to allow any window to participate in full screen tiling, as long as it meets certain requirements, such as being resizable and not a panel or sheet. Windows which are not full screen capable can still become a secondary tile in full screen. A window can explicitly allow itself to be placed into a full screen tile by including \c NSWindowCollectionBehaviorFullScreenAllowsTiling. Even if a window allows itself to be placed in a tile, it still may not be put in the tile if its \c minFullScreenContentSize is too large to fit. A window can explicitly disallow itself from being placed in a full screen tile by including \c NSWindowCollectionBehaviorFullScreenDisallowsTiling. This is useful for non-full screen capable windows to explicitly prevent themselves from being tiled. It can also be used by a full screen window to prevent any other windows from being placed in its full screen tile.
     *
     * @const NSWindowCollectionBehaviorFullScreenAllowsTiling This window can be a full screen tile window. It does not have to have \c NSWindowCollectionBehaviorFullScreenPrimary set.
     * @const NSWindowCollectionBehaviorFullScreenDisallowsTiling This window can NOT be made a full screen tile window; it still may be allowed to be a regular \c NSWindowCollectionBehaviorFullScreenPrimary window.
     */
    [ns_window setCollectionBehavior: ([ns_window collectionBehavior] | NSWindowCollectionBehaviorMoveToActiveSpace)];

    /*
     * @const NSWindowStyleMaskBorderless
     * @const NSWindowStyleMaskTitled
     * @const NSWindowStyleMaskClosable
     * @const NSWindowStyleMaskMiniaturizable
     * @const NSWindowStyleMaskResizable
     * @const NSWindowStyleMaskTexturedBackground  Textured window style is deprecated and should no longer be used. Specifies a window with textured background. Textured windows generally don't draw a top border line under the titlebar/toolbar. To get that line, use the \c NSUnifiedTitleAndToolbarWindowMask mask.
     * @const NSWindowStyleMaskUnifiedTitleAndToolbar  Specifies a window whose titlebar and toolbar have a unified look - that is, a continuous background. Under the titlebar and toolbar a horizontal separator line will appear.
     * @const NSWindowStyleMaskFullScreen  When present, the window will appear full screen. This mask is automatically toggled when \c -toggleFullScreen: is called.
     * @const NSWindowStyleMaskFullSizeContentView If set, the \c contentView will consume the full size of the window; it can be combined with other window style masks, but is only respected for windows with a titlebar. Utilizing this mask opts-in to layer-backing. Utilize the \c contentLayoutRect or auto-layout \c contentLayoutGuide to layout views underneath the titlebar/toolbar area.
     * @const NSWindowStyleMaskUtilityWindow Only applicable for \c NSPanel (or a subclass thereof).
     * @const NSWindowStyleMaskDocModalWindow Only applicable for \c NSPanel (or a subclass thereof).
     * @const NSWindowStyleMaskNonactivatingPanel  Specifies that a panel that does not activate the owning application. Only applicable for \c NSPanel (or a subclass thereof).
     * @const NSWindowStyleMaskHUDWindow Specifies a heads up display panel.  Only applicable for \c NSPanel (or a subclass thereof).
     */
    ns_window.styleMask |= NSWindowStyleMaskNonactivatingPanel;  // will get no key an not return focus without
    ns_window.hidesOnDeactivate = false;  // makes hide on focus out work
    [NSApp hide:nil];  // The app activates on start. undo.
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

class NotificationPrivate
{
public:
    NSUserNotification *notification;
};

albert::Notification::Notification(const QString &title, const QString &body) : d(new NotificationPrivate)
{
    d->notification = [[NSUserNotification alloc] init];
    d->notification.title = title.toNSString();
//    d->notification.subtitle = body.toNSString();
    d->notification.informativeText = body.toNSString();
    d->notification.hasActionButton = NO;
    d->notification.soundName = NSUserNotificationDefaultSoundName;
    [NSUserNotificationCenter.defaultUserNotificationCenter deliverNotification:d->notification];
}

albert::Notification::~Notification()
{
    [NSUserNotificationCenter.defaultUserNotificationCenter removeDeliveredNotification:d->notification];
    [d->notification release];
    delete d;
}

#pragma clang diagnostic pop

































//        Maybe useful trash


//static void requestFullDiskAccessPermissions(){
////    NSURL *url = [NSURL URLWithString:@"x-apple.systempreferences:com.apple.preference.security?Privacy_AllFiles"];
////    if ([[NSWorkspace sharedWorkspace] openURL:url]) {
////        NSLog(@"Opened Security & Privacy preferences");
////    }
//}


////        // Schedule the notification.
////        UNUserNotificationCenter *center = [UNUserNotificationCenter currentNotificationCenter];
////        //        [center addNotificationRequest:request];  // 02-23-2019 don't compile
////        [center addNotificationRequest:request withCompletionHandler:^(NSError * _Nullable error) {
////                     if (!error) {
////                         NSLog(@"Local Notification succeeded");
////                     }
////                     else {
////                         NSLog(@"Local Notification failed");
////                     }
////                 }];

//    UNMutableNotificationContent *content = [[UNMutableNotificationContent alloc] init];
//    content.title = @"Hello!";
//    content.body = @"This is a sample notification.";
//    content.sound = [UNNotificationSound defaultSound];

//    // Set the trigger (e.g., trigger notification after 10 seconds)


//    static UNNotificationRequest *request = [UNNotificationRequest requestWithIdentifier:@"sampleNotification"
//                                                                                 content:content
//                                                                                 trigger:nil];

//    // Add the request to the notification center
//    [[UNUserNotificationCenter currentNotificationCenter] addNotificationRequest:request
//                                                           withCompletionHandler:^(NSError * _Nullable error) {
//                                                               if (error) {
//                                                                   NSLog(@"Error scheduling notification: %@", error.localizedDescription);
//                                                               }
//                                                           }];



//    CRIT << "styleMask" << ns_window.styleMask;
//    CRIT << "level" << ns_window.level;
//    CRIT << "NSPanel" << [ns_window isKindOfClass: [NSPanel class]];
//    CRIT << "hidesOnDeactivate" << ns_window.hidesOnDeactivate;
//    CRIT << "hidesOnDeactivate" << ns_window.hidesOnDeactivate;
//    [NSApp activateIgnoringOtherApps:YES];
//    CRIT << "isKeyWindow" << [ns_window isKeyWindow];
//    CRIT << "canBecomeKeyWindow" << [ns_window canBecomeKeyWindow];
//    CRIT << "[nswindow makeKeyWindow]";
//    [ns_window makeKeyWindow];
//    CRIT << "isKeyWindow" << [ns_window isKeyWindow];

//    NSVisualEffectView *effectsView = [[NSVisualEffectView alloc] init];
//    effectsView.blendingMode = NSVisualEffectBlendingModeBehindWindow;

//void platform::show()
//{
//    NSWindow *ns_window = [reinterpret_cast<id>(frontend()->winId()) window];
//    [ns_window orderFrontRegardless];
//    [ns_window makeKeyWindow];
////    [ns_window showWindow:nil];
//    [ns_window makeKeyAndOrderFront:nil];
//}

//void platform::hide()
//{
//    NSWindow *ns_window = [reinterpret_cast<id>(frontend()->winId()) window];
//    [ns_window resignKeyWindow];

//}

//void platform::resignKey(unsigned long long wid) {
//    NSWindow *ns_window = [reinterpret_cast<id>(wid) window];
//    /*[NSApp hide:nil];*/

//    //    CRIT << QString::fromNSString( NSWorkspace.sharedWorkspace.menuBarOwningApplication.bundleIdentifier);
//    //    [NSWorkspace.sharedWorkspace.menuBarOwningApplication activateWithOptions: NSApplicationActivateAllWindows];
//}

// Agent app
//    [NSApp setActivationPolicy:NSApplicationActivationPolicyAccessory];

// Always dark mode 😎
//[NSApp setAppearance:[NSAppearance appearanceNamed:NSAppearanceNameDarkAqua]];

