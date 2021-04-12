#include <algorithm>
#include <iostream>
#include <numeric>
#include <string>
#include <unordered_map>
#include <vector>

const int ALPHABET_SIZE = 26 + 1;

class SuffixArray {
public:
    std::string str;
    int n;
    std::vector<int> powers;
    std::vector<std::vector<int>> suffix_array;
    int k;

    explicit SuffixArray(const std::string& input_str) : str(input_str + static_cast<char>('z' + 1)),
                                                         n(str.size()),
                                                         powers(powers_of_2(n)),
                                                         suffix_array(powers.size()),
                                                         k(suffix_array.size()) {
        MakeSuffixArray();
    }

    void MakeSuffixArray() {
        suffix_array[0] = count_sort();
        for (int i = 1; i < k; ++i) {
            std::vector<std::pair<int, int>> pairs(n);
            for (int j = 0; j < n; ++j) {
                pairs[j] = {suffix_array[i - 1][j], suffix_array[i - 1][(j + powers[i - 1]) % n]};
            }
            suffix_array[i] = count_pair_sort(pairs);
        }
        for (std::vector<int>& array : suffix_array) {
            std::vector<int> cnt(n);
            for (int c : array) {
                ++cnt[c];
            }
            for (int i = 1; i < n; ++i) {
                cnt[i] += cnt[i - 1];
            }
            std::vector<int> p(n);
            for (int i = n - 1; i >= 0; --i) {
                p[--cnt[array[i]]] = i;
            }
            array = p;
        }
    }

    std::vector<int> Search(const std::string& pattern) {
        if (pattern.empty()) {
            std::vector<int> ans(str.size());
            std::iota(ans.begin(), ans.end(), 0);
            return ans;
        }
        int power = *std::lower_bound(powers.begin(), powers.end(), pattern.size());

        int left = left_bound(pattern, power);
        int right = right_bound(pattern, power);
        std::vector<int> ans;
        for (int i = left; i < right; ++i) {
            ans.push_back(suffix_array[power][i]);
        }
        return ans;
    }

    std::vector<int> RegularSearch(const std::string& pattern) {
        std::vector<std::pair<int, std::string>> small_patterns;
        {
            std::string small_pattern;
            for (size_t i = 0; i < pattern.size(); ++i) {
                char c = pattern[i];
                if (c == '?') {
                    small_patterns.emplace_back(i - small_pattern.size(), small_pattern);
                    small_pattern.clear();
                } else {
                    small_pattern.push_back(c);
                }
            }
            if (!small_pattern.empty()) {
                small_patterns.emplace_back(pattern.size() - small_pattern.size(), small_pattern);
            }
        }
        std::unordered_map<int, int> pos_counts;
        for (auto&[small_pattern_pos, small_pattern] : small_patterns) {
            std::vector<int> res_pos = Search(small_pattern);
            for (int found_pos : res_pos) {
                ++pos_counts[found_pos - small_pattern_pos];
            }
        }
        std::vector<int> ans;
        for (auto[found_pos, count] : pos_counts) {
            if (found_pos >= 0 && count == static_cast<int>(small_patterns.size())) {
                ans.push_back(found_pos);
            }
        }
        return ans;
    }

    // возвращает наименьший суффикс, больший или равный pattern
    int left_bound(const std::string& pattern, int power) {
        // ищем его в (left, right]
        int left = -1;
        int right = n;
        while (left + 1 < right) {
            // (left + right) / 2 - округление вверх
            int middle = left + (right - left + 1) / 2;
            if (is_bigger(pattern, middle, power)) {
                left = middle;
            } else {
                right = middle;
            }
        }
        return right;
    }

    // возвращает наименьший суффикс, больший pattern
    int right_bound(const std::string& pattern, int power) {
        // ищем его в (left, right]
        int left = 0;
        int right = n;
        while (left + 1 < right) {
            int middle = left + (right - left) / 2;
            if (is_lower(pattern, middle, power)) {
                right = middle;
            } else {
                left = middle;
            }
        }
        return right;
    }

    bool is_lower(const std::string& pattern, int index, int power) {
        if (pattern < str.substr(suffix_array[power][index], power)) {
            return true;
        }
        return false;
    }

    bool is_bigger(const std::string& pattern, int index, int power) {
        if (pattern > str.substr(suffix_array[power][index], power)) {
            return true;
        }
        return false;
    }

    bool is_equal(const std::string& pattern, int index, int power) {
        if (pattern == str.substr(suffix_array[power][index], power)) {
            return true;
        }
        return false;
    }

    std::vector<int> count_sort() {
        std::vector<int> cnt(ALPHABET_SIZE);
        // подсчет каждого символа
        for (char c : str) {
            ++cnt[c - 'a'];
        }
        // cnt[i] - место, на котором должен стоять символ 'a' + i
        for (int i = 1; i < ALPHABET_SIZE; ++i) {
            cnt[i] += cnt[i - 1];
        }
        std::vector<int> p(n);
        // p[k] = i означает, что на k месте в отсортированном списке стоит s[i]
        for (int i = 0; i < n; ++i) {
            p[--cnt[str[i] - 'a']] = i;
        }
        // раскидываем буквы по классам
        std::vector<int> classes(n);
        int curr_class = 0;
        classes[0] = curr_class;
        for (int i = 1; i < n; ++i) {
            if (str[p[i]] != str[p[i - 1]]) {
                ++curr_class;
            }
            classes[p[i]] = curr_class;
        }

        return classes;
    }

