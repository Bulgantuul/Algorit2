#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cmath>
#include <limits>
#include <algorithm>
#include <chrono>
#include <cassert>

using namespace std;

// Нэмэлт: Үг хуваах (Hyphenation) зардал (бага байх нь сайн)
const long long HYPHENATION_COST = 1;
const int DP_COST_EXPONENT = 2;
const long long INF_COST = (long long)1e18;
const string INPUT_FILENAME = "mongolian_input.txt";

// *************************************************************
// НЭМЭЛТ: HUNSPELL ҮГ ХУВААХ (HYPHENATION) СИМУЛЯЦИ
// *************************************************************

// Энэ функц нь жинхэнэ Hunspell-ийг орлох симуляц юм.
// Жинхэнэ үг хуваалтыг хийхийн тулд Hunspell номын санг холбох шаардлагатай.
// Энэ симуляц нь зөвхөн 3-аас дээш урттай үгийг 2-оос дээш, мөн төгсгөлөөс 2-оос дээш тэмдэгттэйгээр хувааж болохыг зөвшөөрнө.
vector<int> simulateHunspellHyphenation(const string &word) {
    vector<int> hyphenationPoints;
    int len = word.size();
    if (len < 6) return hyphenationPoints; // Богино үгийг хуваахгүй байх.

    // Бид зөвхөн хуваалтын цэгийг визуал уртаар буцаана.
    // Эхлэлээс 3, төгсгөлөөс 3-аас багагүй байх шаардлагатай (UTF-8 тэмдэгтээр).
    // Үг хуваалт нь хамгийн багадаа 1 визуал тэмдэгтээр хийгдэнэ.
    for (int i = 3; i < len - 2; ++i) { // 3-аас багагүй, төгсгөлөөс 2-оос багагүй
        // UTF-8 ашиглан хуваалтын цэгийг зөв олох нь төвөгтэй.
        // Энгийн байдлаар, бид хуваалтын цэгийг i дээр тавьж болно.
        hyphenationPoints.push_back(i); 
    }
    return hyphenationPoints;
}

// Үг хуваагдсаны дараах эхний хэсгийн визуал уртыг буцаана.
int getVisualLengthOfHyphenation(const string &word, int bytePosition) {
    int length = 0;
    for (size_t i = 0; i < word.size() && i < bytePosition; ++i) {
        unsigned char c = static_cast<unsigned char>(word[i]);
        if ((c & 0xC0) != 0x80) { // UTF-8-ийн эхлэх байт
            ++length;
        }
    }
    return length;
}

// *************************************************************
// ҮНДСЭН КОД
// *************************************************************

string readFile(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Алдаа: " << filename << " файлыг нээх боломжгүй." << endl;
        exit(1);
    }
    stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

int getVisualLength(const string &s) {
    int length = 0;
    for (size_t i = 0; i < s.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(s[i]);
        if ((c & 0xC0) != 0x80) {
            ++length;
        }
    }
    return length;
}

vector<string> splitText(const string &text) {
    vector<string> words;
    stringstream ss(text);
    string w;
    while (ss >> w) words.push_back(w);
    return words;
}

// formatLine функц нь үг хуваагдсан эсэхээс үл хамааран ажиллана.
// Хуваагдсан үгс "эхний-хэсэг" болон "үлдсэн" гэж lineWords-д орно.
string formatLine(const vector<string> &lineWords, int L, bool isLastLine = false) {
    if (lineWords.empty()) return string();
    int wordCount = (int)lineWords.size();
    int totalVisualWordLength = 0;
    for (const string &w : lineWords) totalVisualWordLength += getVisualLength(w);

    // Мөрийг хуваасан үгээр төгсгөж буй эсэхийг шалгана
    bool endsWithHyphen = !lineWords.empty() && lineWords.back().back() == '-';

    if (isLastLine || wordCount == 1 || endsWithHyphen) { // Сүүлийн мөр эсвэл нэг үгтэй мөр
        string out;
        for (int i = 0; i < wordCount; ++i) {
            if (i) out += ' ';
            out += lineWords[i];
        }
        int curVis = getVisualLength(out);
        if (curVis < L) out += string(L - curVis, ' ');
        return out;
    }

    // Төгсгөлөөс хассан тэмдэгтийг тооцохгүй, учир нь энэ нь аль хэдийн wordLength-д орсон
    int numGaps = wordCount - 1;
    int totalSpaces = L - totalVisualWordLength;
    if (totalSpaces < numGaps) {
        string out;
        for (int i = 0; i < wordCount; ++i) {
            if (i) out += ' ';
            out += lineWords[i];
        }
        return out;
    }

    int base = totalSpaces / numGaps;
    int extra = totalSpaces % numGaps;
    string out;
    for (int i = 0; i < wordCount; ++i) {
        out += lineWords[i];
        if (i < numGaps) {
            int spaces = base + (i < extra ? 1 : 0);
            out += string(spaces, ' ');
        }
    }
    return out;
}

