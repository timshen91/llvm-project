// RUN: clang-cc -fsyntax-only -triple x86_64-apple-darwin9 -verify %s 

typedef signed char BOOL;

@class NSInvocation, NSMethodSignature, NSCoder, NSString, NSEnumerator;

@protocol NSObject
- (BOOL)isEqual:(id)object;
@end

@interface NSObject <NSObject> {}
@end

@interface XCDeviceWillExecuteInfoBaton : NSObject {}
  @property (retain) __attribute__((objc_gc(strong))) NSString *sdkPath;
@end

@implementation XCDeviceWillExecuteInfoBaton
  @synthesize sdkPath; 
@end

