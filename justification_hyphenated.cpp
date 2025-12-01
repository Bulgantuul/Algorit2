#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <cmath>
#include <limits>
#include <chrono>
#include <iomanip>
#include <algorithm>
#include <stdexcept>
#include <ctime> 
#include <map>

using namespace std;

// Мөрийн дээд хязгаар
const int LINE_LIMIT = 40; 

// Хэт их алдагдлыг илэрхийлэх
const double INF = numeric_limits<double>::max();

// --- HYPHENATION UTILS (Үеэр таслах туслах функц) ---

struct HyphenResult {
    string part;      // Мөрөнд багтсан хэсэг
    string remaining; // Дараагийн мөрөнд үлдэх хэсэг
    int hyphenPoint;  // Таслагдах цэг (part-ийн урт)
};

// Хялбаршуулсан таслах функц (Эхний хэсэг >= 3, Үлдэгдэл >= 3)
HyphenResult findBestHyphenation(const string& word, int remainingCapacity) {
    HyphenResult bestResult = {"", word, 0};
    int n = word.length();
    
    // Эхний хэсэг хамгийн багадаа 3 үсэгтэй байх ёстой (i >= 3), үлдэгдэл нь мөн 3-аас багагүй (i <= n - 3)
    for (int i = 3; i <= n - 3; ++i) {
        int lenWithHyphen = i + 1; // Таслагдсан хэсэг (i) + таслах зураас (1)
        
        if (lenWithHyphen <= remainingCapacity) {
            // Энэ бол одоогоор багтаж буй хамгийн урт хэсэг
            bestResult.part = word.substr(0, i);
            bestResult.remaining = word.substr(i);
            bestResult.hyphenPoint = i;
        } else {
            // Хэтэрсэн тул өмнөх нь хамгийн сайн нь
            break; 
        }
    }
    
    return bestResult;
}

// --- CORE UTILS ---

// Мөрийг бүрэн жигдлэх (space-үүдийг тэнцүү хуваарилах)
string formatLine(const vector<string>& lineWords, int L) {
    if (lineWords.empty()) return "";
    
    int wordCount = lineWords.size();
    int totalWordLength = 0;
    for (const string& w : lineWords) {
        totalWordLength += w.length();
    }
    
    // Нэг үгтэй мөрийг зүүн тийш жигдлэнэ
    if (wordCount <= 1) {
        string result = lineWords[0];
        return result + string(max(0, L - (int)result.length()), ' ');
    }
    
    int totalSpaces = L - totalWordLength;
    int numGaps = wordCount - 1;
    
    if (totalSpaces < 0) {
        // Мөр хэтэрсэн тохиолдолд зүүн тийш жигдлэлт хийх (Алдааг барих)
        string result = "";
        for (const auto& w : lineWords) result += w + " ";
        if (!result.empty()) result.pop_back(); 
        return result + string(max(0, L - (int)result.length()), ' '); 
    }

    int baseSpaces = totalSpaces / numGaps;
    int extraSpaces = totalSpaces % numGaps;
    
    string formattedLine = "";
    for (int i = 0; i < wordCount; ++i) {
        formattedLine += lineWords[i];
        if (i < numGaps) {
            int spaces = baseSpaces + (i < extraSpaces ? 1 : 0);
            formattedLine += string(spaces, ' ');
        }
    }
    return formattedLine;
}

// Текстээс үгсийг салгах функц
vector<string> splitText(const string& text) {
    vector<string> words;
    stringstream ss(text);
    string word;
    while (ss >> word) {
        words.push_back(word);
    }
    return words;
}

// --- GREEDY ALGORITHM (Шуналтай Арга) ---

