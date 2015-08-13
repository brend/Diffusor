/* Controller */

#import <Cocoa/Cocoa.h>

@interface Controller : NSObject<NSFileManagerDelegate>
{
    IBOutlet NSButton *buttonPerformDiffusion;
    IBOutlet NSButton *buttonResetParameter;
    IBOutlet NSTextField *fieldCedAlpha;
    IBOutlet NSTextField *fieldCedC;
    IBOutlet NSTextField *fieldCedIterations;
    IBOutlet NSTextField *fieldCedRho;
    IBOutlet NSTextField *fieldCedSigma;
    IBOutlet NSTextField *fieldCedTau;
    IBOutlet NSTextField *fieldEedIterations;
    IBOutlet NSTextField *fieldEedLambda;
    IBOutlet NSTextField *fieldEedSigma;
    IBOutlet NSTextField *fieldEedTau;
    IBOutlet NSImageView *imageAfter;
    IBOutlet NSImageView *imageBefore;
    IBOutlet id menuFile;
    IBOutlet NSProgressIndicator *progressIndicator;
    IBOutlet NSTabView *tabView;
	
	float **originalImage, **diffusedImage;
	NSSize originalSize;
	unsigned char *originalPlane, *diffusedPlane;
	
	NSString *filenameLastSaved;
}
- (IBAction)openFile:(id)sender;
- (IBAction)performDiffusion:(id)sender;
- (IBAction)resetParameters:(id)sender;
- (IBAction)saveFile:(id)sender;
- (IBAction)saveFileAs:(id)sender;

@end
