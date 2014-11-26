#import "TiFreewheelAdManagerProxy.h"

@implementation CLLocationManager (TemporaryHack)

- (void)hackLocationFix
{
    CLLocation *location = [[CLLocation alloc] initWithLatitude:42 longitude:-50];
    [[self delegate] locationManager:self didUpdateToLocation:location fromLocation:nil]; 
    
    RELEASE_TO_NIL(location);
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
        DLog(@"[DEBUG] (FreeWheel Module) %@ is set to %@", key, [properties objectForKey:key]);
    }
        
    [self start:nil];

    DLog(@"[DEBUG] (FreeWheel Module) Configured Ad Manager proxy");
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
    
    FWSetLogLevel(FW_LOG_LEVEL_QUIET);
    
    [adManager setLocation:nil];
    [adManager setNetworkId:[networkId longLongValue]];
	[adManager setServerUrl:serverUrl];
    
    locationManager = [[CLLocationManager alloc] init];
	locationManager.delegate = self;
	[locationManager startUpdatingLocation];	
	[adManager setLocation:[locationManager location]];
	
    DLog(@"[DEBUG] (FreeWheel Module) Started Ad Manager");
}

-(void)setCurrentController:(UIViewController*)controller
{
    [adManager setCurrentViewController:controller];
    
    DLog(@"[DEBUG] (FreeWheel Module) Set current controller to: %@", controller);
}

- (void)setAdContext:(id)args
{    
    ENSURE_SINGLE_ARG_OR_NIL(args,NSDictionary);
	ENSURE_UI_THREAD_1_ARG(args);
        
    currentContentUrl = [args objectForKey:@"contentUrl"];
    currentFallbackId = [args objectForKey:@"fallbackId"];
    currentBase = [args objectForKey:@"base"];
    currentCompanionBase = [args objectForKey:@"companionBase"];
    currentSiteSection = [args objectForKey:@"siteSection"];
    currentVideoId = [args objectForKey:@"videoId"];
    currentProfile = [args objectForKey:@"profile"];
    processCompanion = [TiUtils boolValue:[args objectForKey:@"processCompanion"] def:YES];
            
    for (NSString *key in args) {
        DLog(@"[DEBUG] (FreeWheel Module) %@ is set on %@", key, [args objectForKey:key]);
    }
    
    [self setCurrentController:[[TiApp app] controller]];
             
    currentPlayer = [[args objectForKey:@"player"] ensurePlayer]; // we do get access to this, even though xcode is complaining
    
    DLog(@"currentPlayer: %@", currentPlayer);
    
    DLog(@"[DEBUG] (FreeWheel Module) Set current player and created ad context");
        
    [self createAdContext];
}

