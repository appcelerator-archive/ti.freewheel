/**
 * \file FWProtocols.h
 * \brief Protocols in FreeWheel AdManager SDK
 */
@class UIView;
@class CLLocation;
@class MPMoviePlayerController;
@class UIViewController;

@protocol FWAdManager;
@protocol FWContext;
@protocol FWSlot;
@protocol FWAdInstance;
@protocol FWCreativeRendition;
@protocol FWCreativeRenditionAsset;
@protocol FWRendererController;
@protocol FWRenderer;

/** 
 *	\fn id<FWAdManager> newAdManager()
 *	Create a new AdManager instance
 *	\return	an id<FWAdManager>
 */
FW_EXTERN id<FWAdManager> newAdManager(void);

/** 
 *	\fn void FWSetLogLevel(FWLogLevel value); 
 *	Set log level
 *	\param value
 *		-	FW_LOG_LEVEL_INFO	Default value
 *		-	FW_LOG_LEVEL_QUIET
 *		-	FW_LOG_LEVEL_VERBOSE	force the verbose log in both debug & release version
 */
FW_EXTERN void FWSetLogLevel(FWLogLevel value);

/**
 *	\fn void FWSetUncaughtExceptionHandler(NSUncaughtExceptionHandler *handler)
 *	AdManager registers NSSetUncaughtExceptionHandler() to report uncaught exception.
 *	If app needs to perform last-minute logging before the program terminates, use this function instead of NSSetUncaughtExceptionHandler.
 *	\param handler
 */
FW_EXTERN void FWSetUncaughtExceptionHandler(NSUncaughtExceptionHandler *handler);

/** 
 *	\fn void FWClearCookie();
 *	Clear all cookies from fwmrm.net domains.
 */
FW_EXTERN void FWClearCookie(void);

/** 
 *	\fn void FWSetCookieOptOutState(BOOL value);
 *	Opt-out cookies from fwmrm.net domains.
 */
FW_EXTERN void FWSetCookieOptOutState(BOOL value);

/** 
 *	\fn BOOL FWGetCookieOptOutState();
 *	Get MRM cookie opt-out state.
 */
FW_EXTERN BOOL FWGetCookieOptOutState(void);

/**
 *	Protocol for AdManager
 *
 *	Use newAdManager() to create a new id<FWAdManager> instance
 */
@protocol FWAdManager <NSObject>
/**
 *	Set application's current view controller. REQUIRED. Application must be view controller based.
 *	This view controller set will be retained by FWAdManager. 
 *
 *	\param value    current view controller
 */
- (void)setCurrentViewController:(UIViewController *)value;

/**
 *	Set the ad server URL provided by FreeWheel. REQUIRED. Consult your FreeWheel sales engineer for the value.
 *	\param	value	url of the FreeWheel Ad Server
 */
- (void)setServerUrl:(NSString *)value;

/**
 *	Set the network ID of the distributor. REQUIRED. Consult your FreeWheel sales engineer for the value.
 *	\param	value	network ID of the distributor
 */
- (void)setNetworkId:(NSUInteger)value;

/**
 *	Set the current location of the device. This value will only be used for geo targeting purposes.
 *	\param	value	device's current location, CLLocation. nil by default.
 */
- (void)setLocation:(CLLocation *)value;

/**
 *	Get major version of AdManager
 *	\return	version of AdManager, e.g. 0x02060000 for v2.6
 */
- (NSUInteger)version;

/**
 *	Create a new FWContext instance
 *	A FWContext instance is used to set ad request information for a particular ad or ad set.  Multiple contexts can be created throughout the lifecycle of the FreeWheel AdManager and may exist siumultaneously without consequence. Multiple simultaneous contexts are useful to optimize user experience in the network-resource limited environment.
 *	\return	an id<FWContext>
 */
- (id<FWContext>)newContext;

/**
 *	Create a new context from the given context. The new context copies internal state and information of the old one, so you can use this method to create an identical context without having to set all the information again.
 *		
 *	The following methods are called automatically on the new context with values of the old one.
 *		-	-[FWContext setRequestMode:]
 *		-	-[FWContext setCapability:]
 *		-	-[FWContext setVisitor:]
 *		-	-[FWContext setVisitorHttpHeader:]
 *		-	-[FWContext setSiteSection:]
 *		-	-[FWContext setVideoAsset:]
 *		-	-[FWContext setProfile:]
 *		-	-[FWContext startSubsession:]
 *		-	-[FWContext setRequestDuration:]
 *		-	-[FWContext setVideoDisplayCompatibleSizes:]
 *		-	-[FWContext setVideoState:]
 *		-	-[FWContext setMoviePlayerFullscreen:]
 *
 *	You are required to call the following methods again on the new context:
 *		-	-[NSNotificationCeneter addObserver:selector:name:object:] 	(add notification observer for the new context)
 *      -   -[FWContext addTemporalSlot:] (if there's any)
 *      -   -[FWContext addSiteSectionNonTemporalSlot:] (if there's any)
 *      -   -[FWContext addVideoPlayerNonTemporalSlot:] (if there's any)
 *		-	-[FWContext submitRequest:]
 */
- (id<FWContext>)newContextWithContext:(id<FWContext>)context;

/**
 *	Disable FreeWheel crash reporter. By default AdManager will send a crash report to ad server when an app crash is detected.
 */
- (void)disableFWCrashReporter;
@end

/**
 *	Protocol for AdManager context
 */
@protocol FWContext <NSObject>
/**
 *	Set context level video display. Video ad are rendererd within the base view with the same frame.
 *	When video display base view changes, app needs to call this method again with the new video display to notify AdManager to render video ads in the updated video display. App does not need to call this method when video display view's frame changes.
 *	Prior to AdManager 3.8, renderer assumes main video's MPMoviePlayerController view is the video display base. Starting from AdManager 3.8, for iOS>=3.2, app must invoke this method to speicify video display base explicitly. For iOS3.0-3.1, app does not need to invoke this method since the legacy MPMoviePlayerController is always played in fullscreen.
 */
