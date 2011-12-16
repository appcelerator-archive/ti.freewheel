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
        
    currentContentUrl = [args objectForKey:@"contentUrl"];
    currentFallbackId = [args objectForKey:@"fallbackId"];
    currentBase = [args objectForKey:@"base"];
    currentCompanionBase = [args objectForKey:@"companionBase"];
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
	[adContext setVideoAsset:currentVideoId :0 :nil :true :0 :0 :FW_ID_TYPE_CUSTOM :[currentFallbackId longLongValue] :FW_VIDEO_ASSET_DURATION_TYPE_EXACT];
    [adContext setParameter:FW_PARAMETER_COUNTDOWN_TIMER_DISPLAY withValue:@"YES" forLevel:FW_PARAMETER_LEVEL_OVERRIDE];
    [adContext setParameter:FW_PARAMETER_COUNTDOWN_TIMER_POSITION withValue:@"top" forLevel:FW_PARAMETER_LEVEL_OVERRIDE];
    [adContext setParameter:FW_PARAMETER_COUNTDOWN_TIMER_BG_COLOR withValue:@"0x000000" forLevel:FW_PARAMETER_LEVEL_OVERRIDE];
    [adContext setParameter:FW_PARAMETER_COUNTDOWN_TIMER_ALPHA withValue:@"1.0" forLevel:FW_PARAMETER_LEVEL_OVERRIDE];
    [adContext setParameter:FW_PARAMETER_COUNTDOWN_TIMER_TEXT_SIZE withValue:@"small" forLevel:FW_PARAMETER_LEVEL_OVERRIDE];
    // [adContext setParameter:FW_PARAMETER_CLICK_DETECTION withValue:@"NO" forLevel:FW_PARAMETER_LEVEL_OVERRIDE];
    
    [adContext addVideoPlayerNonTemporalSlot:[NSString stringWithFormat:@"%c%c%c", (char)(65 + (arc4random() % 25)), (char)(48 + (arc4random() % 9)), (char)(65 + (arc4random() % 25))] :nil :300 :50 :nil :YES :FW_SLOT_OPTION_INITIAL_AD_STAND_ALONE :nil :nil :nil];
    
    [adContext setVideoDisplayBase:[currentBase view]];
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
    
    if ([notification object] != adContext || [[notification userInfo] objectForKey:@"error"]) {
        NSLog(@"[DEBUG] There was an error with the ad response");
        
        if ([self _hasListeners:@"onadresponseerror"]) {
            [self fireEvent:@"onadresponseerror" withObject:[notification userInfo]];
        }
    } else {
        NSLog(@"[DEBUG] Ad response looks good");
        NSLog(@"[DEBUG] Setting MP controller and attempting to play ads");

        WARN_IF_BACKGROUND_THREAD_OBJ;
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(onAdSlotStarted:) name:FW_NOTIFICATION_SLOT_STARTED object:adContext];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(onAdSlotEnded:) name:FW_NOTIFICATION_SLOT_ENDED object:adContext];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(onContentVideoPauseRequest:) name:FW_NOTIFICATION_CONTENT_PAUSE_REQUEST object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(onContentVideoResumeRequest:) name:FW_NOTIFICATION_CONTENT_RESUME_REQUEST object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(onAdOpen:) name:FW_NOTIFICATION_IN_APP_VIEW_OPEN object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(frameChanged:) name:FW_NOTIFICATION_VIDEO_DISPLAY_BASE_FRAME_CHANGED object:nil];
        
        NSMutableArray *prerollSlots = [[NSMutableArray alloc] init];
        NSMutableArray *midrollSlots = [[NSMutableArray alloc] init];
        NSMutableArray *postrollSlots = [[NSMutableArray alloc] init];
        NSDictionary *adPositions;
        
        for(id<FWSlot> slot in [adContext temporalSlots]) {
            NSLog(@"Time Position: %f", [slot timePosition]);
            
            switch ([slot timePositionClass]) {
                case FW_TIME_POSITION_CLASS_PREROLL:
                    [prerollSlots addObject:[[[NSDictionary alloc] initWithObjectsAndKeys:
                                             [NSNumber numberWithDouble:[slot timePosition]], @"time", 
                                             nil] autorelease]];
                    NSLog(@"[DEBUG] Time Position Class is PREROLL");
                    break;
                case FW_TIME_POSITION_CLASS_MIDROLL:
                    [midrollSlots addObject:[[[NSDictionary alloc] initWithObjectsAndKeys:
                                             [NSNumber numberWithDouble:[slot timePosition]], @"time", 
                                             nil] autorelease]];
                    NSLog(@"[DEBUG] Time Position Class is MIDROLL");
                    break;
                case FW_TIME_POSITION_CLASS_POSTROLL:
                    [postrollSlots addObject:[[[NSDictionary alloc] initWithObjectsAndKeys:
                                              [NSNumber numberWithDouble:[slot timePosition]], @"time", 
                                              nil] autorelease]];
                    NSLog(@"[DEBUG] Time Position Class is POSTROLL");
                    break;
                default:
                    break;
            }
        }
        
        adPositions = [[NSDictionary alloc] initWithObjectsAndKeys:
                                     prerollSlots, @"prerolls", 
                                     midrollSlots, @"midrolls", 
                                     postrollSlots, @"postrolls", 
                                     nil];
        
        if ([self _hasListeners:@"onadresponse"]) {
            [self fireEvent:@"onadresponse" withObject:adPositions];
        }
        
        [prerollSlots release];
        [midrollSlots release];
        [postrollSlots release];
        [adPositions release];
    }
}

