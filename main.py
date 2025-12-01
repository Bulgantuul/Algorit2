import math
import random
import time

# =======================================================
# 1. Бичвэрийг Жигдлэх Туслах Функц (Greedy болон DP-д нийтлэг)
# =======================================================
def format_line(line_words, L):
    """Мөрийг бүрэн жигдлэх (space-үүдийг тэнцүү хуваарилах)."""
    if not line_words:
        return ""
    
    word_count = len(line_words)
    total_word_length = sum(len(w) for w in line_words)
    
    # Хэрэв нэг үгтэй бол эсвэл сүүлийн мөр бол зүүн тийш жигдлэнэ
    if word_count == 1:
        return line_words[0].ljust(L)
    
    total_spaces = L - total_word_length
    
    # Жигдлэх үйлдэл: Үг хоорондын зайг хуваарилах
    num_gaps = word_count - 1
    base_spaces = total_spaces // num_gaps
    extra_spaces = total_spaces % num_gaps
    
    formatted_line = ""
    for i, word in enumerate(line_words):
        formatted_line += word
        if i < num_gaps:
            # Үг хоорондын зайг нэмнэ
            spaces = base_spaces + (1 if i < extra_spaces else 0)
            formatted_line += " " * spaces
            
    return formatted_line

# =======================================================
# 2. Greedy Algorithm (O(N) хугацаа)
# =======================================================
def greedy_justify(text, L):
    """Greedy алгоритмаар бичвэрийг жигдлэх."""
    words = text.split()
    justified_lines = []
    current_line = []
    current_length = 0

    for word in words:
        word_len = len(word)
        
        if current_length == 0:
            new_length = word_len
        else:
            # Одоогийн урт + (нэг зай) + Үгийн урт
            new_length = current_length + 1 + word_len

        if new_length <= L:
            current_line.append(word)
            current_length = new_length
        else:
            # Мөрийг жигдлэнэ
            justified_lines.append(format_line(current_line, L))
            
            # Шинэ мөр эхлүүлнэ
            current_line = [word]
            current_length = word_len

    if current_line:
        # Хамгийн сүүлийн мөрийг зүүн тийш жигдэлнэ
        justified_lines.append(" ".join(current_line)) 
        
    return justified_lines

# =======================================================
# 3. Dynamic Programming Algorithm (O(N^2) хугацаа)
# =======================================================
def calculate_cost(start_index, end_index, words, L):
    """words[start_index] -ээс words[end_index] хүртэлх алдагдлыг тооцох."""
    length = sum(len(words[i]) for i in range(start_index, end_index + 1))
    
    # Үг хоорондын зайг нэмэх
    num_spaces = end_index - start_index
    total_length = length + num_spaces
    
    if total_length > L:
        return math.inf
    elif end_index == len(words) - 1:
        # Хамгийн сүүлийн мөр бол алдагдалгүй (0), учир нь зүүн тийш жигдлэнэ
        return 0
    else:
        # Үлдсэн зайны кубээр алдагдлыг тооцно: (L - length)^3
        remaining_space = L - total_length
        return remaining_space ** 3

def dp_justify(text, L):
    """Dynamic Programming (DP) аргаар бичвэрийг жигдлэх."""
    words = text.split()
    n = len(words)
    
    # DP[i]: i-р үгээс эхлэх хамгийн бага нийт алдагдал
    DP = [math.inf] * (n + 1) 
    # parent[i]: i-р үгээс эхлэх мөрийн төгсгөлийн индекс (дараагийн мөрийн эхлэл)
    parent = [0] * (n + 1) 
    
    DP[n] = 0 # Суурь тохиолдол: Төгсгөлөөс хойш алдагдалгүй

    # Сүүлээс эхлэн тооцоолно (bottom-up)
    for i in range(n - 1, -1, -1):
        for j in range(i, n):
            cost = calculate_cost(i, j, words, L)
            
            if cost != math.inf:
                # Нийт алдагдал = Одоогийн мөрийн алдагдал + Үлдэх хэсгийн хамгийн бага алдагдал
                total_cost = cost + DP[j + 1]
                
                if total_cost < DP[i]:
                    DP[i] = total_cost
                    parent[i] = j + 1 # Оновчтой төгсгөл

    # Шийдлийг сэргээнэ
    justified_lines = []
    i = 0
    while i < n:
        j = parent[i] - 1 # Оновчтой мөрийн төгсгөл
        line_words = words[i : j + 1]
        
        if j == n - 1:
            # Хамгийн сүүлийн мөр бол зүүн тийш жигдлэнэ
            justified_lines.append(" ".join(line_words))
        else:
            # Бусад мөрүүдийг бүрэн жигдлэнэ
            justified_lines.append(format_line(line_words, L))
            
        i = j + 1
        
    return justified_lines