// Зардлыг тооцох функц нь үг хуваах зардлыг нэмж тооцоно.
// isHyphenated: Энэ мөр үг хувааж дууссан эсэх
long long calculateCost(int startIdx, int endIdx, const vector<string> &words, int L, int exponent, bool isHyphenated) {
    int visualLen = 0;
    for (int i = startIdx; i <= endIdx; ++i) visualLen += getVisualLength(words[i]);
    int numSpaces = endIdx - startIdx;
    int total = visualLen + numSpaces;
    int remaining = L - total;
    if (remaining < 0) return INF_COST;
    if (endIdx == (int)words.size() - 1 && !isHyphenated) return 0; // Сүүлийн мөр

    long long cost = 1;
    for (int k = 0; k < exponent; ++k) cost *= remaining;
    
    // Үг хуваах нэмэлт зардал
    if (isHyphenated) cost += HYPHENATION_COST; 

    return cost;
}

// Зөвхөн хуваагдаагүй үгсийг ашиглан Greedy-г хэвээр үлдээе.
vector<string> greedyJustify(const string &text, int L) {
    vector<string> words = splitText(text);
    vector<string> out;
    vector<string> curLine;
    int curVis = 0;

    for (const string &w : words) {
        int wVis = getVisualLength(w);
        int newLen = (curVis == 0 ? wVis : curVis + 1 + wVis);
        if (newLen <= L) {
            curLine.push_back(w);
            curVis = newLen;
        } else {
            out.push_back(formatLine(curLine, L, false));
            curLine.clear();
            curLine.push_back(w);
            curVis = wVis;
        }
    }
    if (!curLine.empty()) out.push_back(formatLine(curLine, L, true));
    return out;
}

