#include <Adafruit_NeoPixel.h>
#include "SPI.h"
#include "SdFat.h"
#include "Adafruit_SPIFlash.h"
#include "Adafruit_TinyUSB.h"
#include <vector>

#include "ducky_parser.hpp"

using namespace std;

#define PIXEL_PIN 12
#define PIXEL_POWER_PIN 11
Adafruit_NeoPixel rgb_led(1, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

/* ============================== HID STUFF ============================== */
uint8_t const desc_hid_report[] = {
    TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(1)),
    TUD_HID_REPORT_DESC_MOUSE(HID_REPORT_ID(2)),
    TUD_HID_REPORT_DESC_CONSUMER(HID_REPORT_ID(3))};
Adafruit_USBD_HID usb_hid(desc_hid_report, sizeof(desc_hid_report), HID_ITF_PROTOCOL_NONE, 2, false);

int32_t msc_read_cb(uint32_t lba, void *buffer, uint32_t bufsize);
int32_t msc_write_cb(uint32_t lba, uint8_t *buffer, uint32_t bufsize);
void msc_flush_cb(void);
/* ======================================================================== */

/* ================================= FLASH ================================= */
Adafruit_FlashTransport_RP2040 flashTransport;
Adafruit_SPIFlash flash(&flashTransport);

FatFileSystem fatfs;
FatFile root;
FatFile file;

Adafruit_USBD_MSC usb_msc;
bool isFsFormatted;
bool isFsChanged;
/* ======================================================================== */

void setupFlashStorage();
void flashError(int times, int delayTime);
vector<HID_output> ParseDuckyScriptLine(String line);

int keyDelay = 5;
int defaultDelay = 100;

vector<HID_output> hidBuffer = {};

FatFile payloadFile;
bool payloadEnded = false;

void setup()
{
  usb_hid.setStringDescriptor("Logitech Keyboard");
  usb_hid.begin();

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PIXEL_POWER_PIN, OUTPUT);
  digitalWrite(PIXEL_POWER_PIN, HIGH);

  rgb_led.begin();
  flash.begin();

  setupFlashStorage();

  root.open("/");
  if (!root.exists("payloads"))
  {
    flashError(5, 500);
  }

  if (!root.exists("test.txt") || !payloadFile.open("test.txt", O_READ))
  {
    flashError(3, 250);
  }

  if (root.isOpen())
    root.close();

  // hidBuffer = ParseDuckyScriptLine("STRING tesz");tesztesz
}

bool lastWasKey = false;
void loop()
{
  // if (test > 0) {
  //   for (int i = 0; i < 255; i++) {
  //     rgb_led.clear();
  //     rgb_led.setPixelColor(0, rgb_led.ColorHSV(i * 255));
  //     rgb_led.show();
  //     delay(10);
  //   }
  // }

  if (!payloadEnded && TinyUSBDevice.mounted() && usb_hid.ready())
  {
    if (hidBuffer.size() == 0)
    {
      payloadEnded = !payloadFile.available();

      String line = "";
      while (payloadFile.available())
      {
        char c = payloadFile.read();
        if (c == '\n')
          break;

        line += c;
      }

      hidBuffer = ParseDuckyScriptLine(line);
      delay(defaultDelay);
    }

    if (lastWasKey)
    {
      usb_hid.keyboardRelease(1);
      lastWasKey = false;
      delay(keyDelay);
    }
    if (hidBuffer.size() > 0)
    {
      uint8_t keycode[6] = {0};
      keycode[0] = hidBuffer[0].keycode;

      usb_hid.keyboardReport(1, hidBuffer[0].modifier, keycode);
      lastWasKey = true;

      hidBuffer.erase(hidBuffer.begin());
    }

    // delay(KEY_DELAY / 2);
  }
}

void setPixelColor(uint32_t color)
{
  rgb_led.clear();
  rgb_led.setPixelColor(0, color);
  rgb_led.show();
}

void flashError(int times, int delayTime)
{
  for (int i = 0; i < times; i++)
  {
    setPixelColor(rgb_led.Color(255, 0, 0));
    delay(delayTime / 2);
    setPixelColor(rgb_led.Color(0, 0, 0));
    delay(delayTime / 2);
  }
}

void setupUsbStorage()
{
  usb_msc.setID("Logitech", "Keyboard", "1.0");
  usb_msc.setReadWriteCallback(msc_read_cb, msc_write_cb, msc_flush_cb);
  usb_msc.setCapacity(flash.size() / 512, 512);
  usb_msc.setUnitReady(true);
  usb_msc.begin();
}

void setupFlashStorage()
{
  setupUsbStorage(); // <---- control here if you want to show usb device or not!

  isFsFormatted = fatfs.begin(&flash);
  if (!isFsFormatted)
  {
    // Serial.println("ERROR: File system not formatted!");
    flashError(3, 500);
  }

  isFsChanged = true;
}

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

vector<HID_output> ParseStringToBuffer(String str)
{
  const char *cArr = str.c_str();
  int len = str.length();

  vector<HID_output> buffer;
  for (int i = 0; i < len; i++)
  {
    buffer.push_back(ParseCharToKeycode(cArr[i]));
  }

  return buffer;
}

HID_output ParseSpecialToken(String token)
{
  HID_output tmp;
  tmp.modifier = GetModifier(token);
  tmp.keycode = GetSpecialKeyCode(token);

  if (tmp.keycode == 0x00 && token.length() == 1)
  {
    tmp.keycode = ParseCharToKeycode(token[0]).keycode;
  }

  return tmp;
}

vector<String> splitStringBySeparator(String str, char separator)
{
  vector<String> tmp = {};
  while (str.length() > 0)
  {
    int index = str.indexOf(separator);
    if (index == -1)
    {
      tmp.push_back(str);
      break;
    }

    tmp.push_back(str.substring(0, index));
    str = str.substring(index + 1);
  }

  return tmp;
}

vector<HID_output> ParseDuckyScriptLine(String line)
{
  if (line.startsWith("DELAY"))
  {
    line = line.substring(6); // Remove "DELAY " from string
    delay(line.toInt());
    return {};
  }
  else if (line.startsWith("STRINGLN"))
  {
    line = line.substring(9); // Remove "STRINGLN " from string
    return ParseStringToBuffer(line + '\n');
  }
  else if (line.startsWith("STRING"))
  {
    line = line.substring(7); // Remove "STRING " from string
    return ParseStringToBuffer(line);
  }
  else if (line.startsWith("DEFAULT_DELAY"))
  {
    line = line.substring(14); // Remove "DEFAULT_DELAY " from string
    defaultDelay = line.toInt();
    return {};
  }
  else if (line.startsWith("DEFAULTCHARDELAY"))
  {
    line = line.substring(17); // Remove "DEFAULTCHARDELAY " from string
    keyDelay = line.toInt();
    return {};
  }
  else
  {
    String token = "";
    vector<HID_output> tmpBuffer;
    tmpBuffer.push_back(HID_output({0x00, 0x00}));

    vector<String> tokens = splitStringBySeparator(line, ' ');
    for (int i = 0; i < tokens.size(); i++)
    {
      HID_output tmp = ParseSpecialToken(tokens[i]);

      tmp.keycode += tmpBuffer[0].keycode;
      tmp.modifier += tmpBuffer[0].modifier;
      tmpBuffer[0] = tmp;
    }

    return tmpBuffer;
  }

  return {};
}