# =======================================================
# 4. Тест хийх болон Хугацаа Хэмжих
# =======================================================
def generate_random_text(num_words, avg_word_len=5):
    """Тест хийхэд зориулж санамсаргүй үгс бүхий бичвэр үүсгэх."""
    text = []
    for _ in range(num_words):
        word_len = random.randint(3, avg_word_len + 3)
        word = ''.join(random.choices('abcdefghijklmnopqrstuvwxyz', k=word_len))
        text.append(word)
    return " ".join(text)

def measure_time(func, text, L):
    """Функцийн гүйцэтгэлийн хугацааг хэмжих."""
    start_time = time.perf_counter()
    func(text, L)
    end_time = time.perf_counter()
    return end_time - start_time

if __name__ == "__main__":
    
    # ------------------ Тохиргоо ------------------
    LINE_LIMIT = 40  # L (Мөрийн дээд хязгаар)
    NUM_WORDS_SMALL = 500
    NUM_WORDS_LARGE = 5000
    
    # Англи бичвэрийн жишээ (Бодит гаралтыг харах)
    ENGLISH_TEXT = "Algorithms are great. Dynamic programming solves the text justification problem optimally. The Greedy approach is faster but does not guarantee the best overall solution. We must analyze both time complexity and the quality of the output."

    # ------------------ Үр дүнг Шаардлагатайгаар Хэвлэх ------------------
    print(f"=====================================================")
    print(f"  АЖЛЫН ҮР ДҮНГИЙН ХАРЬЦУУЛАЛТ (L={LINE_LIMIT})")
    print(f"=====================================================")
    
    # 1. DP Гаралт
    print("\n--- 1. DP АРГААР ЖИГДЛЭСЭН БИЧВЭР (Оновчтой шийдэл) ---")
    dp_result = dp_justify(ENGLISH_TEXT, LINE_LIMIT)
    for line in dp_result:
        print(f"|{line}|")

    # 2. Greedy Гаралт
    print("\n--- 2. Greedy АРГААР ЖИГДЛЭСЭН БИЧВЭР (Хурдан шийдэл) ---")
    greedy_result = greedy_justify(ENGLISH_TEXT, LINE_LIMIT)
    for line in greedy_result:
        print(f"|{line}|")

    # ------------------ Гүйцэтгэлийн Хугацаа Хэмжих ------------------
    print("\n=====================================================")
    print(f"  ХУГАЦААНЫ ХАРЬЦУУЛАЛТ (Random Text, L={LINE_LIMIT})")
    print(f"=====================================================")

    # Тестийн бичвэрүүдийг үүсгэх
    text_small = generate_random_text(NUM_WORDS_SMALL)
    text_large = generate_random_text(NUM_WORDS_LARGE)
    
    # Хэмжилт хийх
    time_dp_small = measure_time(dp_justify, text_small, LINE_LIMIT)
    time_greedy_small = measure_time(greedy_justify, text_small, LINE_LIMIT)

    time_dp_large = measure_time(dp_justify, text_large, LINE_LIMIT)
    time_greedy_large = measure_time(greedy_justify, text_large, LINE_LIMIT)

    print(f"N={NUM_WORDS_SMALL} (Жижиг Бичвэр):")
    print(f"  DP (O(N^2)): {time_dp_small:.6f} секунд")
    print(f"  Greedy (O(N)): {time_greedy_small:.6f} секунд")
    
    print(f"\nN={NUM_WORDS_LARGE} (Том Бичвэр):")
    print(f"  DP (O(N^2)): {time_dp_large:.6f} секунд")
    print(f"  Greedy (O(N)): {time_greedy_large:.6f} секунд")

    print("\n--- Дүгнэлт ---")
    print(f"Greedy нь {time_greedy_large/time_dp_large:.2f} дахин хурдан байна (Том бичвэрийн хувьд).")