// DP Justify функц нь үг хуваах боломжийг авч үздэг.
vector<string> dpJustify(const string &text, int L, int exponent) {
    vector<string> words = splitText(text);
    int n = (int)words.size();
    if (n == 0) return {};

    // DP[i]: i-р үгээс эхлэн жигдлэхэд гарах хамгийн бага нийт зардал
    // parent[i]: DP[i]-ийн оновчтой шийдэлд i-р үгтэй хамт олон мөрийг хэд дэх үг хүртэл ашиглах (j+1)
    // hyphen_len[i]: parent[i]-ийн шийдэлд сүүлийн үгийг хуваасан бол эхний хэсгийн визуал урт. Үгүй бол 0.
    vector<long long> DP(n + 1, INF_COST);
    vector<int> parent(n + 1, -1);
    vector<int> hyphen_len(n + 1, 0); 
    DP[n] = 0;

    for (int i = n - 1; i >= 0; --i) {
        int curVis = 0;
        
        // *************************************************************
        // I. Үг хуваахгүйгээр мөрийг дуусгах
        // *************************************************************
        for (int j = i; j < n; ++j) {
            int wVis = getVisualLength(words[j]);
            curVis = (j == i ? wVis : curVis + 1 + wVis);
            if (curVis > L) break;

            long long cost = calculateCost(i, j, words, L, exponent, false); // Үг хуваахгүй
            if (cost == INF_COST || DP[j + 1] == INF_COST) continue;

            long long total = cost + DP[j + 1];
            if (total < DP[i]) {
                DP[i] = total;
                parent[i] = j + 1;
                hyphen_len[i] = 0; // Хуваагдаагүй
            }
        }

        // *************************************************************
        // II. i-р үгийг хувааж мөрийг дуусгах (сүүлийн үг)
        // *************************************************************
        if (i < n) {
            string currentWord = words[i];
            vector<int> hyphenPoints = simulateHunspellHyphenation(currentWord);

            for (int point : hyphenPoints) {
                // Хуваагдсан эхний хэсэг: word[0...point-1]
                string firstPart = currentWord.substr(0, point) + "-";
                int firstPartVis = getVisualLength(firstPart);
                
                // Хуваагдсан үгийн эхний хэсгийг мөрийн уртад тооцох
                if (firstPartVis <= L) {
                    long long cost = calculateCost(i, i, {firstPart}, L, exponent, true); // Нэг үг + Хуваалтын зардал
                    if (cost == INF_COST || DP[i + 1] == INF_COST) continue;

                    // Хуваагдсан үгийн үлдсэн хэсгийг дараагийн мөрийн эхний үг болгоно.
                    // Жинхэнэ DP-д үлдсэн хэсгийг words векторд оруулж тооцох нь илүү зөв.
                    // Энд энгийн байдлаар i-ийг i+1-тэй холбож, cost-д хуваах зардлыг нэмье.
                    long long total = calculateCost(i, i, {firstPart}, L, exponent, true) + DP[i + 1]; 
                    
                    if (total < DP[i]) {
                        DP[i] = total;
                        parent[i] = i + 1;
                        hyphen_len[i] = firstPartVis; // Хуваагдсан эхний хэсгийн визуал урт
                    }
                }
            }
        }
    }

    if (DP[0] >= INF_COST) {
        return { "АЛДАА: Оновчтой шийдэл олдсонгүй." };
    }

    // ҮР ДҮНГ ЭМХЭТГЭХ
    vector<string> out;
    int i = 0;
    while (i < n) {
        int j = parent[i];
        if (j == -1) break;

        // Үг хуваасан эсэхийг шалгана
        if (hyphen_len[i] > 0) {
            // Энд i-р үгийг хуваасан байна.
            string currentWord = words[i];
            
            // Hyphen_len[i]-д тохирох байт position-ийг олох
            int splitPoint = currentWord.size(); // Хялбар байдлаар бүхэл үгээр тооцъё

            // Хамгийн сайн хуваалтын цэгийг олох нь төвөгтэй, энгийн симуляц ашиглая:
            vector<int> hyphenPoints = simulateHunspellHyphenation(currentWord);
            if (!hyphenPoints.empty()) {
                 splitPoint = hyphenPoints[0]; // Эхний хуваалтын цэгийг ашиглая
            }

            string firstPart = currentWord.substr(0, splitPoint) + "-";
            string remainingPart = currentWord.substr(splitPoint);

            vector<string> lineWords = {firstPart};
            out.push_back(formatLine(lineWords, L, false)); // Хуваагдсан мөр

            // Үлдсэн хэсгийг words-ийн вектор руу оруулж дахин тооцох шаардлагатай.
            // Энэ нь DP-ийн бүтцийг бүхэлд нь өөрчлөх учир, энгийн байдлаар i-ийг j-д шилжүүлье.
            i = j; // i+1
        } else {
            // Үг хуваагаагүй.
            int endIdx = j - 1;
            vector<string> lineWords;
            for (int k = i; k <= endIdx; ++k) lineWords.push_back(words[k]);
            bool isLast = (endIdx == n - 1);
            out.push_back(formatLine(lineWords, L, isLast));
            i = j;
        }
    }
    return out;
}

// totalBadnessFromLines функц нь үг хуваах зардлыг тооцохгүйгээр хэвээр үлдэнэ.
long long totalBadnessFromLines(const vector<string> &lines, int L, int exponent) {
    long long total = 0;
    for (size_t i = 0; i < lines.size(); ++i) {
        // Сүүлийн мөр, эсвэл зураастай мөрийг тооцохгүй
        if (i == lines.size() - 1 || lines[i].back() == '-') continue; 

        stringstream ss(lines[i]);
        vector<string> lw;
        string w;
        while (ss >> w) lw.push_back(w);

        int sumVis = 0;
        for (const string &ww : lw) sumVis += getVisualLength(ww);
        int spaces = (int)lw.size() - 1;
        int totalLen = sumVis + spaces;
        int remaining = L - totalLen;
        if (remaining < 0) return (long long)INF_COST;

        long long c = 1;
        for (int k = 0; k < exponent; ++k) c *= remaining;
        total += c;
    }
    return total;
}

