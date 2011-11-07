#import "FWPlayerCommon.h"

int fwplayer_short_screen_dimension() {
	CGSize size = [UIScreen mainScreen].bounds.size;
	return size.width < size.height ? size.width : size.height;
}

int fwplayer_long_screen_dimension() {
	CGSize size = [UIScreen mainScreen].bounds.size;
	return size.width > size.height ? size.width : size.height;
}

double fwplayer_system_version() {
	return [[[UIDevice currentDevice] systemVersion] doubleValue];
}