    std::vector<int> count_pair_sort(const std::vector<std::pair<int, int>>& pairs) const {
        std::vector<int> cnt(n);
        // сортируем по pair.second
        for (int i = 0; i < n; ++i) {
            ++cnt[pairs[i].second];
        }
        for (int i = 1; i < n; ++i) {
            cnt[i] += cnt[i - 1];
        }

        std::vector<int> p_second(n);
        // p_second[k] = i означает, что на k месте в отсортированном списке по pair.second стоит pairs[i]
        for (int i = n - 1; i >= 0; --i) {
            p_second[--cnt[pairs[i].second]] = i;
        }
        std::vector<std::pair<int, int>> second_cnt_sort(n);
        for (int i = 0; i < n; ++i) {
            second_cnt_sort[i] = pairs[p_second[i]];
        }

        cnt.assign(n, 0);
        // сортируем по pair.first
        for (int i = 0; i < n; ++i) {
            ++cnt[second_cnt_sort[i].first];
        }
        for (int i = 1; i < n; ++i) {
            cnt[i] += cnt[i - 1];
        }

        std::vector<int> p_first(n);
        // p_first[k] = i означает, что на k месте в отсортированном списке стоит second_cnt_sort[i]
        for (int i = n - 1; i >= 0; --i) {
            p_first[--cnt[second_cnt_sort[i].first]] = i;
        }
        // композиция двух перемещений
        std::vector<int> p(n);
        for (int i = 0; i < n; ++i) {
            p[i] = p_second[p_first[i]];
        }
        std::vector<int> classes(n);
        int curr_class = 0;
        classes[p[0]] = 0;
        for (int i = 1; i < n; ++i) {
            if (pairs[p[i]] != pairs[p[i - 1]]) {
                ++curr_class;
            }
            classes[p[i]] = curr_class;
        }
        return classes;
    }

    static std::vector<int> powers_of_2(int max_number) {
        std::vector<int> powers = {1};
        while (powers.back() * 2 < max_number) {
            powers.push_back(powers.back() * 2);
        }
        return powers;
    }
};

void PrintLine(int n = 20) {
    for (int i = 0; i < n; ++i) {
        std::cout << "-";
    }
    std::cout << "\n";
}

void MakeExampleSearch(const std::string& str, const std::vector<std::string>& patterns) {
    std::cout << "Simple search:\n";
    SuffixArray search(str);
    std::cout << "string: " << str << "\n";
    for (const std::string& pattern : patterns) {
        std::cout << "pattern: " << pattern << "\n";
        std::vector<int> pos = search.Search(pattern);
        std::cout << "found:\n";
        for (int p : pos) {
            std::cout << p << " " << str.substr(p, pattern.size()) << "\n";
        }
    }
    PrintLine();
}

void MakeExampleRegularSearch(const std::string& str, const std::vector<std::string>& patterns) {
    std::cout << "Regular search:\n";
    SuffixArray search(str);
    std::cout << "string: " << str << "\n";
    for (const std::string& pattern : patterns) {
        std::cout << "pattern: " << pattern << "\n";
        std::vector<int> pos = search.RegularSearch(pattern);
        std::cout << "found:\n";
        for (int p : pos) {
            std::cout << p << " " << str.substr(p, pattern.size()) << "\n";
        }
    }
    PrintLine();
}

int main() {
    {
        std::string s = "aaba";
        SuffixArray search(s);
        std::cout << "Simple search:\n";
        std::cout << "string: " << s << "\n";
        std::cout << "suffix_array:" << "\n";
        for (int i = 0; i < search.k; ++i) {
            std::cout << "2^" << i << ": ";
            for (int c : search.suffix_array[i]) {
                std::cout << c << " ";
            }
            std::cout << "\n";
            for (int c : search.suffix_array[i]) {
                std::cout
                        << search.str.substr(c, std::min(static_cast<int>(search.str.size() - c - 1), search.powers[i]))
                        << "$ ";
            }
            std::cout << "\n";
        }
        std::string pattern = "ab";
        std::cout << "pattern: " << pattern << "\n";
        std::vector<int> ans = search.Search(pattern);
        for (int c : ans) {
            std::cout << c << " " << s.substr(c, pattern.size());
        }
        std::cout << "\n";
    }
    PrintLine();
    MakeExampleSearch("banana", {"an"});
    MakeExampleRegularSearch("banana", {"a?a"});
    MakeExampleRegularSearch("abracadabra", {"a?a"});
    MakeExampleRegularSearch("abracadabra", {"a?r", "a??b", "a?r?"});

    return 0;
}
