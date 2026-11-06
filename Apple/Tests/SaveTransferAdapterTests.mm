#import <XCTest/XCTest.h>
#import <UIKit/UIKit.h>
#include "Core/Contracts/Game.hpp"
#include "Core/Session/SaveData.hpp"

@protocol MDTransferHostView
- (instancetype)initWithFrame:(CGRect)frame saveDirectory:(NSURL*)directory;
- (void)openTransferField:(std::string)initial readOnly:(BOOL)readOnly bounds:(md::Rect)bounds;
- (void)layoutTransferField:(md::Rect)bounds;
- (std::string)transferFieldText;
- (void)closeTransferField;
- (void)setApplicationActive:(BOOL)active;
- (void)shutdown;
@end
@interface SaveTransferAdapterTests:XCTestCase @end
@implementation SaveTransferAdapterTests
- (void)testCodeFieldPreservesLongInputAndLeavesGameButtonsTouchable {
    NSURL* directory=[NSURL fileURLWithPath:[NSTemporaryDirectory() stringByAppendingPathComponent:NSUUID.UUID.UUIDString] isDirectory:YES];
    Class gameClass=NSClassFromString(@"MDGameView");XCTAssertNotNil(gameClass);if(!gameClass)return;
    UIView<MDTransferHostView>* view=[(id<MDTransferHostView>)[gameClass alloc] initWithFrame:CGRectMake(0,0,320,480) saveDirectory:directory];
    UIWindowScene* scene=nil;for(UIScene* candidate in UIApplication.sharedApplication.connectedScenes)if([candidate isKindOfClass:UIWindowScene.class]){scene=(UIWindowScene*)candidate;break;}
    UIWindow* window=scene?[[UIWindow alloc]initWithWindowScene:scene]:[[UIWindow alloc]initWithFrame:CGRectMake(0,0,320,480)];window.rootViewController=[UIViewController new];
    [window.rootViewController.view addSubview:view];[window makeKeyAndVisible];[view layoutIfNeeded];
    const std::string code="MD1."+std::string(60000,'a');
    [view openTransferField:code readOnly:NO bounds:{10,110,300,90}];
    UITextView* field=[view valueForKey:@"transferField"];
    XCTAssertTrue([field isKindOfClass:UITextView.class]);XCTAssertTrue(field.editable);XCTAssertTrue(field.selectable);XCTAssertTrue(field.isFirstResponder);
    XCTAssertTrue([view transferFieldText]==code);XCTAssertFalse([[view valueForKey:@"nameInputPending"] boolValue]);
    XCTAssertFalse([[view hitTest:CGPointMake(20,40) withEvent:nil] isDescendantOfView:field]);
    XCTAssertTrue([[view hitTest:CGPointMake(CGRectGetMidX(field.frame),CGRectGetMidY(field.frame)) withEvent:nil] isDescendantOfView:field]);
    for(UIView* subview in view.subviews)XCTAssertFalse([subview isKindOfClass:UIControl.class]);
    NSString* huge=[@"x" stringByPaddingToLength:120001 withString:@"x" startingAtIndex:0];
    XCTAssertFalse([field.delegate textView:field shouldChangeTextInRange:NSMakeRange(0,field.text.length) replacementText:huge]);
    [view layoutTransferField:{20,120,280,80}];
    const CGRect before=field.frame;XCTAssertGreaterThan(before.size.width,0);
    view.frame=CGRectMake(0,0,768,1024);[view setNeedsLayout];[view layoutIfNeeded];
    XCTAssertGreaterThan(field.frame.size.width,before.size.width);XCTAssertTrue([view transferFieldText]==code);
    [view setApplicationActive:NO];XCTAssertFalse(field.isFirstResponder);XCTAssertTrue([view transferFieldText]==code);
    [view closeTransferField];XCTAssertNil([view valueForKey:@"transferField"]);XCTAssertNil(field.superview);
    [view openTransferField:code readOnly:YES bounds:{10,110,300,90}];
    field=[view valueForKey:@"transferField"];XCTAssertFalse(field.editable);XCTAssertTrue(field.selectable);
    XCTAssertFalse(field.isFirstResponder);XCTAssertTrue([view transferFieldText]==code);
    [view shutdown];XCTAssertNil([view valueForKey:@"transferField"]);XCTAssertNil(field.superview);window.hidden=YES;
    [NSFileManager.defaultManager removeItemAtURL:directory error:nil];
}
- (void)testStartupCompletesPendingRestoreInIsolatedDirectory {
    NSURL* directory=[NSURL fileURLWithPath:[NSTemporaryDirectory() stringByAppendingPathComponent:NSUUID.UUID.UUIDString] isDirectory:YES];
    XCTAssertTrue([NSFileManager.defaultManager createDirectoryAtURL:directory withIntermediateDirectories:YES attributes:nil error:nil]);
    md::SaveData original=md::SaveData::empty(42);original.createProfile("TRANSFER TEST",123);
    auto before=original.encode();md::SaveData updated=original;updated.setCredits(0,98765);auto after=updated.encode();
    NSURL* save=[directory URLByAppendingPathComponent:@"MachDefense.sav"];
    XCTAssertTrue([[NSData dataWithBytes:before.data() length:before.size()] writeToURL:save atomically:YES]);
    XCTAssertTrue([[NSData dataWithBytes:after.data() length:after.size()] writeToURL:[directory URLByAppendingPathComponent:@"MachDefense.sav.restore-pending"] atomically:YES]);
    Class gameClass=NSClassFromString(@"MDGameView");XCTAssertNotNil(gameClass);if(!gameClass)return;
    UIView<MDTransferHostView>* view=[(id<MDTransferHostView>)[gameClass alloc] initWithFrame:CGRectMake(0,0,320,480) saveDirectory:directory];
    NSData* actual=[NSData dataWithContentsOfURL:save];XCTAssertEqual(actual.length,after.size());
    XCTAssertEqual(memcmp(actual.bytes,after.data(),after.size()),0);
    XCTAssertFalse([NSFileManager.defaultManager fileExistsAtPath:[directory URLByAppendingPathComponent:@"MachDefense.sav.restore-pending"].path]);
    [view shutdown];[NSFileManager.defaultManager removeItemAtURL:directory error:nil];
}
@end
