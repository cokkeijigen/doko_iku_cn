import os

org_text: str = './txt'
out_text: str = './text'

def text_getter(text_array: list[str]) -> str:
    text_filter: str = ''
    text_out: list[int, str] = []
    backlog_write: list[int, str] = []
    for i in range(0, len(text_array)):
        text: str = text_array[i]
        if text.startswith("#TEXTOUT"):
            text_out.append([i, text])
            continue
        if text.startswith("#BACKLOG_WRITE"):
            backlog_write.append([i, text])
            continue
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
                if n != len(lines) -1: result += '\n'
                result += f'○○|0{save_line_nums}|○○|{save_line_text}\n'
                result += f'●●|0{save_line_nums}|●●|{save_line_text}\n\n'
                save_line_text = ''
                save_line_nums = 0
            else:
                save_line_text += '\\n'
            count += 1
        if len(save_line_text) > 0:
            result += f'○○|0{save_line_nums}|○○|{save_line_text}\n'
            result += f'●●|0{save_line_nums}|●●|{save_line_text}\n'
            save_line_text = ''
            save_line_nums = 0
        result += f"[block-end: {end}]#{i}\n\n"
        pass
    if len(text_out) > 0:
        text_out_text: str = '[text-out-start]\n'
        for out in text_out:
            text: str = str(out[1]).strip().split(' ')[1].split(',', 2)[0]
            if len(text.replace('　', '')) == 0:
                continue
            text_out_text += f"○○|{out[0]:04}|○○|{text}\n"
            text_out_text += f"●●|{out[0]:04}|●●|{text}\n\n"
        text_out_text += '[text-out-end]\n\n'
        result += text_out_text
    
    if len(backlog_write) > 0:
        backlog_write_text = "[backlog-write-start]\n"
        for backlog in backlog_write:
            text: str = str(backlog[1]).strip().split(' ')[1]
            backlog_write_text += f"○○|{backlog[0]:04}|○○|{text}\n"
            backlog_write_text += f"●●|{backlog[0]:04}|●●|{text}\n\n"
        backlog_write_text += "[backlog-write-end]\n\n"
        result += backlog_write_text
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