#import "Controller.h"

#include "IP/ip.h"

@implementation Controller

- (void) awakeFromNib
{
	[NSApp setDelegate: self];
}

- (NSImage *) imageWithGreyValues: (float **) f width: (int) nx height: (int) ny plane: (unsigned char **) thePlane
{
	// Copy the grey values into a color plane.
	unsigned char *plane = (unsigned char *) malloc(nx * ny * sizeof(unsigned char)); // TODO Möglicherweise muss man das nach Verwendung des Bildes freigeben (nich aber währenddessen!)
	int i, j;
	
	for (j = 0; j < ny; ++j) {
		for (i = 0; i < nx; ++i) {
			int value = round(f[i + 1][j + 1]);
			
			value = value < 0 ? 0 : (value > 255 ? 255 : value);
			plane[j * (int) nx + i] = (unsigned char) value;
		}
	}
	
	*thePlane = plane;
	
	NSImage *image = [[NSImage alloc] initWithSize: NSMakeSize(nx, ny)];
	NSBitmapImageRep *rep = [[NSBitmapImageRep alloc] 
														initWithBitmapDataPlanes: &plane 
																	  pixelsWide: nx
																	  pixelsHigh: ny
																   bitsPerSample: 8
																 samplesPerPixel: 1
																		hasAlpha: NO 
																		isPlanar: NO
																  colorSpaceName: NSCalibratedWhiteColorSpace
																	 bytesPerRow: nx
																	bitsPerPixel: 8];
	
	[image addRepresentation:rep];
    
    [rep release];
	
	return [image autorelease];
}

- (void) setupNewImageWithGreyvalues: (float **) f width: (int) nx height: (int) ny
{
	// Create NSImage, then release grey value matrix
	unsigned char *plane = NULL;
	NSImage *image = [self imageWithGreyValues: f width: nx height: ny plane: &plane];
	
	[imageBefore setImage: nil];
	[imageAfter setImage: nil];
	
    [imageBefore image];
    [imageAfter image];
	if (originalImage)
		ip_deallocate_image(originalSize.width + 2, originalSize.height + 2, originalImage);
	if (originalPlane)
		free(originalPlane);
	
	if (diffusedImage)
		ip_deallocate_image(originalSize.width + 2, originalSize.height + 2, diffusedImage);
	if (diffusedPlane)
		free(diffusedPlane);
	
	originalImage = f;
	originalPlane = plane;
	originalSize = NSMakeSize(nx, ny);
	
	diffusedImage = NULL;
	diffusedPlane = NULL;
	
	// Display the loaded image
	[imageBefore setImage: image];
	
	[buttonPerformDiffusion setEnabled: YES];
}

- (NSBitmapImageRep *) getBitmapImageRep: (NSImage *) image
{
	NSArray *reps = [image representations];
	NSEnumerator *e = [reps objectEnumerator];
	NSImageRep * r;
	
	while (r = [e nextObject])
		if ([r class] == [NSBitmapImageRep class])
			return (NSBitmapImageRep *) r;
	
	return nil;
}

- (void) runAlert: (NSString *) message
             info: (NSString *) info
{
    NSAlert *alert = [[NSAlert alloc] init];
    
    [alert addButtonWithTitle: @"OK"];
    [alert setMessageText: [NSString stringWithFormat: @"%@", message]];
    [alert setInformativeText: [NSString stringWithFormat: @"%@", info]];
    [alert setAlertStyle: NSCriticalAlertStyle];
    
    [alert runModal];
    [alert release];
}

