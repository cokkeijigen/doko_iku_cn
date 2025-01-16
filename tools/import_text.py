import os, sys
from pprint import pprint


def to_full_char(text: str) -> str:
    half: str = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890%&,?;:'\"()"
    full: str ="ＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯＰＱＲＳＴＵＶＷＸＹＺａｂｃｄｅｆｇｈｉｊｋｌｍｎｏｐｑｒｓｔｕｖｗｘｙｚ１２３４５６７８９０％＆，？；：＇＂（）"
    for i in range(0, len(half)):
        text = text.replace(half[i], full[i])
    return text

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
    print(''.join(data))
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, 'w',  encoding=encoding) as f:
        f.write(''.join(data))
    pass

def text_parse(text_array: list[str]) -> list[list[str]]:
    result: list[list[str | list[list[any]]]] = []
    block_text: list[list[any]] = []
    read_state: bool = False
    start_num  = ''
    start_line = ''
    line_text: str = ''
    for i in range(0, len(text_array)):
        text = str(text_array[i]).strip()
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
                    if text[0] == '【' and text[-1]  == '】':
                        continue
                    # if len(text) > 22: text = formater(text)
                    text = text.replace('・', '·')
                    text = text.replace('♪', '§')
                    text = text.replace('〜', '～')
                    text = text.replace('−', '—')
                    text = text.replace('~', '～')
                    text = text.replace('@', '＠')
                    text = text.replace("⚪", "〇")
                    text = text.replace("❤", "Г")
                    text = text.replace("♡", "Д")
                    line_text += text
                jp_line = str(text_array[i - 1]).strip()
                if read_state and ((line_text.endswith('＠') and jp_line.endswith("＠"))):
                    line_text =  formater(line_text)
                    line_text = line_text.replace('｛', '乷')
                    line_text = line_text.replace('｝', '乸')
                    line_text = line_text.replace('@', '仐')
                    line_text = line_text.replace('＠', '仐')
                    line_text = line_text.replace('「', '乽')
                    line_text = line_text.replace('\u200c', '）')
                    line_text = to_full_char(line_text) 
                    block_text.append(line_text)
                    line_text = ""
            if 'block-end' in text_array[i + 1] and line_text != "":
                ine_text =  formater(line_text)
                line_text = line_text.replace('｛', '乷')
                line_text = line_text.replace('｝', '乸')
                line_text = line_text.replace('@', '仐')
                line_text = line_text.replace('＠', '仐')
                line_text = line_text.replace('「', '乽')
                line_text = line_text.replace('\u200c', '）')
                block_text.append(line_text)
                line_text = ""
        except: pass
    return result

def get_block_line_info(text_array: list[str]) -> list[list[dict[str, int | int | list[int]]]]:
    text_filter: str = ''
    for i in range(0, len(text_array)):
        text: str = text_array[i]
        if ("#NEXT" not in text) and (("#" in text) or ("％" in text) or ("*" in text) or ("GOTO" in text) or ("/" in text) or ('$' in text) or ("亾" in text)): 
            continue
        # print(f'{str('#' in text)}: {text}')
        # if '#' in text : continue
        if len(text.strip().replace('丂', '')) == 0: continue
        if '#NEXT' in text:
            text_filter += text
        else:
            text_filter += f'{i}:{text}'
        pass
    text_block: list[str] = text_filter.split('#NEXT')
    result: list[dict[str, int | list]]= []
    for i in range(0, len(text_block)):
        block = text_block[i]
        if ':' not in block: continue
        lines: list[str] = block.split('\n')
        lines = [line for line in lines if len(line) > 0]
        start: int = int(lines[0].split(':')[0])
        end: int = int(lines[-1].split(':')[0])
        count: int = 1
        save_line_nums: int = 0
        block_lines: list[int] = []
        for n in range(0, len(lines)):
            line: str = lines[n]
            if ":" not in line: continue
            text = line.split(":")[1]
            if text[-1] == '亃' or text[-1] == '】': 
                save_line_nums = 0
                continue
            save_line_nums += 1
            # print(text)
            # print(text.encode('gbk').decode('sjis'))
            if text[-1] == '仐' or text[-1] == '亃' or text[-1] == '＠' or text[-1] == '】' or n == len(lines) - 1:
                block_lines.append({'begin': int(line.split(":")[0]) - (save_line_nums - 1), 'count': save_line_nums})
                # print(f"[{len(result)}] save_line_nums: {str(save_line_nums)} {block_lines}\n\n")
                save_line_nums = 0
            
            count += 1
        # result.append([i, start, end, block_line_count])
        if(len(block_lines) > 0) : 
            result.append(block_lines)
        pass
    # print(len(result), end='\n\n')
    # pprint(result)
    return result