- (void)setVideoDisplayBase:(UIView *)value;

/**
 *	Set the main video's movie player fullscreen mode for rendering video ad.
 *	If application sets main video's MPMoviePlayerController fullscreen as YES or 
 *	present MPMoviePlayerViewController in a modal view, this value should be
 *	set to YES.
 *	
 *	\param	value fullscreen. NO by default.
 *
 *	Availability: iOS >= 3.2 
 */
- (void)setMoviePlayerFullscreen:(BOOL)value;

/**
 *	Deprecated!
 *	Please use setVideoDisplayBase: to set a UIView instance for non-fullscreen video player to display video ads, or use setMoviePlayerFullscreen:YES if content video is played in fullscreen mode.
 */
- (void)setMoviePlayerController:(MPMoviePlayerController *)value;

/**
 *	Set the capabilities supported by the player
 *	\param	capability capability name, should be one of FW_CAPABILITY_* in FWConstants.h
 *	\param	status indicates whether to enable this capability, should be one of:
 *	 	- FW_CAPABILITY_STATUS_ON: enable
 *	 	- FW_CAPABILITY_STATUS_OFF: disable
 *	 	- FW_CAPABILITY_STATUS_DEFAULT: leave it unset, follow the network settings
 *	\return Boolean value, indicating whether the capability is set successfully
 */
- (BOOL)setCapability:(NSString *)capability :(FWCapabilityStatus)status;

/**
 *	Add a key-value pair. The key-value pair is used in ad targeting. If called with the same key multiple times, all the values will be added to the same key.
 *	\param	key		key of the key-value pair, NSString. Can not be nil or empty
 *	\param	value	value of the key-value pair, NSString. Can not be nil
 */
- (void)addKeyValue:(NSString *)key :(NSString *)value;

/**
 *	Set the profiles names. Consult your FreeWheel sale engineer for available values.
 *	\param	playerProfile					name of the global profile
 *	\param	defaultTemporalSlotProfile		name of the temporal slot default profile, nil by default
 *	\param	defaultVideoPlayerSlotProfile	name of the video player slot default profile, nil by default
 *	\param	defaultSiteSectionSlotProfile	name of the site section slot default profile, nil by default
 */
- (void)setProfile:(NSString *)playerProfile :(NSString *)defaultTemporalSlotProfile :(NSString *)defaultVideoPlayerSlotProfile :(NSString *)defaultSiteSectionSlotProfile;

/**
 *	Set the attributes of the visitor
 *	\param	customId		custom ID of the visitor
 *	\param	ipV4Address		ip address of the visitor
 *	\param	bandwidth		bandwidth of the visitor
 *	\param	bandwidthSource	bandwidth source of the visitor
 */
- (void)setVisitor:(NSString *)customId :(NSString *)ipV4Address :(NSUInteger)bandwidth :(NSString *)bandwidthSource;

/**
 *	Set the HTTP headers of the visitor
 *	\param	name	name of the header
 *	\param	value	value of the header. If set to nil, the original value of the HTTP header name will be removed.
 */
- (void)setVisitorHttpHeader:(NSString *)name :(NSString *)value;

/**
 *	Set the attributes of the current video asset
 *	\param	videoAssetId	id of the video asset
 *	\param	duration		duration of the video in seconds
 *	\param	location		location(URI) of the video, nil by default
 *	\param	autoPlayType	whether the video starts playing automatically without user interaction
 *								-	FW_VIDEO_ASSET_AUTO_PLAY_TYPE_NONE
 *								-	FW_VIDEO_ASSET_AUTO_PLAY_TYPE_ATTENDED		(default)
 *								-	FW_VIDEO_ASSET_AUTO_PLAY_TYPE_UNATTENDED
 *	\param	videoPlayRandom	random number generated everytime a user watches the video asset
 *	\param	networkId		id of the network the video belongs to, 0 by default
 *	\param	idType			type of video id, should be one of
 *								-	FW_ID_TYPE_CUSTOM
 *								-	FW_ID_TYPE_FW
 *								-	FW_ID_TYPE_FWGROUP
 *	\param	fallbackId		video ID to fallback to. When ad server fails to find the video asset specified by videoAssetId, this ID will be used. 0 by default
 *	\param	durationType	type of duration, should be one of
 *								-	FW_VIDEO_ASSET_DURATION_TYPE_EXACT		default
 *								-	FW_VIDEO_ASSET_DURATION_TYPE_VARIABLE	for live video
 */
- (void)setVideoAsset:(NSString *)videoAssetId :(NSTimeInterval)duration :(NSString *)location :(FWVideoAssetAutoPlayType)autoPlayType :(NSUInteger)videoPlayRandom :(NSUInteger)networkId :(FWIdType)idType :(NSUInteger)fallbackId :(FWVideoAssetDurationType)durationType;

/**
 *	Set the current time position of the content asset.
 *	\param	timePosition	time position value in seconds.
 *	
 *	Notes:
 *			  If the stream is broken into multiple distinct files, this should be the time position within the asset as a whole.
 */
- (void)setVideoAssetCurrentTimePosition:(NSTimeInterval)timePosition;
/**
 *	Set the attributes of the site section
 *	\param	siteSectionId	id of the site section
 *	\param	pageViewRandom	random number generated everytime a user visits current site section
 *	\param	networkId		id of the network the site section belongs to, 0 by default
 *	\param	idType			type of the ID, should be one of
 *								-	FW_ID_TYPE_CUSTOM
 *								-	FW_ID_TYPE_FW
 *								-	FW_ID_TYPE_FWGROUP
 *	\param	fallbackId		site section ID to fallback to. When ad server fails to find the site section specified by siteSectionId, this ID will be used. 0 by default
 */