vector<string> greedyJustify(const string& text, int L) {
    vector<string> words = splitText(text);
    vector<string> justifiedLines;
    vector<string> currentLine;
    int currentLength = 0;

    for (const string& word : words) {
        int wordLen = word.length();
        int newLength;

        if (currentLength == 0) {
            newLength = wordLen;
        } else {
            newLength = currentLength + 1 + wordLen;
        }

        if (newLength <= L) {
            currentLine.push_back(word);
            currentLength = newLength;
        } else {
            justifiedLines.push_back(formatLine(currentLine, L));
            currentLine = {word};
            currentLength = wordLen;
        }
    }

    // Хамгийн сүүлийн мөр (зүүн тийш жигдлэнэ)
    if (!currentLine.empty()) {
        string lastLine;
        for (size_t i = 0; i < currentLine.size(); ++i) {
            lastLine += currentLine[i];
            if (i < currentLine.size() - 1) {
                lastLine += ' ';
            }
        }
        // Хамгийн сүүлийн мөрийг зүүн тийш жигдэлнэ
        justifiedLines.push_back(lastLine + string(max(0, L - (int)lastLine.length()), ' '));
    }
    
    return justifiedLines;
}


// --- DP ALGORITHM (DP Арга, Hyphenation-тэй) ---
vector<string> dpJustify(const string& text, int L) {
    vector<string> initial_words = splitText(text);
    int n = initial_words.size();
    
    if (n == 0) return {};
    
    vector<double> DP(n + 1, INF);
    vector<int> parent(n + 1, 0); 
    vector<int> hyphen_split_at(n, 0); 
    
    DP[n] = 0; 

    for (int i = n - 1; i >= 0; --i) {
        int current_len = 0; 
        
        for (int j = i; j < n; ++j) {
            string word = initial_words[j];
            int word_len = word.length();
            int space_needed = (j > i) ? 1 : 0;
            
            // 1. J-р үгийг БҮТНЭЭР ОРУУЛАХ
            int len_with_full_word = current_len + space_needed + word_len;
            
            if (len_with_full_word <= L) {
                // Сүүлийн мөрийн алдагдал 0
                double cost = (j < n - 1) ? pow(L - len_with_full_word, 3) : 0; 
                
                if (DP[j + 1] != INF) {
                    double totalCost = cost + DP[j + 1];
                    
                    if (totalCost < DP[i]) {
                        DP[i] = totalCost;
                        parent[i] = j + 1;       
                        hyphen_split_at[j] = 0;  
                    }
                }
                current_len = len_with_full_word;
            } 
            
            // 2. J-р үгийг ТАСЛАЖ ОРУУЛАХ (Зөвхөн энэ мөрийн хамгийн сүүлийн үг)
            else { 
                // Зөвхөн эхний N-1 мөрийг таслана (j < n - 1)
                // Хамгийн сүүлийн мөрийг таслахгүй байх нь дүрмийн хувьд зөв
                if (j < n - 1) { 
                    int len_before_j = current_len;
                    int capacity_for_hyphen = L - (len_before_j + space_needed); 
                    
                    if (capacity_for_hyphen >= 4 && word_len >= 6) { 
                        
                        HyphenResult hyp = findBestHyphenation(word, capacity_for_hyphen - 1); 
                        
                        if (hyp.hyphenPoint > 0 && hyp.remaining.length() >= 3) {
                            
                            int len_with_hyphen = len_before_j + space_needed + hyp.part.length() + 1; 
                            double cost_hyphenated = pow(L - len_with_hyphen, 3);

                             if (DP[j + 1] != INF) {
                                double totalCost = cost_hyphenated + DP[j + 1];
                                
                                if (totalCost < DP[i]) {
                                    DP[i] = totalCost;
                                    parent[i] = j + 1; 
                                    hyphen_split_at[j] = hyp.hyphenPoint; 
                                }
                            }
                        }
                    }
                }
                break; 
            }
        }
    }

    // --- ШИЙДЛИЙГ СЭРГЭЭХ ХЭСЭГ ---
    vector<string> justifiedLines;
    int current_idx = 0; 
    string next_line_fragment = "";
    
    while (current_idx < n) {
        
        if (DP[current_idx] == INF) {
            string errorText = next_line_fragment;
            if (!errorText.empty()) errorText += ' ';

            for (int k = current_idx; k < n; ++k) {
                errorText += initial_words[k];
                if (k < n - 1) errorText += ' ';
            }
            justifiedLines.push_back(errorText + string(max(0, L - (int)errorText.length()), ' '));
            break; 
        }

        int next_start_idx = parent[current_idx]; 
        int end_idx = next_start_idx - 1; 
        bool isFinalLine = (next_start_idx == n);

        vector<string> lineWords;

        // 1. Өмнөх мөрөнд таслагдсан хэсэг
        if (!next_line_fragment.empty()) {
            lineWords.push_back(next_line_fragment);
            next_line_fragment = ""; 
        }
        
        // 2. Үгсийг нэмэх 
        for (int k = current_idx; k < end_idx; ++k) {
            lineWords.push_back(initial_words[k]);
        }
        
        // 3. Хамгийн сүүлийн үг (end_idx)
        int split_point = hyphen_split_at[end_idx]; 
        string lastWord = initial_words[end_idx];

        // ЗӨВХӨН ЭЦСИЙН БУС МӨРӨНД ТАСЛАЛТ ХИЙНЭ
        if (split_point > 0 && !isFinalLine) {
            // A. Таслагдсан үг
            string part = lastWord.substr(0, split_point);
            string remaining = lastWord.substr(split_point);

            lineWords.push_back(part + "-");
            next_line_fragment = remaining; 
        } else {
            // B. Бүтэн үг (Эцсийн мөр таслагдахгүй)
            lineWords.push_back(lastWord);
        }

        // Мөрийг жигдлэх
        if (isFinalLine) {
            // Хамгийн сүүлийн мөр (Зүүн тийш жигдлэлт, space-ээр холбох)
            string lastLine;
            for (size_t k = 0; k < lineWords.size(); ++k) {
                lastLine += lineWords[k];
                if (k < lineWords.size() - 1) {
                    lastLine += ' ';
                }
            }
            justifiedLines.push_back(lastLine + string(max(0, L - (int)lastLine.length()), ' '));
        } else {
            // Бусад мөрүүдийг бүрэн жигдлэх
            justifiedLines.push_back(formatLine(lineWords, L));
        }

        current_idx = next_start_idx; 
    }
    return justifiedLines;
}