def merge_text(cn_parsed_text: list[str], jp_raw_text: list[str], raw_block_info: list[list[dict[str, int | int | list[int]]]]) -> list[str]:
    cn_block_text: list[list[str]] = [block[1] for block in cn_parsed_text]
    cn_block_count, jp_block_count = [len(cn_block_text), len(raw_block_info)]
    if  cn_block_count !=  jp_block_count:
        for i in range(0, len(cn_block_text)):
            print(f"[{i}] {cn_block_text[i]}")
        for info in raw_block_info:
            # pprint(info)
            for line in info:
                # print(jp_raw_text[line.get('begin')])
                pass
        raise Exception(f"cn_block_count: {cn_block_count} jp_block_count: {jp_block_count}")
    for block_lines in raw_block_info:
        for block_line in block_lines:
            begin, count = [block_line.get("begin"), block_line.get('count')]
            for i in range(begin, begin + count):
                jp_raw_text[i] = None
    for  i in range(0, len(cn_block_text)):
        block: list[str] = cn_block_text[i]
        # print(f'[{i}] jp: {len(raw_block_info[i])} cn: {len(block)} {raw_block_info[i]}')
        for ii in range(0, len(block)):
            begin: int = raw_block_info[i][ii].get('begin')
            jp_raw_text[begin] = block[ii] + '\n'
    
    return [line.replace('doko_op.mpg', 'doko_op_cn.mpg') for line in jp_raw_text if line != None]

def import_all_text(jp_raw_text_file: str, cn_text_file: str) -> list[str]:
    jp_text: list[str] = file_read(jp_raw_text_file, 'gbk')
    cn_text: list[str] = file_read(cn_text_file, 'utf-8')
    jp_line_raw_info: list[list[dict[str, int | int | list[int]]]] = get_block_line_info(jp_text)
    return merge_text(text_parse(cn_text), jp_text, jp_line_raw_info)

def import_and_save_all_text(jp_raw_text_file: str, cn_text_file: str, save_text_file: str) -> None:
    file_write(save_text_file, import_all_text(jp_raw_text_file, cn_text_file), 'gbk')
    pass

if __name__ == '__main__':
    if len(sys.argv) != 4:
        print("Invalid Parameter!!!")
        print(f"python {sys.argv[0]} [in] cn_import_text_path  [in] jp_raw_text_path [out] output_text_path")
        print(f"Example: python {sys.argv[0]} ./cn_import_text/ ./jp_raw_text/ ./output/")
        import_and_save_all_text("./txt/0121_02ka_1.txt",
                   "./doko_iku_text/0121_02ka_1.txt", 
                    "./new_text/0121_02ka_1.txt")
    else:
        for file in os.listdir(sys.argv[1]):
            name: str = os.path.basename(file)
            try:
                import_and_save_all_text(
                        os.path.join(sys.argv[2], name), 
                        os.path.join(sys.argv[1], name), 
                        os.path.join(sys.argv[3], name))
                print(f"saved: {name}")
            except Exception as err:
                print(name)
                raise err
    
    pass