- (BOOL) loadImageFromFile: (NSString *) s
{
	float **f = NULL;
	int nx = 0, ny = 0;
	
	if ([s hasSuffix: @".pgm"]) {
		// Load PGM data with IP-functions
		const char *filename = [s UTF8String];
		FILE *file = fopen(filename, "r");
		
		if (file == NULL) {
			NSLog(@"File couldn't be opened: %s", filename);
            
            [self runAlert: [NSString stringWithFormat: NSLocalizedStringFromTable(@"BodyCannotBeOpenedForReading", @"Alerts", nil), s, nil] info: NSLocalizedStringFromTable(@"TitleCannotBeOpenedForReading", @"Alerts", nil)];
			
			return NO;
		}
		
		if (NULL == (f = ip_load_image(file, &nx, &ny, NULL)))
			NSLog(@"ip_load_image returned NULL");
		
		fclose(file);
	} else {
		// Hopefully NSImage is able to load the data
		NSImage *image = [[NSImage alloc] initWithContentsOfFile: s];
		
		if (image == nil) {
			NSLog(@"File couldn't be opened or read: %@", s);
            [self runAlert: [NSString stringWithFormat: NSLocalizedStringFromTable(@"BodyCannotOpenFileForReading", @"Alerts", nil), s, nil]
                      info: NSLocalizedStringFromTable(@"TitleCannotOpenFileForReading", @"Alerts", nil)];
			
			return NO;
		}
		
		NSImageRep * r = [self getBitmapImageRep: image];
		
		if (r != nil && [r class] == [NSBitmapImageRep class]) {
            // query rep for image size - querying image will yield resolution-dependent results
            nx = [r pixelsWide];
            ny = [r pixelsHigh];
			f = ip_allocate_image(nx + 2, ny + 2);
			
			float wr = 0.28f, wg = 0.59f, wb = 0.13f; // well-known weights for greyscale conversion of colour images
			int i, j;
			for (j = 1; j <= ny; ++j)
				for (i = 1; i <= nx; ++i) {
					NSColor *c = [(NSBitmapImageRep *) r colorAtX: i y: j];
					CGFloat cr, cg, cb;
					
					c = [c colorUsingColorSpaceName: NSCalibratedRGBColorSpace];
					
					[c getRed: &cr green: &cg blue: &cb alpha: nil];
					f[i][j] = 255.0f * (wr * cr + wg * cg + wb * cb);
				}
		} else
            [self runAlert: NSLocalizedStringFromTable(@"BodyCannotReadImage", @"Alerts", nil)
                      info: NSLocalizedStringFromTable(@"TitleCannotReadImage", @"Alerts", nil)];
        
			[image release];
	}

	if (f == NULL) {
		NSLog(@"File couldn't be loaded.");
		
		return NO;
	}
		
	[self setupNewImageWithGreyvalues: f width: nx height: ny];
	
	if (filenameLastSaved != nil) {
		[filenameLastSaved release];
		filenameLastSaved = nil;
	}
		
	return YES;
}

- (IBAction) openFile:(id)sender
{
    NSArray *allowedFileTypes =
        [NSArray arrayWithObjects: @"pgm", @"tiff", @"gif", @"bmp", @"jpg", @"png", nil];
	NSOpenPanel *panel = [NSOpenPanel openPanel];
	
    [panel setAllowedFileTypes: allowedFileTypes];
    
	if (NSModalResponseOK == [panel runModal]) {
		// Load the image data in a manner according to the file type
        NSURL *url = [panel URL];
        
        if (![url isFileURL]) {
            [self runAlert: @"Only file URLs are supported currently." info: @"Invalid URL"];
            return;
        }
        
		NSString *filename = [url path];
		
		[self loadImageFromFile: filename];
		[[NSDocumentController sharedDocumentController] noteNewRecentDocumentURL: [NSURL fileURLWithPath: filename]];
	}
}

- (IBAction) performDiffusion: (id) sender 
{
	[NSThread detachNewThreadSelector: @selector(diffuse:) toTarget: self withObject: nil];
}

