import time
import random
import math

NUM_WORDS_SMALL = 500
NUM_WORDS_LARGE = 5000

def generate_random_text(num_words, avg_word_len=5):
    text = []
    for _ in range(num_words):
        word_len = random.randint(3, avg_word_len + 3)
        word = ''.join(random.choices('abcdefghijklmnopqrstuvwxyz', k=word_len))
        text.append(word)
    return " ".join(text)

TEXT_SMALL = generate_random_text(NUM_WORDS_SMALL)
TEXT_LARGE = generate_random_text(NUM_WORDS_LARGE)

def format_line(line_words, L):
    if not line_words:
        return ""
    word_count = len(line_words)
    total_word_length = sum(len(w) for w in line_words)
    if word_count == 1:
        return line_words[0].ljust(L)
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
    if current_line: 
        justified_lines.append(" ".join(current_line))
    return justified_lines

def calculate_cost(start_index, end_index, words, L):
    length = sum(len(words[i]) for i in range(start_index, end_index + 1))
    num_spaces = end_index - start_index
    total_length = length + num_spaces
    if total_length > L: 
        return math.inf
    elif end_index == len(words) - 1: 
        return 0
    else:
        remaining_space = L - total_length
        return remaining_space ** 3

def dp_justify(text, L):
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

def measure_time(func, text, L):
    start_time = time.perf_counter()
    func(text, L)
    end_time = time.perf_counter()
    return end_time - start_time

def read_text_file(filename):
    try:
        with open(filename, "r", encoding="utf-8") as f:
            return f.read()
    except FileNotFoundError:
        print(f"File {filename} not found. Exiting.")
        exit(1)

def run_tests(text_file="input.txt"):
    while True:
        try:
            L = int(input("Текстийн үндэслэлийг LINE_LIMIT оруулна уу: "))
            if L <= 0:
                print("Error")
                continue
            break
        except ValueError:
            print("Buruu baina")

    text = read_text_file(text_file)

    print(f"  АЖЛЫН ҮР ДҮНГИЙН ХАРЬЦУУЛАЛТ (L={L})")
    
    print("\nDP АРГААР ЖИГДЛЭСЭН")
    dp_result = dp_justify(text, L)
    for line in dp_result:
        print(f"|{line}|")

    print("\nGreedy АРГААР ЖИГДЛЭСЭН ")
    greedy_result = greedy_justify(text, L)
    for line in greedy_result:
        print(f"|{line}|")

    print(f"  ХУГАЦААНЫ ХАРЬЦУУЛАЛТ (Random Text, L={L})")
    
    time_dp_small = measure_time(dp_justify, TEXT_SMALL, L)
    time_greedy_small = measure_time(greedy_justify, TEXT_SMALL, L)
    time_dp_large = measure_time(dp_justify, TEXT_LARGE, L)
    time_greedy_large = measure_time(greedy_justify, TEXT_LARGE, L)

    print(f"N={NUM_WORDS_SMALL} (Жижиг Бичвэр):")
    print(f"  DP (O(N^2)): {time_dp_small:.6f} секунд")
    print(f"  Greedy (O(N)): {time_greedy_small:.6f} секунд")
    
    print(f"\nN={NUM_WORDS_LARGE} (Том Бичвэр):")
    print(f"  DP (O(N^2)): {time_dp_large:.6f} секунд")
    print(f"  Greedy (O(N)): {time_greedy_large:.6f} секунд")

    if time_greedy_large > 0:
        speed_difference = time_dp_large / time_greedy_large
        print("\n--- Дүгнэлт ---")
        print(f"Greedy нь Том бичвэрийн хувьд DP-ээс {speed_difference:.2f} дахин хурдан ажиллаж байна.")

def unit_tests():
    text = read_text_file("input.txt")
    
    dp_lines = dp_justify(text, LINE_LIMIT)
    dp_joined = " ".join(line.strip() for line in dp_lines)
    words_original = text.split()
    words_dp = dp_joined.split()
    
    for line in dp_lines:
        assert len(line) <= LINE_LIMIT, f"DP шугам хязгаараас хэтэрсэн: {line}"
    
    assert words_original == words_dp, f"DP үг таарахгүй байна: {words_dp}"
    
    greedy_lines = greedy_justify(text, LINE_LIMIT)
    greedy_joined = " ".join(line.strip() for line in greedy_lines)
    words_greedy = greedy_joined.split()
    
    for line in greedy_lines:
        assert len(line) <= LINE_LIMIT, f"Greedy шугам хязгаараас хэтэрсэн: {line}"

    assert words_original == words_greedy, f"Greedy үг таарахгүй байна: {words_greedy}"
    
    print("Бүх нэгжийн шалгалтууд DB болон Greedy тэнцсэн!")

if __name__ == "__main__":
    run_tests("mongolian_input.txt")
    unit_tests()
