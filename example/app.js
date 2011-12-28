var freewheel = require('ti.freewheel');

var token = {
	base: 'https://secure.services.nbc.nbcuni.com/mobile/portalFeed/portalGetVideoURLV0130/.json?videoId=',
	bitrate: '&bitRate=496'
};

var msg = {
	prep: 'Current Status: ',
	wait: 'Waiting...',
	invalid: 'Invalid Video ID',
	submit: 'Submitting Video Request',
	error: 'Video Request Error',
	set: 'Setting Video Player URL',
	context: 'Ad Context Created - Check Logs',
	slot: {
		started: 'Slot Started - Check Logs',
		ended: 'Slot Ended - Check Logs'
	},
	content: 'Content Playing'
};

var win = Ti.UI.createWindow({ backgroundColor:'#fff' });
var videoContainer = Ti.UI.createView({ width:300, height:169, top:170 });
var companionView = Ti.UI.createView({ width:300, height:50, bottom:10, backgroundColor:'#000' });
var slotPositionContainer = Ti.UI.createView({ width:300, height:20, borderColor:'#f00', borderWidth:1, top:350 });
var videoPlayer = Ti.Media.createVideoPlayer({ zIndex:-1});
var field = Ti.UI.createTextField({ height:44, top:10, left:10, right:10, backgroundColor:'#fff', clearButtonMode:Ti.UI.INPUT_BUTTONMODE_ONFOCUS, keyboardType:Titanium.UI.KEYBOARD_NUMBER_PAD, borderStyle:Titanium.UI.INPUT_BORDERSTYLE_ROUNDED });
var submit = Ti.UI.createButton({ top:64, height:44, left:10, right:10, title:'Set New Video ID' });
var label = Ti.UI.createLabel({ backgroundColor:'#000', color:'#fff', height:30, textAlign:'center', font:{fontSize:12, fontWeight:'bold'}, top:116 });
var scrubber = Ti.UI.createView({ height:20, width:1, backgroundColor:'#000', left:0 });

function createState() {
	return {
		slotsBuilt: false,
		adPositions: {},
		lastPlaybackTime: 0,
		playbackTimeSet: false,
		duration: 0,
		prerollFinished: false,
		playingAd: false
	}
}

var state = {};

var timers = {
	position: null,
	checkAd: null
};

var contentUrl = null;

var adManager = freewheel.createAdManager({
	networkId: '171224',
	serverUrl: 'http://29773.v.fwmrm.net/ad/p/1'
});

function resetState() {
	state = createState();
	
	if (timers.position) { clearInterval(timers.position); }
	if (timers.checkAd) { clearInterval(timers.checkAd); }
}

function updateStatus(type) {
	label.text = msg.prep + type;
}

function createAdContext() {
	Ti.API.info('Creating Ad Context');
	
	adManager.setAdContext({
		contentUrl: contentUrl,
		base: videoContainer, // base view to attach FW views
		companionBase: companionView,
		player: videoPlayer,
		profile: '171224:nbcu_test_ios',
		videoId: String(field.value),
		fallbackId: '35437174',
		siteSection: 'ios_test_section'
	});
}

function makeTokenRequest(value) {
	var request = Ti.Network.createHTTPClient({ 
		timeout: 5000,
		onload: function() {
			var url = JSON.parse(this.responseText.replace('//', '')).tokenizedURL;
			Ti.API.info('Request Response: ' + this.responseText);
			
			if (url.search('This') === -1) {
				updateStatus(msg.set);
				contentUrl = url;
				videoPlayer.setUrl(url);
				createAdContext(field.value);
			} else {
				throwRequestError();
			}
			
		},
		onerror: throwRequestError
	});
	
	function throwRequestError() {
		Ti.API.info('There was an error requesting tokenized URL');
		updateStatus(msg.error);
	}
	
	Ti.API.info('REQUEST URL: ' + token.base + value + token.bitrate);
	
	request.open('GET', token.base + value + token.bitrate);
	request.send(null);
}

