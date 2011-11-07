#import "TiFreewheelAdManagerProxy.h"

@implementation CLLocationManager (TemporaryHack)

- (void)hackLocationFix
{
    CLLocation *location = [[CLLocation alloc] initWithLatitude:42 longitude:-50];
    [[self delegate] locationManager:self didUpdateToLocation:location fromLocation:nil]; 
    [location release];
}

- (void)startUpdatingLocation
{
    [self performSelector:@selector(hackLocationFix) withObject:nil afterDelay:0.1];
}

@end

@implementation TiFreewheelAdManagerProxy

- (void)_destroy
{
    [super _destroy];
}

- (void)configureFromDictonary:(NSDictionary *)properties
{    
        
    networkId = [properties objectForKey:@"networkId"];
    serverUrl = [properties objectForKey:@"serverUrl"];
        
    for (NSString *key in properties) {
        NSLog(@"[DEBUG] %@ is set to %@", key, [properties objectForKey:key]);
    }
        
    [self start:nil];

    NSLog(@"[DEBUG] Configured Ad Manager proxy");
}

- (void)_initWithProperties:(NSDictionary *)properties
{
    [super _initWithProperties:properties];
    [self configureFromDictonary:properties];
}

- (void)start:(id)args
{
    ENSURE_UI_THREAD_0_ARGS;
        
    adManager = newAdManager();
    
    [adManager setLocation:nil];
    [adManager setNetworkId:[networkId longLongValue]];
	[adManager setServerUrl:serverUrl];
    
    locationManager = [[CLLocationManager alloc] init];
	locationManager.delegate = self;
	[locationManager startUpdatingLocation];	
	[adManager setLocation:[locationManager location]];
	
    NSLog(@"[DEBUG] Started Ad Manager");
}

-(void)setCurrentController:(UIViewController*)controller
{
    [adManager setCurrentViewController:controller];
    
    NSLog(@"[DEBUG] Set current controller to: %@", controller);
}

- (void)setAdContext:(id)args
{    
    ENSURE_SINGLE_ARG_OR_NIL(args,NSDictionary);
	ENSURE_UI_THREAD_1_ARG(args);
    
    TiMediaVideoPlayerProxy *videoPlayer = [args objectForKey:@"player"];
    currentSiteSection = [args objectForKey:@"siteSection"];
    currentVideoId = [args objectForKey:@"videoId"];
    currentProfile = [args objectForKey:@"profile"];
    
    for (NSString *key in args) {
        NSLog(@"[DEBUG] %@ is set on %@", key, [args objectForKey:key]);
    }
    
    [self setCurrentController:[[TiApp app] controller]];
             
    currentPlayer = [videoPlayer player]; // we do get access to this, even though xcode is complaining
    
    NSLog(@"[DEBUG] Set current player and created ad context");
    
    [self createAdContext];
}

- (void)createAdContext
{        
    adContext = [adManager newContext];
    
    WARN_IF_BACKGROUND_THREAD_OBJ;
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(onAdRequestComplete:) name:FW_NOTIFICATION_REQUEST_COMPLETE object:adContext];
    
    [adContext setProfile:currentProfile :nil :nil :nil];
    [adContext setSiteSection:currentSiteSection :0 :[networkId longLongValue] :FW_ID_TYPE_CUSTOM :0];		
	[adContext setVideoAsset:currentVideoId :160 :nil :true :0 :0 :FW_ID_TYPE_CUSTOM :35437170 :FW_VIDEO_ASSET_DURATION_TYPE_EXACT];
	
    [adContext setMoviePlayerController:currentPlayer];
    
    NSLog(@"[DEBUG] Created ad context and submitting request");
    
    [adContext submitRequest:2];  
}

- (void)locationManager:(CLLocationManager *)manager didUpdateToLocation:(CLLocation *)newLocation fromLocation:(CLLocation *)oldLocation
{    
    [adManager setLocation:newLocation];
    
    NSLog(@"[DEBUG] Set location");
}

- (void)locationManager:(CLLocationManager *)manager didFailWithError:(NSError *)error 
{
    NSLog(@"[DEBUG] Location manager had an error: %@", error);
}

- (void)onAdRequestComplete:(NSNotification *)notification 
{    
    NSLog(@"[DEBUG] Add response recieved");
    NSLog(@"[DEBUG] Attempting to setup player based on response");
    
    if ([self _hasListeners:@"onadresponse"]) {
        [self fireEvent:@"onadresponse" withObject:[notification userInfo]];
    }
    
    if ([notification object] != adContext || [[notification userInfo] objectForKey:@"error"]) {
        NSLog(@"[DEBUG] There was an error with the ad response");
        
        if ([self _hasListeners:@"onadresponseerror"]) {
            [self fireEvent:@"onadresponseerror" withObject:[notification userInfo]];
        }
    } else {
        NSLog(@"[DEBUG] Ad response looks good");
        NSLog(@"[DEBUG] Setting MP controller and attempting to play ads");
        
        [self playAds];
    }
}

- (void)playAds
{
    NSLog(@"[DEBUG] Playing ads");
    
    ENSURE_UI_THREAD_0_ARGS;
}

@end
