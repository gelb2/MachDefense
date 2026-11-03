#import <Foundation/Foundation.h>
@interface MDAudio : NSObject
- (instancetype)initWithResourceRoot:(NSURL*)root;
- (BOOL)prepareSounds;
- (void)play:(NSString*)name gain:(float)gain pan:(float)pan;
- (void)playMusic:(NSString*)name gain:(float)gain;
- (void)stopMusic;
- (void)setActive:(BOOL)active;
@property(nonatomic,readonly) NSUInteger playCount;
@property(nonatomic,readonly) NSUInteger loadedSounds;
@property(nonatomic,readonly) NSString* lastError;
@property(nonatomic,readonly) NSString* musicName;
@property(nonatomic,readonly) BOOL musicPlaying;
@property(nonatomic,readonly) BOOL musicLooping;
@end
