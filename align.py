from typing import List
import pyphen
import math
import time
import random

LINE_LIMIT = 40
DP_COST_EXPONENT = 3
NUM_WORDS_SMALL = 500
NUM_WORDS_LARGE = 5000

hyphen_en = pyphen.Pyphen(lang='en_US')
try:
    hyphen_mn = pyphen.Pyphen(lang='mn')
except Exception:
    hyphen_mn = None

ENGLISH_TEXT = (
    "Algorithms are great. Dynamic programming solves the text justification problem optimally. "
    "The Greedy approach is faster but does not guarantee the best overall solution. "
    "We must analyze both time complexity and the quality of the output."
)

MONGOLIAN_TEXT = (
    "Оновчлолын алгоритмууд нь програм хангамжийн бүтцэд чухал үүрэг гүйцэтгэдэг. "
    "Динамик программчлал нь бичвэрийг хамгийн бага алдагдалтайгаар жигдлэх оновчтой шийдлийг олох боломжийг олгодог. "
    "Шуналтай арга нь хурдан боловч оновчтой бус байх магадлалтай. "
    "Бидний гол зорилго бол хурд болон үр дүнгийн чанарыг харьцуулах явдал юм."
)

def generate_random_text(num_words, avg_word_len=5):
    text = []
    for _ in range(num_words):
        word_len = random.randint(3, avg_word_len + 3)
        word = ''.join(random.choices('abcdefghijklmnopqrstuvwxyz', k=word_len))
        text.append(word)
    return " ".join(text)

TEXT_SMALL = generate_random_text(NUM_WORDS_SMALL)
TEXT_LARGE = generate_random_text(NUM_WORDS_LARGE)

def visual_len(s: str) -> int:
    return len(s)

def format_line(words: List[str], L: int, is_last: bool=False) -> str:
    if not words:
        return ''
    if is_last or len(words) == 1:
        line = ' '.join(words)
        return line + ' ' * max(0, L - visual_len(line))
    total_chars = sum(visual_len(w) for w in words)
    gaps = len(words) - 1
    total_spaces = L - total_chars
    base = total_spaces // gaps
    extra = total_spaces % gaps
    out = ''
    for i, w in enumerate(words):
        out += w
        if i < gaps:
            out += ' ' * (base + (1 if i < extra else 0))
    return out

def hyphenate_pyphen(word: str, hyphenator: pyphen.Pyphen, width_remaining: int) -> (str, str, bool):
    if hyphenator is None:
        return word, '', False
    inserted = hyphenator.inserted(word, hyphen='-')
    parts = inserted.split('-')
    prefix = ''
    for i in range(len(parts)):
        if i > 0: prefix += '-'
        prefix += parts[i]
        remaining = ''.join(parts[i+1:]) if i+1 < len(parts) else ''
        if len(prefix) <= width_remaining and len(prefix.replace('-', '')) >= 2 and len(remaining) >= 2:
            return prefix + '-', remaining, True
    return word, '', False

def greedy_justify(text: str, L: int, hyphenator=None) -> List[str]:
    words = text.split()
    lines = []
    cur = []
    cur_len = 0
    i = 0
    while i < len(words):
        w = words[i]
        wlen = visual_len(w)
        newlen = wlen if cur_len == 0 else cur_len + 1 + wlen
        if newlen <= L:
            cur.append(w)
            cur_len = newlen
            i += 1
        else:
            if hyphenator and cur_len > 0:
                remaining_space = L - cur_len - 1
                part, rem, did = hyphenate_pyphen(w, hyphenator, remaining_space)
                if did:
                    cur.append(part)
                    lines.append(format_line(cur, L, False))
                    cur = []
                    cur_len = 0
                    words[i] = rem
                    continue
            lines.append(format_line(cur, L, False))
            cur = []
            cur_len = 0
    if cur:
        lines.append(format_line(cur, L, True))
    return lines

def calculate_cost(start_index, end_index, words, L):
    length = sum(len(words[i]) for i in range(start_index, end_index + 1))
    num_spaces = end_index - start_index
    total_length = length + num_spaces
    if total_length > L:
        return math.inf
    elif end_index == len(words) - 1:
        return 0
    else:
        return (L - total_length) ** DP_COST_EXPONENT

