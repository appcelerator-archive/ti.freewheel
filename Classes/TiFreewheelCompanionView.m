#import "TiFreewheelCompanionView.h"

@implementation TiFreewheelCompanionView

- (void)dealloc
{
    RELEASE_TO_NIL(companion);
    [super dealloc];
}

- (UIView*)companion
{
    if (companion == nil)
    {
        companion = [[UIView alloc] initWithFrame:[self frame]];
        [self addSubview:companion];
    }
    
    return companion;
}

- (void)frameSizeChanged:(CGRect)frame bounds:(CGRect)bounds
{
    if (companion != nil)
    {
        [TiUtils setView:companion positionRect:bounds];
    }
}

@end