- (void)frameChanged:(NSNotification*)notification
{
    NSLog(@"[ERROR] FRAME CHANGED");
    
    for (UIView *subview in [[currentBase view] subviews])
    {
        NSLog(@"[ERROR] %@", subview);
    }
}

- (void)onAdOpen:(NSNotification*)notification
{
    if ([self _hasListeners:@"onadopen"]) {
        [self fireEvent:@"onadopen" withObject:[notification userInfo]];
    }
}

- (void)onContentVideoPauseRequest:(NSNotification*)notification
{
    NSLog(@"[DEBUG] Pause request sent from AdManager");

    [currentPlayer pause];
    [adContext setVideoState:FW_VIDEO_STATE_PAUSED];
}

- (void)onContentVideoResumeRequest:(NSNotification*)notification
{
    NSLog(@"[DEBUG] Resume request sent from AdManager");
    
    [currentPlayer play];
    [adContext setVideoState:FW_VIDEO_STATE_PLAYING];
}

- (void)onAdSlotStarted:(NSNotification *)notification
{
//    [[[currentPlayer view] layer] setOpacity:0];
    
    if ([[notification userInfo] objectForKey:@"error"]) {
        NSLog(@"[ERROR] SLOT START FAILED: %@", [[notification userInfo] objectForKey:@"error"]);
    }
    
    NSMutableArray *ads = [[NSMutableArray alloc] init];
        
    for (id<FWAdInstance> instance in [[adContext getSlotByCustomId:[[notification userInfo] objectForKey:FW_INFO_KEY_CUSTOM_ID]] adInstances]) {
        [ads addObject:[[[NSDictionary alloc] initWithObjectsAndKeys:
                         [NSNumber numberWithLongLong:[instance creativeId]], @"creativeId",
                         [NSNumber numberWithLongLong:[[adContext getSlotByCustomId:[[notification userInfo] objectForKey:FW_INFO_KEY_CUSTOM_ID]] totalDuration]], @"duration",
                         nil] autorelease]];
        
        [[currentCompanionBase view] addSubview:[[adContext getSlotByCustomId:[[[instance companionSlots] objectAtIndex:0] customId]] slotBase]]; // add companion view
        
        NSLog(@"[DEBUG] Ad Instance: %@", instance);
    }
            
    if ([self _hasListeners:@"onslotstarted"]) {
        [self fireEvent:@"onslotstarted" withObject:[[[NSDictionary alloc] initWithObjectsAndKeys:
                                                     ads, @"ads",
                                                     nil] autorelease]];   
    }
    
    NSLog(@"[DEBUG] Companion Base Subview Count: %d", [[[currentCompanionBase view] subviews] count]); // just checking to see if the slotBase is being removed correctly
    
    [ads release];
}

- (void)onAdSlotEnded:(NSNotification *)notification
{
//    [[[currentPlayer view] layer] setOpacity:1];
    
    if ([[notification userInfo] objectForKey:@"error"]) {
        NSLog(@"[ERROR] SLOT END FAILED: %@", [[notification userInfo] objectForKey:@"error"]);
    }
    
    if ([self _hasListeners:@"onslotended"]) {
        [self fireEvent:@"onslotended" withObject:[notification userInfo]];
    }    
}

- (void)destroyContext:(id)args
{
    ENSURE_UI_THREAD_0_ARGS;
    
    NSLog(@"[DEBUG] Releasing ad context");
    
    for (id<FWSlot> iter in [adContext siteSectionNonTemporalSlots]) {
		[iter stop];
		[[iter slotBase] removeFromSuperview];
	}
    
	for (id<FWSlot> iter in [adContext temporalSlots]) {
		[iter stop];
	}
    
    [adContext release];
}

- (void)playAds:(id)args
{
    ENSURE_SINGLE_ARG_OR_NIL(args,NSDictionary);
	ENSURE_UI_THREAD_1_ARG(args);
    
    NSLog(@"[DEBUG] Playing ads");
        
    for (id<FWSlot> iter in [adContext temporalSlots]) {
		if ([iter timePosition] == [[args objectForKey:@"time"] longLongValue]) {
			[iter play];
			return;
		}
	}
    
    
}

@end