def dp_justify(text: str, L: int, hyphenator=None) -> List[str]:
    words = text.split()
    n = len(words)
    INF = 10**18
    dp = [INF] * (n+1)
    parent = [-1] * (n+1)
    hyphen_info = [None] * n
    dp[n] = 0
    for i in range(n-1, -1, -1):
        cur_len = 0
        for j in range(i, n):
            cur_len = visual_len(words[j]) if j==i else cur_len + 1 + visual_len(words[j])
            if cur_len > L:
                if hyphenator and j < n:
                    prev_len = cur_len - (1 + visual_len(words[j])) if j>i else 0
                    capacity = L - (prev_len + (1 if j>i else 0))
                    if capacity >= 2:
                        part, rem, did = hyphenate_pyphen(words[j], hyphenator, capacity)
                        if did:
                            bad = 0 if j==n-1 else (L - (prev_len + (1 if j>i else 0) + len(part)))**DP_COST_EXPONENT
                            if dp[j+1] + bad < dp[i]:
                                dp[i] = dp[j+1] + bad
                                parent[i] = j+1
                                hyphen_info[j] = (part, rem)
                break
            bad = 0 if j==n-1 else (L - cur_len)**DP_COST_EXPONENT
            if dp[j+1] + bad < dp[i]:
                dp[i] = dp[j+1] + bad
                parent[i] = j+1
                hyphen_info[j] = None
    out = []
    i = 0
    while i < n:
        j = parent[i]
        if j == -1:
            w = words[i]
            if hyphenator:
                ins = hyphenator.inserted(w, '-').split('-')
                if len(ins) > 1:
                    part = ins[0] + '-'
                    rem = ''.join(ins[1:])
                    out.append(format_line([part], L, False))
                    words[i] = rem
                    continue
            out.append(w)
            i += 1
            continue
        line_words = [words[k] for k in range(i, j)]
        k = j-1
        if hyphen_info[k] is not None and j!=n:
            part, rem = hyphen_info[k]
            line_words.append(part)
            words[k] = rem
        else:
            line_words.append(words[k])
        is_last = (j==n)
        out.append(format_line(line_words, L, is_last))
        i = j
    return out

def measure_time(func, text, L, hyphenator=None):
    start = time.perf_counter()
    func(text, L, hyphenator)
    end = time.perf_counter()
    return end - start

def test_and_print():
    texts = [(ENGLISH_TEXT, hyphen_en, "English"), (MONGOLIAN_TEXT, hyphen_mn, "Mongolian")]
    for text, hyphenator, lang in texts:
        print(f"\n=== GREEDY {lang} ===")
        for ln in greedy_justify(text, LINE_LIMIT, hyphenator):
            print(f"|{ln}| ({visual_len(ln)})")
        print(f"\n=== DP {lang} ===")
        for ln in dp_justify(text, LINE_LIMIT, hyphenator):
            print(f"|{ln}| ({visual_len(ln)})")

def unit_tests():
    texts = [(ENGLISH_TEXT, hyphen_en), (MONGOLIAN_TEXT, hyphen_mn)]
    for text, h in texts:
        for justify_func in [dp_justify, greedy_justify]:
            lines = justify_func(text, LINE_LIMIT, h)
            for line in lines:
                assert visual_len(line) <= LINE_LIMIT, f"Line exceeds limit: {line}"
            words_original = text.split()
            words_out = " ".join(line.strip() for line in lines).split()
            assert words_original == words_out, f"Words mismatch for {justify_func.__name__}"
    print("All unit tests passed!")

if __name__ == "__main__":
    test_and_print()
    unit_tests()

    print("\n=== Timing Random Text ===")
    for text, label in [(TEXT_SMALL, "Small"), (TEXT_LARGE, "Large")]:
        t_dp = measure_time(dp_justify, text, LINE_LIMIT)
        t_greedy = measure_time(greedy_justify, text, LINE_LIMIT)
        print(f"{label} text: DP {t_dp:.6f}s, Greedy {t_greedy:.6f}s")
