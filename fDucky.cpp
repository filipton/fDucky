#include <Adafruit_NeoPixel.h>
#include "SPI.h"
#include "SdFat.h"
#include "Adafruit_SPIFlash.h"
#include "Adafruit_TinyUSB.h"
#include <vector>

using namespace std;

#define PIXEL_PIN 12
#define PIXEL_POWER_PIN 11

Adafruit_NeoPixel rgb_led(1, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

Adafruit_FlashTransport_RP2040 flashTransport;
Adafruit_SPIFlash flash(&flashTransport);

FatFileSystem fatfs;
FatFile root;
FatFile file;

Adafruit_USBD_MSC usb_msc;
bool isFsFormatted;
bool isFsChanged;

int test = -1;

enum
{
  RID_KEYBOARD = 1,
  RID_MOUSE,
  RID_CONSUMER_CONTROL
};
struct HID_output
{
  uint8_t keycode;
  uint8_t modifier;
};

uint8_t const desc_hid_report[] = {
    TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(RID_KEYBOARD)),
    TUD_HID_REPORT_DESC_MOUSE(HID_REPORT_ID(RID_MOUSE)),
    TUD_HID_REPORT_DESC_CONSUMER(HID_REPORT_ID(RID_CONSUMER_CONTROL))};

Adafruit_USBD_HID usb_hid(desc_hid_report, sizeof(desc_hid_report), HID_ITF_PROTOCOL_NONE, 2, false);
Adafruit_USBD_CDC USBSer1;

void setupFlashStorage();
void flashError(int times, int delayTime);

int32_t msc_read_cb(uint32_t lba, void *buffer, uint32_t bufsize);
int32_t msc_write_cb(uint32_t lba, uint8_t *buffer, uint32_t bufsize);
void msc_flush_cb(void);

vector<HID_output> ParseDuckyScriptLine(String line);
vector<HID_output> hidOutputs = {};

void setup()
{
  // Serial.begin(115200);
  // delay(1000);
  // Serial.println("Starting up...");
  // delay(1000);
  // Serial.println("Starting up...");
  // delay(1000);
  // Serial.println("Starting up...");
  // delay(1000);
  // Serial.println("Starting up...");

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

  FatFile testFile;
  if (root.exists("test.txt") && testFile.open("test.txt", O_READ))
  {
    String fileContents = "";

    while (testFile.available())
    {
      fileContents += (char)testFile.read();
    }

    fileContents.trim();
    test = fileContents.toInt();
    testFile.close();
  }

  if (root.isOpen())
    root.close();

  hidOutputs = ParseDuckyScriptLine("STRING tesz");
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

  if (TinyUSBDevice.mounted() && usb_hid.ready())
  {
    if (BOOTSEL)
    {
      if (lastWasKey)
      {
        usb_hid.keyboardRelease(RID_KEYBOARD);
        lastWasKey = false;
        delay(5);
      }

      if(hidOutputs.size() > 0)
      {
        // flashError(2, 500);
        // Serial.println(hidOutputs[i].keycode);
        // Serial.println(hidOutputs[i].modifier);

        uint8_t keycode[6] = {0};
        keycode[0] = hidOutputs[0].keycode;

        usb_hid.keyboardReport(RID_KEYBOARD, hidOutputs[0].modifier, keycode);
        lastWasKey = true;

        hidOutputs.erase(hidOutputs.begin());
      }

      delay(5);
    }
  }
}