- (void)setSiteSection:(NSString *)siteSectionId :(NSUInteger)pageViewRandom :(NSUInteger)networkId :(FWIdType)idType :(NSUInteger)fallbackId;

/**
 *	Add candidate ads into the ad request
 *	\param	candidateAdId	id of the candidate ad
 */
- (void)addCandidateAd:(NSUInteger)candidateAdId;

/**
 *	Add a temporal slot
 *	\param	customId					custom ID of the slot. If slot with specified ID already exists, the function call will be ignored.
 *	\param	adUnit						ad unit supported by the slot
 *	\param	timePosition				time position of the slot
 *	\param	slotProfile					profile name of the slot, nil by default
 *	\param	cuePointSequence			slot cue point sequence
 *	\param	maxDuration					maximum duration of the slot allowed, 0 by default
 *	\param	acceptPrimaryContentType	accepted primary content types, comma separated values, use "," as delimiter, nil by default
 *	\param	acceptContentType			accepted content types, comma separated values, use "," as delimiter, nil by default
 *	\param	minDuration					minimum duration of the slot allowed, 0 by default
 */
- (void)addTemporalSlot:(NSString *)customId :(NSString *)adUnit :(NSTimeInterval)timePosition :(NSString *)slotProfile :(NSUInteger)cuePointSequence :(NSTimeInterval)maxDuration :(NSString *)acceptPrimaryContentType :(NSString *)acceptContentType :(NSTimeInterval)minDuration;

/**
 *	Add a video player non-temporal slot.
 *	\param	customId					custom ID of the slot. If slot with specified ID already exists, the function call will be ignored.
 *	\param	adUnit						ad unit supported by the slot
 *	\param	width						width of the slot
 *	\param	height						height of the slot
 *	\param	slotProfile					profile name of the slot, nil by default
 *	\param	acceptCompanion				whether companion ads are accepted
 *	\param	initialAdOption				choice of the initial ad in this slot, should be one of
 *		- FW_SLOT_OPTION_INITIAL_AD_STAND_ALONE: Display a new ad in this slot
 *		- FW_SLOT_OPTION_INITIAL_AD_KEEP_ORIGINAL: Keep the original ad in this slot
 *		- FW_SLOT_OPTION_INITIAL_AD_FIRST_COMPANION_ONLY: Ask ad server to fill this slot with the first companion ad, or keep the original ad if there is no companion ad available
 *		- FW_SLOT_OPTION_INITIAL_AD_FIRST_COMPANION_OR_STAND_ALONE: Ask ad server to fill this slot with the first companion ad, or display a new ad if there is no companion ad available
 *	\param	acceptPrimaryContentType	accepted primary content types, comma separated values, use "," as delimiter, nil by default
 *	\param	acceptContentType			accepted content types, comma separated values, use "," as delimiter, nil by default
 *	\param	compatibleDimensions        an array of compatible dimensions, The dimension must be a NSDictionary object with key @'width' and @'height', the value of should be a positive integer. Examples:
 *		-	NSArray *keys = [NSArray arrayWithObjects:@"width", @"height", nil];
 *		-	NSArray *dimension1 = [NSArray arrayWithObjects:[NSNumber numberWithInt:1980], [NSNumber numberWithInt:1080], nil];
 *		-	NSArray *dimension2 = [NSArray arrayWithObjects:[NSNumber numberWithInt:1280], [NSNumber numberWithInt:720], nil];
 *		-	NSArray *myDimensions = [NSArray arrayWithObjects:[NSDictionary dictionaryWithObjects:dimension1 forKeys:keys], [NSDictionary dictionaryWithObjects:dimension2 forKeys:keys], nil];
 */
- (void)addVideoPlayerNonTemporalSlot:(NSString *)customId :(NSString *)adUnit :(NSUInteger)width :(NSUInteger)height :(NSString *)slotProfile :(BOOL)acceptCompanion :(FWInitialAdOption)initialAdOption :(NSString *)acceptPrimaryContentType :(NSString *)acceptContentType :(NSArray *)compatibleDimensions;

/**
 *	Add a site section non-temporal slot
 *	\param	customId					custom ID of the slot. If slot with specified ID already exists, the function call will be ignored.
 *	\param	adUnit						ad unit supported by the slot
 *	\param	width						width of the slot
 *	\param	height						height of the slot
 *	\param	slotProfile					profile name of the slot, nil by default
 *	\param	acceptCompanion				whether companion ads are accepted
 *	\param	initialAdOption				choice of the initial ad in this slot, should be one of
 *		- FW_SLOT_OPTION_INITIAL_AD_STAND_ALONE: Display a new ad in this slot
 *		- FW_SLOT_OPTION_INITIAL_AD_KEEP_ORIGINAL: Keep the original ad in this slot
 *		- FW_SLOT_OPTION_INITIAL_AD_FIRST_COMPANION_ONLY: Ask ad server to fill this slot with the first companion ad, or keep the original ad if there is no companion ad available
 *		- FW_SLOT_OPTION_INITIAL_AD_FIRST_COMPANION_OR_STAND_ALONE: Ask ad server to fill this slot with the first companion ad, or display a new ad if there is no companion ad available
 *	\param	acceptPrimaryContentType	accepted primary content types, comma separated values, use "," as delimiter, nil by default
 *	\param	acceptContentType			accepted content types, comma separated values, use "," as delimiter, nil by default
 *	\param	compatibleDimensions        an array of compatible dimensions, The dimension must be a NSDictionary object with key @'width' and @'height', the value of should be a positive integer. Examples:
 *		-	NSArray *keys = [NSArray arrayWithObjects:@"width", @"height", nil];
 *		-	NSArray *dimension1 = [NSArray arrayWithObjects:[NSNumber numberWithInt:1980], [NSNumber numberWithInt:1080], nil];
 *		-	NSArray *dimension2 = [NSArray arrayWithObjects:[NSNumber numberWithInt:1280], [NSNumber numberWithInt:720], nil];
 *		-	NSArray *myDimensions = [NSArray arrayWithObjects:[NSDictionary dictionaryWithObjects:dimension1 forKeys:keys], [NSDictionary dictionaryWithObjects:dimension2 forKeys:keys], nil];
 */
