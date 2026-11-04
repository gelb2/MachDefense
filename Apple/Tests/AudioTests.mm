#import <XCTest/XCTest.h>
#import "../../Sources/Platform/Apple/MDAudio.h"
@interface AudioTests:XCTestCase @end
@implementation AudioTests
- (void)testPreparedSoundsReuseAndInterruption {
    NSBundle* testBundle=[NSBundle bundleForClass:self.class];
    NSURL* root=[testBundle URLForResource:@"MachDefenseAssets" withExtension:@"bundle"];
    XCTAssertNotNil(root);if(!root)return;
    MDAudio* audio=[[MDAudio alloc] initWithResourceRoot:root];
    XCTAssertTrue([audio prepareSounds],@"%@",audio.lastError);
    XCTAssertEqual(audio.loadedSounds,77u);
    NSUInteger context=audio.currentContextIdentity;XCTAssertNotEqual(context,0u);
    NSArray<NSURL*>* files=[NSFileManager.defaultManager contentsOfDirectoryAtURL:[root URLByAppendingPathComponent:@"Audio"] includingPropertiesForKeys:nil options:0 error:nil];
    NSString* name=nil;for(NSURL* file in files)if([file.pathExtension.lowercaseString isEqualToString:@"wav"]){name=file.lastPathComponent;break;}
    XCTAssertNotNil(name);if(!name)return;
    for(int i=0;i<100;++i){[audio play:name gain:.01f pan:0];XCTAssertEqual(audio.currentContextIdentity,context);}
    XCTAssertEqual(audio.playCount,100u);XCTAssertEqual(audio.loadedSounds,77u);
    [audio setActive:NO];XCTAssertEqual(audio.currentContextIdentity,0u);[audio play:name gain:.01f pan:0];XCTAssertEqual(audio.playCount,100u);
    [audio setActive:YES];XCTAssertEqual(audio.currentContextIdentity,context);[audio play:name gain:.01f pan:0];XCTAssertEqual(audio.playCount,101u);
    XCTAssertTrue([audio prepareSounds]);XCTAssertEqual(audio.loadedSounds,77u);
}
- (void)testMusicLoopIsIndependentAndResumesAfterInterruption {
    NSURL* root=[[NSBundle bundleForClass:self.class] URLForResource:@"MachDefenseAssets" withExtension:@"bundle"];
    XCTAssertNotNil(root);if(!root)return;
    MDAudio* audio=[[MDAudio alloc]initWithResourceRoot:root];XCTAssertTrue([audio prepareSounds]);
    [audio playMusic:@"Intro.mp3" gain:.01];XCTAssertTrue(audio.musicPlaying);XCTAssertTrue(audio.musicLooping);XCTAssertEqualObjects(audio.musicName,@"Intro.mp3");
    [audio play:@"UI_Btn_OK.wav" gain:.01 pan:0];XCTAssertTrue(audio.musicPlaying);XCTAssertEqual(audio.playCount,1u);
    [audio playMusic:@"Intro.mp3" gain:.02];XCTAssertTrue(audio.musicPlaying);XCTAssertEqual(audio.loadedSounds,77u);
    [audio setActive:NO];XCTAssertFalse(audio.musicPlaying);XCTAssertEqualObjects(audio.musicName,@"Intro.mp3");
    [audio setActive:YES];XCTAssertTrue(audio.musicPlaying);XCTAssertTrue(audio.musicLooping);
    [audio setActive:NO];[audio playMusic:@"UI_BGM.mp3" gain:.01];XCTAssertFalse(audio.musicPlaying);
    [audio setActive:YES];XCTAssertTrue(audio.musicPlaying);XCTAssertEqualObjects(audio.musicName,@"UI_BGM.mp3");
    [audio playMusic:@"GamePlay.mp3" gain:.01];XCTAssertTrue(audio.musicPlaying);XCTAssertEqualObjects(audio.musicName,@"GamePlay.mp3");
    [audio stopMusic];XCTAssertFalse(audio.musicPlaying);XCTAssertNil(audio.musicName);
    [audio setActive:NO];[audio setActive:YES];XCTAssertFalse(audio.musicPlaying);XCTAssertEqual(audio.loadedSounds,77u);
}
- (void)testRepeatedCreatePlayDisposeDoesNotExhaustDevice {
    NSURL* root=[[NSBundle bundleForClass:self.class] URLForResource:@"MachDefenseAssets" withExtension:@"bundle"];
    for(unsigned cycle=0;cycle<20;++cycle){@autoreleasepool{
        MDAudio* audio=[[MDAudio alloc]initWithResourceRoot:root];
        XCTAssertTrue([audio prepareSounds],@"Cycle %u: %@",cycle,audio.lastError);
        [audio play:@"HMG01.wav" gain:.01 pan:0];XCTAssertEqual(audio.playCount,1u);
        [audio setActive:NO];[audio setActive:YES];
        [audio play:@"HMG01.wav" gain:.01 pan:0];XCTAssertEqual(audio.playCount,2u);
        [audio setActive:NO];audio=nil;
    }}
}
- (void)testMusicQueueLoopsPastEndOfShortResource {
    NSURL* root=[[NSBundle bundleForClass:self.class] URLForResource:@"MachDefenseAssets" withExtension:@"bundle"];
    MDAudio* audio=[[MDAudio alloc]initWithResourceRoot:root];XCTAssertTrue([audio prepareSounds]);
    // This WAV lasts about 0.6 seconds. The same packet queue must refill from
    // its beginning repeatedly; playing=true alone cannot prove output advanced.
    [audio playMusic:@"BuildMachComplete.wav" gain:.01];
    auto done=[self expectationWithDescription:@"AudioQueue loops past EOF"];
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW,int64_t(2.5*NSEC_PER_SEC)),dispatch_get_main_queue(),^{[done fulfill];});
    [self waitForExpectations:@[done] timeout:4];
    XCTAssertTrue(audio.musicPlaying);XCTAssertGreaterThan(audio.musicPlaybackTime,2.f);
    [audio stopMusic];XCTAssertFalse(audio.musicPlaying);
}
@end
