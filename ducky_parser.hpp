struct HID_output
{
    uint8_t keycode;
    uint8_t modifier;
};

u_int8_t GetModifier(String token)
{
    if (token == "CTRL" || token == "CONTROL")
    {
        return KEYBOARD_MODIFIER_LEFTCTRL;
    }
    if (token == "SHIFT")
    {
        return KEYBOARD_MODIFIER_LEFTSHIFT;
    }
    if (token == "GUI" || token == "WINDOWS")
    {
        return KEYBOARD_MODIFIER_LEFTGUI;
    }
    if (token == "ALT")
    {
        return KEYBOARD_MODIFIER_LEFTALT;
    }

    return 0x00;
}

u_int8_t GetSpecialKeyCode(String token)
{
    if (token == "ESCAPE" || token == "ESC")
    {
        return 0x29;
    }
    else if (token == "ENTER")
    {
        return 0x28;
    }
    else if (token == "TAB")
    {
        return 0x2B;
    }
    else if (token == "BACKSPACE")
    {
        return 0x2A;
    }
    else if (token == "SPACE")
    {
        return 0x2C;
    }
    else if (token == "F1")
    {
        return 0x3A;
    }
    else if (token == "F2")
    {
        return 0x3B;
    }
    else if (token == "F3")
    {
        return 0x3C;
    }
    else if (token == "F4")
    {
        return 0x3D;
    }
    else if (token == "F5")
    {
        return 0x3E;
    }
    else if (token == "F6")
    {
        return 0x3F;
    }
    else if (token == "F7")
    {
        return 0x40;
    }
    else if (token == "F8")
    {
        return 0x41;
    }
    else if (token == "F9")
    {
        return 0x42;
    }
    else if (token == "F10")
    {
        return 0x43;
    }
    else if (token == "F11")
    {
        return 0x44;
    }
    else if (token == "F12")
    {
        return 0x45;
    }
    else if (token == "UP" || token == "UPARROW")
    {
        return 0x52;
    }
    else if (token == "DOWN" || token == "DOWNARROW")
    {
        return 0x51;
    }
    else if (token == "LEFT" || token == "LEFTARROW")
    {
        return 0x50;
    }
    else if (token == "RIGHT" || token == "RIGHTARROW")
    {
        return 0x4F;
    }
    else if (token == "PAGEUP")
    {
        return 0x4B;
    }
    else if (token == "PAGEDOWN")
    {
        return 0x4E;
    }
    else if (token == "HOME")
    {
        return 0x4A;
    }
    else if (token == "END")
    {
        return 0x4D;
    }
    else if (token == "INSERT")
    {
        return 0x49;
    }
    else if (token == "DELETE")
    {
        return 0x4C;
    }
    else if (token == "PAUSE" || token == "BREAK")
    {
        return 0x48;
    }
    else if (token == "PRINTSCREEN")
    {
        return 0x46;
    }
    else if (token == "MENU")
    {
        return 0x76;
    }
    else if (token == "APP")
    {
        return 0x65;
    }
    else if (token == "+")
    {
        return 0x2E;
    }

    return 0x00;
}

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

    switch (c)
    {
    case ' ':
        hid_outp.keycode = 0x2C;
        break;
    case '`':
        hid_outp.keycode = 0x35;
        break;
    case '~':
        hid_outp.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
        hid_outp.keycode = 0x35;
        break;
    case '!':
        hid_outp.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
        hid_outp.keycode = 0x1E;
        break;
    case '@':
        hid_outp.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
        hid_outp.keycode = 0x1F;
        break;
    case '#':
        hid_outp.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
        hid_outp.keycode = 0x20;
        break;
    case '$':
        hid_outp.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
        hid_outp.keycode = 0x21;
        break;
    case '%':
        hid_outp.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
        hid_outp.keycode = 0x22;
        break;
    case '^':
        hid_outp.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
        hid_outp.keycode = 0x23;
        break;
    case '&':
        hid_outp.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
        hid_outp.keycode = 0x24;
        break;
    case '*':
        hid_outp.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
        hid_outp.keycode = 0x25;
        break;
    case '(':
        hid_outp.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
        hid_outp.keycode = 0x26;
        break;
    case ')':
        hid_outp.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
        hid_outp.keycode = 0x27;
        break;
    case '-':
        hid_outp.keycode = 0x2D;
        break;
    case '_':
        hid_outp.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
        hid_outp.keycode = 0x2D;
        break;
    case '=':
        hid_outp.keycode = 0x2E;
        break;
    case '+':
        hid_outp.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
        hid_outp.keycode = 0x2E;
        break;
    case '[':
        hid_outp.keycode = 0x2F;
        break;
    case '{':
        hid_outp.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
        hid_outp.keycode = 0x2F;
        break;
    case ']':
        hid_outp.keycode = 0x30;
        break;
    case '}':
        hid_outp.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
        hid_outp.keycode = 0x30;
        break;
    case '\\':
        hid_outp.keycode = 0x31;
        break;
    case '|':
        hid_outp.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
        hid_outp.keycode = 0x31;
        break;
    case ';':
        hid_outp.keycode = 0x33;
        break;
    case ':':
        hid_outp.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
        hid_outp.keycode = 0x33;
        break;
    case '\'':
        hid_outp.keycode = 0x34;
        break;
    case '\"':
        hid_outp.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
        hid_outp.keycode = 0x34;
        break;
    case ',':
        hid_outp.keycode = 0x36;
        break;
    case '<':
        hid_outp.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
        hid_outp.keycode = 0x36;
        break;
    case '.':
        hid_outp.keycode = 0x37;
        break;
    case '>':
        hid_outp.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
        hid_outp.keycode = 0x37;
        break;
    case '/':
        hid_outp.keycode = 0x38;
        break;
    case '?':
        hid_outp.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
        hid_outp.keycode = 0x38;
        break;
    case '\n':
        hid_outp.keycode = 0x28;
        break;
    default:
        hid_outp.keycode = 0x00;
        break;
    }

    return hid_outp;
}