function checkAd() {	
	if (!state.playingAd) {
		for (var m = 0, ml = state.adPositions.midrolls.length; m < ml; m++) {
			if (state.adPositions.midrolls[m].time === videoPlayer.currentPlaybackTime >> 0) {
				state.lastPlaybackTime = state.adPositions.midrolls[m].time + 10;
				playAd(state.adPositions.midrolls[m]);
			} else {
				Ti.API.info('Did not find midroll ad.');
			}
		}
		
		for (var ps = 0, psl = state.adPositions.postrolls.length; ps < psl; ps++) {
			if (state.adPositions.postrolls[ps].time === videoPlayer.currentPlaybackTime >> 0) {
				state.lastPlaybackTime = state.adPositions.postrolls[ps].time + 10;
				playAd(state.adPositions.postrolls[ps]);
			} else {
				Ti.API.info('Did not find postroll ad.');
			}
		}
	}
}

function playAd(slot) {
	state.playingAd = true;
	state.playbackTimeSet = false;
	adManager.playAds({ time:slot.time });
}

/* event
 * - source (TiFreewheelAdManager)
 * - type
 * - prerolls (array)
 *  - time (number)
 * - midrolls (array)
 *  - time (number)
 * - postrolls (array)
 *  - time (number)
 */
adManager.addEventListener('onadresponse', function(e) {
	Ti.API.info('Ad Response Event Info: ');
	Ti.API.info(e);	
	
	updateStatus(msg.context);
	
	state.adPositions = {
		prerolls: e.prerolls || [],
		midrolls: e.midrolls || [],
		postrolls: e.postrolls || []
	};

	state.lastPlaybackTime = 0;
	
	playAd(state.adPositions.prerolls[0]);

	timers.checkAd = setInterval(checkAd, 250);
	
	if (e.error) {
		Ti.API.info('Ad response error: ' + e.error);
	}
});

adManager.addEventListener('onadopen', function() {
	if (!state.playingAd) {
		videoPlayer.pause();
	}
});

videoPlayer.addEventListener('loadstate', function(e) {
	if (e.loadState === 3 && !state.playbackTimeSet) {
		Ti.API.info('Video has loaded and should be playing.');
		state.playbackTimeSet = true;
		videoPlayer.currentPlaybackTime = state.lastPlaybackTime;
		updateStatus(msg.content);
	}
});

videoPlayer.addEventListener('durationAvailable', function(e) {
	if (!state.slotsBuilt) {		
		for (var p = 0, pl = state.adPositions.prerolls.length; p < pl; p++) {
			slotPositionContainer.add(Ti.UI.createView({ width:5, backgroundColor:'#00f', opacity:0.5, height:20, left:(state.adPositions.prerolls[p].time * 300) / e.duration }));
		}
		
		for (var m = 0, ml = state.adPositions.midrolls.length; m < ml; m++) {
			slotPositionContainer.add(Ti.UI.createView({ width:5, backgroundColor:'#0ff', opacity:0.5, height:20, left:(state.adPositions.midrolls[m].time * 300) / e.duration }));
		}
		
		for (var ps = 0, psl = state.adPositions.postrolls.length; ps < psl; ps++) {
			slotPositionContainer.add(Ti.UI.createView({ width:5, backgroundColor:'#f0f', opacity:0.5, height:20, left:(state.adPositions.postrolls[ps].time * 300) / e.duration }));
		}
		
		state.duration = e.duration;
		
		slotPositionContainer.add(scrubber);
		
		setInterval(function() {
			scrubber.left = (Math.round(videoPlayer.currentPlaybackTime) * 300) / state.duration;
		}, 250);
		
		state.slotsBuilt = true;
	}
});

adManager.addEventListener('onslotstarted', function(e) {	
	Ti.API.info('Slot Started Event Info: ');
	Ti.API.info(e);
	
	updateStatus(msg.slot.started);
	state.adPlaying = true;	
});

adManager.addEventListener('onslotended', function(e) {
	Ti.API.info('Slot Ended Event Info: ');
	Ti.API.info(e);
	
	if (!state.prerollFinished) {
		state.prerollFinished = true;
		videoPlayer.play();
	}
	
	updateStatus(msg.slot.ended);
	state.playingAd = false;
});

submit.addEventListener('click', function(e) {
	if (field.value) {
		adManager.destroyContext();
		resetState();		
		updateStatus(msg.submit);
		makeTokenRequest(field.value);
	} else {
		updateStatus(msg.invalid);
	}
	
	field.blur();
});

updateStatus(msg.wait);

videoContainer.add(videoPlayer);
win.add(videoContainer);
win.add(slotPositionContainer);
win.add(field);
win.add(submit);
win.add(label);
win.add(companionView);

win.open();