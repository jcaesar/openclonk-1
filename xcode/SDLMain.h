/*   SDLMain.m - main entry point for our Cocoa-ized SDL app
       Initial Version: Darrell Walisser <dwaliss1@purdue.edu>
       Non-NIB-Code & other changes: Max Horn <max@quendi.de>

    Feel free to customize this file to suit your needs
*/

#import <Cocoa/Cocoa.h>

@interface SDLMain: NSObject
{
	NSString* clonkDirectory;
	NSMutableArray* gatheredArguments;
	NSString* addonSupplied;
	BOOL doNotLaunch;
	BOOL hasFinished;
	BOOL terminateRequested;
}
- (NSString*) clonkDirectory;
- (BOOL) argsLookLikeItShouldBeInstallation:(char**)argv argc:(int)argc;
- (void)makeFakeArgs:(char***)argv argc:(int*)argc;
- (BOOL)installAddOn;
- (void)terminate:(NSApplication*)sender;
@end
