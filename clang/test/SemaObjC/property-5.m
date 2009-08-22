// RUN: clang-cc -verify %s

@protocol P1 @end
@protocol P2 @end
@protocol P3 @end

@interface NSData @end

@interface MutableNSData : NSData @end

@interface Base : NSData <P1>
@property(readonly) id ref;
@property(readonly) Base *p_base;
@property(readonly) NSData *nsdata;
@property(readonly) NSData * m_nsdata;
@end

@interface Data : Base <P1, P2>
@property(readonly) NSData *ref;	
@property(readonly) Data *p_base;	
@property(readonly) MutableNSData * m_nsdata;  
@end

@interface  MutedData: Data
@property(readonly) id p_base; 
@end

@interface ConstData : Data <P1, P2, P3>
@property(readonly) ConstData *p_base;
@end

void foo(Base *b, id x) {
  [ b setRef: x ]; // expected-warning {{method '-setRef:' not found}}
}
