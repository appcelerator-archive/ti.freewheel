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
    
    // we'll lose data if we aren't careful with how we deal with this value
    long long convertedNetworkId = [networkId longLongValue];
        
    adManager = newAdManager();
    
    [adManager setLocation:nil];
    [adManager setNetworkId:convertedNetworkId]; // converted string before setting
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
    
    NSLog(@"Set current controller to: %@", controller);
}

- (void)setAdContext:(NSDictionary *)properties
{    
    ENSURE_UI_THREAD_0_ARGS;
    
    TiMediaVideoPlayerProxy *videoPlayer = [properties objectForKey:@"player"];
    currentSiteSection = [properties objectForKey:@"siteSection"];
    currentVideoId = [properties objectForKey:@"videoId"];
    currentProfile = [properties objectForKey:@"profile"];
    
    for (NSString *key in properties) {
        NSLog(@"[DEBUG] %@ is set on %@", key, [properties objectForKey:key]);
    }
    
    [self setCurrentController:[[TiApp app] controller]];
             
    currentPlayer = [videoPlayer player];
    
    NSLog(@"[DEBUG] Set current player and created ad context");
    
    [self createAdContext];
}

- (void)createAdContext
{        
    adContext = [adManager newContext];
    
    WARN_IF_BACKGROUND_THREAD_OBJ;
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(onAdRequestComplete:) name:FW_NOTIFICATION_REQUEST_COMPLETE object:adContext];
    
    [adContext addKeyValue:@"module" :@"VideoAdRenderer"];	
	[adContext addKeyValue:@"adType" :@"video/mp4-mepg4_simple"];
    
    [adContext setProfile:currentProfile :nil :nil :nil];
    [adContext setSiteSection:currentSiteSection :0 :0 :FW_ID_TYPE_CUSTOM :0];		
	[adContext setVideoAsset:currentVideoId :160 :nil :true :0 :0 :FW_ID_TYPE_CUSTOM :0 :FW_VIDEO_ASSET_DURATION_TYPE_EXACT];
	
	[adContext addSiteSectionNonTemporalSlot:@"nontemporal-slot-1-1" :nil :320 :44 :nil :YES :FW_SLOT_OPTION_INITIAL_AD_STAND_ALONE :@"text/html_doc_lit_mobile" :nil];	
	[adContext addTemporalSlot:@"temporal-slot-preroll" :FW_ADUNIT_PREROLL :0 :nil :0 :0 :nil :nil :0];			
    
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
    NSLog(@"Attempting to setup player based on response");
    
    if ([self _hasListeners:@"onadresponse"]) {
        [self fireEvent:@"onadresponse" withObject:[notification userInfo]];
    }
    
    if ([notification object] != adContext || [[notification userInfo] objectForKey:@"error"]) {
        NSLog(@"[DEBUG] There was an error with the ad response");
        
        if ([self _hasListeners:@"onadresponseerror"]) {
            [self fireEvent:@"onadresponseerror" withObject:[notification userInfo]];
        }
    } else {
        NSLog(@"Ad response looks good");
        NSLog(@"Setting MP controller and attempting to play ads");
        
        [adContext setMoviePlayerController:currentPlayer];
        [self playAds];
    }
}

- (void)playAds
{
    NSLog(@"Playing ads");
    
    NSArray *temporalAdSlots = [adContext temporalSlots];
    
    for(id<FWSlot> slot in temporalAdSlots) {
        if( [slot timePositionClass] == FW_TIME_POSITION_CLASS_PREROLL ) {        
            [slot play];
        }
    }
}

- (UIViewController *)childViewController;
{
	return nil;
}


@end
