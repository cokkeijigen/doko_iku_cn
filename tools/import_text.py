import os

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
                    # text = text.replace('＠', '仐')
                    # text = text.replace('@', '仐')
                    # text = text.replace('【', '亂')
                    # text = text.replace('】', '亃')
                    # text = text.replace('｛', '乷')
                    # text = text.replace('｝', '乸')
                    text = text.replace('「', '乽')
                    text = text.replace('@', '＠')
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
            if '#' in cur_text or '/' in cur_text or '％' in cur_text or '*' in cur_text or 'GOTO' in cur_text or '$' in cur_text: continue
            old_text[i] = None
        text_map: dict[int, str] = {}
        for text in text_list:
            num  = text[0]
            text = text[1]
            t_val = text_map.get(num)
            if t_val != None:
                t_val += text
            else:
                t_val = text
            text_map[num] = t_val
        offset: int = 0
        for num, text in text_map.items():
            if old_text[start - 1 + num + offset] != None:
                while True:
                    offset += 1
                    if old_text[start - 1 + num + offset] == None:
                        break
            old_text[start - 1 + num + offset] = text;
    result = [text for text in old_text if text != None]
    return result

if __name__ == '__main__':
    for f in os.listdir('./doko_iku_text/'):
        old_text_array: list[str] = file_read(f'./old/txt/{f}', 'gbk')
        new_text_array: list[str] = file_read(f'./doko_iku_text/{f}', 'utf-8')
        parsed_text: list[list[str]]  = text_parse(new_text_array)
        result: list[str] = text_com(parsed_text, old_text_array)
        try:
            file_write(f'./new/{f}', result, 'gbk')
            print(f)
        except Exception as err:
            print(f"{f}: {str(err)}")
    # print(result)