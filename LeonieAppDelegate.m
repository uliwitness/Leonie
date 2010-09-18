//
//  LeonieAppDelegate.m
//  Leonie
//
//  Created by Uli Kusterer on 06.09.10.
//  Copyright 2010 Uli Kusterer. All rights reserved.
//

#import "LeonieAppDelegate.h"
#import "LEOValue.h"



@implementation LeonieAppDelegate

@synthesize window;


-(void)	applicationDidFinishLaunching: (NSNotification *)aNotification
{
	char				buf[1024];
	LEOValueString		theStr;
	LEOInitStringConstantValue( (LEOValuePtr)&theStr, "Top 'o the mornin' to ya!" );
	LEOGetValueAsString( &theStr,buf,sizeof(buf) );
	
	NSLog( @"%s", buf );
}

@end
