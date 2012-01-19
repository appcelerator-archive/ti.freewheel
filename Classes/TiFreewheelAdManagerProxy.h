#import "TiProxy.h"
#import "TiViewController.h"
#import "TiMediaVideoPlayerProxy.h"
#import <CoreLocation/CoreLocation.h>
#import "TiApp.h"
#import "FWPlayerCommon.h"
#import "TiFreewheelCompanionView.h"

@interface TiFreewheelAdManagerProxy : TiProxy <CLLocationManagerDelegate> {
    TiProxy *proxy; // proxy that is returned
    NSDictionary *managerProperties;
    CLLocationManager *locationManager;
    NSString *networkId;
    NSString *serverUrl;
    NSString *currentSiteSection;
    NSString *currentVideoId;
    NSString *currentFallbackId;
    NSString *currentProfile;
    NSURL *currentContentUrl; // video content url used after slots are finished
    MPMoviePlayerController *currentPlayer; // video player that is rendering content
    TiViewProxy *currentBase; // used to attach FW views
    TiViewProxy *currentCompanionBase; // used to attach companion ad
    BOOL processCompanion;
    id<FWAdManager> adManager;
    id<FWContext> adContext;
}

- (void)start:(id)args;
- (void)locationManager:(CLLocationManager *)manager didUpdateToLocation:(CLLocation *)newLocation fromLocation:(CLLocation *)oldLocation;
- (void)locationManager:(CLLocationManager *)manager didFailWithError:(NSError *)error;

- (void)createAdContext;
- (void)onAdRequestComplete:(NSNotification *)notification;
- (void)playAds:(id)args; // time

@end