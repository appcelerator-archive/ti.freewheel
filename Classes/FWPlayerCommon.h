#import <UIKit/UIKit.h>
#import <CoreLocation/CoreLocation.h>
#import <MediaPlayer/MediaPlayer.h>
#import <AdManager/FWSDK.h>

#define FWPLAYER_AD_REQUEST_TIMEOUT 5

#define FWPLAYER_INTEGRATE_WITH_ADSERVER 1 // 0 indicate with local response, otherwise integrate with adserver

int fwplayer_short_screen_dimension();
int fwplayer_long_screen_dimension();
double fwplayer_system_version();