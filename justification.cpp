#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <cmath>
#include <limits>
#include <chrono>
#include <iomanip>
#include <algorithm>

using namespace std;

// Мөрийн дээд хязгаар
const int LINE_LIMIT = 40; 

// Хэт их алдагдлыг илэрхийлэх
const double INF = numeric_limits<double>::max();

// Мөрийг бүрэн жигдлэх (space-үүдийг тэнцүү хуваарилах)
string formatLine(const vector<string>& lineWords, int L) {
    if (lineWords.empty()) {
        return "";
    }
    
    int wordCount = lineWords.size();
    int totalWordLength = 0;
    for (const string& w : lineWords) {
        totalWordLength += w.length();
    }
    
    // Хэрэв нэг үгтэй бол зүүн тийш жигдлэнэ
    if (wordCount == 1) {
        return lineWords[0] + string(L - lineWords[0].length(), ' ');
    }
    
    int totalSpaces = L - totalWordLength;
    int numGaps = wordCount - 1;
    
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
            // Одоогийн урт + (нэг зай) + Үгийн урт
            newLength = currentLength + 1 + wordLen;
        }

        if (newLength <= L) {
            currentLine.push_back(word);
            currentLength = newLength;
        } else {
            // Мөрийг жигдлээд, шинэ мөр эхлүүлнэ
            justifiedLines.push_back(formatLine(currentLine, L));
            currentLine = {word};
            currentLength = wordLen;
        }
    }

    // Хамгийн сүүлийн мөр
    if (!currentLine.empty()) {
        string lastLine;
        for (size_t i = 0; i < currentLine.size(); ++i) {
            lastLine += currentLine[i];
            if (i < currentLine.size() - 1) {
                lastLine += ' ';
            }
        }
        justifiedLines.push_back(lastLine);
    }
    
    return justifiedLines;
}

// Алдагдлыг тооцох функц
double calculateCost(int startIdx, int endIdx, const vector<string>& words, int L) {
    int length = 0;
    for (int i = startIdx; i <= endIdx; ++i) {
        length += words[i].length();
    }
    
    int numSpaces = endIdx - startIdx;
    int totalLength = length + numSpaces;
    
    if (totalLength > L) {
        return INF;
    } 
    
    // Хамгийн сүүлийн мөр бол алдагдалгүй (зүүн тийш жигдлэнэ)
    if (endIdx == words.size() - 1) {
        return 0;
    }
    
    // Үлдсэн зайны куб: (L - length)^3
    int remainingSpace = L - totalLength;
    return pow(remainingSpace, 3);
}

