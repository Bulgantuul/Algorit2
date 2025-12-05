from typing import List
import pyphen
import math
import time
import random
import sys

LINE_LIMIT = None
DP_COST_EXPONENT = 3
NUM_WORDS_SMALL = 500
NUM_WORDS_LARGE = 5000

hyphen_en = pyphen.Pyphen(lang='en_US')
try:
    hyphen_mn = pyphen.Pyphen(lang='mn')
except Exception:
    hyphen_mn = None

ENGLISH_FILE_PATH = "english_input.txt"
MONGOLIAN_FILE_PATH = "mongolian_input.txt"

def read_text_from_file(path: str) -> str:
    try:
        with open(path, "r", encoding="utf-8") as f:
            return " ".join(f.read().split())
    except FileNotFoundError:
        return ""
    except Exception:
        return ""

ENGLISH_TEXT = read_text_from_file(ENGLISH_FILE_PATH)
MONGOLIAN_TEXT = read_text_from_file(MONGOLIAN_FILE_PATH)

def generate_random_text(num_words, avg_word_len=5):
    text = []
    for _ in range(num_words):
        word_len = random.randint(3, avg_word_len + 3)
        word = ''.join(random.choices('abcdefghijklmnopqrstuvwxyz', k=word_len))
        text.append(word)
    return " ".join(text)

TEXT_SMALL = ENGLISH_TEXT
TEXT_LARGE = ENGLISH_TEXT

def visual_len(s: str) -> int:
    return len(s)

def format_line(words: List[str], L: int, is_last: bool=False) -> str:
    if not words:
        return ''
    line_str = ' '.join(words)
    if is_last or len(words) == 1:
        line = ' '.join(words)
        return line + ' ' * max(0, L - visual_len(line))
    total_chars = sum(visual_len(w) for w in words)
    gaps = len(words) - 1
    total_spaces = L - total_chars
    if total_spaces < gaps:
        return line_str + ' ' * max(0, L - visual_len(line_str))
    base = total_spaces // gaps
    extra = total_spaces % gaps
    out = ''
    for i, w in enumerate(words):
        out += w
        if i < gaps:
            out += ' ' * (base + (1 if i < extra else 0))
    return out

def hyphenate_pyphen(word: str, hyphenator: pyphen.Pyphen, width_remaining: int):
    if hyphenator is None:
        return word, '', False
    inserted = hyphenator.inserted(word, hyphen='-')
    parts = inserted.split('-')
    current_prefix = ''
    for i in range(len(parts) - 1):
        if i > 0:
            current_prefix += '-'
        current_prefix += parts[i]
        hyphenated_word_part = current_prefix + '-'
        remaining_part = '-'.join(parts[i+1:])
        if visual_len(remaining_part.replace('-', '')) < 2:
            continue
        if visual_len(current_prefix.replace('-', '')) < 2:
            continue
        if visual_len(hyphenated_word_part) <= width_remaining:
            return hyphenated_word_part, remaining_part, True
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
            if hyphenator and cur_len > 0 and wlen > 2:
                remaining_space = L - cur_len - 1
                part, rem, did = hyphenate_pyphen(w, hyphenator, remaining_space)
                if did:
                    cur.append(part)
                    lines.append(format_line(cur, L, False))
                    cur = []
                    cur_len = 0
                    words[i] = rem
                    continue
            if cur:
                lines.append(format_line(cur, L, False))
                cur = []
                cur_len = 0
            if visual_len(w) > L:
                lines.append(format_line([w], L, False))
                i += 1
    if cur:
        lines.append(format_line(cur, L, True))
    return lines

def calculate_cost(line_length, L):
    if line_length < 0:
        return math.inf
    return (L - line_length) ** DP_COST_EXPONENT

def dp_justify(text: str, L: int, hyphenator=None) -> List[str]:
    original_words = text.split()
    n = len(original_words)
    INF = 10**18
    dp = [INF] * (n + 1)
    parent = [-1] * (n + 1)
    hyphen_info = {}
    dp[n] = 0
    for i in range(n - 1, -1, -1):
        cur_len = 0
        for j in range(i, n):
            w = original_words[j]
            wlen = visual_len(w)
            if j == i:
                line_len_to_j = wlen
            else:
                line_len_to_j = cur_len + 1 + wlen
            if line_len_to_j <= L:
                cost = 0 if j == n - 1 else calculate_cost(line_len_to_j, L)
                new_cost = dp[j + 1] + cost
                if new_cost < dp[i]:
                    dp[i] = new_cost
                    parent[i] = j + 1
                    if i in hyphen_info:
                        del hyphen_info[i]
                cur_len = line_len_to_j
            else:
                if j > i and hyphenator:
                    line_len_prev = cur_len
                    rem_space_for_hyphen = L - (line_len_prev + 1)
                    w_j = original_words[j]
                    part, rem, did = hyphenate_pyphen(w_j, hyphenator, rem_space_for_hyphen)
                    if did:
                        line_len_with_part = line_len_prev + 1 + visual_len(part)
                        cost_with_part = 0 if j == n else calculate_cost(line_len_with_part, L)
                        new_cost = dp[j] + cost_with_part
                        if new_cost < dp[i]:
                            dp[i] = new_cost
                            parent[i] = j
                            hyphen_info[i] = (j, part, rem)
                if j > i:
                    line_len_prev = cur_len
                    cost_prev = 0 if j == n else calculate_cost(line_len_prev, L)
                    new_cost = dp[j] + cost_prev
                    if new_cost < dp[i]:
                        dp[i] = new_cost
                        parent[i] = j
                        if i in hyphen_info:
                            del hyphen_info[i]
                break
    out = []
    i = 0
    temp_words = list(original_words)
    while i < n:
        j = parent[i]
        if j == -1:
            w = temp_words[i]
            part, rem, did = hyphenate_pyphen(w, hyphenator, L)
            if did:
                out.append(format_line([part], L, False))
                temp_words[i] = rem
                continue
            out.append(format_line([w], L, False))
            i += 1
            continue
        line_words = [temp_words[k] for k in range(i, j)]
        is_last = (j == n)
        if i in hyphen_info:
            j_hyphen, part, rem = hyphen_info[i]
            if j_hyphen == j:
                line_words[-1] = part
                temp_words[j - 1] = rem
        out.append(format_line(line_words, L, is_last))
        i = j
    return out