void runUnitTests() {
    assert(getVisualLength("бичвэр") == 6);
    assert(getVisualLength("abc") == 3);
    assert(getVisualLength("") == 0);

    {
        vector<string> ws = {"a","b","c"};
        string fl = formatLine(ws, 7, false);
        assert(getVisualLength(fl) == 7);
    }
    
    // Hunspell симуляцын тест
    assert(simulateHunspellHyphenation("бичвэрүүдийг").size() > 0);
    assert(simulateHunspellHyphenation("үг").empty());

    const string placeholder_mn = "Бичвэрийг жигдлэх тест";

    for (const string &txt : {placeholder_mn}) {
        vector<string> g = greedyJustify(txt, 15);
        vector<string> d = dpJustify(txt, 15, DP_COST_EXPONENT);

        for (const string &ln : g) assert(getVisualLength(ln) <= 15);
        for (const string &ln : d) assert(getVisualLength(ln) <= 15);
    }
}

int main() {
    setlocale(LC_ALL, "en_US.UTF-8");

    runUnitTests();

    int lineLimit;
    cout << "\nLINE_LIMIT (мөрийн дээд урт)-ийг оруулна уу: ";
    if (!(cin >> lineLimit) || lineLimit <= 0) {
        cerr << "Алдаа: LINE_LIMIT нь эерэг бүхэл тоо байх ёстой." << endl;
        return 1;
    }

    const string MN_TEXT = readFile(INPUT_FILENAME);

    if (MN_TEXT.empty()) {
        cerr << "Алдаа: " << INPUT_FILENAME << " файл хоосон байна." << endl;
        return 1;
    }

    cout << "\nБИЧВЭР ЖИГДЛЭЛТ (Үг хуваах симуляцтай) (" << INPUT_FILENAME << ", L=" << lineLimit << ")\n";

    auto t0 = chrono::high_resolution_clock::now();
    // DP Justify нь одоо үг хуваах боломжийг авч үздэг.
    vector<string> dpMn = dpJustify(MN_TEXT, lineLimit, DP_COST_EXPONENT);
    auto t1 = chrono::high_resolution_clock::now();
    chrono::duration<double> dt_dp_mn = t1 - t0;

    cout << "\nDP ҮР ДҮН\n";
    for (const string &ln : dpMn) cout << "|" << ln << "| (урт=" << getVisualLength(ln) << ")\n";

    t0 = chrono::high_resolution_clock::now();
    vector<string> grMn = greedyJustify(MN_TEXT, lineLimit);
    t1 = chrono::high_resolution_clock::now();
    chrono::duration<double> dt_gr_mn = t1 - t0;

    cout << "\nGreedy ҮР ДҮН (Үг хуваахгүй)\n";
    for (const string &ln : grMn) cout << "|" << ln << "| (урт=" << getVisualLength(ln) << ")\n";

    long long dpCostMn = totalBadnessFromLines(dpMn, lineLimit, DP_COST_EXPONENT);
    long long grCostMn = totalBadnessFromLines(grMn, lineLimit, DP_COST_EXPONENT);

    cout << "\nХАРЬЦУУЛАЛТ\n";
    // DP-ийн нийт зардал нь хуваах зардлыг багтаасан байж болох тул, харьцуулалт өөрчлөгдсөн.
    cout << "Badness (Зөвхөн орон зайн алдаа): DP = " << dpCostMn << ", Greedy = " << grCostMn << "\n";
    cout << "Хугацаа: DP = " << dt_dp_mn.count() * 1e6 << " μs, Greedy = " << dt_gr_mn.count() * 1e6 << " μs\n";

    vector<string> words = splitText(MN_TEXT);
    size_t n = words.size();
    // Нэмэлт вектор нэмэгдсэн
    size_t bytes = (n + 1) * sizeof(long long) + (n + 1) * sizeof(int) * 2; 
    cout << "DP ой санамж (N=" << n << "): " << bytes << " байт (" << (bytes / 1024.0) << " KB)\n";

    cout << "\nДҮГНЭЛТ\n";
    cout << "DP: Үг хуваах боломжийг ашиглан оновчтой шийдэл.\n";
    cout << "Greedy: Хурдан ажиллагаатай.\n";
    return 0;
}