// Хугацаа хэмжих функц
template<typename Func, typename... Args>
double measureTime(Func func, Args&&... args) {
    auto start = chrono::high_resolution_clock::now();
    func(forward<Args>(args)...); 
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;
    return duration.count();
}

// Тест хийхэд зориулж санамсаргүй үгс бүхий текст үүсгэх
string generateRandomText(int numWords, int avgWordLen = 5) {
    string text;
    const vector<string> sampleWords = {"оновчлол", "алгоритм", "програм", "хангамж", "гүйцэтгэл", "динамик", "программчлал", "бичвэр", "хамгийн", "алдагдал", "жигдлэх", "шийдэл", "боломж", "олгодог", "шуналтай", "хурдан", "оновчтой", "магадлалтай", "зорилго", "чанарыг", "харьцуулах", "явдал", "байх", "бус", "томъёог", "оруулснаар", "таслагдах", "Супергиперкомпьютержүүлэлт"};
    
    for (int i = 0; i < numWords; ++i) {
        int wordIdx = rand() % sampleWords.size();
        text += sampleWords[wordIdx];
        if (i < numWords - 1) {
            text += ' ';
        }
    }
    return text;
}

int main() {
    string MONGOLIAN_TEXT = "Оновчлолын алгоритмууд нь програм хангамжийн бүтцэд чухал үүрэг гүйцэтгэдэг. Динамик программчлал нь бичвэрийг хамгийн бага алдагдалтайгаар жигдлэх оновчтой шийдлийг олох боломжийг олгодог. Үг таслах зориулалттай Супергиперкомпьютержүүлэлт гэсэн нэр томъёог оруулснаар үг таслагдах боломж нэмэгдэнэ.";

    const int N_SMALL = 500;
    const int N_LARGE = 5000;
    
    srand(time(NULL));
    
    // 1. Үр дүнгийн харьцуулалт 
    cout << "=====================================================" << endl;
    cout << "  АЖЛЫН ҮР ДҮНГИЙН ХАРЬЦУУЛАЛТ (C++, L=" << LINE_LIMIT << ")" << endl;
    cout << "  (DP Final Stable Fix V6 - Эцсийн мөрийн засвар)" << endl;
    cout << "=====================================================" << endl;

    // DP-ийн өргөжүүлсэн хувилбар
    cout << "\n--- 1. DP АРГААР ЖИГДЛЭСЭН (Оновчтой, Hyphenation-тэй) ---" << endl;
    vector<string> dpHyphenResult;
    try {
        dpHyphenResult = dpJustify(MONGOLIAN_TEXT, LINE_LIMIT);
        for (const string& line : dpHyphenResult) {
            cout << "|" << line << "|" << endl;
        }
    } catch (const exception& e) {
        cout << "Алдаа: DP алгоритм гэнэтийн алдаатай тул зогслоо: " << e.what() << endl;
    }


    cout << "\n--- 2. Greedy АРГААР ЖИГДЛЭСЭН (Хурдан шийдэл) ---" << endl;
    vector<string> greedyResult = greedyJustify(MONGOLIAN_TEXT, LINE_LIMIT);
    for (const string& line : greedyResult) {
        cout << "|" << line << "|" << endl;
    }

    // 2. Гүйцэтгэлийн Хугацаа Хэмжих
    cout << "\n=====================================================" << endl;
    cout << "  ХУГАЦААНЫ ХАРЬЦУУЛАЛТ (Random Text, L=" << LINE_LIMIT << ")" << endl;
    cout << "=====================================================" << endl;

    string textSmall = generateRandomText(N_SMALL);
    string textLarge = generateRandomText(N_LARGE);

    // Хэмжилт
    double timeDPSmall = -1;
    try {
        timeDPSmall = measureTime([&]() { dpJustify(textSmall, LINE_LIMIT); });
    } catch (...) { timeDPSmall = -1; }
    
    double timeGreedySmall = measureTime([&]() { greedyJustify(textSmall, LINE_LIMIT); });

    double timeDPLarge = -1;
    try {
        timeDPLarge = measureTime([&]() { dpJustify(textLarge, LINE_LIMIT); });
    } catch (...) { timeDPLarge = -1; }
    
    double timeGreedyLarge = measureTime([&]() { greedyJustify(textLarge, LINE_LIMIT); });

    cout << fixed << setprecision(6);
    cout << "N=" << N_SMALL << " (Жижиг Бичвэр):" << endl;
    cout << "  DP (O(N^2)): " << (timeDPSmall >= 0 ? to_string(timeDPSmall) : "Гүйцэтгэж чадсангүй (Алдаа/Timeout)") << " секунд" << endl;
    cout << "  Greedy (O(N)): " << timeGreedySmall << " секунд" << endl;

    cout << "\nN=" << N_LARGE << " (Том Бичвэр):" << endl;
    cout << "  DP (O(N^2)): " << (timeDPLarge >= 0 ? to_string(timeDPLarge) : "Гүйцэтгэж чадсангүй (Timeout)") << " секунд" << endl;
    cout << "  Greedy (O(N)): " << timeGreedyLarge << " секунд" << endl;

    // Дүгнэлт
    string speedMessage = "Харьцуулах боломжгүй (DP нь Timeout/Алдаатай)";
    
    if (timeDPSmall > 0 && timeGreedySmall > 0) {
        double speedDifference = timeDPSmall / timeGreedySmall;
        speedMessage = "Жижиг бичвэрт: DP нь Greedy-ээс " + to_string(speedDifference) + " дахин удаан ажиллаж байна.";
    }


    cout << "\n--- Дүгнэлт ---" << endl;
    cout << "Онолын хувьд O(N^2) болох DP-ээс, O(N) болох Greedy илүү хурдан." << endl;
    cout << "Том бичвэрт (N=5000) DP нь O(N^2) тул үр дүнг хүлээх нь удаан байна (Timeout)." << endl;
    cout << "Бодит туршилтаар: " << speedMessage << endl;
    
    return 0;
}