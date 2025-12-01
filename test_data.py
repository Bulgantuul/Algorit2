# test_data.py
import random

LINE_LIMIT = 40  # L
NUM_WORDS_SMALL = 500
NUM_WORDS_LARGE = 5000

# Англи болон Монгол хэл дээрх оролтын бичвэрүүд
ENGLISH_TEXT = "Algorithms are great. Dynamic programming solves the text justification problem optimally. The Greedy approach is faster but does not guarantee the best overall solution. We must analyze both time complexity and the quality of the output."

# Монгол бичвэр (Тест хийхэд бэлэн)
MONGOLIAN_TEXT = "Оновчлолын алгоритмууд нь програм хангамжийн бүтцэд чухал үүрэг гүйцэтгэдэг. Динамик программчлал нь бичвэрийг хамгийн бага алдагдалтайгаар жигдлэх оновчтой шийдлийг олох боломжийг олгодог. Шуналтай арга нь хурдан боловч оновчтой бус байх магадлалтай. Бидний гол зорилго бол хурд болон үр дүнгийн чанарыг харьцуулах явдал юм."

def generate_random_text(num_words, avg_word_len=5):
    """Тест хийхэд зориулж санамсаргүй үгс бүхий бичвэр үүсгэх."""
    text = []
    for _ in range(num_words):
        word_len = random.randint(3, avg_word_len + 3)
        word = ''.join(random.choices('abcdefghijklmnopqrstuvwxyz', k=word_len))
        text.append(word)
    return " ".join(text)

# Хэмжилтийн бичвэрүүд
TEXT_SMALL = generate_random_text(NUM_WORDS_SMALL)
TEXT_LARGE = generate_random_text(NUM_WORDS_LARGE)