#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <chrono>

using namespace std;

const long long INF_COST = 1e15;
const int DP_COST_EXPONENT = 2;
const int HYPHEN_PENALTY = 5;

// --- UTF-8 Safe Helpers ---
int getVisualLength(const string &s) {
    int length = 0;
    for (size_t i = 0; i < s.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(s[i]);
        if ((c & 0xC0) != 0x80) ++length;
    }
    return length;
}

string getUTF8Substr(const string& s, int charCount) {
    int bytes = 0, charsFound = 0;
    while (charsFound < charCount && bytes < (int)s.size()) {
        unsigned char c = static_cast<unsigned char>(s[bytes]);
        if ((c & 0xC0) != 0x80) charsFound++;
        bytes++;
        while (bytes < (int)s.size() && (static_cast<unsigned char>(s[bytes]) & 0xC0) == 0x80) bytes++;
    }
    return s.substr(0, bytes);
}

void solveDP(const vector<string>& words, int L) {
    int n = words.size();
    vector<long long> dp(n + 1, INF_COST);
    vector<int> nextWord(n + 1, -1);
    vector<string> lineContent(n + 1, "");

    dp[n] = 0;

    for (int i = n - 1; i >= 0; --i) {
        string currentLineText = "";
        int currentLineVis = 0;

        for (int j = i; j < n; ++j) {
            string word = words[j];
            int wVis = getVisualLength(word);
            int spaceNeeded = (currentLineVis == 0 ? 0 : 1);

            // 1. Try whole word
            if (currentLineVis + spaceNeeded + wVis <= L) {
                if (currentLineVis > 0) currentLineText += " ";
                currentLineText += word;
                currentLineVis += spaceNeeded + wVis;

                long long badness = (j == n - 1) ? 0 : pow(L - currentLineVis, DP_COST_EXPONENT);
                if (dp[j + 1] != INF_COST && (badness + dp[j + 1] < dp[i])) {
                    dp[i] = badness + dp[j + 1];
                    nextWord[i] = j + 1;
                    lineContent[i] = currentLineText;
                }
            }

            // 2. Try Hyphenation split (Only if the word doesn't fit or we want to fill space)
            if (wVis > 4) {
                for (int s = 2; s <= wVis - 2; ++s) {
                    string p1 = getUTF8Substr(word, s) + "-";
                    int p1Vis = getVisualLength(p1);
                    int splitVis = currentLineVis + (currentLineVis == 0 ? 0 : 1) + p1Vis;

                    if (splitVis <= L) {
                        long long badness = (j == n - 1 && currentLineVis == 0) ? 0 : pow(L - splitVis, DP_COST_EXPONENT) + HYPHEN_PENALTY;
                        if (dp[j + 1] != INF_COST && (badness + dp[j + 1] < dp[i])) {
                            dp[i] = badness + dp[j + 1];
                            nextWord[i] = j + 1;
                            string prefix = (currentLineVis == 0) ? "" : currentLineText + " ";
                            lineContent[i] = prefix + p1;
                        }
                    }
                }
            }

            // If the whole word didn't fit, stop looking at further words for this line
            if (currentLineVis + spaceNeeded + wVis > L) break;
        }
    }

    // Output
    int curr = 0;
    while (curr < n && nextWord[curr] != -1) {
        string line = lineContent[curr];
        cout << "|" << line << string(max(0, L - getVisualLength(line)), ' ') << "| (" << L << ")\n";
        curr = nextWord[curr];
    }
}

int main() {
    int L;
    cout << "Enter Line Character Limit: ";
    if (!(cin >> L)) return 1;

    auto process = [L](string filename) {
        ifstream file(filename);
        if (!file) return;
        cout << "\n>>> Processing: " << filename << endl;
        string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        stringstream ss(content);
        vector<string> words;
        string w;
        while (ss >> w) words.push_back(w);
        solveDP(words, L);
    };

    process("mongolian_input.txt");
    process("english_input.txt");
    return 0;
}