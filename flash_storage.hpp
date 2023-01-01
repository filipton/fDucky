#ifndef FLASH_STORAGE_HPP
#define FLASH_STORAGE_HPP

#include "rgb_led.hpp"

static Adafruit_FlashTransport_RP2040 flashTransport;
static Adafruit_SPIFlash flash(&flashTransport);

static FatFileSystem fatfs;
static FatFile root;

static Adafruit_USBD_MSC usb_msc;
static bool isFsFormatted;
static bool isFsChanged;

int32_t msc_read_cb(uint32_t lba, void *buffer, uint32_t bufsize)
{
  return flash.readBlocks(lba, (uint8_t *)buffer, bufsize / 512) ? bufsize : -1;
}
int32_t msc_write_cb(uint32_t lba, uint8_t *buffer, uint32_t bufsize)
{
  digitalWrite(LED_BUILTIN, HIGH);
  return flash.writeBlocks(lba, buffer, bufsize / 512) ? bufsize : -1;
}
void msc_flush_cb(void)
{
  flash.syncBlocks();
  fatfs.cacheClear();
  isFsChanged = true;

  digitalWrite(LED_BUILTIN, LOW);
}

void setupUsbStorage()
{
  usb_msc.setID("Logitech", "Keyboard", "1.0");
  usb_msc.setReadWriteCallback(msc_read_cb, msc_write_cb, msc_flush_cb);
  usb_msc.setCapacity(flash.size() / 512, 512);
  usb_msc.setUnitReady(true);
  usb_msc.begin();
}

void setupFlashStorage(bool programMode)
{
  if (programMode)
    setupUsbStorage();

  isFsFormatted = fatfs.begin(&flash);
  if (!isFsFormatted)
  {
    // Serial.println("ERROR: File system not formatted!");
    flashError(new bool[3]{false, false, false});
  }

  isFsChanged = true;
}

#endif