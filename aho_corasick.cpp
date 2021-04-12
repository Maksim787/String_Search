#include "templates.cpp"

#include <iostream>
#include <queue>
#include <string>
#include <vector>


struct Bor {
private:
    struct Node {
        int terminal;
        Node* suffix_link;
        Node* compressed_suffix_link;
        std::vector<Node*> children;

        explicit Node(int terminal = 0) : terminal(terminal), suffix_link(nullptr), compressed_suffix_link(nullptr) {
            children.resize(26, nullptr);
        }

        Node*& at(char c) {
            return children[c - 'a'];
        }

        static char GetLetter(int i) {
            return 'a' + i;
        }
    };

    Node* root;
    int n_patterns;
    std::vector<size_t> pattern_sizes;

    void MakeSuffixLinks() {
        root->suffix_link = root;
        std::queue<Node*> bfs;
        for (Node* node : root->children) {
            if (node == nullptr) {
                continue;
            }
            node->suffix_link = root;
            bfs.push(node);
        }
        while (!bfs.empty()) {
            Node* node = bfs.front();
            bfs.pop();
            for (size_t i = 0; i < node->children.size(); ++i) {
                if (node->children[i] == nullptr) {
                    continue;
                }
                Node* parent_node = node->suffix_link;
                while (parent_node != root && parent_node->children[i] == nullptr) {
                    parent_node = parent_node->suffix_link;
                }
                if (parent_node->children[i] == nullptr) {
                    node->children[i]->suffix_link = root;
                } else {
                    node->children[i]->suffix_link = parent_node->children[i];
                }
                bfs.push(node->children[i]);
            }
        }
    }

    void CompressSuffixLinks() {
        std::queue<Node*> bfs;
        bfs.push(root);
        while (!bfs.empty()) {
            Node* node = bfs.front();
            bfs.pop();
            if (node->suffix_link->terminal) {
                node->compressed_suffix_link = node->suffix_link;
            } else if (node->suffix_link->compressed_suffix_link != nullptr) {
                node->compressed_suffix_link = node->suffix_link->compressed_suffix_link;
            }
            for (Node* next_node : node->children) {
                if (next_node != nullptr) {
                    bfs.push(next_node);
                }
            }
        }
    }

    void FreeNodes(Node* node) {
        if (node == nullptr) {
            return;
        }
        for (Node* next_node : node->children) {
            FreeNodes(next_node);
        }
        delete node;
    }

public:
    explicit Bor(const std::vector<std::string>& words) {
        root = new Node();
        n_patterns = words.size();
        pattern_sizes.resize(n_patterns);
        for (size_t j = 0; j < words.size(); ++j) {
            const std::string& word = words[j];
            pattern_sizes[j] = word.size();
            Node* current = root;
            for (size_t i = 0; i < word.size() - 1; ++i) {
                char c = word[i];
                if (current->at(c) == nullptr) {
                    current->at(c) = new Node();
                }
                current = current->at(c);
            }
            char c = word.back();
            if (current->at(c) == nullptr) {
                current->at(c) = new Node(j);
            } else {
                current->at(c)->terminal = j;
            }
        }

        MakeSuffixLinks();
        CompressSuffixLinks();
    }

    [[nodiscard]] std::vector<int> CountOnString(const std::string& str) const {
        Node* current = root;
        std::vector<int> ans;
        ans.resize(n_patterns);
        for (char c : str) {
            while (current != root && current->at(c) == nullptr) {
                current = current->suffix_link;
            }
            if (current->at(c) == nullptr) {
                continue;
            }
            current = current->at(c);
            Node* terminal_node = current;
            while (terminal_node != nullptr &&
                   (terminal_node->terminal || terminal_node->compressed_suffix_link != nullptr)) {
                ++ans[terminal_node->terminal];
                terminal_node = terminal_node->compressed_suffix_link;
            }
        }
        return ans;
    }

    [[nodiscard]] std::vector<std::vector<int>> FindOnString(const std::string& str) const {
        Node* current = root;
        std::vector<std::vector<int>> ans;
        ans.resize(n_patterns);
        for (size_t i = 0; i < str.size(); ++i) {
            char c = str[i];
            while (current != root && current->at(c) == nullptr) {
                current = current->suffix_link;
            }
            if (current->at(c) == nullptr) {
                continue;
            }
            current = current->at(c);
            Node* terminal_node = current;
            while (terminal_node != nullptr &&
                   (terminal_node->terminal || terminal_node->compressed_suffix_link != nullptr)) {
                size_t index = i + 1 - pattern_sizes[terminal_node->terminal];
                ans[terminal_node->terminal].push_back(index);
                terminal_node = terminal_node->compressed_suffix_link;
            }
        }
        return ans;
    }

    void Print() {
        std::string pref;
        PrintNodes(pref, root);
    }

    void PrintNodes(std::string& pref, Node* node) {
        if (node == nullptr) {
            return;
        }
        if (node->terminal) {
            std::cout << pref << "\n";
        }
        for (size_t i = 0; i < node->children.size(); ++i) {
            pref.push_back(Node::GetLetter(i));
            PrintNodes(pref, node->children[i]);
            pref.pop_back();
        }
    }

    void PrintStructure() {
        std::string pref;
        PrintStructureNodes(pref, root);
    }

    void PrintStructureNodes(std::string& pref, Node* node) {
        std::cout << pref << ":" << "\n";
        std::cout << "from\t" << node << "\tto\t";
        for (size_t i = 0; i < node->children.size(); ++i) {
            if (node->children[i] != nullptr) {
                std::cout << Node::GetLetter(i) << " " << node->children[i] << "\t";
            }
        }
        std::cout << "\n";
        std::cout << "suffix link:\t" << node->suffix_link << "\n";
        std::cout << "compressed_suffix_link:\t" << node->compressed_suffix_link << "\n";
        std::cout << "\n";
        for (size_t i = 0; i < node->children.size(); ++i) {
            if (node->children[i] != nullptr) {
                pref.push_back(Node::GetLetter(i));
                PrintStructureNodes(pref, node->children[i]);
                pref.pop_back();
            }
        }
    }

    ~Bor() {
        FreeNodes(root);
    }
};

int main() {
    std::vector<std::string> words = {"abcd", "aab", "aac", "bac", "baa", "ba", "aa", "a"};
    Bor bor(words);
//    bor.Print();
//    bor.PrintStructure();
    std::string s = "aabaabcd";
    std::vector<int> occur = bor.CountOnString(s);
    for (size_t i = 0; i < occur.size(); ++i) {
        std::cout << words[i] << ":\t" << occur[i] << "\n";
    }
    std::vector<std::vector<int>> occur_pos = bor.FindOnString(s);
    for (size_t i = 0; i < occur_pos.size(); ++i) {
        std::cout << words[i] << ":\n";
        for (int pos : occur_pos[i]) {
            std::cout << pos << " ";
        }
        std::cout << "\n";
    }

    return 0;
}