def measure_time(func, text, L, hyphenator=None):
    start = time.perf_counter()
    func(text, L, hyphenator)
    end = time.perf_counter()
    return end - start

def test_and_print(L):
    texts = [(ENGLISH_TEXT, hyphen_en, "English"), (MONGOLIAN_TEXT, hyphen_mn, "Mongolian")]
    for text, hyphenator, lang in texts:
        if not text:
            print(f"{lang} text empty.")
            continue
        print(f"GREEDY {lang}")
        for ln in greedy_justify(text, L, hyphenator):
            print(f"|{ln}| ({visual_len(ln)})")
        print(f"DP {lang}")
        for ln in dp_justify(text, L, hyphenator):
            print(f"|{ln}| ({visual_len(ln)})")

def unit_tests(L):
    tests_passed = True
    texts = [(ENGLISH_TEXT, hyphen_en, "English"), (MONGOLIAN_TEXT, hyphen_mn, "Mongolian")]
    for text, h, lang in texts:
        if not text:
            continue
        for justify_func in [dp_justify, greedy_justify]:
            lines = justify_func(text, L, h)
            for line in lines:
                if visual_len(line) > L:
                    print("Line too long.")
                    tests_passed = False
            words_original = text.split()
            words_out_flat = "".join(line.strip().replace(" ", "") for line in lines)
            words_out_flat_no_hyphen = words_out_flat.replace("-", "")
            words_original_flat = "".join(words_original)
            if words_original_flat != words_out_flat_no_hyphen:
                tests_passed = False
    if tests_passed:
        print(f"All tests passed for L={L}!")
    else:
        print(f"Tests FAILED for L={L}.")

def get_line_limit() -> int:
    while True:
        try:
            sys.stdout.write("Enter L: ")
            sys.stdout.flush()
            limit = sys.stdin.readline().strip()
            if not limit:
                return 40
            L = int(limit)
            if L <= 0:
                continue
            return L
        except:
            return 40

if __name__ == "__main__":
    FINAL_LINE_LIMIT = get_line_limit()
    test_and_print(FINAL_LINE_LIMIT)
    unit_tests(FINAL_LINE_LIMIT)

    print("Timing Random Text")
    if ENGLISH_TEXT:
        t_dp_en = measure_time(dp_justify, ENGLISH_TEXT, FINAL_LINE_LIMIT, hyphen_en)
        t_greedy_en = measure_time(greedy_justify, ENGLISH_TEXT, FINAL_LINE_LIMIT, hyphen_en)
        print(f"English: DP {t_dp_en:.6f}s, Greedy {t_greedy_en:.6f}s")
    if MONGOLIAN_TEXT and hyphen_mn:
        t_dp_mn = measure_time(dp_justify, MONGOLIAN_TEXT, FINAL_LINE_LIMIT, hyphen_mn)
        t_greedy_mn = measure_time(greedy_justify, MONGOLIAN_TEXT, FINAL_LINE_LIMIT, hyphen_mn)
        print(f"Mongolian: DP {t_dp_mn:.6f}s, Greedy {t_greedy_mn:.6f}s")

    try:
        text_small_rand = generate_random_text(NUM_WORDS_SMALL)
        t_dp_small = measure_time(dp_justify, text_small_rand, FINAL_LINE_LIMIT, hyphen_en)
        t_greedy_small = measure_time(greedy_justify, text_small_rand, FINAL_LINE_LIMIT, hyphen_en)
        print(f"Random Small: DP {t_dp_small:.6f}s, Greedy {t_greedy_small:.6f}s")
        text_large_rand = generate_random_text(NUM_WORDS_LARGE)
        t_dp_large = measure_time(dp_justify, text_large_rand, FINAL_LINE_LIMIT, hyphen_en)
        t_greedy_large = measure_time(greedy_justify, text_large_rand, FINAL_LINE_LIMIT, hyphen_en)
        print(f"Random Large: DP {t_dp_large:.6f}s, Greedy {t_greedy_large:.6f}s")
    except:
        pass