- (void)addSiteSectionNonTemporalSlot:(NSString *)customId :(NSString *)adUnit :(NSUInteger)width :(NSUInteger)height :(NSString *)slotProfile :(BOOL)acceptCompanion :(FWInitialAdOption)initialAdOption :(NSString *)acceptPrimaryContentType :(NSString *)acceptContentType :(NSArray *)compatibleDimensions;

/**
 *	Set the current state of the video
 *	\param	videoState	video state, should be one of:
 *		-	FW_VIDEO_STATE_PLAYING
 *		-	FW_VIDEO_STATE_PAUSED
 *		-	FW_VIDEO_STATE_STOPPED
 *		-	FW_VIDEO_STATE_COMPLETED
 */
- (void)setVideoState:(FWVideoState)videoState;

/**
 *	Get all temporal slots
 *	\return An NSArray of id<FWSlot> objects
 */
- (NSArray * /* id<FWSlot> */)temporalSlots;

/**
 *	Get all video player non-temporal slots
 *	\return An NSArray of id<FWSlot> objects
 */
- (NSArray * /* id<FWSlot> */)videoPlayerNonTemporalSlots;  

/**
 *	Get all site section non-temporal slots
 *	\return An NSArray of id<FWSlot> objects
 */
- (NSArray * /* id<FWSlot> */)siteSectionNonTemporalSlots;  

/**
 *	Get all slots in specified time position class
 *	\param	timePositionClass	time position class, should be one of:
 *   - FW_TIME_POSITION_CLASS_PREROLL
 *   - FW_TIME_POSITION_CLASS_MIDROLL
 *   - FW_TIME_POSITION_CLASS_POSTROLL
 *   - FW_TIME_POSITION_CLASS_OVERLAY
 *   - FW_TIME_POSITION_CLASS_DISPLAY
 *   - FW_TIME_POSITION_CLASS_PAUSE_MIDROLL
 *	\return An Array of id<FWSlot> objects
 */
- (NSArray * /* id<FWSlot> */)getSlotsByTimePositionClass:(FWTimePositionClass)timePositionClass;

/**
 *	Get a slot by its custom ID
 *	\param	customId	custom ID of the slot
 *	\return An id<FWSlot> object, or nil if not found
 */
- (id<FWSlot>)getSlotByCustomId:(NSString *)customId;

/**
 *	Set a nofication center which will receive notifiations published from AdManager.
 *	Deprecated
 */
- (void)setNotificationCenter:(NSNotificationCenter *)nc;

/**
 *	Submit the request to FreeWheel Ad Server
 *	\param	timeoutInterval ad request timeout value in seconds. 3 by default.
 */
- (void)submitRequest:(NSTimeInterval)timeoutInterval;

- (void)addRendererClass:(NSString *)className forContentType:(NSString *)contentType creativeAPI:(NSString *)creativeAPI slotType:(NSString *)slotType baseUnit:(NSString *)baseAdUnit adUnit:(NSString *)soldAsAdUnit withParameters:(NSDictionary *)parameters;

/**
 *	Set a parameter on a specified level. 
 *	
 *	\param	name    parameter name
 *	\param	value	parameter value
 *	\param	level	level of the parameter, must be one of:
 *					-	FW_PARAMETER_LEVEL_GLOBAL
 *					-	FW_PARAMETER_LEVEL_OVERRIDE					
 */
- (void)setParameter:(NSString *)name withValue:(id)value forLevel:(FWParameterLevel)level;

/**
 *	Retrieve a parameter
 *  \param  name  The name of the parameter
 */
- (id)getParameter:(NSString *)name;

/**
 *	Set a list of acceptable alternative dimensions
 *	
 *	\param	compatibleDimensions	an array of compatible dimensions, The dimension must be a NSDictionary object with key @'width' and @'height', the value of should be a positive integer. Examples:
 *					-	NSArray *keys = [NSArray arrayWithObjects:@"width", @"height", nil];
 *					-	NSArray *dimension1 = [NSArray arrayWithObjects:[NSNumber numberWithInt:1980], [NSNumber numberWithInt:1080], nil];
 *					-	NSArray *dimension2 = [NSArray arrayWithObjects:[NSNumber numberWithInt:1280], [NSNumber numberWithInt:720], nil];
 *					-	NSArray *myDimensions = [NSArray arrayWithObjects:[NSDictionary dictionaryWithObjects:dimension1 forKeys:keys], [NSDictionary dictionaryWithObjects:dimension2 forKeys:keys], nil];
 *					-	[context setVideoDisplayCompatibleSizes:myDimensions];
 */
- (void)setVideoDisplayCompatibleSizes:(NSArray *)compatibleDimensions;

/**
 *	Set request mode of AdManager.
 *	\param	mode	request mode, must be one of:
 *					- FW_REQUEST_MODE_ON_DEMAND (default)
 *					- FW_REQUEST_MODE_LIVE
 *			
 *	Notes:
 *		AdManager runs in On-Demand mode(_FW_REQUEST_MODE_ON_DEMAND) by default. If your video asset is a live stream, invoke this method to set the player to live mode. This method should be called (if needed) right after a new FWContext instance is created.
 *	
 */
- (void)setRequestMode:(FWRequestMode)mode;

/**
 *	Set the duration for which the player is requesting ads. Optional.
 *	\param	requestDuration		requesting duration value, in seconds.
 */
- (void)setRequestDuration:(NSTimeInterval)requestDuration;

