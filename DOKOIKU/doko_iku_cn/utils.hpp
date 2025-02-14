#pragma once

namespace Utils {

    static DWORD OsCurrentCodePage{ ::GetACP() };

    static std::wstring ConvertToUTF16(const char* str, uint32_t cdpg = 936) {
        if(str) {
            auto&& result = std::wstring(::MultiByteToWideChar(cdpg, 0, str, -1, 0, 0), 0);
            ::MultiByteToWideChar(cdpg, 0, str, -1, const_cast<wchar_t*>(result.c_str()), result.size());
            return result;
        }
        return L"";
    }

    static UINT UCharFull2Half(UINT uChar) {
        switch (uint16_t(uChar)) {
        case 0xA1A1: return 0x20; // '　' -> ' '
        case 0x8260: return 0x41; // '俙' -> 'A'
        case 0x8261: return 0x42; // '俛' -> 'B'
        case 0x8262: return 0x43; // '俠' -> 'C'
        case 0x8263: return 0x44; // '俢' -> 'D'
        case 0x8264: return 0x45; // '俤' -> 'E'
        case 0x8265: return 0x46; // '俥' -> 'F'
        case 0x8266: return 0x47; // '俧' -> 'G'
        case 0x8267: return 0x48; // '俫' -> 'H'
        case 0x8268: return 0x49; // '俬' -> 'I'
        case 0x8269: return 0x4A; // '俰' -> 'J'
        case 0x826A: return 0x4B; // '俲' -> 'K'
        case 0x826B: return 0x4C; // '俴' -> 'L'
        case 0x826C: return 0x4D; // '俵' -> 'M'
        case 0x826D: return 0x4E; // '俶' -> 'N'
        case 0x826E: return 0x4F; // '俷' -> 'O'
        case 0x826F: return 0x50; // '俹' -> 'P'
        case 0x8270: return 0x51; // '俻' -> 'Q'
        case 0x8271: return 0x52; // '俼' -> 'R'
        case 0x8272: return 0x53; // '俽' -> 'S'
        case 0x8273: return 0x54; // '俿' -> 'T'
        case 0x8274: return 0x55; // '倀' -> 'U'
        case 0x8275: return 0x56; // '倁' -> 'V'
        case 0x8276: return 0x57; // '倂' -> 'W'
        case 0x8277: return 0x58; // '倃' -> 'X'
        case 0x8278: return 0x59; // '倄' -> 'Y'
        case 0x8279: return 0x5A; // '倅' -> 'Z'
        case 0x8281: return 0x61; // '倎' -> 'a'
        case 0x8282: return 0x62; // '倐' -> 'b'
        case 0x8283: return 0x63; // '們' -> 'c'
        case 0x8284: return 0x64; // '倓' -> 'd'
        case 0x8285: return 0x65; // '倕' -> 'e'
        case 0x8286: return 0x66; // '倖' -> 'f'
        case 0x8287: return 0x67; // '倗' -> 'g'
        case 0x8288: return 0x68; // '倛' -> 'h'
        case 0x8289: return 0x69; // '倝' -> 'i'
        case 0x828A: return 0x6A; // '倞' -> 'j'
        case 0x828B: return 0x6B; // '倠' -> 'k'
        case 0x828C: return 0x6C; // '倢' -> 'l'
        case 0x828D: return 0x6D; // '倣' -> 'm'
        case 0x828E: return 0x6E; // '値' -> 'n'
        case 0x828F: return 0x6F; // '倧' -> 'o'
        case 0x8290: return 0x70; // '倫' -> 'p'
        case 0x8291: return 0x71; // '倯' -> 'q'
        case 0x8292: return 0x72; // '倰' -> 'r'
        case 0x8293: return 0x73; // '倱' -> 's'
        case 0x8294: return 0x74; // '倲' -> 't'
        case 0x8295: return 0x75; // '倳' -> 'u'
        case 0x8296: return 0x76; // '倴' -> 'v'
        case 0x8297: return 0x77; // '倵' -> 'w'
        case 0x8298: return 0x78; // '倶' -> 'x'
        case 0x8299: return 0x79; // '倷' -> 'y'
        case 0x829A: return 0x7A; // '倸' -> 'z'
        case 0x8250: return 0x31; // '侾' -> '1'
        case 0x8251: return 0x32; // '俀' -> '2'
        case 0x8252: return 0x33; // '俁' -> '3'
        case 0x8253: return 0x34; // '係' -> '4'
        case 0x8254: return 0x35; // '俆' -> '5'
        case 0x8255: return 0x36; // '俇' -> '6'
        case 0x8256: return 0x37; // '俈' -> '7'
        case 0x8257: return 0x38; // '俉' -> '8'
        case 0x8258: return 0x39; // '俋' -> '9'
        case 0x824F: return 0x30; // '侽' -> '0'
        case 0x8193: return 0x25; // '亾' -> '%'
        case 0x8195: return 0x26; // '仌' -> '&'
        case 0x8143: return 0x2C; // '丆' -> ','
        case 0x8148: return 0x3F; // '丠' -> '?'
        case 0x8147: return 0x3B; // '丟' -> ';'
        case 0x8146: return 0x3A; // '丗' -> ':'
        case 0xFA56: return 0x27; // '鶹' -> '''
        case 0xFA57: return 0x22; // '鶺' -> '"'
        case 0x8169: return 0x28; // '乮' -> '('
        case 0x816A: return 0x29; // '乯' -> ')'
        case 0x8144: return 0x2E; // '丏' -> '.'
        case 0x8149: return 0x21; // '両' -> '!'
        default: return uChar;
        }
    }
}