/**
 * This file was auto-generated by the Titanium Module SDK helper for Android
 * Appcelerator Titanium Mobile
 * Copyright (c) 2009-2010 by Appcelerator, Inc. All Rights Reserved.
 * Licensed under the terms of the Apache Public License
 * Please see the LICENSE included with this distribution for details.
 *
 */
package ti.freewheel;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Random;

import org.appcelerator.kroll.KrollDict;
import org.appcelerator.kroll.KrollProxy;
import org.appcelerator.kroll.annotations.Kroll;
import org.appcelerator.kroll.common.AsyncResult;
import org.appcelerator.kroll.common.TiMessenger;
import org.appcelerator.titanium.TiApplication;
import org.appcelerator.titanium.proxy.TiViewProxy;
import org.appcelerator.kroll.common.Log;
import org.appcelerator.kroll.common.TiConfig;
import org.appcelerator.titanium.view.TiCompositeLayout;

import ti.modules.titanium.media.VideoPlayerProxy;
import tv.freewheel.ad.factories.AdManagerLoaderFactory;
import tv.freewheel.ad.interfaces.IAdContext;
import tv.freewheel.ad.interfaces.IAdManager;
import tv.freewheel.ad.interfaces.IAdManagerLoader;
import tv.freewheel.ad.interfaces.IConstants;
import tv.freewheel.ad.interfaces.IEvent;
import tv.freewheel.ad.interfaces.IEventListener;
import tv.freewheel.ad.interfaces.ISlot;
import android.app.Activity;
import android.content.Context;
import android.location.Location;
import android.location.LocationManager;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.widget.FrameLayout;
import android.widget.TiVideoView8;

// This proxy can be created by calling Freewheel.createExample({message: "hello world"})
@Kroll.proxy(creatableInModule = FreewheelModule.class)
public class AdManagerProxy extends KrollProxy {

	// Standard Debugging variables
	private static final String LCAT = "FreewheelModule";
	private static final boolean DBG = TiConfig.LOGD;

	// TODO: should this really be hard coded like this?
	public static final String ADMANAGER_URL = "http://adm.fwmrm.net/p/android_tutorial/AdManager.fpk";

	private int networkId;
	private String serverUrl;

	private IAdManagerLoader adManagerLoader;
	protected IAdManager adManager;
	private IAdContext adContext;

	private String currentContentUrl;
	private int currentFallbackId;
	private TiViewProxy currentBase;
	private TiViewProxy currentCompanionBase;
	private VideoPlayerProxy currentPlayer;
	private String currentSiteSection;
	private String currentVideoId;
	private String currentProfile;

	// Constructor
	public AdManagerProxy() {
		super();
	}

	// Handle creation options
	@Override
	public void handleCreationDict(KrollDict options) {
		super.handleCreationDict(options);

		networkId = options.getInt("networkId");
		serverUrl = options.getString("serverUrl");

		adManagerLoader = AdManagerLoaderFactory.getInstance(getActivity());

		adManagerLoader.loadAdManager(ADMANAGER_URL, new Handler() {

			public void handleMessage(final Message msg) {
				Log.d(LCAT, "onAdManagerLoadedHanlder");
				boolean success = msg.getData().getBoolean("success");

				if (success) {
					onAdManagerLoaded();
				} else {
					Log.e(LCAT, "ad manager failed to load!");
				}
			}
		});
	}

	private HashMap[] loadSlot(int position) {
		ArrayList<ISlot> slots = adContext.getSlotsByTimePositionClass(position);
		HashMap[] retVal = new HashMap[slots.size()];
		for (int i = 0; i < slots.size(); i++) {
			ISlot slot = slots.get(i);
			retVal[i] = new HashMap();
			retVal[i].put("time", slot.getTimePosition());
			retVal[i].put("customId", slot.getCustomId());
		}
		return retVal;
	}

	private void onAdManagerLoaded() {
		adManager = adManagerLoader.newAdManager();

		if (adManager == null) {
			Log.e(LCAT, "ad manager failed to load!");
			return;
		}

		adManager.setLocation(getLocationInfo());
		adManager.setServer(serverUrl);
		adManager.setNetwork(networkId);
	}