/**
 *	Reset the exclusivity scope
 */
- (void)resetExclusivity;

/**
 *	Start a subsession in live mode. Subsequent requests will be in the same subsession, until this method is called again with a different token.
 *	\param	subsessionToken		a token to identify the subsession, should be unique across different subsessions. Use a differnt token to start a new subsession.
 *	    
 *	Note:
 *		Calling this method multiple times with the same token will have no effect.
 *		Subsession only works when FW_CAPABILITY_SYNC_MULTI_REQUESTS is on, calling this method will turn on this capability.
 */
- (void)startSubsession:(NSUInteger)subsessionToken;

/**
 *	Get video asset's location URL
 *
 *	\return video asset's URL, nil if not set
 */
- (NSString *)getVideoLocation;

/**
 *  Get the AdManager instance for this context
 *
 *  \return A id<FWAdManager> instance
 */
- (id<FWAdManager>)getAdManager;

/**
 *	Return the object [NSNotificationCenter defaultCenter]
 */
- (NSNotificationCenter *)notificationCenter;

/**
 *	Notify AdManager about user actions
 *	\param	userAction		user actions, must be one of:
 *							- FW_USER_ACTION_PAUSE_BUTTON_CLICKED
 *							- FW_USER_ACTION_RESUME_BUTTON_CLICKED
 */
- (void)notifyUserAction:(FWUserAction)userAction;
@end


/**
 *	Protocol for slot 
 */
@protocol FWSlot <NSObject>

/**
 *	Get the slot's custom ID
 *	\return Custom ID of the slot
 */
- (NSString *)customId;  

/**
 *	Get the slot's type
 *	\return Type of the slot, the value can be one of:
 *		-	FW_SLOT_TYPE_TEMPORAL
 *		-	FW_SLOT_TYPE_VIDEOPLAYER_NONTEMPORAL
 *		-	FW_SLOT_TYPE_SITESECTION_NONTEMPORAL
 */
- (FWSlotType)type;				

/**
 *	Get the slot's time position
 *	\return Time position of the slot
 */
- (NSTimeInterval)timePosition;     

/**
 *	Get slot's time position class
 *	\return Time position class of the slot, the value can be one of:
 *   - FW_TIME_POSITION_CLASS_PREROLL
 *   - FW_TIME_POSITION_CLASS_MIDROLL
 *   - FW_TIME_POSITION_CLASS_POSTROLL
 *   - FW_TIME_POSITION_CLASS_OVERLAY
 *   - FW_TIME_POSITION_CLASS_DISPLAY
 *   - FW_TIME_POSITION_CLASS_PAUSE_MIDROLL
 */
- (FWTimePositionClass)timePositionClass;	   

/**
 *	Get the ad instances in the slot
 *	\return An array of id<FWAdInstance>
 */
- (NSArray * /* <id>FWAdInstance */)adInstances;

/**
 *	Get the width of the slot in pixels as returned in ad response.
 *	\return Width in pixels
 */
- (NSInteger)width;

/**
 *	Get the height of the slot in pixels as returned in ad response.
 *	\return Height in pixels
 */
- (NSInteger)height;

/**
 *	Process slot event
 *	\param	eventName Event to be processed, one of FW_EVENT_SLOT_* in FWConstants.h
 */
- (void)processEvent:(NSString *)eventName;

/**
 *	Play the slot.
 *
 *	Note: If your app uses MPMoviePlayerController for content video playback, you will need to release the player when a midroll video slot is about to start, and re-create a new MPMoviePlayerController after the midroll slot ends, starting from where it was left off. This is due to the limitation that MPMoviePlayerController only supports one active stream at a time.
 *	AVPlayer does not have this limitation.
 */
- (void)play;

/**
 *	Stop the slot
 */
- (void)stop;

/**
 *	Get slot's visibility. Only applicable to non-temporal slots. YES by default.
 */
- (BOOL)visible;

/**
 *	Set the visibility for a nontemporal slot. 
 *
 *	If a nontemporal slot view should not be visible, call setVisible:NO before the slot starts. In this case there will be no impression sent to FreeWheel ad server, even if [slot play] has already been called. 
 *  If a nontemporal slot has started([slot play] has been called) when it is invisible, calling setVisible:YES will display the slot and send an impression.
 *
 *	Note:
 *		This method has no effect on temporal slots.
 *      This method has no effect anymore if an impression has been sent.
 *
 *  \param  value	YES or NO
 */
- (void)setVisible:(BOOL)value;

/**
 *	Get slot base UIView object.
 *	For nontemporal slots, the returned UIView should be added to a parent UIView that is already in your apps view hierarchy.
 *	For temporal slot, return value is the object set by -[FWContext setVideoDisplayBase:].
 */
- (UIView *)slotBase;

/**
 *	Set a parameter on the slot level
 *	\param	name	name of the parameter
 *	\param	value	value of the parameter
 */
- (void)setParameter:(NSString *)name withValue:(id)value;

/**
 *	Get the value of a parameter by its name
 *  \param  name    Parameter name
 *  \return value of the parameter
 */
- (id)getParameter:(NSString *)name;

/** 
 *  Get the slot's duration
 *  \return the duration in seconds, greater than or equal to 0 
 */  
- (NSTimeInterval)totalDuration;

/** 
 *  Get the slot's playhead
 *  \return the playhead time in seconds, greater than or equal to 0
 */  
- (NSTimeInterval)playheadTime;

/**
 *	Get the currently playing ad instance
 *	\return the AdInstance object of the currently playing ad in this slot. Return nil if no ad instance is currently playing.
 */
- (id<FWAdInstance>)currentAdInstance;
@end


/**
 *	Protocol for ad instance
 */
@protocol FWAdInstance <NSObject>

/**
 *	Get the ad ID of the ad instance. This value can also be found in the advertising module of the FreeWheel MRM UI.
 *	\return ID of the ad instance
 */
