# justify_with_pyphen.py
# Python: greedy + dp justification + hyphenation via Pyphen (Hunspell patterns)
# Requires: pip install pyphen

from typing import List
import pyphen
import math

LINE_LIMIT = 40
DP_COST_EXPONENT = 3  # use cubic badness like many text-justifiers

# Create pyphen hyphenators:
# Use built-in language code if available, else create Pyphen instance with language=None and load patterns manually
hyphen_en = pyphen.Pyphen(lang='en_US')   # english (usually available)
# For Mongolian, try 'mn' or load a custom file later.
# If you have a hyphenation file, use Pyphen(filename='path/to/mn.dic') — see instructions
try:
    hyphen_mn = pyphen.Pyphen(lang='mn')  # may not exist by default
except Exception:
    hyphen_mn = None

def hyphenate_pyphen(word: str, hyphenator: pyphen.Pyphen, width_remaining: int) -> (str, str, bool):
    """
    Try to hyphenate 'word' to fit into 'width_remaining' characters.
    Returns (part_with_dash_or_fullword, remainder, did_hyphenate_flag).
    If cannot hyphenate to fit, returns (word, '', False).
    """
    if hyphenator is None:
        return (word, '', False)
    inserted = hyphenator.inserted(word, hyphen='-')  # ex: "ex-am-ple"
    # Build candidates split at hyphenation points from rightmost usable to leftmost
    parts = inserted.split('-')
    # Rebuild progressive prefixes
    prefix = ''
    for i in range(len(parts)):
        if i > 0:
            prefix += '-'
        prefix += parts[i]
        # visible length = number of Unicode codepoints
        if len(prefix) <= width_remaining:
            # But ensure both pieces >= min size (commonly left>=2 right>=2)
            remaining = ''.join(parts[i+1:]) if i+1 < len(parts) else ''
            if remaining == '':
                # It's full word (no remainder)
                return (word, '', False)
            # Typical hyphenation minima (left>=2,right>=2)
            if len(prefix.replace('-', '')) >= 2 and len(remaining) >= 2:
                return (prefix + '-', remaining, True)
    return (word, '', False)

# Utility: visible length (Unicode codepoints)
def visual_len(s: str) -> int:
    return len(s)  # Python3 str is Unicode codepoints which is fine for our use

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
    for i,w in enumerate(words):
        out += w
        if i < gaps:
            out += ' ' * (base + (1 if i < extra else 0))
    return out

def greedy_justify(text: str, L: int, hyphenator=None) -> List[str]:
    words = text.split()
    lines = []
    cur = []
    cur_len = 0
    i = 0
    while i < len(words):
        w = words[i]
        wlen = visual_len(w)
        if cur_len == 0:
            newlen = wlen
        else:
            newlen = cur_len + 1 + wlen
        if newlen <= L:
            cur.append(w)
            cur_len = newlen
            i += 1
        else:
            # try hyphenate this word to fit into current line if hyphenator provided and cur not empty
            if hyphenator and cur_len > 0:
                remaining_space = L - cur_len - 1  # 1 for space before word
                part, rem, did = hyphenate_pyphen(w, hyphenator, remaining_space)
                if did:
                    cur.append(part)
                    lines.append(format_line(cur, L, False))
                    cur = []
                    cur_len = 0
                    # replace current word with remainder (do not increment i)
                    words[i] = rem
                    continue
            # finish current line
            lines.append(format_line(cur, L, False))
            cur = []
            cur_len = 0
    # last line
    if cur:
        lines.append(format_line(cur, L, True))
    return lines

