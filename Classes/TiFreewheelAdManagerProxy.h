#import "TiProxy.h"
#import "TiViewController.h"
#import "TiMediaVideoPlayerProxy.h"
#import <CoreLocation/CoreLocation.h>
#import "TiApp.h"
#import "FWPlayerCommon.h"

@interface TiFreewheelAdManagerProxy : TiProxy <CLLocationManagerDelegate> {
    TiProxy *proxy;
    NSDictionary *managerProperties;
    CLLocationManager *locationManager;
    NSString *networkId;
    NSString *serverUrl;
    NSString *currentSiteSection;
    NSString *currentVideoId;
    NSString *currentProfile;
    MPMoviePlayerController *currentPlayer;
    id<FWAdManager> adManager;
    id<FWContext> adContext;
}

- (void)start:(id)args;
- (void)locationManager:(CLLocationManager *)manager didUpdateToLocation:(CLLocation *)newLocation fromLocation:(CLLocation *)oldLocation;
- (void)locationManager:(CLLocationManager *)manager didFailWithError:(NSError *)error;

- (void)createAdContext;
- (void)onAdRequestComplete:(NSNotification *)notification;
- (void)playAds;

@end