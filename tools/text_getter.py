import os

org_text: str = './old/txt'
out_text: str = './text'

def text_getter(text_array: list[str]) -> str:
    text_filter: str = ''
    for i in range(0, len(text_array)):
        text: str = text_array[i]
        if ("#NEXT" not in text) and (("#" in text) or ("％" in text) or ("*" in text) or ("GOTO" in text) or ("/" in text) or ('$' in text)): 
            continue
        if len(text.strip()) == 0: continue
        if '#NEXT' in text:
            text_filter += text
        else:
            text_filter += f'{i}:{text}'
        pass
    text_block: list[str] = text_filter.split('#NEXT')
    result: str = ''
    for i in range(0, len(text_block)):
        block = text_block[i]
        if ':' not in block: continue
        lines: list[str] = block.split('\n')
        lines = [line for line in lines if len(line) > 0]
        start: str = lines[0].split(':')[0]
        end: str = lines[-1].split(':')[0]
        result += f"[block-start: {start}]#{i}\n"
        count: int = 1
        save_line_text: str = ''
        save_line_nums: int = 0
        for n in range(0, len(lines)):
            line: str = lines[n]
            if ":" not in line: continue
            line = line.split(":")[1]
            save_line_text += line
            save_line_nums += 1
            if line[-1] == '＠' or line[-1] == '】':
                result += f'○○|0{save_line_nums}|○○|{save_line_text}\n'
                result += f'●●|0{save_line_nums}|●●|{save_line_text}\n'
                if n != len(lines) -1: result += '\n'
                save_line_nums = 0
                save_line_text = ''
            else:
                save_line_text += '\\n'
            count += 1
        result += f"[block-end: {end}]#{i}\n\n"
        pass
    return result

if __name__ == '__main__':
    for f in os.listdir(org_text):
        fp = open(f'{org_text}/{f}', 'r', encoding='shift-jis')
        text_array: list[str] = fp.readlines()
        fp.close()
        text = text_getter(text_array)
        fp = open(f'{out_text}/{f}', 'w' ,encoding='utf-8')
        fp.write(text)
        fp.close()