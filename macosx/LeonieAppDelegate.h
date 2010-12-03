//
//  LeonieAppDelegate.h
//  Leonie
//
//  Created by Uli Kusterer on 06.09.10.
//  Copyright 2010 Uli Kusterer. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface LeonieAppDelegate : NSObject <NSApplicationDelegate>
{
    NSWindow			*window;
	NSProgressIndicator	*busyIndicator;
	NSTextField			*messageBoxField;
}

@property (assign) IBOutlet NSWindow			*window;
@property (assign) IBOutlet NSProgressIndicator	*busyIndicator;
@property (assign) IBOutlet NSTextField			*messageBoxField;


-(void)	printMessage: (NSString*)inMessage;

@end