void flashError(int times, int delayTime)
{
  for (int i = 0; i < times; i++)
  {
    rgb_led.clear();
    rgb_led.setPixelColor(0, rgb_led.Color(255, 0, 0));
    rgb_led.show();
    delay(delayTime / 2);

    rgb_led.clear();
    rgb_led.setPixelColor(0, rgb_led.Color(0, 0, 0));
    rgb_led.show();
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

u_int8_t GetSpecialKeyCode(String str)
{
  if (str == "ESCAPE" || str == "ESC")
  {
    return 0x29;
  }
  else if (str == "ENTER")
  {
    return 0x28;
  }
  else if (str == "TAB")
  {
    return 0x2B;
  }
  else if (str == "CTRL" || str == "CONTROL")
  {
    return 0xE0;
  }
  else if (str == "SHIFT")
  {
    return 0xE1;
  }
  else if (str == "GUI" || str == "WINDOWS")
  {
    return 0xE3;
  }
  else if (str == "ALT")
  {
    return 0xE2;
  }
  else if (str == "BACKSPACE")
  {
    return 0x2A;
  }
  else if (str == "SPACE")
  {
    return 0x2C;
  }
  else if (str == "F1")
  {
    return 0x3A;
  }
  else if (str == "F2")
  {
    return 0x3B;
  }
  else if (str == "F3")
  {
    return 0x3C;
  }
  else if (str == "F4")
  {
    return 0x3D;
  }
  else if (str == "F5")
  {
    return 0x3E;
  }
  else if (str == "F6")
  {
    return 0x3F;
  }
  else if (str == "F7")
  {
    return 0x40;
  }
  else if (str == "F8")
  {
    return 0x41;
  }
  else if (str == "F9")
  {
    return 0x42;
  }
  else if (str == "F10")
  {
    return 0x43;
  }
  else if (str == "F11")
  {
    return 0x44;
  }
  else if (str == "F12")
  {
    return 0x45;
  }
  else if (str == "UP" || str == "UPARROW")
  {
    return 0x52;
  }
  else if (str == "DOWN" || str == "DOWNARROW")
  {
    return 0x51;
  }
  else if (str == "LEFT" || str == "LEFTARROW")
  {
    return 0x50;
  }
  else if (str == "RIGHT" || str == "RIGHTARROW")
  {
    return 0x4F;
  }
  else if (str == "PAGEUP")
  {
    return 0x4B;
  }
  else if (str == "PAGEDOWN")
  {
    return 0x4E;
  }
  else if (str == "HOME")
  {
    return 0x4A;
  }
  else if (str == "END")
  {
    return 0x4D;
  }
  else if (str == "INSERT")
  {
    return 0x49;
  }
  else if (str == "DELETE")
  {
    return 0x4C;
  }
  else if (str == "PAUSE" || str == "BREAK")
  {
    return 0x48;
  }
  else if (str == "PRINTSCREEN")
  {
    return 0x46;
  }
  else if (str == "MENU")
  {
    return 0x76;
  }
  else if (str == "APP")
  {
    return 0x65;
  }
  else if (str == "+")
  {
    return 0x2E;
  }

  return 0x00;
}

auto ParseCharToKeycode(char c)
{
  HID_output hid_outp;
  int c_code = (int)c;

  if (c_code >= 65 && c_code <= 90) // [A-Z]
  {
    hid_outp.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
    hid_outp.keycode = c_code - 61;
    return hid_outp;
  }

  if (c_code >= 97 && c_code <= 122) // [a-z]
  {
    hid_outp.keycode = c_code - 93;
    return hid_outp;
  }

  if (c_code >= 49 && c_code <= 57) // [1-9]
  {
    hid_outp.keycode = c_code - 19;
    return hid_outp;
  }

  if (c_code == 48) // 0
  {
    hid_outp.keycode = 0x27;
    return hid_outp;
  }

  switch (c)
  {
  case ' ':
    hid_outp.keycode = 0x2C;
    return hid_outp;
  case '`':
    hid_outp.keycode = 0x35;
    return hid_outp;
  case '~':
    hid_outp.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
    hid_outp.keycode = 0x35;
    return hid_outp;
  case '!':
    hid_outp.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
    hid_outp.keycode = 0x1E;
    return hid_outp;
  case '@':
    hid_outp.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
    hid_outp.keycode = 0x1F;
    return hid_outp;
  case '#':
    hid_outp.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
    hid_outp.keycode = 0x20;
    return hid_outp;
  case '$':
    hid_outp.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
    hid_outp.keycode = 0x21;
    return hid_outp;
  case '%':
    hid_outp.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
    hid_outp.keycode = 0x22;
    return hid_outp;
  case '^':
    hid_outp.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
    hid_outp.keycode = 0x23;
    return hid_outp;
  case '&':
    hid_outp.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
    hid_outp.keycode = 0x24;
    return hid_outp;
  case '*':
    hid_outp.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
    hid_outp.keycode = 0x25;
    return hid_outp;
  case '(':
    hid_outp.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
    hid_outp.keycode = 0x26;
    return hid_outp;
  case ')':
    hid_outp.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
    hid_outp.keycode = 0x27;
    return hid_outp;
  case '-':
    hid_outp.keycode = 0x2D;
    return hid_outp;
  case '_':
    hid_outp.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
    hid_outp.keycode = 0x2D;
    return hid_outp;
  case '=':
    hid_outp.keycode = 0x2E;
    return hid_outp;
  case '+':
    hid_outp.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
    hid_outp.keycode = 0x2E;
    return hid_outp;
  case '[':
    hid_outp.keycode = 0x2F;
    return hid_outp;
  case '{':
    hid_outp.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
    hid_outp.keycode = 0x2F;
    return hid_outp;
  case ']':
    hid_outp.keycode = 0x30;
    return hid_outp;
  case '}':
    hid_outp.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
    hid_outp.keycode = 0x30;
    return hid_outp;
  case '\\':
    hid_outp.keycode = 0x31;
    return hid_outp;
  case '|':
    hid_outp.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
    hid_outp.keycode = 0x31;
    return hid_outp;
  case ';':
    hid_outp.keycode = 0x33;
    return hid_outp;
  case ':':
    hid_outp.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
    hid_outp.keycode = 0x33;
    return hid_outp;
  case '\'':
    hid_outp.keycode = 0x34;
    return hid_outp;
  case '\"':
    hid_outp.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
    hid_outp.keycode = 0x34;
    return hid_outp;
  case ',':
    hid_outp.keycode = 0x36;
    return hid_outp;
  case '<':
    hid_outp.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
    hid_outp.keycode = 0x36;
    return hid_outp;
  case '.':
    hid_outp.keycode = 0x37;
    return hid_outp;
  case '>':
    hid_outp.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
    hid_outp.keycode = 0x37;
    return hid_outp;
  case '/':
    hid_outp.keycode = 0x38;
    return hid_outp;
  case '?':
    hid_outp.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
    hid_outp.keycode = 0x38;
    return hid_outp;
  case '\n':
    hid_outp.keycode = 0x28;
    return hid_outp;
  }

  hid_outp.keycode = 0x00;
  return hid_outp;
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

vector<HID_output> ParseSpecialToken(String token)
{
  u_int8_t keyCode = GetSpecialKeyCode(token);
  vector<HID_output> buffer;
  buffer.push_back(HID_output());

  if (keyCode != 0x00)
  {
    buffer[0].keycode = keyCode;
  }
  else
  {
    // MODIFIER KEY + [a-z] (also [A-Z] but its chaned back to [a-z])
    if (token.length() == 1)
    {
      buffer[0].keycode = token[0] - (token[0] >= 97 ? 93 : 61);
    }
  }

  return buffer;
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
    // line = line.substring(14); // Remove "DEFAULT_DELAY " from string
    // defaultDelay = line.toInt();
    return {};
  }
  else
  {
    // int keycodeOffset = 0;
    // String token = "";

    // for (int i = 0; i < line.length(); i++) {
    //   if (line[i] == ' ' || line[i] == '-') {
    //     ParseSpecialToken(token, keycodeOffset++, buffer);
    //     token = "";

    //     continue;
    //   }

    //   token = token + line[i];
    // }

    // if (token.length() != 0) {
    //   ParseSpecialToken(token, keycodeOffset++, buffer);
    // }

    return {};
  }

  return {};
}