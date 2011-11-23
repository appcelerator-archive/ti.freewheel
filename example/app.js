// This is a test harness for your module
// You should do something interesting in this harness 
// to test out the module and to provide instructions 
// to users on how to use it by example.


// open a single window
var window = Ti.UI.createWindow({
	backgroundColor:'white'
});
var label = Ti.UI.createLabel();
window.add(label);
window.open();

// TODO: write your module tests here
var FreeWheel = require('ti.freewheel');
Ti.API.info("module is => " + FreeWheel);

label.text = FreeWheel.example();

Ti.API.info("module exampleProp is => " + FreeWheel.exampleProp);
FreeWheel.exampleProp = "This is a test value";

if (Ti.Platform.name == "android") {
	var proxy = FreeWheel.createExample({message: "Creating an example Proxy"});
	proxy.printMessage("Hello world!");
}