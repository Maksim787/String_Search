#include <iostream>
#include <string>
#include <vector>
#include <map>

struct Node {
    int begin;
    int end;
    std::map<char, Node*> next;

    Node(int start, int end) : begin(start), end(end) {}

    int size() const {
        return end - begin;
    }
};

class SuffixTree {
    Node* root;
    std::string str;
    int n;

public:
    // построение суффиксного дерева за O(N^2)
    explicit SuffixTree(const std::string& input_str) : str(input_str + '$'), n(str.size()) {
        root = new Node(0, 0);
        // последовательно вставляем все суффиксы
        for (int i = 0; i < static_cast<int>(str.size()); ++i) {
            insert(i);
        }
    }

    void insert(int suffix_begin) {
        // если от корня нет такой буквы, то вставляем новый лист
        if (!root->next.contains(str[suffix_begin])) {
            root->next[str[suffix_begin]] = new Node(suffix_begin, n);
            return;
        }
        Node* node = root->next[str[suffix_begin]];
        // идем по новому суффиксу
        for (int i = suffix_begin; i < n;) {
            // идем по буквам в текущем узле
            for (int j = 0; j < node->size(); ++j) {
                // если есть несовпадение, делаем ответвление
                if (str[node->begin + j] != str[i + j]) {
                    // новый лист
                    Node* new_list = new Node(i + j, n);
                    // новая внутренняя вершина
                    Node* new_node = new Node(node->begin + j, node->end);
                    node->end = node->begin + j;
                    new_node->next = node->next;
                    node->next.clear();
                    node->next[str[node->begin + j]] = new_node;
                    node->next[str[i + j]] = new_list;
                    return;
                }
            }
            i += node->size();
            // если в следующях узлах нет такой буквы, создаем лист
            if (str[node->end] != str[i]) {
                Node* new_list = new Node(i, n);
                node->next[str[i]] = new_list;
                return;
            }
            // иначе переходим к следующему узлу
            node = node->next[str[i]];
        }
    }

    // проверка за O(P) на то, что простой pattern содержится в строке
    bool contains(const std::string& pattern) const {
        if (pattern.empty()) {
            return true;
        }
        if (!root->next.contains(pattern[0])) {
            return false;
        }
        Node* node = root->next[pattern[0]];
        int j = 0;
        // идем по буквам в паттерне
        for (int i = 0; i < static_cast<int>(pattern.size()); ++i, ++j) {
            // узел закончился, переходим к следующему
            if (node->begin + j == node->end) {
                if (!node->next.contains(pattern[i])) {
                    return false;
                }
                node = node->next[pattern[i]];
                j = 0;
            }
            if (str[node->begin + j] != pattern[i]) {
                return false;
            }
        }
        // мы успешно прошли по дереву весь pattern => он есть в строке
        return true;
    }

    void DeleteNode(Node* node) {
        for (const auto&[c, next] : node->next) {
            DeleteNode(next);
        }
        delete node;
    }

    void Print() const {
        PrintNode(root);
    }

    void PrintNode(Node* node, int cnt = 0) const {
        for (int i = 0; i < cnt - 1; ++i) {
            std::cout << "    ";
        }
        std::cout << "|";
        std::cout << str.substr(node->begin, node->size()) << "\n";
        for (const auto&[c, next] : node->next) {
            PrintNode(next, cnt + 1);
        }
    }

    ~SuffixTree() {
        DeleteNode(root);
    }
};


#include <cassert>

void Check(const SuffixTree& tree, const std::string& str) {
    for (int begin = 0; begin < static_cast<int>(str.size()); ++begin) {
        for (int end = begin + 1; end <= static_cast<int>(str.size()); ++end) {
            assert(tree.contains(str.substr(begin, end - begin)));
        }
    }
}

void PrintLine(int n = 50) {
    std::cout << "\n";
    for (int i = 0; i < n; ++i) {
        std::cout << "-";
    }
    std::cout << "\n";
}

int main() {
    {
        // пример дерева
        std::string str = "xabxa";
        SuffixTree tree(str);
        // печать дерева
        tree.Print();

        assert(tree.contains("abx"));
        assert(tree.contains("xabxa"));
        assert(tree.contains("a"));
        assert(tree.contains("xa"));
        assert(!tree.contains("xabxc"));
        assert(!tree.contains("ax"));
        // проверка, что дерево содержит все свои подстроки
        Check(tree, str);
    }
    PrintLine();
    {
        // еще один пример дерева
        std::string str = "banana";
        SuffixTree tree(str);
        // печать дерева
        tree.Print();
        // проверка, что оно содержит все свои подстроки
        Check(tree, str);
        assert(!tree.contains("anab"));
    }

    return 0;
}
