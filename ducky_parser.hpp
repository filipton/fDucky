#include <map>
#include <vector>

struct HID_output
{
  uint8_t keycode;
  uint8_t modifier;
};

std::map<String, u_int8_t> modifiersMap = {
    {"CTRL", KEYBOARD_MODIFIER_LEFTCTRL},
    {"CONTROL", KEYBOARD_MODIFIER_LEFTCTRL},
    {"SHIFT", KEYBOARD_MODIFIER_LEFTSHIFT},
    {"GUI", KEYBOARD_MODIFIER_LEFTGUI},
    {"WINDOWS", KEYBOARD_MODIFIER_LEFTGUI},
    {"ALT", KEYBOARD_MODIFIER_LEFTALT}};

std::map<String, u_int8_t> specialKeyCodeMap = {
    {"ESCAPE", 0x29},
    {"ESC", 0x29},
    {"ENTER", 0x28},
    {"TAB", 0x2B},
    {"BACKSPACE", 0x2A},
    {"SPACE", 0x2C},
    {"CAPSLOCK", 0x39},
    {"NUMLOCK", 0x53},
    {"SCROLLOCK", 0x47},
    {"F1", 0x3A},
    {"F2", 0x3B},
    {"F3", 0x3C},
    {"F4", 0x3D},
    {"F5", 0x3E},
    {"F6", 0x3F},
    {"F7", 0x40},
    {"F8", 0x41},
    {"F9", 0x42},
    {"F10", 0x43},
    {"F11", 0x44},
    {"F12", 0x45},
    {"UP", 0x52},
    {"UPARROW", 0x52},
    {"DOWN", 0x51},
    {"DOWNARROW", 0x51},
    {"LEFT", 0x50},
    {"LEFTARROW", 0x50},
    {"RIGHT", 0x4F},
    {"RIGHTARROW", 0x4F},
    {"PAGEUP", 0x4B},
    {"PAGEDOWN", 0x4E},
    {"HOME", 0x4A},
    {"END", 0x4D},
    {"INSERT", 0x49},
    {"DELETE", 0x4C},
    {"PAUSE", 0x48},
    {"BREAK", 0x48},
    {"PRINTSCREEN", 0x46},
    {"MENU", 0x76},
    {"APP", 0x65}};

std::map<char, HID_output> charToHID_output = {
    {' ', HID_output({0x2C, 0x00})},
    {'`', HID_output({0x35, 0x00})},
    {'-', HID_output({0x2D, 0x00})},
    {'=', HID_output({0x2E, 0x00})},
    {'[', HID_output({0x2F, 0x00})},
    {']', HID_output({0x30, 0x00})},
    {'\\', HID_output({0x31, 0x00})},
    {';', HID_output({0x33, 0x00})},
    {'\'', HID_output({0x34, 0x00})},
    {',', HID_output({0x36, 0x00})},
    {'.', HID_output({0x37, 0x00})},
    {'/', HID_output({0x38, 0x00})},
    {'\n', HID_output({0x28, 0x00})},
    {'\t', HID_output({0x2B, 0x00})},
    {'~', HID_output({0x35, KEYBOARD_MODIFIER_LEFTSHIFT})},
    {'!', HID_output({0x1E, KEYBOARD_MODIFIER_LEFTSHIFT})},
    {'@', HID_output({0x1F, KEYBOARD_MODIFIER_LEFTSHIFT})},
    {'#', HID_output({0x20, KEYBOARD_MODIFIER_LEFTSHIFT})},
    {'$', HID_output({0x21, KEYBOARD_MODIFIER_LEFTSHIFT})},
    {'%', HID_output({0x22, KEYBOARD_MODIFIER_LEFTSHIFT})},
    {'^', HID_output({0x23, KEYBOARD_MODIFIER_LEFTSHIFT})},
    {'&', HID_output({0x24, KEYBOARD_MODIFIER_LEFTSHIFT})},
    {'*', HID_output({0x25, KEYBOARD_MODIFIER_LEFTSHIFT})},
    {'(', HID_output({0x26, KEYBOARD_MODIFIER_LEFTSHIFT})},
    {')', HID_output({0x27, KEYBOARD_MODIFIER_LEFTSHIFT})},
    {'_', HID_output({0x2D, KEYBOARD_MODIFIER_LEFTSHIFT})},
    {'+', HID_output({0x2E, KEYBOARD_MODIFIER_LEFTSHIFT})},
    {'{', HID_output({0x2F, KEYBOARD_MODIFIER_LEFTSHIFT})},
    {'}', HID_output({0x30, KEYBOARD_MODIFIER_LEFTSHIFT})},
    {'|', HID_output({0x31, KEYBOARD_MODIFIER_LEFTSHIFT})},
    {':', HID_output({0x33, KEYBOARD_MODIFIER_LEFTSHIFT})},
    {'"', HID_output({0x34, KEYBOARD_MODIFIER_LEFTSHIFT})},
    {'<', HID_output({0x36, KEYBOARD_MODIFIER_LEFTSHIFT})},
    {'>', HID_output({0x37, KEYBOARD_MODIFIER_LEFTSHIFT})},
    {'?', HID_output({0x38, KEYBOARD_MODIFIER_LEFTSHIFT})}};

HID_output ParseCharToKeycode(char c)
{
  HID_output hid_outp;
  hid_outp.modifier = 0x00; // No modifier

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

  hid_outp = charToHID_output[c];
  return hid_outp;
}

/*
    Real parsing
*/
std::vector<HID_output> ParseStringToBuffer(String str)
{
  const char *cArr = str.c_str();
  int len = str.length();

  std::vector<HID_output> buffer;
  for (int i = 0; i < len; i++)
  {
    buffer.push_back(ParseCharToKeycode(cArr[i]));
  }

  return buffer;
}

HID_output ParseSpecialToken(String token)
{
  HID_output tmp;
  tmp.modifier = modifiersMap[token];
  tmp.keycode = specialKeyCodeMap[token];

  if (tmp.keycode == 0x00 && token.length() == 1)
  {
    tmp.keycode = ParseCharToKeycode(token[0]).keycode;
  }

  return tmp;
}

std::vector<String> splitStringBySeparator(String str, char separator)
{
  std::vector<String> tmp = {};
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