- (NSUInteger)adId;

/**
 *	Get the creative ID of the ad instance
 *	This is the creative ID associated with this ad. The value can also be found in the advertising module of the FreeWheel MRM UI.
 *	\return Creative ID as an unsigned int
 */
- (NSUInteger)creativeId;

/**
 *	Get the primary rendition of the ad instance
 *	\return An id<FWCreativeRendition>
 */
- (id<FWCreativeRendition>)primaryCreativeRendition;

/**
 *	Get the callback urls for the specified event
 *
 *	\param	eventName	name of the event, FW_EVENT_AD_*
 *	\param	eventType	type of the event, FW_EVENT_TYPE_*
 *	Valid eventName/evetType pairs:
 *		- (FW_EVENT_AD_IMPRESSION,        FW_EVENT_TYPE_IMPRESSION)	-	ad impression
 *		- (FW_EVENT_AD_FIRST_QUARTILE,	  FW_EVENT_TYPE_IMPRESSION) -	firstQuartile
 *		- (FW_EVENT_AD_MIDPOINT,		  FW_EVENT_TYPE_IMPRESSION) -	midPoint
 *		- (FW_EVENT_AD_THIRD_QUARTILE,	  FW_EVENT_TYPE_IMPRESSION) -	thirdQuartile
 *		- (FW_EVENT_AD_COMPLETE,          FW_EVENT_TYPE_IMPRESSION)	-	complete
 *		- (FW_EVENT_AD_CLICK,             FW_EVENT_TYPE_CLICK)		-	click through
 *		- (FW_EVENT_AD_CLICK,             FW_EVENT_TYPE_CLICKTRACKING)	-	click tracking
 *		- ("custom_click_name",           FW_EVENT_TYPE_CLICK)			-	custom click
 *		- ("custom_click_tracking_name",  FW_EVENT_TYPE_CLICKTRACKING)	-	custom click tracking
 *		- (FW_EVENT_AD_PAUSE,             FW_EVENT_TYPE_STANDARD)		-	IAB metric, pause
 *		- (FW_EVENT_AD_RESUME,            FW_EVENT_TYPE_STANDARD)		-	IAB metric, resume
 *		- (FW_EVENT_AD_REWIND,            FW_EVENT_TYPE_STANDARD)		-	IAB metric, rewind
 *		- (FW_EVENT_AD_MUTE,              FW_EVENT_TYPE_STANDARD)		-	IAB metric, mute
 *		- (FW_EVENT_AD_UNMUTE,            FW_EVENT_TYPE_STANDARD)		-	IAB metric, unmute
 *		- (FW_EVENT_AD_COLLAPSE,          FW_EVENT_TYPE_STANDARD)		-	IAB metric, collapse
 *		- (FW_EVENT_AD_EXPAND,            FW_EVENT_TYPE_STANDARD)		-	IAB metric, expand
 *		- (FW_EVENT_AD_MINIMIZE,          FW_EVENT_TYPE_STANDARD)		-	IAB metric, minimize
 *		- (FW_EVENT_AD_CLOSE,             FW_EVENT_TYPE_STANDARD)		-	IAB metric, close
 *		- (FW_EVENT_AD_ACCEPT_INVITATION, FW_EVENT_TYPE_STANDARD)		-	IAB metric, accept invitation
 *	
 *	\return: Array of url strings
 *	
 */
- (NSArray *)getEventCallbackUrls:(NSString *)eventName :(NSString *)eventType;

/**
 *	Set callback urls for the specified event
 *
 *	\param	eventName	name of event, FW_EVENT_AD_*
 *	\param	eventType	type of event, FW_EVENT_TYPE_*
 *	\param	urls		external urls to ping 
 *	Valid eventName/evetType pairs:
 *		- (FW_EVENT_AD_IMPRESSION,        FW_EVENT_TYPE_IMPRESSION)	-	ad impression
 *		- (FW_EVENT_AD_FIRST_QUARTILE,    FW_EVENT_TYPE_IMPRESSION)	-	1st quartile
 *		- (FW_EVENT_AD_MIDPOINT,          FW_EVENT_TYPE_IMPRESSION)	-	midpoint
 *		- (FW_EVENT_AD_THIRD_QUARTILE,    FW_EVENT_TYPE_IMPRESSION)	-	3rd quartile
 *		- (FW_EVENT_AD_COMPLETE,          FW_EVENT_TYPE_IMPRESSION)	-	complete
 *		- (FW_EVENT_AD_CLICK,             FW_EVENT_TYPE_CLICK)		-	click through
 *		- (FW_EVENT_AD_CLICK,             FW_EVENT_TYPE_CLICK_TRACKING)	-	click tracking
 *		- ("custom_click_name",           FW_EVENT_TYPE_CLICK)			-	custom click through
 *		- ("custom_click_name",           FW_EVENT_TYPE_CLICK_TRACKING)	-	custom click tracking
 *		- (FW_EVENT_AD_PAUSE,             FW_EVENT_TYPE_STANDARD)		-	IAB metric, pause
 *		- (FW_EVENT_AD_RESUME,            FW_EVENT_TYPE_STANDARD)		-	IAB metric, resume
 *		- (FW_EVENT_AD_REWIND,            FW_EVENT_TYPE_STANDARD)		-	IAB metric, rewind
 *		- (FW_EVENT_AD_MUTE,              FW_EVENT_TYPE_STANDARD)		-	IAB metric, mute
 *		- (FW_EVENT_AD_UNMUTE,            FW_EVENT_TYPE_STANDARD)		-	IAB metric, unmute
 *		- (FW_EVENT_AD_COLLAPSE,          FW_EVENT_TYPE_STANDARD)		-	IAB metric, collapse
 *		- (FW_EVENT_AD_EXPAND,            FW_EVENT_TYPE_STANDARD)		-	IAB metric, expand
 *		- (FW_EVENT_AD_MINIMIZE,          FW_EVENT_TYPE_STANDARD)		-	IAB metric, minimize
 *		- (FW_EVENT_AD_CLOSE,             FW_EVENT_TYPE_STANDARD)		-	IAB metric, close
 *		- (FW_EVENT_AD_ACCEPT_INVITATION, FW_EVENT_TYPE_STANDARD)		-	IAB metric, accept invitation
 */
