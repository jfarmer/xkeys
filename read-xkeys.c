#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)       \
    (byte & 0x80 ? '1' : '0'),     \
        (byte & 0x40 ? '1' : '0'), \
        (byte & 0x20 ? '1' : '0'), \
        (byte & 0x10 ? '1' : '0'), \
        (byte & 0x08 ? '1' : '0'), \
        (byte & 0x04 ? '1' : '0'), \
        (byte & 0x02 ? '1' : '0'), \
        (byte & 0x01 ? '1' : '0')

#include <IOKit/hid/IOHIDManager.h>
#include <IOKit/hid/IOHIDValue.h>

#include "Xkeys.h"

void clearConsole()
{
    printf("\e[1;1H\e[2J");
}

void callbackReceivedUserInput(void* context, IOReturn result, void* sender, IOHIDValueRef value)
{
    const uint8_t* bytes = IOHIDValueGetBytePtr(value);
    long byte_count = IOHIDValueGetLength(value);

    IOHIDElementRef elem = IOHIDValueGetElement(value);
    uint32_t usage_page = IOHIDElementGetUsagePage(elem);

    if (usage_page != 0x07 && usage_page != 0x08) {
        printf("Unknown usage page: %d\n", usage_page);
        return;
    }

    IOHIDDeviceRef device = sender;
    int32_t pid = 1;
    CFNumberGetValue(IOHIDDeviceGetProperty(device, CFSTR("idProduct")), kCFNumberSInt32Type, &pid);

    uint32_t scancode = IOHIDElementGetUsage(elem);

    if (scancode < 4 || scancode > 231) {
        return;
    }

    uint64_t pressed = IOHIDValueGetIntegerValue(value);
    if (pressed < 400) {
        return;
    }

    clearConsole();
    printf("pid: %d, page: %d, scancode: %d, pressed: %llu\n", pid, usage_page, scancode, pressed);
    printf("Bytes:\n");

    for (int i = 0; i < byte_count; i++) {
        printf("%5d " BYTE_TO_BINARY_PATTERN "\n", i, BYTE_TO_BINARY(bytes[i]));
    }
}

CFMutableDictionaryRef createHIDMatchingDict(uint32_t vid, uint32_t pid, uint32_t usagePage, uint32_t usage)
{
    CFMutableDictionaryRef dict = CFDictionaryCreateMutable(
        kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

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

int main(void)
{
    IOHIDManagerRef hidManager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);

    CFMutableDictionaryRef keypad = createHIDMatchingDict(Xkeys_XK16_VendorID,
        Xkeys_XK16_ProductID,
        Xkeys_XK16_UsagePage,
        Xkeys_XK16_Usage);

    IOHIDManagerSetDeviceMatching(hidManager, keypad);

    IOHIDManagerRegisterInputValueCallback(hidManager, callbackReceivedUserInput, NULL);

    IOHIDManagerScheduleWithRunLoop(hidManager, CFRunLoopGetMain(), kCFRunLoopDefaultMode);

    IOHIDManagerOpen(hidManager, kIOHIDOptionsTypeNone);

    clearConsole();
    printf("Press a key on your X-keys stick...\n");

    CFRunLoopRun();
}
