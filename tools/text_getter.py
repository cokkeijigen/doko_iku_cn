import os

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
        for n in range(0, len(lines)):
            line: str = lines[n]
            if ":" not in line: continue
            line = line.split(":")[1]
            if count < 10:
                c_text = '00' + str(count)
            elif count < 100:
                c_text = '0' + str(count)
            result += f'○○|{c_text}|○○|{line}\n'
            result += f'●●|{c_text}|●●|{line}\n'
            if(n != len(lines) - 1):
                result += '\n'
            count += 1
        result += f"[block-end: {end}]#{i}\n\n"
        pass
    return result

if __name__ == '__main__':
    for f in os.listdir('./old/txt'):
        fp = open(f'./old/txt/{f}', 'r', encoding='shift-jis')
        text_array: list[str] = fp.readlines()
        fp.close()
        text = text_getter(text_array)
        fp = open(f'./text/{f}', 'w' ,encoding='utf-8')
        fp.write(text)
        fp.close()