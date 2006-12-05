#include <string>

#ifdef __APPLE__
#include <CFBundle.h>
#endif

static inline std::string getDataPath( void)
{
	std::string appPath = "";
#ifdef __APPLE__
        CFURLRef appURLRef = CFBundleCopyBundleURL(CFBundleGetMainBundle());
        CFStringRef macPath = CFURLCopyFileSystemPath(appURLRef, 
                                               kCFURLPOSIXPathStyle);
        const char *pathPtr = CFStringGetCStringPtr(macPath, 
                                               CFStringGetSystemEncoding());
	if( pathPtr)
		appPath = pathPtr;

        CFRelease(appURLRef);
        CFRelease(macPath);

	appPath = appPath + DATA_DIR;
#else
	appPath = std::string(DATA_DIR);
#endif
	return appPath;
}