- (void) diffuse: (id) nuffin
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	[buttonPerformDiffusion setEnabled: NO];
	[menuFile setEnabled: NO];
	
	int nx = originalSize.width, ny = originalSize.height;
	float **u = ip_duplicate_image(nx, ny, originalImage);
	
	// Selection of the diffusion method depends on the label of the selected tab view item
	NSString *method = [[tabView selectedTabViewItem] label];
	
	if ([method isEqualToString: @"CED"]) {
		// Gather parameters for ced
		float C, rho, sigma, alpha, tau;
		int iterations;
		
		C = [fieldCedC floatValue];
		rho = [fieldCedRho floatValue];
		sigma = [fieldCedSigma floatValue];
		alpha = [fieldCedAlpha floatValue];
		tau = [fieldCedTau floatValue];
		iterations = [fieldCedIterations intValue];
		
		// Examine validity of the parameters
		if (!(C > 0 && rho > 0 && sigma > 0 && alpha > 0 && tau > 0 && tau < 0.25f && iterations > 0))
            [self runAlert: NSLocalizedStringFromTable(@"BodyInvalidParameter", @"Alerts", nil)
                      info: NSLocalizedStringFromTable(@"TitleInvalidParameter", @"Alerts", nil)];
		else {		
			// Perform diffusion and update progress indicator
			[progressIndicator setDoubleValue: 0];
			[progressIndicator setHidden: NO];
			
			int i = 0;
			while (i++ < iterations) {
				ip_ced(C, rho, alpha, tau, nx, ny, u);
				
				[progressIndicator setDoubleValue: (i * 100.0 / iterations)];
			}
			[progressIndicator setHidden: YES];
		}
	} else if ([method isEqualToString: @"EED"]) {
		// Gather parameters for eed
		float sigma, lambda, tau;
		int iterations;
		
		sigma = [fieldEedSigma floatValue];
		lambda = [fieldEedLambda floatValue];
		tau = [fieldEedTau floatValue];
		iterations = [fieldEedIterations intValue];
		
		// Examine validity of the parameters
		if (!(sigma > 0 && lambda > 0 && tau > 0 && tau < 0.25f && iterations > 0))
            [self runAlert: NSLocalizedStringFromTable(@"BodyInvalidParameter", @"Alerts", nil)
                      info: NSLocalizedStringFromTable(@"TitleInvalidParameter", @"Alerts", nil)];
		else {
			// Perform diffusion and update progress indicator
			[progressIndicator setDoubleValue: 0];
			[progressIndicator setHidden: NO];
			
			int i = 0;
			while (i++ < iterations) {
				ip_eed(lambda, tau, nx, ny, u);
				
				[progressIndicator setDoubleValue: (i * 100.0 / iterations)];
			}
			[progressIndicator setHidden: YES];
		}
	} else
		NSLog(@"Unknown diffusion method: %@", method);
	
	unsigned char *plane = NULL;
	NSImage *result = [self imageWithGreyValues: u width: nx height: ny plane: &plane];
	
	[imageAfter setImage: nil];
	
	if (diffusedImage)
		ip_deallocate_image(nx, ny, diffusedImage);
	if (diffusedPlane)
		free(diffusedPlane);
	
	diffusedImage = u;
	diffusedPlane = plane;
	
	[imageAfter setImage: result];
	
	[buttonPerformDiffusion setEnabled: YES];
	[menuFile setEnabled: YES];
	
	[pool release];
}

- (IBAction)resetParameters:(id)sender
{
	NSString *method = [[tabView selectedTabViewItem] label];
	
	if ([method isEqualToString: @"CED"]) {
		[fieldCedC setFloatValue:			1.0f];
		[fieldCedRho setFloatValue:			4.0f];
		[fieldCedSigma setFloatValue:		0.5f];
		[fieldCedAlpha setFloatValue:		0.001f];
		[fieldCedTau setFloatValue:			0.2f];
		[fieldCedIterations setIntValue:	20];		
	} else if ([method isEqualToString: @"EED"]) {
		[fieldEedSigma setFloatValue:		2.0f];
		[fieldEedLambda setFloatValue:		3.0f];
		[fieldEedTau setFloatValue:			0.2f];
		[fieldEedIterations setIntValue:	20];
	} else {
		NSLog(@"Unknown diffusion method: %@", method);
		
		return;
	}
}

