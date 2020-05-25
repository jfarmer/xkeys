#include <stdio.h>
#include <IOKit/hid/IOHIDManager.h>
#include <IOKit/hid/IOHIDValue.h>

#include "Xkeys.h"

char * CStringCopyFromCFString(CFStringRef stringRef) {
  char *str;
  CFIndex length = CFStringGetLength(stringRef);
  CFIndex maxSize = CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8);

  str = (char *)malloc(maxSize);

  CFStringGetCString(stringRef, str, maxSize, kCFStringEncodingUTF8);

  return str;
}

void callbackOnDeviceMatch(void *context, IOReturn result, void *sender, IOHIDDeviceRef device) {
  printf("----\n");
  printf("Matching device.\n");

  CFStringRef vendorNameRef;
  char *vendorName;

  vendorNameRef = IOHIDDeviceGetProperty(device, CFSTR(kIOHIDManufacturerKey));
  if (!vendorNameRef) {
    printf("No vendor name\n");
    return;
  }

  vendorName = CStringCopyFromCFString(vendorNameRef);

  printf("We got a device from manufacturer: %s\n", vendorName);
  free(vendorName);

  uint8_t reportId = 0x0;
  uint8_t reportData[36];
  size_t reportLength = 35;
  IOReturn results;

  memset(reportData, 0, sizeof(reportData));

  reportData[0] = 0xB5;
  reportData[1] = 0x00;
  reportData[2] = 0x00;

  results = IOHIDDeviceSetReport(device, kIOHIDReportTypeOutput, reportId, reportData, reportLength);

  if (results == kIOReturnSuccess) {
    printf("Successfully sent report.\n");
    CFRunLoopStop(CFRunLoopGetCurrent());
  } else {
    printf("Failed to send report. Return value: %llu\n", (uint64_t)results);
  }
}

CFMutableDictionaryRef createHIDMatchingDict(uint32_t vid, uint32_t pid, uint32_t usagePage, uint32_t usage) {
  CFMutableDictionaryRef dict = CFDictionaryCreateMutable(
      kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

  if (!dict) { return NULL; }

  CFNumberRef vidRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &vid);
  if (!vidRef) { goto _dict_cleanup; }
  CFDictionarySetValue(dict, CFSTR(kIOHIDVendorIDKey), vidRef);
  CFRelease(vidRef);

  CFNumberRef pidRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &pid);
  if (!pidRef) { goto _dict_cleanup; }
  CFDictionarySetValue(dict, CFSTR(kIOHIDProductIDKey), pidRef);
  CFRelease(pidRef);

  CFNumberRef pageNumberRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &usagePage);
  if (!pageNumberRef) { goto _dict_cleanup; }
  CFDictionarySetValue(dict, CFSTR(kIOHIDDeviceUsagePageKey), pageNumberRef);
  CFRelease(pageNumberRef);

  CFNumberRef usageNumberRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &usage);
  if (!usageNumberRef) { goto _dict_cleanup; }
  CFDictionarySetValue(dict, CFSTR(kIOHIDDeviceUsageKey), usageNumberRef);
  CFRelease(usageNumberRef);

  return dict;
_dict_cleanup:
  CFRelease(dict);
  return NULL;
}

int main(int argc, char **argv) {
  IOHIDManagerRef hidManager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);

  // X-Keys XK-16 has vendor ID 1523, with default PID 1049
  CFMutableDictionaryRef keypad = createHIDMatchingDict(Xkeys_XK16_VendorID,
      Xkeys_XK16_ProductID,
      Xkeys_XK16_UsagePage,
      Xkeys_XK16_Usage);

  IOHIDManagerSetDeviceMatching(hidManager, keypad);
  IOHIDManagerRegisterDeviceMatchingCallback(hidManager, callbackOnDeviceMatch, NULL);

  IOHIDManagerScheduleWithRunLoop(hidManager, CFRunLoopGetMain(), kCFRunLoopDefaultMode);

  IOHIDManagerOpen(hidManager, kIOHIDOptionsTypeNone);

  CFRunLoopRun();
}
