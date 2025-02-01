# 游戏 “何処へ行くの、あの日” 汉化项目
## 0x00 本项目仅供学习交流使用，无任何盈利，严禁商业或特殊用途!!!
![Image text](https://github.com/cokkeijigen/doko_iku_cn/blob/main/test.png)<br>

## 0x01 如何解包
**游戏目录下的`.pak`就是资源封包文件，其实这个封包的结构很简单，用`winhex`打开看个大概就能猜出结构<br>如下图：**
![Image text](https://github.com/cokkeijigen/doko_iku_cn/blob/main/file_head.png)<br>
**这是一个很经典的数据结构，头部存储文件信息，后面接着文件数据。通过观察头部的数据规律，很容易发现一个`entry`是`0x40`个字节，其中`name`占了`0x30`个字节，然后到`offset`和`length`为`int32`，~~至于为什么会有两个`offset`呢？我猜的我也不知道~~。<br><br>**
**然后你可能会发现每个封包的第0x00个引索`entry`的`name`都是`DATA$TOP`，这时候就能进行一个大胆的猜测，它的`length`有没有可能是用来记录着所有`entry`的数量？那么如何进行一个简单的验证呢？<br><br>**
**已知一个`entry`是`0x40`个字节，那么可以通过`DATA$TOP`的`length`乘以`0x40`计算出来第一个文件的地址，跳转到那个位置去看看。如上图`DATA$TOP`的`length`为`0x9A1`，乘以`0x40`得到`0x26840`，跳转过去看看：**
![Image text](https://github.com/cokkeijigen/doko_iku_cn/blob/main/file_data.png)<br>
**可以看到wav文件的magic了，这时候基本就能证实刚才的猜测。<br><br>如果还不太确定，可以从`0x26840`这个位置开始使用自定义选块，大小填写第`0x01`的`entry`的`length`也就是`0x6A9A`**<br>
![Image text](https://github.com/cokkeijigen/doko_iku_cn/blob/main/file_data1.png)<br>
**然后右键`复制选块`->`至新文件`，文件名就为`-select.wav`，然后保存，最后在用音乐播放器打开，如果能正常播放那就是再次证实了**
![Image text](https://github.com/cokkeijigen/doko_iku_cn/blob/main/file_data2.png)<br>
**到这里你已经学会了手动解包了，接下来只需要写一个批处理的程序即可。** <br>
```cpp
struct Entry {
    char name[0x30];
    int32_t offset;
    int32_t value1;
    int32_t length;
    int32_t value2;
};
struct PakHeader {
    Entry TOP;
    Entry DATA[NULL];
};
```
**解包：[PackManger.hpp](https://github.com/cokkeijigen/doko_iku_cn/blob/main/DOKOIKU/doko_iku_cn/PackManger.hpp)、[unpack_main.cpp](https://github.com/cokkeijigen/doko_iku_cn/blob/main/DOKOIKU/unpack/unpack_main.cpp)**<br>
**封包：[filepack_mian.cpp](https://github.com/cokkeijigen/doko_iku_cn/blob/main/DOKOIKU/filepack/filepack_mian.cpp)、[filepack.exe](https://github.com/cokkeijigen/doko_iku_cn/blob/main/tools/filepack.exe)(其实这个是我后面在游戏封包中发现的，可能是官方打包时不小心也一起打包进来了)**

## 0x02 如何显示GBK编码的字符以及支持半角字符
**这很简单，这游戏用到了`GetGlyphOutlineA`这个api，所以我们只需要去对它进行一个`hook`，使用`CreateFont`创建一个GBK字符集的字体再使用`SelectObject`设置`hdc`的字体即可。** <br><br>
**那么如何支持半角字符呢？先说结论，这游戏在解析脚本时，如果开头的第一个字符是半角的就会走宏或者内置操作符的解析逻辑，如果是全角的才走文本字符串的解析。<br>**
在这种情况下要直接去改它的解析逻辑有点麻烦，不现实。我们可以另辟蹊径，例如使用一些不常用的GBK字符来替换，然后再到`GetGlyphOutlineA`里再次做一个替换！那么，哪些字符比较合适替换呢？<br>
当然是直接使用SJIS编码的全角字符，它们在GBK编码下长这样：
`俙俛俠俢俤俥俧俫俬俰俲俴俵俶俷俹俻俼俽俿倀倁倂倃倄倅倎倐們倓倕倖倗倛倝倞倠倢倣値倧倫倯倰倱倲倳倴倵倶倷倸侾俀俁係俆俇俈俉俋侽亾仌丆丠丟丗鶹鶺乮乯丏両`
对应字符`ＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯＰＱＲＳＴＵＶＷＸＹＺａｂｃｄｅｆｇｈｉｊｋｌｍｎｏｐｑｒｓｔｕｖｗｘｙｚ１２３４５６７８９０％＆，？；：＇＂（）．！`。<br>
这些字符在中文中基本不可能用得到，用来做替换是最合适不过了。我们还可以举一反三，GBK编码的脚本也能支持显示一些特殊符号。<br>
不过不要急，游戏的字符宽度是固定，我通过ida的反编译分析，找到这个宽度就存储在`dword_4537F8`这个全局变量
![Image text](https://github.com/cokkeijigen/doko_iku_cn/blob/main/ida_code.png) <br>
我们只需要在每次调用`GetGlyphOutlineA`后设置字符宽度，于是我们最终得到以下代码
```cpp
// dllmain.cpp
static DWORD WINAPI GetGlyphOutlineA(HDC hdc, UINT uChar, UINT fuf, LPGLYPHMETRICS lpgm, DWORD cjbf, LPVOID pvbf, MAT2* lpmat) {
    if (uChar == 0xA1EC) {
        //uChar = 0x81F4; // 替换♪
        ::SelectObject(hdc, Hook::DefualtSymbolFont);
        return ::GetGlyphOutlineW(hdc, L'♪', fuf, lpgm, cjbf, pvbf, lpmat);
    }
    else if (uChar == 0xA7A4) {
        ::SelectObject(hdc, Hook::DefualtSymbolFont);
        return ::GetGlyphOutlineW(hdc, L'❤', fuf, lpgm, cjbf, pvbf, lpmat);
    }
    else if (uChar == 0xA7A5) {
        ::SelectObject(hdc, Hook::DefualtSymbolFont);
        return ::GetGlyphOutlineW(hdc, L'♡', fuf, lpgm, cjbf, pvbf, lpmat);
    }
    else {
        // 一些字符的替换
        if (uChar == 0x8140) uChar = 0xA1A1; // 全角空格
        else if (uChar == 0x8145) uChar = 0xA1A4;  // ·
        else if (uChar == 0x8175) uChar = 0xA1B8;  // 「
        else if (uChar == 0x8179) uChar = 0xA1BE;  // 【
        else uChar = Utils::UCharFull2Half(uChar); // 替换半角字符
        ::SelectObject(hdc, Hook::TextCharacterFont);
    }
    auto result = Patch::Hooker::Call<Hook::GetGlyphOutlineA>(hdc, uChar, fuf, lpgm, cjbf, pvbf, lpmat);
    *reinterpret_cast<int32_t*>(0x4537F8) = lpgm->gmCellIncX + 1; // 设置字符宽度，要半宽显示半角字符这点很重要！
    return result;
}
```
详细：[dllmain.cpp](https://github.com/cokkeijigen/doko_iku_cn/blob/main/DOKOIKU/doko_iku_cn/dllmain.cpp)、[utils.hpp](https://github.com/cokkeijigen/doko_iku_cn/blob/main/DOKOIKU/doko_iku_cn/utils.hpp)
---
![Image text](https://github.com/cokkeijigen/doko_iku_cn/blob/main/test1.png)
![Image text](https://github.com/cokkeijigen/doko_iku_cn/blob/main/test2.png)<br>
**非常奈斯** ![Image text](https://github.com/cokkeijigen/doko_iku_cn/blob/main/w.jpg)
