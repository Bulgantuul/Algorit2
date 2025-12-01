#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <cmath>
#include <limits>
#include <algorithm>
#include <chrono> // Гүйцэтгэлийн хугацааг хэмжихэд зориулав

using namespace std;

// Мөрийн дээд хязгаар (Харагдах тэмдэгтийн тоо)
const int LINE_LIMIT = 70; 
// DP-д ашиглах алдагдлын зэрэг (2-Квадрат, 3-Куб гэх мэт)
const int DP_COST_EXPONENT = 2; 

// Хэт их алдагдлыг илэрхийлэхэд ашиглах маш том тоо (long long-д шилжүүлсэн)
const long long INF_COST = 1e18; 

/**
 * @brief UTF-8 кодлолтой мөрийн харагдах тэмдэгтийн уртыг тооцох.
 * Энэ нь байтын дарааллаас зөвхөн эхлэлийн байтыг (шинэ тэмдэгт) тоолно.
 * @param s UTF-8 кодлолтой string.
 * @return int Харагдах тэмдэгтийн тоо.
 */
int getVisualLength(const string& s) {
    int length = 0;
    for (size_t i = 0; i < s.length(); ++i) {
        // UTF-8-ийн үргэлжлэл байт (0x80 - 0xBF) биш бол шинэ тэмдэгт гэж тооцно.
        if ((s[i] & 0xC0) != 0x80) {
            length++;
        }
    }
    return length;
}

/**
 * @brief Мөрийг бүрэн жигдлэх (Full Justification).
 * Үлдсэн зайг үг хоорондын зайнд тэнцүү хуваарилна.
 *
 * @param lineWords Тухайн мөрөн дэх үгсийн вектор.
 * @param L Мөрийн дээд хязгаар (LINE_LIMIT).
 * @param isLastLine Хэрэв сүүлийн мөр бол true (зүүн тийш жигдлэнэ).
 * @return string Бүрэн жигдлэгдсэн мөр.
 */