	private Location getLocationInfo() {
		LocationManager locationManager = (LocationManager) getActivity().getSystemService(Context.LOCATION_SERVICE);
		List<String> allProviders = locationManager.getAllProviders();
		if (allProviders.size() == 0) {
			Location mock = new Location("network");
			mock.setLatitude(42);
			mock.setLongitude(-50);
			return mock;
		} else {
			return locationManager.getLastKnownLocation(allProviders.get(0));
		}
	}

	@Kroll.method
	public void setAdContext(HashMap args) {
		KrollDict argsDict = new KrollDict(args);
		currentContentUrl = argsDict.getString("contentUrl");
		currentFallbackId = argsDict.getInt("fallbackId");

		currentBase = (TiViewProxy) args.get("base");
		currentCompanionBase = (TiViewProxy) args.get("companionBase");

		currentPlayer = (VideoPlayerProxy) args.get("player");

		currentSiteSection = argsDict.getString("siteSection");
		currentVideoId = argsDict.getString("videoId");
		currentProfile = argsDict.getString("profile");

		if (DBG) {
			for (String key : argsDict.keySet()) {
				Log.d(LCAT, key + " is set on " + args.get(key));
			}
			Log.d(LCAT, "Set current player and created ad context");
		}

		if (!TiApplication.isUIThread()) {
			TiMessenger.sendBlockingMainMessage(handler.obtainMessage(MSG_CREATE_AD_CONTEXT));
		} else {
			handleCreateAdContext();
		}
	}

	@Kroll.method
	public void playAds(HashMap args) {
		KrollDict argsDict = new KrollDict(args);
		int time = argsDict.getInt("time");

		if (!TiApplication.isUIThread()) {
			TiMessenger.sendBlockingMainMessage(handler.obtainMessage(MSG_PLAY_ADS, time, 0));
		} else {
			handlePlayAds(time);
		}
	}

	private static final int MSG_CREATE_AD_CONTEXT = 50000;
	private static final int MSG_PLAY_ADS = 50001;

	private final Handler handler = new Handler(TiMessenger.getMainMessenger().getLooper(), new Handler.Callback() {
		public boolean handleMessage(Message msg) {
			switch (msg.what) {
				case MSG_CREATE_AD_CONTEXT: {
					AsyncResult result = (AsyncResult) msg.obj;
					handleCreateAdContext();
					result.setResult(null);
					return true;
				}
				case MSG_PLAY_ADS: {
					AsyncResult result = (AsyncResult) msg.obj;
					handlePlayAds(msg.arg1);
					result.setResult(null);
					return true;
				}
			}
			return false;
		}
	});

