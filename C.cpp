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

const int DP_COST_EXPONENT = 2;
const long long INF_COST = (long long)1e18;
const string INPUT_FILENAME = "mongolian_input.txt";

string readFile(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error" << filename << endl;
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

string formatLine(const vector<string> &lineWords, int L, bool isLastLine = false) {
    if (lineWords.empty()) return string();
    int wordCount = (int)lineWords.size();
    int totalVisualWordLength = 0;
    for (const string &w : lineWords) totalVisualWordLength += getVisualLength(w);

    if (isLastLine || wordCount == 1) {
        string out;
        for (int i = 0; i < wordCount; ++i) {
            if (i) out += ' ';
            out += lineWords[i];
        }
        int curVis = getVisualLength(out);
        if (curVis < L) out += string(L - curVis, ' ');
        return out;
    }

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

long long calculateCost(int startIdx, int endIdx, const vector<string> &words, int L, int exponent) {
    int visualLen = 0;
    for (int i = startIdx; i <= endIdx; ++i) visualLen += getVisualLength(words[i]);
    int numSpaces = endIdx - startIdx;
    int total = visualLen + numSpaces;
    int remaining = L - total;
    if (remaining < 0) return INF_COST;
    if (endIdx == (int)words.size() - 1) return 0;

    long long c = 1;
    for (int k = 0; k < exponent; ++k) c *= remaining;
    return c;
}

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

long long totalBadnessFromLines(const vector<string> &lines, int L, int exponent) {
    long long total = 0;
    for (size_t i = 0; i < lines.size(); ++i) {
        if (i == lines.size() - 1) continue;

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

vector<string> dpJustify(const string &text, int L, int exponent) {
    vector<string> words = splitText(text);
    int n = (int)words.size();
    if (n == 0) return {};

    vector<long long> DP(n + 1, INF_COST);
    vector<int> parent(n + 1, -1);
    DP[n] = 0;

    for (int i = n - 1; i >= 0; --i) {
        int curVis = 0;
        for (int j = i; j < n; ++j) {
            int wVis = getVisualLength(words[j]);
            curVis = (j == i ? wVis : curVis + 1 + wVis);
            if (curVis > L) break;

            long long cost = calculateCost(i, j, words, L, exponent);
            if (cost == INF_COST) continue;
            if (DP[j + 1] == INF_COST) continue;

            long long total = cost + DP[j + 1];
            if (total < DP[i]) {
                DP[i] = total;
                parent[i] = j + 1;
            }
        }
    }

    if (DP[0] >= INF_COST) {
        return { "АЛДАА" };
    }

    vector<string> out;
    int i = 0;
    while (i < n) {
        int j = parent[i];
        if (j == -1) break;
        int endIdx = j - 1;
        vector<string> lineWords;
        for (int k = i; k <= endIdx; ++k) lineWords.push_back(words[k]);
        bool isLast = (endIdx == n - 1);
        out.push_back(formatLine(lineWords, L, isLast));
        i = j;
    }
    return out;
}

void runUnitTests() {
    assert(getVisualLength("бичвэр") == 6);
    assert(getVisualLength("abc") == 3);
    assert(getVisualLength("") == 0);

    {
        vector<string> ws = {"a","b","c"};
        string fl = formatLine(ws, 7, false);
        assert(getVisualLength(fl) == 7);
        assert(fl.find("a") != string::npos);
        assert(fl.find("b") != string::npos);
        assert(fl.find("c") != string::npos);
    }

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

    cout << "\nБИЧВЭР ЖИГДЛЭЛТ (" << INPUT_FILENAME << ", L=" << lineLimit << ")\n";

    auto t0 = chrono::high_resolution_clock::now();
    vector<string> dpMn = dpJustify(MN_TEXT, lineLimit, DP_COST_EXPONENT);
    auto t1 = chrono::high_resolution_clock::now();
    chrono::duration<double> dt_dp_mn = t1 - t0;

    cout << "\nDP ҮР ДҮН\n";
    for (const string &ln : dpMn) cout << "|" << ln << "| (урт=" << getVisualLength(ln) << ")\n";

    t0 = chrono::high_resolution_clock::now();
    vector<string> grMn = greedyJustify(MN_TEXT, lineLimit);
    t1 = chrono::high_resolution_clock::now();
    chrono::duration<double> dt_gr_mn = t1 - t0;

    cout << "\nGreedy ҮР ДҮН\n";
    for (const string &ln : grMn) cout << "|" << ln << "| (урт=" << getVisualLength(ln) << ")\n";

    long long dpCostMn = totalBadnessFromLines(dpMn, lineLimit, DP_COST_EXPONENT);
    long long grCostMn = totalBadnessFromLines(grMn, lineLimit, DP_COST_EXPONENT);

    cout << "\nХАРЬЦУУЛАЛТ\n";
    cout << "Badness: DP = " << dpCostMn << ", Greedy = " << grCostMn << "\n";
    cout << "Хугацаа: DP = " << dt_dp_mn.count() * 1e6 << " μs, Greedy = " << dt_gr_mn.count() * 1e6 << " μs\n";

    vector<string> words = splitText(MN_TEXT);
    size_t n = words.size();
    size_t bytes = (n + 1) * sizeof(long long) + (n + 1) * sizeof(int);
    cout << "DP ой санамж (N=" << n << "): " << bytes << " байт (" << (bytes / 1024.0) << " KB)\n";

    cout << "\nДҮГНЭЛТ\n";
    cout << "DP: Оновчтой шийдэл.\n";
    cout << "Greedy: Хурдан ажиллагаатай.\n";
    return 0;
}