string formatLine(const vector<string>& lineWords, int L, bool isLastLine = false) {
    if (lineWords.empty()) {
        return "";
    }
    
    int wordCount = lineWords.size();
    int totalVisualWordLength = 0;
    
    for (const string& w : lineWords) {
        totalVisualWordLength += getVisualLength(w);
    }
    
    // Сүүлийн мөр эсвэл нэг үгтэй бол зүүн тийш жигдлэнэ.
    if (isLastLine || wordCount == 1) {
        string lastLine = "";
        for (size_t i = 0; i < lineWords.size(); ++i) {
            lastLine += lineWords[i];
            if (i < lineWords.size() - 1) {
                lastLine += ' ';
            }
        }
        // Харагдах уртыг тооцож, үлдсэн зайг ' ' тэмдэгтээр дүүргэнэ.
        int currentVisualLength = getVisualLength(lastLine);
        return lastLine + string(max(0, L - currentVisualLength), ' ');
    }
    
    // Бүрэн жигдлэх (Full Justification)
    int totalSpaces = L - totalVisualWordLength;
    int numGaps = wordCount - 1;

    // Мөр хэтэрсэн эсэхийг шалгах (DP болон Greedy нь үүнийг аль хэдийн баталгаажуулсан)
    if (totalSpaces < numGaps) { 
        string overflowLine = "";
         for (size_t i = 0; i < lineWords.size(); ++i) {
            overflowLine += lineWords[i];
            if (i < lineWords.size() - 1) {
                overflowLine += ' ';
            }
        }
        return overflowLine;
    }
    
    // Нэг зайнд оногдох зайны үндсэн тоо
    int baseSpaces = totalSpaces / numGaps;
    // Үлдсэн зайны тоо (эдгээрийг эхний хэдэн зайнд нэмнэ)
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

/**
 * @brief Текстээс үгсийг салгах функц.
 */
vector<string> splitText(const string& text) {
    vector<string> words;
    stringstream ss(text);
    string word;
    while (ss >> word) {
        words.push_back(word);
    }
    return words;
}

/**
 * @brief Greedy алгоритм ашиглан текстийг жигдлэх.
 */
vector<string> greedyJustify(const string& text, int L) {
    vector<string> words = splitText(text);
    vector<string> justifiedLines;
    vector<string> currentLine;
    int currentLength = 0; 

    for (const string& word : words) {
        int wordVisualLen = getVisualLength(word);
        int newLength;

        if (currentLength == 0) {
            newLength = wordVisualLen;
        } else {
            newLength = currentLength + 1 + wordVisualLen;
        }

        if (newLength <= L) {
            currentLine.push_back(word);
            currentLength = newLength;
        } else {
            justifiedLines.push_back(formatLine(currentLine, L));
            currentLine = {word};
            currentLength = wordVisualLen;
        }
    }

    // Хамгийн сүүлийн мөр
    if (!currentLine.empty()) {
        justifiedLines.push_back(formatLine(currentLine, L, true)); 
    }
    
    return justifiedLines;
}

/**
 * @brief Тухайн мөрийн алдагдлыг тооцох функц.
 * Энэ нь зөвхөн үлдсэн зайнаас үүдэх өртгийг тооцдог. 
 * Мөрийн уртыг хянах ажлыг dpJustify функц гүйцэтгэнэ.
 */
long long calculateCost(int startIdx, int endIdx, const vector<string>& words, int L, int exponent) {
    int visualLength = 0;
    for (int i = startIdx; i <= endIdx; ++i) {
        visualLength += getVisualLength(words[i]);
    }
    
    // Үг хоорондын зайны тоо (N-1)
    int numSpaces = endIdx - startIdx;
    
    // Мөрний нийт урт (үг + зай)
    int totalVisualLength = visualLength + numSpaces; 
    
    // Үлдсэн зай
    int remainingSpace = L - totalVisualLength;
    
    // Хэрэв үлдсэн зай сөрөг бол (хязгаараас хэтэрсэн), INF_COST буцаана.
    if (remainingSpace < 0) { 
        return INF_COST;
    }
    
    // Хэрэв үлдсэн зай 0 бол өртөг 0, бусад тохиолдолд үлдсэн зайн зэргийг тооцно.
    if (exponent == 0) return 1; 
    
    // Манай тохиолдолд exponent=2 (Квадрат алдагдал)
    long long cost = 1;
    for (int k = 0; k < exponent; ++k) {
        cost *= remainingSpace;
    }
    return cost;
}

/**
 * @brief Dynamic Programming алгоритм ашиглан текстийг жигдлэх.
 * Хамгийн бага нийт алдагдалтай жигдлэлтийг олно.
 */
vector<string> dpJustify(const string& text, int L, int exponent) {
    vector<string> words = splitText(text);
    int n = words.size();
    
    // DP[i] нь i-р үгээс эхлэх үгсийг жигдлэхэд гарах хамгийн бага нийт өртөг
    vector<long long> DP(n + 1, INF_COST);
    // parent[i] нь i-р үгээс эхлэх хамгийн оновчтой мөрний төгсгөлийн дараагийн индекс (j+1)
    vector<int> parent(n + 1, 0); 
    
    DP[n] = 0; // Үлдэгдэл үг байхгүй тул өртөг 0

    // Сүүлийн үгээс эхлэн урагш тооцно
    for (int i = n - 1; i >= 0; --i) {
        int currentVisualLength = 0; // i-р үгээс эхэлсэн мөрний явцын урт
        
        // i-р үгээс эхлээд j-р үгээр дуусах боломжит мөрүүдийг шалгана
        for (int j = i; j < n; ++j) {
            int wordVisualLen = getVisualLength(words[j]);
            
            if (i == j) {
                currentVisualLength = wordVisualLen;
            } else {
                // Үг хоорондын зайг (1) нэмнэ
                currentVisualLength += 1 + wordVisualLen;
            }
            
            // Хэрэв мөрний урт хязгаараас хэтэрвэл, цаашид үг нэмж шалгах шаардлагагүй.
            if (currentVisualLength > L) {
                break; // j гогцооноос гарч, i-ийн дараагийн эхлэлийг тооцоолно.
            }

            // Мөр (i-ээс j хүртэл) хязгаарт багтаж байгаа тул өртгийг тооцно.
            long long lineCost = calculateCost(i, j, words, L, exponent);
            
            // DP[j + 1] нь дараагийн хэсгийн хамгийн бага өртөг.
            // Хэрэв дараагийн хэсэг нь боломжгүй (DP[j+1] == INF_COST) бол энэ замыг алгасна.
            if (DP[j + 1] == INF_COST) {
                // DP[j+1] боломжгүй бол энэ нь тухайн текстийн төгсгөл хүртэл жигдлэх боломжгүй гэсэн үг. 
                // Гэхдээ сүүлийн мөрийн тохиолдлыг доор DP[n]=0-т хамруулсан. 
                // Хэрэв DP[n] бус, жишээлбэл DP[n-1] нь INF байвал, n-1-ээс эхлэх боломжит мөр байхгүй гэсэн үг.
                continue; 
            }

            // Хэрэв энэ мөр нь баримт бичгийн сүүлийн мөр бол өртөг нь 0 (зүүн тийш жигдлэнэ)
            long long currentCost = (j == n - 1) ? 0 : lineCost; 
            
            // Нийт өртөг = Одоогийн мөрийн өртөг + Үлдэгдэл хэсгийн хамгийн бага өртөг (DP[j + 1])
            long long totalCost = currentCost + DP[j + 1];
            
            // Хэрэв шинэ нийт өртөг оновчтой бол шинэчилнэ.
            if (totalCost < DP[i]) {
                DP[i] = totalCost;
                parent[i] = j + 1; // Дараагийн мөрийн эхлэлийг тэмдэглэнэ
            }
        }
    }

    // Шийдлийг сэргээнэ
    vector<string> justifiedLines;
    int i = 0;
    while (i < n) {
        int j = parent[i] - 1; // Оновчтой мөрийн төгсгөлийн индекс
        
        // DP нь шийдэл олсонгүй (жишээ нь, нэг үг нь хэт урт)
        if (parent[i] == 0) {
             break;
        }

        vector<string> lineWords;
        for (int k = i; k <= j; ++k) {
            lineWords.push_back(words[k]);
        }
        
        bool isLastLine = (j == n - 1); // Энэ мөр нь баримт бичгийн сүүлийн мөр мөн үү?
        justifiedLines.push_back(formatLine(lineWords, L, isLastLine));
        
        i = j + 1; // Дараагийн мөрний эхлэл
    }
    
    // Хэрэв DP[0] INF_COST байсан бол (бүхэл текстийг жигдлэх боломжгүй)
    if (DP[0] >= INF_COST && justifiedLines.empty()) {
        return {"!!! АЛДАА: DP нь шийдэл олсонгүй (Урт > LINE_LIMIT-тэй үг байж магадгүй) !!!"};
    }
    
    return justifiedLines;
}

int main() {
    // Монгол текст
    const string MONGOLIAN_TEXT = "Оновчлолын алгоритмууд нь програм хангамжийн бүтцэд чухал үүрэг гүйцэтгэдэг. Динамик программчлал нь бичвэрийг хамгийн бага алдагдалтайгаар жигдлэх оновчтой шийдлийг олох боломжийг олгодог. Шуналтай арга нь хурдан боловч оновчтой бус байх магадлалтай. Бидний гол зорилго бол хурд болон үр дүнгийн чанарыг харьцуулах явдал юм.";

    // 1. Үр дүнгийн харьцуулалт
    cout << "========================================================================" << endl;
    cout << "  АЖЛЫН ҮР ДҮНГИЙН ХАРЬЦУУЛАЛТ (C++, L=" << LINE_LIMIT << " тэмдэгт)" << endl;
    cout << "  (Урт: " << LINE_LIMIT << ") гэсэн тоо гарч байвал жигдлэлт зөв байна." << endl;
    cout << "========================================================================" << endl;

    // --- DP: Гүйцэтгэлийн хугацааг хэмжих (Start) ---
    auto start_dp = chrono::high_resolution_clock::now();
    vector<string> dpResult = dpJustify(MONGOLIAN_TEXT, LINE_LIMIT, DP_COST_EXPONENT);
    auto end_dp = chrono::high_resolution_clock::now();
    chrono::duration<double> duration_dp = end_dp - start_dp;
    // --- DP: Гүйцэтгэлийн хугацааг хэмжих (End) ---

    cout << "\n--- 1. DP АРГААР ЖИГДЛЭСЭН (Оновчтой шийдэл, L=" << LINE_LIMIT << ") ---" << endl;
    for (const string& line : dpResult) {
        // Мөрийн төгсгөлд визуал уртыг хэвлэж, жигдлэлтийг баталгаажуулна.
        cout << "|" << line << "|" << " (Урт: " << getVisualLength(line) << ")" << endl;
    }

    // --- Greedy: Гүйцэтгэлийн хугацааг хэмжих (Start) ---
    auto start_greedy = chrono::high_resolution_clock::now();
    vector<string> greedyResult = greedyJustify(MONGOLIAN_TEXT, LINE_LIMIT);
    auto end_greedy = chrono::high_resolution_clock::now();
    chrono::duration<double> duration_greedy = end_greedy - start_greedy;
    // --- Greedy: Гүйцэтгэлийн хугацааг хэмжих (End) ---

    cout << "\n--- 2. Greedy АРГААР ЖИГДЛЭСЭН (Хурдан шийдэл, L=" << LINE_LIMIT << ") ---" << endl;
    for (const string& line : greedyResult) {
        cout << "|" << line << "|" << " (Урт: " << getVisualLength(line) << ")" << endl;
    }
    
    // 2. Хугацааны харьцуулалт (Assignment Requirement #2)
    cout << "\n========================================================================" << endl;
    cout << "  ХУГАЦААНЫ ХАРЬЦУУЛАЛТ (C++ Chrono-гоор хэмжсэн)" << endl;
    cout << "  N=Үгсийн тоо. (Онолын хувьд: DP: O(N^2), Greedy: O(N))" << endl;
    cout << "========================================================================" << endl;
    cout << "  DP (O(N^2)) гүйцэтгэлийн хугацаа: " << duration_dp.count() * 1e6 << " μs" << endl;
    cout << "  Greedy (O(N)) гүйцэтгэлийн хугацаа: " << duration_greedy.count() * 1e6 << " μs" << endl;
    cout << "  (Жич: Бодит гүйцэтгэлийг үнэн зөв хэмжихийн тулд том хэмжээний текст ашиглана уу)" << endl;


    return 0;
}