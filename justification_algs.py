# justification_algs.py
import math

def format_line(line_words, L):
    """Мөрийг бүрэн жигдлэх (space-үүдийг тэнцүү хуваарилах)."""
    # [Өмнөх format_line функцийн бүрэн кодыг энд хуулж тавина]
    if not line_words: return ""
    word_count = len(line_words)
    total_word_length = sum(len(w) for w in line_words)
    if word_count == 1: return line_words[0].ljust(L)
    
    total_spaces = L - total_word_length
    num_gaps = word_count - 1
    base_spaces = total_spaces // num_gaps
    extra_spaces = total_spaces % num_gaps
    
    formatted_line = ""
    for i, word in enumerate(line_words):
        formatted_line += word
        if i < num_gaps:
            spaces = base_spaces + (1 if i < extra_spaces else 0)
            formatted_line += " " * spaces
    return formatted_line

def greedy_justify(text, L):
    # [Өмнөх greedy_justify функцийн бүрэн кодыг энд хуулж тавина]
    words = text.split()
    justified_lines, current_line, current_length = [], [], 0
    for word in words:
        word_len = len(word)
        new_length = word_len if current_length == 0 else current_length + 1 + word_len
        if new_length <= L:
            current_line.append(word)
            current_length = new_length
        else:
            justified_lines.append(format_line(current_line, L))
            current_line = [word]
            current_length = word_len
    if current_line: justified_lines.append(" ".join(current_line))
    return justified_lines

def calculate_cost(start_index, end_index, words, L):
    # [Өмнөх calculate_cost функцийн бүрэн кодыг энд хуулж тавина]
    length = sum(len(words[i]) for i in range(start_index, end_index + 1))
    num_spaces = end_index - start_index
    total_length = length + num_spaces
    if total_length > L: return math.inf
    elif end_index == len(words) - 1: return 0
    else:
        remaining_space = L - total_length
        return remaining_space ** 3

def dp_justify(text, L):
    # [Өмнөх dp_justify функцийн бүрэн кодыг энд хуулж тавина]
    words = text.split()
    n = len(words)
    DP = [math.inf] * (n + 1)
    parent = [0] * (n + 1) 
    DP[n] = 0

    for i in range(n - 1, -1, -1):
        for j in range(i, n):
            cost = calculate_cost(i, j, words, L)
            if cost != math.inf:
                total_cost = cost + DP[j + 1]
                if total_cost < DP[i]:
                    DP[i] = total_cost
                    parent[i] = j + 1

    justified_lines = []
    i = 0
    while i < n:
        j = parent[i] - 1
        line_words = words[i : j + 1]
        if j == n - 1:
            justified_lines.append(" ".join(line_words))
        else:
            justified_lines.append(format_line(line_words, L))
        i = j + 1
    return justified_lines