- (void)setEventCallbackUrls:(NSString *)eventName :(NSString *)eventType :(NSArray *)urls;

/**
 *	Add a creative rendition to the ad instance 
 *	\return the FWCreativeRendition object added to the ad instance
 */
- (id<FWCreativeRendition>)addCreativeRendition;

/**
 *	Get the renderer controller of the ad instance
 *	\return An id<FWRendererController>
 */
- (id<FWRendererController>)rendererController;


/**
 *	Get the companion slots of the ad instance
 * \return an array of id<FWSlot>
 */
- (NSArray *)companionSlots;

/**
 * Get all creative renditions of the ad instance
 */
- (NSArray* /*id<FWCreativeRendition>*/)creativeRenditions;

/**
 * Set the primary creative rendition
 * \param  primaryCreativeRendition     a pointer to the primary creative rendition
 */
- (void)setPrimaryCreativeRendition:(id<FWCreativeRendition>)primaryCreativeRendition;

/**
 *	Get the slot that contains this ad instance
 */
- (id<FWSlot>)slot;
@end


/**
 *	Protocol for creative rendition
 */
@protocol FWCreativeRendition <NSObject>

/**
 *	Get content type of the rendition
 *	\return  Content type in a string
 */
- (NSString *)contentType;

/**
 *	Set content type of the rendition
 */
- (void)setContentType:(NSString *)value;

/**
 *	Get wrapper type of the rendition
 *	\return  Wrapper type in a string
 */
- (NSString *)wrapperType;

/**
 *	Set wrapper type of the rendition
 */
- (void)setWrapperType:(NSString *)value;

/**
 *	Get wrapper url of the rendition
 *	\return  Wrapper url in a string
 */
- (NSString *)wrapperUrl;

/**
 *	Set wrapper url of the rendition
 */
- (void)setWrapperUrl:(NSString *)value;

/**
 *	Get creativeAPI of the rendition
 *	\return  creativeAPI in a string
 */
- (NSString *)creativeAPI;

/**
 *	Set creativeAPI of the rendition
 */
- (void)setCreativeAPI:(NSString *)value;

/**
 *	Get base unit of the rendition
 *	\return Base unit in a string
 */
- (NSString *)baseUnit;

/**
 *	Get preference of the rendition
 *	\return A number, the higher is preferred among all renditions in the creative
 */
- (int)preference;

/**
 *	Set preference of the rendition
 */
- (void)setPreference:(int)value;

/**
 *	Get width of the rendition
 *	\return Width in pixels
 */
- (NSUInteger)width;

/**
 *	Set width of the rendition
 */
- (void)setWidth:(NSUInteger)value;

/**
 *	Get height of the rendition
 *	\return Height in pixels
 */
- (NSUInteger)height;

/**
 *	Set height of the rendition
 */
- (void)setHeight:(NSUInteger)value;

/**
 *	Get duration of the rendition
 *	\return Duration in seconds
 */
- (NSTimeInterval)duration;

/**
 *	Set duration of the rendition
 *	\return Duration in seconds
 */
- (void)setDuration:(NSTimeInterval)value;

/**
 *	Set parameter of the rendition
 */
- (void)setParameter:(NSString *)name :(NSString *)value;

/**
 *	Get primary asset of the rendition
 *	\return An id<FWCreativeRenditionAsset>
 */
- (id<FWCreativeRenditionAsset>)primaryCreativeRenditionAsset;

/**
 *	Get all non-primary assets of the rendition
 *	\return An array of id<FWCreativeRenditionAsset>
 */
- (NSArray * /* <id>FWCreativeRenditionAsset */)otherCreativeRenditionAssets;

/**
 *	Add an asset to the rendition
 */
- (id<FWCreativeRenditionAsset>)addCreativeRenditionAsset;
@end


/**
 *	Protocol for creative rendition asset
 */
@protocol FWCreativeRenditionAsset <NSObject>
/**
 *	Get name of the asset
 *	\return Name in a string
 */
- (NSString *)name;

/**
 *	Set name of the asset
 */
- (void)setName:(NSString *)value;

/**
 *	Get URL of the asset
 *	\return URL in a string
 */
- (NSString *)url;

/**
 *	Set URL of the asset
 */
- (void)setUrl:(NSString *)value;

/**
 *	Get content of the asset
 *	\return Content in a string
 */
- (NSString *)content;

/**
 *	Set the content of the asset
 */
- (void)setContent:(NSString *)value;

/**
 *	Get mime type of the asset
 *	\return Mime type in a string
 */
- (NSString *)mimeType;

/**
 *	Set mime type of the asset
 */
- (void)setMimeType:(NSString *)value;

/**
 *	Get content type of the asset
 *	\return Content type in a string
 */
- (NSString *)contentType;

/**
 *	Set content type of the asset
 */
- (void)setContentType:(NSString *)value;

/**
 *	Get size of the asset
 *	\return Size in bytes, or -1 if unknown
 */
- (NSInteger)bytes;

/**
 *	Set size of the asset
 *	\return Size in bytes, or -1 if unknown
 */
- (void)setBytes:(NSInteger)value;
@end


/**
 *	Protocol for renderer controller
 *	The FWRendererController class provides methods for reporting metric events and changing renderer states.
 */
@protocol FWRendererController <NSObject>

/**
 *  Return the current location.
 *  See also: -[FWAdManager setLocation:]
 *  \return current location
 */
