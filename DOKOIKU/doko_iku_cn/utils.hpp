
namespace Utils {

    static UINT UCharFull2Half(UINT uChar) {
        switch (uint16_t(uChar)) {
        case 0xA1A1: return 0x20; // 'กก' -> ' '
        case 0x8260: return 0x41; // '`' -> 'A'
        case 0x8261: return 0x42; // 'a' -> 'B'
        case 0x8262: return 0x43; // 'b' -> 'C'
        case 0x8263: return 0x44; // 'c' -> 'D'
        case 0x8264: return 0x45; // 'd' -> 'E'
        case 0x8265: return 0x46; // 'e' -> 'F'
        case 0x8266: return 0x47; // 'f' -> 'G'
        case 0x8267: return 0x48; // 'g' -> 'H'
        case 0x8268: return 0x49; // 'h' -> 'I'
        case 0x8269: return 0x4A; // 'i' -> 'J'
        case 0x826A: return 0x4B; // 'j' -> 'K'
        case 0x826B: return 0x4C; // 'k' -> 'L'
        case 0x826C: return 0x4D; // 'l' -> 'M'
        case 0x826D: return 0x4E; // 'm' -> 'N'
        case 0x826E: return 0x4F; // 'n' -> 'O'
        case 0x826F: return 0x50; // 'o' -> 'P'
        case 0x8270: return 0x51; // 'p' -> 'Q'
        case 0x8271: return 0x52; // 'q' -> 'R'
        case 0x8272: return 0x53; // 'r' -> 'S'
        case 0x8273: return 0x54; // 's' -> 'T'
        case 0x8274: return 0x55; // 't' -> 'U'
        case 0x8275: return 0x56; // 'u' -> 'V'
        case 0x8276: return 0x57; // 'v' -> 'W'
        case 0x8277: return 0x58; // 'w' -> 'X'
        case 0x8278: return 0x59; // 'x' -> 'Y'
        case 0x8279: return 0x5A; // 'y' -> 'Z'
        case 0x8281: return 0x61; // '' -> 'a'
        case 0x8282: return 0x62; // '' -> 'b'
        case 0x8283: return 0x63; // '' -> 'c'
        case 0x8284: return 0x64; // '' -> 'd'
        case 0x8285: return 0x65; // '' -> 'e'
        case 0x8286: return 0x66; // '' -> 'f'
        case 0x8287: return 0x67; // '' -> 'g'
        case 0x8288: return 0x68; // '' -> 'h'
        case 0x8289: return 0x69; // '' -> 'i'
        case 0x828A: return 0x6A; // '' -> 'j'
        case 0x828B: return 0x6B; // '' -> 'k'
        case 0x828C: return 0x6C; // '' -> 'l'
        case 0x828D: return 0x6D; // '' -> 'm'
        case 0x828E: return 0x6E; // '' -> 'n'
        case 0x828F: return 0x6F; // '' -> 'o'
        case 0x8290: return 0x70; // '' -> 'p'
        case 0x8291: return 0x71; // '' -> 'q'
        case 0x8292: return 0x72; // '' -> 'r'
        case 0x8293: return 0x73; // '' -> 's'
        case 0x8294: return 0x74; // '' -> 't'
        case 0x8295: return 0x75; // '' -> 'u'
        case 0x8296: return 0x76; // '' -> 'v'
        case 0x8297: return 0x77; // '' -> 'w'
        case 0x8298: return 0x78; // '' -> 'x'
        case 0x8299: return 0x79; // '' -> 'y'
        case 0x829A: return 0x7A; // '' -> 'z'
        case 0x8250: return 0x31; // 'P' -> '1'
        case 0x8251: return 0x32; // 'Q' -> '2'
        case 0x8252: return 0x33; // 'R' -> '3'
        case 0x8253: return 0x34; // 'S' -> '4'
        case 0x8254: return 0x35; // 'T' -> '5'
        case 0x8255: return 0x36; // 'U' -> '6'
        case 0x8256: return 0x37; // 'V' -> '7'
        case 0x8257: return 0x38; // 'W' -> '8'
        case 0x8258: return 0x39; // 'X' -> '9'
        case 0x824F: return 0x30; // 'O' -> '0'
        case 0x8193: return 0x25; // '' -> '%'
        case 0x8195: return 0x26; // '' -> '&'
        case 0x8143: return 0x2C; // 'C' -> ','
        case 0x8148: return 0x3F; // 'H' -> '?'
        case 0x8147: return 0x3B; // 'G' -> ';'
        case 0x8146: return 0x3A; // 'F' -> ':'
        case 0xFA56: return 0x27; // '๚V' -> '''
        case 0xFA57: return 0x22; // '๚W' -> '"'
        case 0x8169: return 0x28; // 'i' -> '('
        case 0x816A: return 0x29; // 'j' -> ')'
        case 0x8144: return 0x2E; // 'D' -> '.'
        case 0x8149: return 0x21; // 'I' -> '!'
        default: return NULL;
        }
    }
}