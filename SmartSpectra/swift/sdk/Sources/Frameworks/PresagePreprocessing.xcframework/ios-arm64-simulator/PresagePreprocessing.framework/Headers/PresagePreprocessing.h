//
//  PresagePreprocessing.h
//  PresagePreprocessing Objective C++ Binding Layer
//
//  Credits:
//  Inspired by MPIrisTracker / MPIrisTrackerDelegate code by Yuki Yamato on 2021/05/05.
//

#import <Foundation/Foundation.h>
#import <CoreVideo/CoreVideo.h>
#import <AVFoundation/AVFoundation.h>

typedef NS_ENUM(NSInteger, PresageMode) {
    PresageModeSpot,
    PresageModeContinuous
};

typedef NS_ENUM(NSInteger, PresageServer) {
    PresageServerTest,
    PresageServerProd,
    PresageServerBeta
};

@class PresagePreprocessing;

@protocol PresagePreprocessingDelegate <NSObject>

- (void)frameWillUpdate:(PresagePreprocessing *)tracker
   didOutputPixelBuffer:(CVPixelBufferRef)pixelBuffer
              timestamp:(long)timestamp;

- (void)frameDidUpdate:(PresagePreprocessing *)tracker
  didOutputPixelBuffer:(CVPixelBufferRef)pixelBuffer;

// Serialized StatusValue proto for Swift consumers
- (void)statusBufferChanged:(PresagePreprocessing *)tracker
             serializedBytes:(NSData *)data;

- (void)metricsBufferChanged:(PresagePreprocessing *)tracker
               serializedBytes:(NSData *)data;

- (void)edgeMetricsChanged:(PresagePreprocessing *)tracker
              serializedBytes:(NSData *)data;

- (void)timerChanged:(double)timerValue;


- (void)handleGraphError:(NSError *)error;

@end

@interface PresagePreprocessing : NSObject

@property(nonatomic, weak) id <PresagePreprocessingDelegate> delegate;
@property(nonatomic, assign) PresageMode mode;
@property(nonatomic, copy) NSString *apiKey;
@property(nonatomic, copy) NSString *graphName;
@property(nonatomic, assign) AVCaptureDevicePosition cameraPosition;
@property(nonatomic, assign) double spotDuration;

+ (void)configureAuthClientWith:(NSDictionary *)plistData;
+ (NSString *)fetchAuthChallenge;
+ (NSString *)respondToAuthChallengeWith:(NSString *)base64EncodedAnswer for:(NSString *)bundleID;
+ (BOOL)isAuthTokenExpired;
+ (void)useTestServer; // TODO: Deprecate this and remove
+ (void)setServer:(PresageServer)server;

- (instancetype)init;
- (void)start;
- (void)stop;
- (void)buttonStateChangedInFramework:(BOOL)isRecording;
// Returns a human-readable hint string for a given status code value.
// The value must match presage.physiology.StatusCode numeric values.
- (NSString * _Nonnull)getStatusHintFromCodeValue:(NSInteger)codeValue;
// Returns a human-readable description string for a given status code value.
// The value must match presage.physiology.StatusCode numeric values.
- (NSString * _Nonnull)getStatusDescriptionFromCodeValue:(NSInteger)codeValue;
- (void)setCameraPosition:(AVCaptureDevicePosition)cameraPosition;
- (void)setMode:(PresageMode)mode;
- (void)setSpotDuration:(double)spotDuration;

@end