	private void handleCreateAdContext() {

		if (adManager == null) {
			Log.e(LCAT, "ad manager failed to load!");
			return;
		}

		adContext = adManager.newContext();
		Activity appCurrentActivity = TiApplication.getAppCurrentActivity();
		adContext.setActivity(appCurrentActivity);

		final IConstants adConstants = adContext.getConstants();

		// TODO: I have no idea what these next couple set and adds need to be...
		adContext.addKeyValue("contentUrl", currentContentUrl);

		adContext.setProfile(currentProfile, null, null, null);
		adContext.setSiteSection(currentSiteSection, 0, networkId, adConstants.ID_TYPE_CUSTOM(), 0);
		adContext.setVideoAsset(currentVideoId, 0, null, true, 0, networkId, adConstants.ID_TYPE_CUSTOM(), currentFallbackId);

		// TODO: the iOS module sets several "COUNTDOWN_TIMER_* parameters that are not exposed on Android
		adContext.setParameter(adConstants.PARAMETER_DISPLAY_AD_HTML_CONTENT_CLICK_PROCESSING(), false, adConstants.PARAMETER_LEVEL_OVERRIDE());

		adContext.addEventListener(adConstants.EVENT_REQUEST_COMPLETE(), new IEventListener() {
			public void run(final IEvent event) {
				HashMap evt = new HashMap();
				IConstants adConstants = adContext.getConstants();
				evt.put("prerolls", loadSlot(adConstants.TIME_POSITION_CLASS_PREROLL()));
				evt.put("midrolls", loadSlot(adConstants.TIME_POSITION_CLASS_MIDROLL()));
				evt.put("postrolls", loadSlot(adConstants.TIME_POSITION_CLASS_POSTROLL()));
				fireEvent("onadresponse", evt);
			}
		});

		adContext.addEventListener(adConstants.EVENT_SLOT_STARTED(), new IEventListener() {
			public void run(final IEvent event) {
				/*
				 * TODO: should contain these keys: [NSDictionary dictionaryWithObjectsAndKeys: ads, @"ads", nil];
				 */
				fireEvent("onslotstarted", new HashMap());
			}
		});
		adContext.addEventListener(adConstants.EVENT_SLOT_ENDED(), new IEventListener() {
			public void run(final IEvent event) {
				fireEvent("onslotended", new HashMap());
			}
		});

		adContext.addEventListener(adConstants.EVENT_REQUEST_CONTENT_VIDEO_PAUSE(), new IEventListener() {
			public void run(final IEvent event) {
				currentPlayer.pause();
				adContext.setVideoState(adConstants.VIDEO_STATE_PAUSED());
				// currentPlayer.hide();
			}
		});
		adContext.addEventListener(adConstants.EVENT_REQUEST_CONTENT_VIDEO_RESUME(), new IEventListener() {
			public void run(final IEvent event) {
				currentPlayer.play();
				adContext.setVideoState(adConstants.VIDEO_STATE_PLAYING());
			}
		});

		// TODO: iOS listens for the ad "opening", but we don't have a literal translation on Android: it's probably LOADED or STARTED.
		adContext.addEventListener(adConstants.EVENT_AD_LOADED(), new IEventListener() {
			public void run(final IEvent event) {
				fireEvent("onadopen", new HashMap());
			}
		});
		adContext.addEventListener(adConstants.EVENT_ERROR(), new IEventListener() {
			public void run(final IEvent event) {
				fireEvent("onadresponseerror", new HashMap());
			}
		});

		// request temporal slots here
		// adContext.setVideoPlayer(9001);
		// adContext.setVideoAsset("android_videoAds", 500, null, true, 0, 0, 0, 0);
		// adContext.setCapability(adConstants.CAPABILITY_SLOT_TEMPLATE(), adConstants.CAPABILITY_STATUS_OFF());

		adContext.addTemporalSlot("pre", adConstants.ADUNIT_PREROLL(), 0, null, 0, 0, null, null, 0);
		adContext.addTemporalSlot("mid", adConstants.ADUNIT_MIDROLL(), 10, null, 0, 0, null, null, 0);
		adContext.addTemporalSlot("post", adConstants.ADUNIT_POSTROLL(), 100, null, 0, 0, null, null, 0);

		// TODO: this probably works (for generating a random 3 character id). Not sure though...
		Random rnd = new Random();
		char[] rawCustomId = { Character.toChars(65 + rnd.nextInt(25))[0], Character.toChars(48 + rnd.nextInt(9))[0],
				Character.toChars(65 + rnd.nextInt(25))[0] };
		String customId = new String(rawCustomId);
		adContext.addVideoPlayerNonTemporalSlot(customId, null, 300, 50, null, true, null, null);

		// TODO: we need to pass in a SurfaceDisplay to "registerVideoDisplay", and it MUST be a child of a FrameLayout!
		TiViewProxy videoProxy = (TiViewProxy) currentPlayer;
		TiCompositeLayout layout = (TiCompositeLayout) videoProxy.peekView().getNativeView();
		TiVideoView8 videoView = null;
		for (int i = 0; i < layout.getChildCount(); i++) {
			View child = layout.getChildAt(i);
			if (child instanceof TiVideoView8) {
				videoView = (TiVideoView8) child;
				break;
			}
		}

		if (videoView != null) {
			layout.removeView(videoView);

			FrameLayout frameLayout = new FrameLayout(appCurrentActivity);
			frameLayout.addView(videoView);
			layout.addView(frameLayout);

			adContext.registerVideoDisplay(videoView);
		} else {
			Log.e(LCAT, "currentPlayer was not properly intialized!");
		}

		adContext.submitRequest(2);
	}

	@Kroll.method
	public void handlePlayAds(int time) {
		currentPlayer.pause();

		ArrayList<ISlot> temporalSlots = adContext.getTemporalSlots();
		for (int i = 0; i < temporalSlots.size(); i++) {
			ISlot slot = temporalSlots.get(i);
			// Note that we don't do exact equality, but check if it is close enough to the provided time.
			if (slot.getTimePosition() - time < 0.1) {
				slot.play();
				return;
			}
		}
	}
}