# main_runner.py (ЗАСВАРЛАСАН ХУВИЛБАР)

import time
import justification_algs as ja
import test_data as td

# Хугацаа хэмжих функц (өөрчлөлтгүй)
def measure_time(func, text, L):
    """Функцийн гүйцэтгэлийн хугацааг хэмжих."""
    start_time = time.perf_counter()
    func(text, L)
    end_time = time.perf_counter()
    return end_time - start_time

def run_tests():
    L = td.LINE_LIMIT
    
    print(f"=====================================================")
    print(f"  АЖЛЫН ҮР ДҮНГИЙН ХАРЬЦУУЛАЛТ (L={L})")
    print(f"=====================================================")
    
    # 1. DP Гаралт (Монгол хэлээр)
    print("\n--- 1. DP АРГААР ЖИГДЛЭСЭН (Монгол, Оновчтой шийдэл) ---")
    dp_result_mn = ja.dp_justify(td.MONGOLIAN_TEXT, L)
    for line in dp_result_mn:
        print(f"|{line}|")

    # 2. Greedy Гаралт (Монгол хэлээр)
    print("\n--- 2. Greedy АРГААР ЖИГДЛЭСЭН (Монгол, Хурдан шийдэл) ---")
    greedy_result_mn = ja.greedy_justify(td.MONGOLIAN_TEXT, L)
    for line in greedy_result_mn:
        print(f"|{line}|")

    # ------------------ Гүйцэтгэлийн Хугацаа Хэмжих ------------------
    print("\n=====================================================")
    print(f"  ХУГАЦААНЫ ХАРЬЦУУЛАЛТ (Random Text, L={L})")
    print(f"=====================================================")
    
    # Хэмжилт хийх (Хуучин үр дүнгээс арай өөр утга гарч болно)
    time_dp_small = measure_time(ja.dp_justify, td.TEXT_SMALL, L)
    time_greedy_small = measure_time(ja.greedy_justify, td.TEXT_SMALL, L)

    time_dp_large = measure_time(ja.dp_justify, td.TEXT_LARGE, L)
    time_greedy_large = measure_time(ja.greedy_justify, td.TEXT_LARGE, L)

    print(f"N={td.NUM_WORDS_SMALL} (Жижиг Бичвэр):")
    print(f"  DP (O(N^2)): {time_dp_small:.6f} секунд")
    print(f"  Greedy (O(N)): {time_greedy_small:.6f} секунд")
    
    print(f"\nN={td.NUM_WORDS_LARGE} (Том Бичвэр):")
    print(f"  DP (O(N^2)): {time_dp_large:.6f} секунд")
    print(f"  Greedy (O(N)): {time_greedy_large:.6f} секунд")

    # Дүгнэлтийн томьёог зөв болгох: DP-ийн хугацааг Greedy-ийн хугацаанд хуваана.
    if time_greedy_large > 0:
        speed_difference = time_dp_large / time_greedy_large
        
        # Хэрэв зөрүү 1000-аас их бол таслалаас хойш тоог 0 байлгаж, 'мянган' гэх мэт үгээр илэрхийлэх нь зүйтэй.
        if speed_difference >= 1000:
            print("\n--- Дүгнэлт (Засварласан) ---")
            print(f"Онолын хувьд O(N^2) болох DP-ээс, O(N) болох Greedy илүү хурдан.")
            print(f"Бодит туршилтаар, Greedy нь Том бичвэрийн хувьд DP-ээс {speed_difference:,.0f} дахин хурдан ажиллаж байна.")
        else:
            print("\n--- Дүгнэлт (Засварласан) ---")
            print(f"Онолын хувьд O(N^2) болох DP-ээс, O(N) болох Greedy илүү хурдан.")
            print(f"Бодит туршилтаар, Greedy нь Том бичвэрийн хувьд DP-ээс {speed_difference:.2f} дахин хурдан ажиллаж байна.")

if __name__ == "__main__":
    run_tests()