- (CLLocation *)location;

/**
 *	Return application's current view controller. 
 *  See also: -[FWAdManager setCurrentViewController:].
 *  \return current view controller
 */
- (UIViewController *)currentViewController;

/**
 *	deprecated, use [slot slotBase] to determine where to display the ad.
 */
- (MPMoviePlayerController *)moviePlayerController;

/**
 *	Return the video player's fullscreen state.
 *  See also: -[FWContext setMoviePlayerFullscreen:]
 *
 *	Availability: iOS >= 3.2 
 *  \return fullscreen state
 */
- (BOOL)moviePlayerFullscreen;

/**
 *	Process renderer event
 *	\param	eventName Event to be processed, one of FW_EVENT_AD_* in FWConstants.h
 *  \param  details  Additional information. Available keys:
 * 					- FW_INFO_KEY_CUSTOM_EVENT_NAME Optional. 
 * 					Name of the custom event.
 * 					- FW_INFO_KEY_SHOW_BROWSER Optional. 
 * 					Force opening/not openning the event callback url in new app.
 * 					If this key is not provided, AdManager will use the setting booked in MRM UI (recommanded).
 * 					- FW_INFO_KEY_URL Optional. 
 * 					URL to open or used as redirect url on FW_EVENT_AD_CLICK.
 * 					If this key is not provided, the URL booked in MRM UI will be used.
 */
- (void)processEvent:(NSString *)eventName info:(NSDictionary *)details;

/**
 *	Declare a capability of the renderer
 *	\param	eventCapability One of FW_EVENT_AD_* (NOT including FW_EVENT_AD_FIRST_QUARTILE, FW_EVENT_AD_THIRD_QUARTILE, FW_EVENT_AD_IMPRESSION) in FWConstants.h
 *	\param	status ON if renderer has specified capability or is able to send specified event
 *
 *	Note: \n
 *	Changing renderer capability after renderer starts playing may result in undefined behaviour
 */
- (void)setCapability:(NSString *)eventCapability :(FWCapabilityStatus)status;

/**
 *	Return the Major version of AdManager, e.g. 0x02060000 for v2.6
 *  \return version as NSUInteger
 */
- (NSUInteger)version;

/**
 *	Retrieve a parameter
 *  \param  name  Parameter name
 */
- (id)getParameter:(NSString *)name;

/**
 * Get rendering ad instance
 */
- (id<FWAdInstance>)adInstance;

/**
 * Transit renderer state
 * \param  state	destination transition state attempted, available values:
 * 					- FW_RENDERER_STATE_STARTED
 * 					- FW_RENDERER_STATE_COMPLETED
 * 					- FW_RENDERER_STATE_FAILED
 * \param  details	detail info
 * 					- For FW_RENDERER_STATE_FAILED:FW_INFO_KEY_ERROR_CODE are required. FW_INFO_KEY_ERROR_INFO is optional.
 */
- (void)handleStateTransition:(FWRendererStateType)state info:(NSDictionary *)details;

/**
 *	Return an id<FWContext> instance for posting and observing notifications
 *
 *	To post a notification: \n
 *	[[[rendererController notificationContext] notificationCenter] postNotificationName:EVENT_NAME object:[_rendererController notificationContext] userInfo:nil]; \n
 *	To observe a notification: \n
 *	[[[rendererController notificationContext] notificationCenter] addObserver:self selector:@selector(handler) name:EVENT_NAME object:nil];
 */
- (id<FWContext>)notificationContext;

/**
 *	Schedule ad instances for given slots.
 */
- (NSArray * /* id<FWAdInstance> */)scheduleAdInstances:(NSArray * /* id<FWSlot> */)slots;

/**
 *	Request main video to pause or resume, the notification sender is the current FWContext object
 */
- (void)requestContentStateChange:(BOOL)pause;

/**
 *  Renderer should use this API to trace all logs
 */
- (void)log:(NSString *)msg;
@end


/**
 *	Protocol for FWRenderer
 */
@protocol FWRenderer <NSObject>
/**
 *	Initialize the renderer with a renderer controller. 
 *	
 *	\param	rendererController	reference to id<FWRendererController>
 *		
 *	Note:
 *		Typically the renderer declares all available capabilities and events when this method is called.
 */
- (id)initWithRendererController:(id<FWRendererController>)rendererController;

/**
 *	Start ad playback.
 *	
 *	Note:
 *		The renderer should start the ad playback when this method is called,
 *		and transit to FW_RENDERER_STATE_STARTED state as soon as the ad has started.
 *		
 *		When the ad stops (either interrupted or reached the end), the renderer should
 *		transit to FW_RENDERER_STATE_COMPLETED state.
 */
- (void)start;

/**
 *	Stop ad playback.
 *
 *	Note:
 *		Typically the renderer will dispose playing images/videos from screen when receive this notificaiton, and 
 *      transit to FW_RENDERER_STATE_COMPLETED state as soon as the ad is stopped.
 */
- (void)stop;

/**
 *	Get module info. The returned dictionary should contain key FW_INFO_KEY_MODULE_TYPE with FW_MODULE_TYPE_* value,
 *  and should contain key FW_INFO_KEY_REQUIRED_API_VERSION with the FreeWheel RDK version when the component is compiled.
 */
- (NSDictionary *)moduleInfo;


/**
 *	Get duration of the ad
 *  \return a positive number in seconds as NSTimeInterval, or -1 if the duration is N/A
 */
- (NSTimeInterval)duration;

/** 
 *	Get playheadTime of the ad
 * 	\return a positive number in seconds as NSTimeInterval, or -1 if the playhead time is N/A
 */  
- (NSTimeInterval)playheadTime;

@optional

/**
 *	Pause ad playback.
 */
- (void)pause;

/**
 *	Resume ad playback.
 */
- (void)resume;

@end

