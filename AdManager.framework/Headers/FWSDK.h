/** 
 * \mainpage FreeWheel AdManager Cocoa Touch SDK
 * 
 * The FreeWheel AdManager Cocoa Touch SDK is a static library for integrating your cocoa touch applications with FreeWheel ad services.
 *
 * The SDK supports iOS 3.0 ~ 4.2 running on iPhone, iPod Touch and iPad. It is a universal binary bundle with armv6, armv7 and i386 architectures. 
 *
 *
 * To use the SDK, 
 *	- Open "Project Info" window, select "Build" tab, add "-ObjC" in "Other Linker Flags".
 *	- Add libxml2.dylib, UIKit.framework, CoreGraphics.framework, QuartzCore.framework, MediaPlayer.framework, CoreLocation.framework frameworks 
 *	to your project (ctrl+click "Frameworks" group, Add -> Existing Frameworks).
 *	- Add AdManager.framework to your project (ctrl+click "Frameworks" group, Add -> Existing Frameworks -> Add Other, select AdManager.framework).
 *	- Add AdManager.framework/Resources/*.png to your project (ctrl+click "Resources" group, Add -> Existing Files ->, select *.png).
 *	- Add "#import <AdManager/FWSDK.h>" in your code.
 *
 * (Optional) To use the iAd renderer (iOS >= 4.0),
 *	- Add iAd.framework to your project (ctrl+click "Frameworks" group, Add -> Existing Frameworks).
 *
 * (Optional) To use the AdMob renderer,
 *- Add TouchJSON to your projectject (ctrl+click "Classes" group, Add -> Existing Files, select TouchJSON folder).
 *- Add AudioToolbox.framework, MessageUI.framework frameworks to yourour project (ctrl+click "Frameworks" group, Add -> Existing Frameworks).
 *- Add libAdmob.a to your project (ctrl+click "Frameworks" group, Add -> Existing Frameworksorks -> Add Other, select libAdMob.a).
 *
 */

#import "FWConstants.h"
#import "FWProtocols.h"