- (void)createAdContext
{        
    contextDestroyed = NO;
    
    adContext = [adManager newContext];
        
    WARN_IF_BACKGROUND_THREAD_OBJ;
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(onAdRequestComplete:) name:FW_NOTIFICATION_REQUEST_COMPLETE object:adContext];
        
    [adContext setSiteSection:currentSiteSection :0 :[networkId longLongValue] :FW_ID_TYPE_CUSTOM :0];
    [adContext setVideoAsset:currentVideoId :0 :nil :true :0 :0 :FW_ID_TYPE_CUSTOM :[currentFallbackId longLongValue] :FW_VIDEO_ASSET_DURATION_TYPE_EXACT];
    [adContext setParameter:FW_PARAMETER_COUNTDOWN_TIMER_DISPLAY withValue:@"NO" forLevel:FW_PARAMETER_LEVEL_OVERRIDE];
    // [adContext setParameter:FW_PARAMETER_COUNTDOWN_TIMER_POSITION withValue:@"10, 10" forLevel:FW_PARAMETER_LEVEL_OVERRIDE];
    // [adContext setParameter:FW_PARAMETER_COUNTDOWN_TIMER_WIDTH withValue:@"200" forLevel:FW_PARAMETER_LEVEL_OVERRIDE];
    // [adContext setParameter:FW_PARAMETER_COUNTDOWN_TIMER_BG_COLOR withValue:@"0x000000" forLevel:FW_PARAMETER_LEVEL_OVERRIDE];
    // [adContext setParameter:FW_PARAMETER_COUNTDOWN_TIMER_ALPHA withValue:@"1.0" forLevel:FW_PARAMETER_LEVEL_OVERRIDE];
    // [adContext setParameter:FW_PARAMETER_COUNTDOWN_TIMER_TEXT_SIZE withValue:@"small" forLevel:FW_PARAMETER_LEVEL_OVERRIDE];
    [adContext setParameter:FW_PARAMETER_VIDEO_AD_USE_APPLICATION_AUDIO_SESSION withValue:@"YES" forLevel:FW_PARAMETER_LEVEL_GLOBAL];
    // [adContext setParameter:FW_PARAMETER_CLICK_DETECTION withValue:@"NO" forLevel:FW_PARAMETER_LEVEL_OVERRIDE];
    
    [adContext addVideoPlayerNonTemporalSlot:[NSString stringWithFormat:@"%c%c%c", (char)(65 + (arc4random() % 25)), (char)(48 + (arc4random() % 9)), (char)(65 + (arc4random() % 25))] :nil :300 :50 :nil :YES :FW_SLOT_OPTION_INITIAL_AD_STAND_ALONE :nil :nil :nil];
    
    [adContext setProfile:currentProfile :nil :nil :nil];
    
    TiThreadPerformOnMainThread(^{
        [adContext setVideoDisplayBase:[currentBase view]];
    }, YES);
    
    [adContext setMoviePlayerController:currentPlayer];
    
    // TODO: latest version of admanager will not always work well with currentBase layout that is not fixed width/height
    
    DLog(@"[DEBUG] (FreeWheel Module) Created ad context and submitting request");
    
    [adContext submitRequest:6];
}

- (void)locationManager:(CLLocationManager *)manager didUpdateToLocation:(CLLocation *)newLocation fromLocation:(CLLocation *)oldLocation
{    
    [adManager setLocation:newLocation];
    
    DLog(@"[DEBUG] (FreeWheel Module) Set location");
}

- (void)locationManager:(CLLocationManager *)manager didFailWithError:(NSError *)error 
{
    DLog(@"[DEBUG] (FreeWheel Module) Location manager had an error: %@", error);
}