vector<string> dpJustify(const string& text, int L) {
    vector<string> words = splitText(text);
    int n = words.size();
    
    // DP[i]: i-р үгээс эхлэх хамгийн бага нийт алдагдал
    vector<double> DP(n + 1, INF);
    // parent[i]: i-р үгээс эхлэх мөрийн оновчтой төгсгөл (j+1)
    vector<int> parent(n + 1, 0); 
    
    DP[n] = 0; // Суурь тохиолдол

    // Сүүлээс эхлэн тооцоолно (bottom-up)
    for (int i = n - 1; i >= 0; --i) {
        for (int j = i; j < n; ++j) {
            double cost = calculateCost(i, j, words, L);
            
            if (cost != INF) {
                double totalCost = cost + DP[j + 1];
                
                if (totalCost < DP[i]) {
                    DP[i] = totalCost;
                    parent[i] = j + 1; // Дараагийн мөрийн эхлэл
                }
            }
        }
    }

    // Шийдлийг сэргээнэ
    vector<string> justifiedLines;
    int i = 0;
    while (i < n) {
        int j = parent[i] - 1; // Оновчтой мөрийн төгсгөлийн индекс
        
        vector<string> lineWords;
        for (int k = i; k <= j; ++k) {
            lineWords.push_back(words[k]);
        }
        
        if (j == n - 1) {
            // Хамгийн сүүлийн мөр бол зүүн тийш жигдлэнэ
            string lastLine;
            for (size_t k = 0; k < lineWords.size(); ++k) {
                lastLine += lineWords[k];
                if (k < lineWords.size() - 1) {
                    lastLine += ' ';
                }
            }
            justifiedLines.push_back(lastLine);
        } else {
            // Бусад мөрүүдийг бүрэн жигдлэнэ
            justifiedLines.push_back(formatLine(lineWords, L));
        }
        i = j + 1;
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
    for (int i = 0; i < numWords; ++i) {
        int wordLen = rand() % 4 + 3; // 3-6 тэмдэгт
        for (int j = 0; j < wordLen; ++j) {
            text += (char)('a' + (rand() % 26));
        }
        if (i < numWords - 1) {
            text += ' ';
        }
    }
    return text;
}

int main() {
    // Монгол текст: Энэ кодонд кирил үсэг зөв хэвлэгдэх эсэх нь 
    // консол болон кодынхоо encoding-ээс (UTF-8) хамаарна.
    string MONGOLIAN_TEXT = "Оновчлолын алгоритмууд нь програм хангамжийн бүтцэд чухал үүрэг гүйцэтгэдэг. Динамик программчлал нь бичвэрийг хамгийн бага алдагдалтайгаар жигдлэх оновчтой шийдлийг олох боломжийг олгодог. Шуналтай арга нь хурдан боловч оновчтой бус байх магадлалтай. Бидний гол зорилго бол хурд болон үр дүнгийн чанарыг харьцуулах явдал юм.";

    const int N_SMALL = 500;
    const int N_LARGE = 5000;

    // 1. Үр дүнгийн харьцуулалт
    cout << "=====================================================" << endl;
    cout << "  АЖЛЫН ҮР ДҮНГИЙН ХАРЬЦУУЛАЛТ (L=" << LINE_LIMIT << ")" << endl;
    cout << "=====================================================" << endl;

    cout << "\n--- 1. DP АРГААР ЖИГДЛЭСЭН (Оновчтой шийдэл) ---" << endl;
    vector<string> dpResult = dpJustify(MONGOLIAN_TEXT, LINE_LIMIT);
    for (const string& line : dpResult) {
        cout << "|" << line << "|" << endl;
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

    srand(time(NULL));
    string textSmall = generateRandomText(N_SMALL);
    string textLarge = generateRandomText(N_LARGE);

    // Хэмжилт
    double timeDPSmall = measureTime([&]() { dpJustify(textSmall, LINE_LIMIT); });
    double timeGreedySmall = measureTime([&]() { greedyJustify(textSmall, LINE_LIMIT); });

    double timeDPLarge = measureTime([&]() { dpJustify(textLarge, LINE_LIMIT); });
    double timeGreedyLarge = measureTime([&]() { greedyJustify(textLarge, LINE_LIMIT); });

    cout << fixed << setprecision(6);
    cout << "N=" << N_SMALL << " (Жижиг Бичвэр):" << endl;
    cout << "  DP (O(N^2)): " << timeDPSmall << " секунд" << endl;
    cout << "  Greedy (O(N)): " << timeGreedySmall << " секунд" << endl;

    cout << "\nN=" << N_LARGE << " (Том Бичвэр):" << endl;
    cout << "  DP (O(N^2)): " << timeDPLarge << " секунд" << endl;
    cout << "  Greedy (O(N)): " << timeGreedyLarge << " секунд" << endl;

    // Дүгнэлт
    double speedDifference = 0;
    if (timeGreedyLarge > 0) {
        speedDifference = timeDPLarge / timeGreedyLarge;
    }

    cout << "\n--- Дүгнэлт ---" << endl;
    cout << "Онолын хувьд O(N^2) болох DP-ээс, O(N) болох Greedy илүү хурдан." << endl;
    cout << "Бодит туршилтаар, Greedy нь Том бичвэрийн хувьд DP-ээс " << speedDifference << " дахин хурдан ажиллаж байна." << endl;
    
    return 0;
}