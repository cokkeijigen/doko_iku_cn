import os

ipt_text: str = './doko_iku_text'
org_text: str = './old/txt'
out_text: str = './new'

def formater(text: str) -> str:
    min_length: int = 22;
    beg_symbols: str = "。、？’”，！～】；：）」』"
    end_symbols: str = "（(「『【‘“…";
    text = text.replace('\\n　', '')
    text = text.replace('\\n', '')
    text = text.replace("/", "／")
    text = text.replace("{", "｛")
    text = text.replace("}", "｝")
    is_talking: bool = False;
    rtext: str = text
    if rtext.endswith('@') or rtext.endswith('＠'):
        rtext = rtext[:-1]
    if rtext.startswith('「') and rtext.endswith('」'):
        is_talking = True
    elif rtext.startswith('『') and rtext.endswith('』'):
        is_talking = True
    elif rtext.startswith('“') and rtext.endswith('”'):
        is_talking = True
    length: int = len(text)
    add_new_line: bool = False
    add_w_count: int = 0
    can_getc: bool = True
    index: int = 0
    result: str = ''
    char: str = ''
    while index < length:
        if can_getc: char = text[index]
        if not can_getc or (add_new_line and (char not in beg_symbols)):
            result += '\n'
            if is_talking: result += '　'
            add_w_count = int(is_talking)
            if not can_getc:
                can_getc = True
                continue
        if char !=  "｛" and char !=  "｝" and char != "＠" and char != "@":
            add_w_count += 1
        result += char
        add_new_line = add_w_count >= min_length
        if add_new_line and char in end_symbols:
            can_getc = False
        index += 1
    return result

def file_read(path: str, encoding: str) -> list[str]:
    result: list[str]
    with open(path, 'r',  encoding=encoding) as f:
        result = f.readlines()
    return result

def file_write(path: str, data: list[str] ,encoding: str) -> None:
    with open(path, 'w',  encoding=encoding) as f:
        f.write(''.join(data))
    pass

def text_parse(text_array: list[str]) -> list[list[str]]:
    result: list[list[str | list[list[any]]]] = []
    block_text: list[list[any]] = []
    read_state: bool = False
    start_num  = ''
    start_line = ''
    for text in text_array:
        text = text.strip()
        try:
            if not read_state and '#' in text and 'block-start:' in text:
                block_start = text.split('#')
                start_num = block_start[1].strip()
                start_line = block_start[0].split(':')[1].strip()[:-1]
                read_state = True
                # print('start: ' + start_line)
            elif read_state and '#' in text and 'block-end:' in text:
                block_end = text.split('#')
                end_num = block_end[1].strip()
                end_line = block_end[0].split(':')[1].strip()[:-1]
                block = f"{start_line}:{end_line}"
                result.append([block, block_text])
                block_text = []
                start_line = ''
                start_num  = ''
                read_state = False
            elif read_state and '|●●|' in text:
                text = text.split('|●●|')
                if len(text) > 0:
                    num = text[0].split('|')[1].strip()
                    text = text[1]
                    if len(text) > 22: text = formater(text)
                    text = text.replace('｛', '乷')
                    text = text.replace('｝', '乸')
                    text = text.replace('@', '仐')
                    text = text.replace('＠', '仐')
                    text = text.replace('「', '乽')
                    text = text.replace('・', '·')
                    text = text.replace('♪', '§')
                    text = text.replace('〜', '～')
                    text = text.replace('−', '—')
                    block_text.append([int(num), text + '\n'])
        except: pass
    return result

def text_com(cn_text: list[list[str]], old_text: list[str]) -> list[str]:
    for block in cn_text:
        if len(block) != 2 : continue
        line_num: list[str] = block[0].split(':')
        start: int = int(line_num[0])
        end: int   = int(line_num[1])
        text_list: list[list[int|str]] = block[1]
        for i in range(start, end + 1):
            cur_text: str = old_text[i]
            if '#' in cur_text or '/' in cur_text or '％' in cur_text or '*' in cur_text or 'GOTO' in cur_text or '$' in cur_text or ('亂' in cur_text and '亃' in cur_text ): continue
            old_text[i] = None
        offset: int = 0
        for text in text_list:
            tlen = int(text[0])
            text = str(text[1])
            if '【' in text and '】' in text:
                continue
            if old_text[start - 1 + offset] != None:
                while True:
                    offset += 1
                    if old_text[start - 1 + offset] == None:
                        break
                pass
            old_text[start - 1 + offset] = text;
            offset += tlen
    result = [text for text in old_text if text != None]
    return result

if __name__ == '__main__':
    for f in os.listdir(ipt_text):
        old_text_array: list[str] = file_read(f'{org_text}/{f}', 'gbk')
        new_text_array: list[str] = file_read(f'{ipt_text}/{f}', 'utf-8')
        parsed_text: list[list[str]]  = text_parse(new_text_array)
        result: list[str] = text_com(parsed_text, old_text_array)
        try:
            file_write(f'{out_text}/{f}', result, 'gbk')
            print(f)
        except Exception as err:
            print(f"{f}: {str(err)}")
    # print(result)