- (void)onAdRequestComplete:(NSNotification *)notification 
{    
    DLog(@"[DEBUG] (FreeWheel Module) Ad response recieved");
    DLog(@"[DEBUG] (FreeWheel Module) Attempting to setup player based on response");
    
    if ([notification object] != adContext || [[notification userInfo] objectForKey:@"error"]) {
        DLog(@"[DEBUG] (FreeWheel Module) There was an error with the ad response");
        
        if ([self _hasListeners:@"onadresponseerror"]) {
            [self fireEvent:@"onadresponseerror" withObject:[notification userInfo]];
        }
    } else {
        DLog(@"[DEBUG] (FreeWheel Module) Ad response looks good");
        DLog(@"[DEBUG] (FreeWheel Module) Setting MP controller and attempting to play ads");

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
        NSDictionary *adPositions = nil;
        
        for (id<FWSlot> temporalSlot in [adContext temporalSlots]) {
            DLog(@"Time Position: %f", [temporalSlot timePosition]);
            
            switch ([temporalSlot timePositionClass]) {
                case FW_TIME_POSITION_CLASS_PREROLL:
                    [prerollSlots addObject:[[[NSDictionary alloc] initWithObjectsAndKeys:
                                             [NSNumber numberWithDouble:[temporalSlot timePosition]], @"time", 
                                             nil] autorelease]];
                    DLog(@"[DEBUG] (FreeWheel Module) Time Position Class is PREROLL");
                    break;
                case FW_TIME_POSITION_CLASS_MIDROLL:
                    [midrollSlots addObject:[[[NSDictionary alloc] initWithObjectsAndKeys:
                                             [NSNumber numberWithDouble:[temporalSlot timePosition]], @"time", 
                                             nil] autorelease]];
                    DLog(@"[DEBUG] (FreeWheel Module) Time Position Class is MIDROLL");
                    break;
                case FW_TIME_POSITION_CLASS_POSTROLL:
                    [postrollSlots addObject:[[[NSDictionary alloc] initWithObjectsAndKeys:
                                              [NSNumber numberWithDouble:[temporalSlot timePosition]], @"time", 
                                              nil] autorelease]];
                    DLog(@"[DEBUG] (FreeWheel Module) Time Position Class is POSTROLL");
                    break;
                default:
                    break;
            }
        }
        
        if (processCompanion) {
            DLog(@"[DEBUG] (FreeWheel Module) Processing companion at ad response");

            DLog(@"[DEBUG] (FreeWheel Module) Looking for non-temporal video slots");
            for (id<FWSlot> nonTemporalVideoSlot in [adContext videoPlayerNonTemporalSlots]) {
                DLog(@"[DEBUG] (FreeWheel Module) Found non-temporal video slot");
                [[currentCompanionBase view] addSubview:[nonTemporalVideoSlot slotBase]];
                [nonTemporalVideoSlot play];
            }
            
            DLog(@"[DEBUG] (FreeWheel Module) Looking for non-temporal site section slots");
            for (id<FWSlot> nonTemporalSiteSectionSlot in [adContext siteSectionNonTemporalSlots]) {
                DLog(@"[DEBUG] (FreeWheel Module) Found non-temporal site section slot");
                [[currentCompanionBase view] addSubview:[nonTemporalSiteSectionSlot slotBase]];
                [nonTemporalSiteSectionSlot play];
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
        
        RELEASE_TO_NIL(prerollSlots);
        RELEASE_TO_NIL(midrollSlots);
        RELEASE_TO_NIL(postrollSlots);
        RELEASE_TO_NIL(adPositions);
    }
}

- (void)updateVideoDisplayBase:(id)arg
{
    ENSURE_UI_THREAD_0_ARGS;
    
    DLog(@"[DEBUG] (FreeWheel Module) Updating video display base.");
        
    // TODO: We might be able to move this to frameChanged so that this occurs automatically
    
    TiThreadPerformOnMainThread(^{
        [[currentBase view] sizeToFit];  
    }, YES);
        
    DLog(@"[DEBUG] (FreeWheel Module) Display Base: %@", [[currentBase view] description]); 
    DLog(@"[DEBUG] (FreeWheel Module) %@", [[UIWindow keyWindow] recursiveDescription]);
}

- (id)remainingAdTime
{
    return [NSNumber numberWithLongLong:[[adContext getSlotByCustomId:currentSlotID] totalDuration] - [[adContext getSlotByCustomId:currentSlotID] playheadTime]];
}

- (void)frameChanged:(NSNotification*)notification
{
    DLog(@"[DEBUG] (FreeWheel Module) FRAME CHANGED");
    
    for (UIView *subview in [[currentBase view] subviews])
    {
        DLog(@"[DEBUG] (FreeWheel Module) %@", subview);
    }
}

- (void)changeStatePaused:(id)args
{
    ENSURE_UI_THREAD_0_ARGS;
    
    [adContext setVideoState:FW_VIDEO_STATE_PAUSED];
}

- (void)changeStatePlaying:(id)args
{
    ENSURE_UI_THREAD_0_ARGS;
    
    [adContext setVideoState:FW_VIDEO_STATE_PLAYING];
}

- (void)changeStateCompleted:(id)args
{
    ENSURE_UI_THREAD_0_ARGS;
    
    [adContext setVideoState:FW_VIDEO_STATE_COMPLETED];
}

- (void)onAdOpen:(NSNotification*)notification
{    
    // This causes an selector exception. Disabling as it's not currently needed.
    // if ([self _hasListeners:@"onadopen"]) {
    //    [self fireEvent:@"onadopen" withObject:[notification userInfo]];
    // }
}

- (void)onContentVideoPauseRequest:(NSNotification*)notification
{
    DLog(@"[DEBUG] (FreeWheel Module) Pause request sent from AdManager");
    
    TiThreadPerformOnMainThread(^{
		[self fireEvent:@"onpauserequest"];
	}, YES);    
}

- (void)onContentVideoResumeRequest:(NSNotification*)notification
{
    DLog(@"[DEBUG] (FreeWheel Module) Resume request sent from AdManager");
    
    TiThreadPerformOnMainThread(^{
		[self fireEvent:@"onresumerequest"];
	}, YES);
}


- (void)onAdSlotStarted:(NSNotification *)notification
{    
    DLog(@"[DEBUG] (FreeWheel Module) Attempting to play slot");
    
    
    if ([[notification userInfo] objectForKey:@"error"]) {
        DLog(@"[ERROR] SLOT START FAILED: %@", [[notification userInfo] objectForKey:@"error"]);
    }

    NSMutableArray *ads = [[NSMutableArray alloc] init];
    
    currentSlotID = [[notification userInfo] objectForKey:FW_INFO_KEY_CUSTOM_ID];
        
    @try {
        for (id<FWAdInstance> instance in [[adContext getSlotByCustomId:[[notification userInfo] objectForKey:FW_INFO_KEY_CUSTOM_ID]] adInstances]) {
            [ads addObject:[[[NSDictionary alloc] initWithObjectsAndKeys:
                             [NSNumber numberWithLongLong:[instance creativeId]], @"creativeId",
                             [NSNumber numberWithLongLong:[[adContext getSlotByCustomId:[[notification userInfo] objectForKey:FW_INFO_KEY_CUSTOM_ID]] totalDuration]], @"duration",
                             nil] autorelease]];
                        
            if (processCompanion) {
                DLog(@"[DEBUG] (FreeWheel Module) Processing companion at slot");
                if ([[instance companionSlots] count] > 0) {
                    [[currentCompanionBase view] addSubview:[[adContext getSlotByCustomId:[[[instance companionSlots] objectAtIndex:0] customId]] slotBase]]; // add companion view
                } else {
                    DLog(@"[DEBUG] (FreeWheel Module) Did not find companion slots");
                }
            }
            
            DLog(@"[DEBUG] (FreeWheel Module) Ad Instance: %@", instance);
        }
    }
    @catch (NSException *exception) {
        DLog(@"[ERROR] Issue parsing slot data (most likely companion): %@", exception);
    }
            
    if ([self _hasListeners:@"onslotstarted"]) {
        [self fireEvent:@"onslotstarted" withObject:[[[NSDictionary alloc] initWithObjectsAndKeys:
                                                     ads, @"ads",
                                                     nil] autorelease]];   
    }
        
    DLog(@"[DEBUG] (FreeWheel Module) Companion Base Subview Count: %d", [[[currentCompanionBase view] subviews] count]); // just checking to see if the slotBase is being removed correctly

    DLog(@"[DEBUG] (FreeWheel Module) Display Base: %@", [[currentBase view] description]);
    DLog(@"[DEBUG] (FreeWheel Module) %@", [[UIWindow keyWindow] recursiveDescription]);
        
    RELEASE_TO_NIL(ads);
}

- (void)onAdSlotEnded:(NSNotification *)notification
{    
    NSString *type = [[NSString alloc] init];
    
    if ([[notification userInfo] objectForKey:@"error"]) {
        DLog(@"[ERROR] SLOT END FAILED: %@", [[notification userInfo] objectForKey:@"error"]);
    }
    
    switch ([[adContext getSlotByCustomId:[[notification userInfo] objectForKey:FW_INFO_KEY_CUSTOM_ID]] timePositionClass]) {
        case FW_TIME_POSITION_CLASS_PREROLL:
            type = @"preroll";
            break;
        case FW_TIME_POSITION_CLASS_MIDROLL:
            type = @"midroll";
            break;
        case FW_TIME_POSITION_CLASS_POSTROLL:
            type = @"postroll";
            break;
        default:
            type = @"unknown";
    }
    
    if ([self _hasListeners:@"onslotended"] && !contextDestroyed) {
        [self fireEvent:@"onslotended" withObject:[[[NSDictionary alloc] initWithObjectsAndKeys:
                                                   type, @"adType",
                                                   nil] autorelease]];
    }
    
    RELEASE_TO_NIL(type);
}

- (void)processClick:(id)args
{
    ENSURE_UI_THREAD_0_ARGS;
    
    DLog(@"[DEBUG] (FreeWheel Module) Clicked an ad.");    
    
    [[[[adContext getSlotByCustomId:currentSlotID] currentAdInstance] rendererController] processEvent:FW_EVENT_AD_CLICK info:nil];
}

- (void)destroyContext:(id)args
{
    ENSURE_UI_THREAD_0_ARGS;
    
    DLog(@"[DEBUG] (FreeWheel Module) Releasing ad context");
    
    if (adContext) {
        contextDestroyed = YES;
        
        [[NSNotificationCenter defaultCenter] removeObserver:self name:FW_NOTIFICATION_SLOT_STARTED object:adContext];
        [[NSNotificationCenter defaultCenter] removeObserver:self name:FW_NOTIFICATION_SLOT_ENDED object:adContext];
        [[NSNotificationCenter defaultCenter] removeObserver:self name:FW_NOTIFICATION_CONTENT_PAUSE_REQUEST object:nil];
        [[NSNotificationCenter defaultCenter] removeObserver:self name:FW_NOTIFICATION_CONTENT_RESUME_REQUEST object:nil];
        [[NSNotificationCenter defaultCenter] removeObserver:self name:FW_NOTIFICATION_IN_APP_VIEW_OPEN object:nil];
        [[NSNotificationCenter defaultCenter] removeObserver:self name:FW_NOTIFICATION_VIDEO_DISPLAY_BASE_FRAME_CHANGED object:nil];
        
        for (id<FWSlot> slot in [adContext siteSectionNonTemporalSlots]) {
            [slot stop];
            [[slot slotBase] removeFromSuperview];
        }
        
        for (id<FWSlot> slot in [adContext temporalSlots]) {
            [slot stop];
        }
        
        RELEASE_TO_NIL(adContext);
        RELEASE_TO_NIL(currentPlayer);

        currentVideoId = nil;
        currentSiteSection = nil;
        currentFallbackId = nil;
        currentProfile = nil;
        currentSlotID = nil;
        currentContentUrl = nil;
        currentBase = nil;
        currentCompanionBase = nil;
    }
}

- (void)pauseAds:(id)args
{
    ENSURE_UI_THREAD_0_ARGS;
    
    DLog(@"[DEBUG] (FreeWheel Module) Attempting to pause ads.");
    
    if ([[adContext getSlotByCustomId:currentSlotID] respondsToSelector:@selector(pause)]) {
        [[adContext getSlotByCustomId:currentSlotID] performSelector:@selector(pause)];
    }
    
    // Used in 4.4.0 and earlier.
    // [[adContext getSlotByCustomId:currentSlotID] stop];
    // [currentPlayer pause];    
}

- (void)resumeAds:(id)args
{
    ENSURE_UI_THREAD_0_ARGS;
    
    DLog(@"[DEBUG] (FreeWheel Module) Attempting to resume ads.");
    
    if ([[adContext getSlotByCustomId:currentSlotID] respondsToSelector:@selector(resume)]) {
        [[adContext getSlotByCustomId:currentSlotID] performSelector:@selector(resume)];
    }
}

- (void)playAds:(id)args
{
    ENSURE_SINGLE_ARG_OR_NIL(args,NSDictionary);
	ENSURE_UI_THREAD_1_ARG(args);
    
    DLog(@"[DEBUG] (FreeWheel Module) Playing ads");
        
    for (id<FWSlot> slot in [adContext temporalSlots]) {
        if ([slot timePosition] == [[args objectForKey:@"time"] longLongValue]) {
            [slot play];
            break;
        }        
    }
}

@end