- (void) saveImageToFile: (NSString *) filename
{
	NSString *ext = [filename pathExtension];
	
	if ([ext isEqualToString: @"pgm"]) {
		// Save file as PGM using IP functions
		const char *asciiFilename = [filename UTF8String];
		FILE *file = fopen(asciiFilename, "w");
		
		if (file == NULL) {
			NSLog(@"File couldn't be opened for writing: %@", filename);
            [self runAlert: [NSString stringWithFormat: NSLocalizedStringFromTable(@"BodyCannotOpenForWriting", @"Alerts", nil), filename, nil]
                      info: NSLocalizedStringFromTable(@"TitleCannotOpenForWriting", @"Alerts", nil)];
			
			return;
		}
		
		ip_save_image(file, originalSize.width, originalSize.height, diffusedImage, "Created with DIFFUSOR.", 1);
		
		fclose(file);
	} else {
		// Try to obtain a suitable image representation
		NSBitmapImageFileType type;
		
		if ([ext isEqualToString: @"tiff"])
			type = NSTIFFFileType;
		else if ([ext isEqualToString: @"gif"])
			type = NSGIFFileType;
		else if ([ext isEqualToString: @"bmp"])
			type = NSBMPFileType;
		else if ([ext isEqualToString: @"jpg"])
			type = NSJPEGFileType;
		else if ([ext isEqualToString: @"png"])
			type = NSPNGFileType;
		else {
            [self runAlert: [NSString stringWithFormat: NSLocalizedStringFromTable(@"BodyUnkownFileTypeForWriting", @"Alerts", nil), filename, ext, nil]
                      info: NSLocalizedStringFromTable(@"TitleUnknownFileTypeForWriting", @"Alerts", nil)];
			
			return;
		}
		
		NSBitmapImageRep *rep = [self getBitmapImageRep: [imageAfter image]];
		NSData *data = [rep representationUsingType: type properties: nil];
		
		if (data != nil) {
			[data writeToFile: filename atomically: YES];
		} else
            [self runAlert: [NSString stringWithFormat: NSLocalizedStringFromTable(@"BodyCannotRepresent", @"Alerts", nil), ext, nil]
                      info: NSLocalizedStringFromTable(@"TitleCannotRepresent", @"Alerts", nil)];
	}
}

- (IBAction)saveFile:(id)sender
{
	if (filenameLastSaved) {
		[self saveImageToFile: filenameLastSaved];
	} else
		[self saveFileAs: sender];
}

- (IBAction)saveFileAs:(id)sender
{
	if ([imageAfter image] == nil) {
        [self runAlert: NSLocalizedStringFromTable(@"BodyNoImageCreated", @"Alerts", nil)
                  info: NSLocalizedStringFromTable(@"TitleNoImageCreated", @"Alerts", nil)];
		
		return;
	}
	
	NSSavePanel *panel = [NSSavePanel savePanel];
	
	[panel setAllowedFileTypes: [NSArray arrayWithObjects: @"tiff", @"jpg", @"bmp", @"png", @"pgm", nil]];
	
	if (NSModalResponseOK == [panel runModal]) {
        NSURL *url = [panel URL];
        
        if (![url isFileURL]) {
            [self runAlert: @"Only file URLs are supported currently."
                      info: @"Invalid URL"];
            return;
        }
        
        NSString *filename = [url path];
        
		[self saveImageToFile: filename];
		
		if (filenameLastSaved != nil)
			[filenameLastSaved release];
		filenameLastSaved = [filename copy];
	}
}

- (BOOL) application: (NSApplication *) theApplication openFile: (NSString *) filename
{
	return [self loadImageFromFile: filename];
}

@end