def dp_justify(text: str, L: int, hyphenator=None) -> List[str]:
    words = text.split()
    n = len(words)
    INF = 10**18
    dp = [INF] * (n+1)
    parent = [-1] * (n+1)
    hyphen_info = [None] * (n)  # if the j-th word was hyphenated, store (prefix_with_dash, remainder)
    dp[n] = 0
    for i in range(n-1, -1, -1):
        # try placing words i..j wholly, or hyphenate j to fit
        cur_len = 0
        for j in range(i, n):
            if j == i:
                cur_len = visual_len(words[j])
            else:
                cur_len += 1 + visual_len(words[j])
            if cur_len > L:
                # maybe hyphenate words[j] to fit if possible and not last word
                if hyphenator and j < n:
                    # capacity for hyphenating j as last on line = L - (cur_len - visual_len(words[j]) - (1 if j>i else 0)) - (1 if j>i else 0)
                    # simpler: compute available for word j
                    prev_len = cur_len - (1 + visual_len(words[j])) if j>i else 0
                    capacity = L - (prev_len + (1 if j>i else 0))
                    if capacity >= 2:
                        part, rem, did = hyphenate_pyphen(words[j], hyphenator, capacity)
                        if did:
                            bad = (0 if j == n-1 else (L - (prev_len + (1 if j>i else 0) + len(part)))**DP_COST_EXPONENT)
                            if dp[j+1] + bad < dp[i]:
                                dp[i] = dp[j+1] + bad
                                parent[i] = j+1
                                hyphen_info[j] = (part, rem)
                break
            # compute cost
            bad = 0 if j == n-1 else (L - cur_len)**DP_COST_EXPONENT
            if dp[j+1] + bad < dp[i]:
                dp[i] = dp[j+1] + bad
                parent[i] = j+1
                hyphen_info[j] = None
    # reconstruct
    out = []
    i = 0
    remainder_fragment = None
    while i < n:
        j = parent[i]
        if j == -1:
            # can't place (very long word) — fallback: try to hyphenate leftmost
            w = words[i]
            if hyphenator:
                # force hyphenate to first possible split
                ins = hyphenator.inserted(w, '-').split('-')
                if len(ins) > 1:
                    part = ins[0] + '-'
                    rem = ''.join(ins[1:])
                    line_words = [part]
                    out.append(format_line(line_words, L, False))
                    words[i] = rem
                    continue
            # no hyphenation possible → put whole word in its on line (overflow)
            out.append(w)
            i += 1
            continue
        line_words = []
        for k in range(i, j):
            # if k is last of line and was hyphenated we will adjust below
            if k < j-1:
                line_words.append(words[k])
        k = j-1
        if hyphen_info[k] is not None and j != n:
            part, rem = hyphen_info[k]
            line_words.append(part)
            words[k] = rem
        else:
            line_words.append(words[k])
        is_last = (j == n)
        out.append(format_line(line_words, L, is_last))
        i = j
    return out

def test_and_print():
    en = "Optimization algorithms play a crucial role in software engineering. Dynamic programming finds the optimal text justification solution."
    mn = "Оновчлолын алгоритмууд нь програм хангамжийн бүтцэд чухал үүрэг гүйцэтгэдэг. Динамик программчлал нь бичвэрийг жигдлэх оновчтой шийдлийг олдог."
    # choose hyphenators
    h_en = hyphen_en
    h_mn = hyphen_mn  # may be None

    print("=== GREEDY English ===")
    for ln in greedy_justify(en, LINE_LIMIT, h_en):
        print("|%s| (%d)" % (ln, visual_len(ln)))
    print("\n=== DP English ===")
    for ln in dp_justify(en, LINE_LIMIT, h_en):
        print("|%s| (%d)" % (ln, visual_len(ln)))

    print("\n=== GREEDY Mongolian ===")
    for ln in greedy_justify(mn, LINE_LIMIT, h_mn):
        print("|%s| (%d)" % (ln, visual_len(ln)))
    print("\n=== DP Mongolian ===")
    for ln in dp_justify(mn, LINE_LIMIT, h_mn):
        print("|%s| (%d)" % (ln, visual_len(ln)))

if __name__ == "__main__":
    test_and_print()
