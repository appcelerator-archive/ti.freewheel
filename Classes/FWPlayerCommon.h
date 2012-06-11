#import <UIKit/UIKit.h>
#import <CoreLocation/CoreLocation.h>
#import <MediaPlayer/MediaPlayer.h>
#import <AdManager/FWSDK.h>

#define FWPLAYER_AD_REQUEST_TIMEOUT 5

#define FWPLAYER_INTEGRATE_WITH_ADSERVER 1 // 0 indicate with local response, otherwise integrate with adserver

int fwplayer_short_screen_dimension();
int fwplayer_long_screen_dimension();
double fwplayer_system_version();

// APPCELERATOR-SPECIFIC MOD (START)
#ifdef DEBUG
#   define DLog(fmt, ...) NSLog((@"%s [Line %d] " fmt), __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__);
#else
#   define DLog(...)
#endif

// ALog always displays output regardless of the DEBUG setting
#define ALog(fmt, ...) NSLog((@"%s [Line %d] " fmt), __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__);
// APPCELERATOR-SPECIFIC MOD (END)