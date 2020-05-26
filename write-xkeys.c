#include <IOKit/hid/IOHIDManager.h>
#include <IOKit/hid/IOHIDValue.h>
#include <stdio.h>

#include "Xkeys.h"

char* CStringCopyFromCFString(CFStringRef stringRef)
{
    char* str;
    CFIndex length = CFStringGetLength(stringRef);
    CFIndex maxSize = CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8);

    str = (char*)malloc(maxSize);

    CFStringGetCString(stringRef, str, maxSize, kCFStringEncodingUTF8);

    return str;
}

void callbackOnDeviceMatch(void* context, IOReturn result, void* sender, IOHIDDeviceRef device)
{

    CFStringRef vendorNameRef;
    char* vendorName;

    vendorNameRef = IOHIDDeviceGetProperty(device, CFSTR(kIOHIDManufacturerKey));
    if (!vendorNameRef) {
        printf("No vendor name\n");
        return;
    }

    vendorName = CStringCopyFromCFString(vendorNameRef);

    printf("We got a device from manufacturer: %s\n", vendorName);
    free(vendorName);
    CFRelease(vendorNameRef);

    uint8_t* reportData = context;
    uint8_t reportId = 0x0;
    IOReturn results;

    results = IOHIDDeviceSetReport(device, kIOHIDReportTypeOutput, reportId, reportData, Xkeys_XK16_REPORT_LENGTH - 1);

    if (results == kIOReturnSuccess) {
        printf("Successfully sent report.\n");
        CFRunLoopStop(CFRunLoopGetCurrent());
    } else {
        printf("Failed to send report. Return value: %llu\n", (uint64_t)results);
    }
}

CFMutableDictionaryRef createHIDMatchingDict(uint32_t vid, uint32_t pid, uint32_t usagePage, uint32_t usage)
{
    CFMutableDictionaryRef dict = CFDictionaryCreateMutable(
        kCFAllocatorDefault,
        0,
        &kCFTypeDictionaryKeyCallBacks,
        &kCFTypeDictionaryValueCallBacks);

    if (!dict) {
        return NULL;
    }

    CFNumberRef vidRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &vid);
    if (!vidRef) {
        goto _dict_cleanup;
    }
    CFDictionarySetValue(dict, CFSTR(kIOHIDVendorIDKey), vidRef);
    CFRelease(vidRef);

    CFNumberRef pidRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &pid);
    if (!pidRef) {
        goto _dict_cleanup;
    }
    CFDictionarySetValue(dict, CFSTR(kIOHIDProductIDKey), pidRef);
    CFRelease(pidRef);

    CFNumberRef pageNumberRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &usagePage);
    if (!pageNumberRef) {
        goto _dict_cleanup;
    }
    CFDictionarySetValue(dict, CFSTR(kIOHIDDeviceUsagePageKey), pageNumberRef);
    CFRelease(pageNumberRef);

    CFNumberRef usageNumberRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &usage);
    if (!usageNumberRef) {
        goto _dict_cleanup;
    }
    CFDictionarySetValue(dict, CFSTR(kIOHIDDeviceUsageKey), usageNumberRef);
    CFRelease(usageNumberRef);

    return dict;
_dict_cleanup:
    CFRelease(dict);
    return NULL;
}

void printUsage(char* programName)
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "    %s <Byte1> <Byte2> <Byte3> ... <ByteN>\n", programName);
    fprintf(stderr, "\n");
    fprintf(stderr, "Example:\n");
    fprintf(stderr, "    %s 0xB5 0x00 0x01\n", programName);
}

int main(int argc, char** argv)
{
    IOHIDManagerRef hidManager = NULL;
    CFMutableDictionaryRef keypad = NULL;
    uint8_t* reportData = NULL;

    int numBytes = argc - 1;

    if (numBytes == 0) {
        fprintf(stderr, "Error: You must specify at least one byte to send.\n");
        printUsage(argv[0]);

        exit(EXIT_FAILURE);
    }

    if (numBytes > Xkeys_XK16_REPORT_LENGTH) {
        fprintf(stderr, "Error: Cannot send more than %d bytes (received %d)\n", Xkeys_XK16_REPORT_LENGTH, numBytes);
        printUsage(argv[0]);

        exit(EXIT_FAILURE);
    }

    reportData = (uint8_t*)malloc(Xkeys_XK16_REPORT_LENGTH);

    if (!reportData) {
        fprintf(stderr, "Unable to allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    memset(reportData, 0, Xkeys_XK16_REPORT_LENGTH);

    for (int i = 0; i < numBytes; i++) {
        /* The third argument to strtoul is the radix/base. A value of 0 tells
           strtoul to derive the radix from the format of the provided string.

           That way we each byte can be given in hex or dec, e.g., 0xB5 or 181.
        */
        reportData[i] = (uint8_t)strtoul(argv[i + 1], NULL, 0);
    }

    hidManager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
    keypad = createHIDMatchingDict(Xkeys_XK16_VendorID,
        Xkeys_XK16_ProductID,
        Xkeys_XK16_UsagePage,
        Xkeys_XK16_Usage);

    IOHIDManagerSetDeviceMatching(hidManager, keypad);
    IOHIDManagerRegisterDeviceMatchingCallback(hidManager, callbackOnDeviceMatch, reportData);

    IOHIDManagerScheduleWithRunLoop(hidManager, CFRunLoopGetMain(), kCFRunLoopDefaultMode);

    IOHIDManagerOpen(hidManager, kIOHIDOptionsTypeNone);

    CFRunLoopRun();

    free(reportData);
    CFRelease(keypad);
    CFRelease(hidManager);
}
