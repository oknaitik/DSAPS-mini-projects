#include <iostream>
#include <vector>
using namespace std;
#define NUM_ALPHABETS 26 
#define MAX_EDITS 3

struct TrieNode {
    bool isEndOfStr;
    struct TrieNode *children[NUM_ALPHABETS];
};

struct TrieNode *getNode() {
    struct TrieNode *parent = new TrieNode;
    parent->isEndOfStr = false;
    for (int i = 0; i < NUM_ALPHABETS; i ++) {
        parent->children[i] = NULL;
    }
    return parent;
}


class Trie {
public:
    struct TrieNode *root;
    Trie() {
        root = getNode();
    }

    void insert(string s) {
        struct TrieNode *ptr = root;
        for (char ch: s) {
            if (ptr->children[ch - 'a'] == NULL) {
                ptr->children[ch - 'a'] = getNode();
            }
            ptr = ptr->children[ch - 'a'];
        }

        // mark last node as end of string
        ptr->isEndOfStr = true;
    }

    int isPresent(string s) {
        struct TrieNode *ptr = root;
        for (char ch: s) {
            if (ptr->children[ch - 'a'] == NULL) {
                return 0;
            }
            ptr = ptr->children[ch - 'a'];
        }
        if (ptr->isEndOfStr == false) {
            return 0;
        }
        return 1;
    }

    void printAllStringsFrom(struct TrieNode *ptr, string curr, vector<string> &result) {
        if (ptr->isEndOfStr == true) {
            // push the string 
            // note there may be more strings from this point
            // if present, they would be found in the for block
            result.push_back(curr); 
        }

        for (int i = 0; i < NUM_ALPHABETS; i ++) {
            if (ptr->children[i] != NULL) {
                printAllStringsFrom(ptr->children[i], curr + (char)('a' + i), result);
            }
        }

        return;
    }

    void stringsWithPrefix(string s, vector<string> &result) {
        struct TrieNode *ptr = root;

        for (char ch: s) {
            if (ptr->children[ch - 'a'] == NULL) {
                // no prefix found
                return;
            }
            ptr = ptr->children[ch - 'a'];
        }   
        printAllStringsFrom(ptr, s, result);
    }

    void solve(struct TrieNode *ptr, string orig, string curr, vector<string> &result, vector<int> prev_col, int j) {
        if (ptr->isEndOfStr == true) {
            if (prev_col[orig.length()] <= MAX_EDITS) {
                result.push_back(curr);
            }
        }

        vector<int> curr_col(orig.length()+1, 0);
        curr_col[0] = j;
        for (int c = 0; c < NUM_ALPHABETS; c ++) {
            if (ptr->children[c] != NULL) {

                // get current column using previous column (DP) 
                char ch = 'a' + c;
                for (int i = 1; i < orig.length()+1; i ++) {
                    int t1 = min(curr_col[i - 1], prev_col[i]) + 1;
                    int t2 = (orig[i - 1] != ch);
                    curr_col[i] = min(t1, prev_col[i - 1] + t2);
                }
                solve(ptr->children[c], orig, curr + ch, result, curr_col, j+1);
            }
        }
    }

    void stringsWithin3Edits(string s, vector<string> &result) {
        // number of columns in dp array would be maximum possible string length in trie
        const int COLS = 101;
        vector<int> col(s.length()+1, 0);
        for (int i = 0; i < s.length()+1; i ++) {
            col[i] = i;
        }
        solve(root, s, "", result, col, 1);
    }

};

int main() {
    int n, q;
    cin >> n >> q;

    Trie dict;
    for (int i = 0; i < n; i ++) {
        string x;
        cin >> x;
        dict.insert(x);
    }
    
    vector<vector<string>> results;

    while (q --) {
        // 1, 2 or 3
        // 1: spellcheck (check if input exists in dictionary)
        // 2: autocomplete (find all words in dictionary with input as prefix)
        // 3: autocorrect (find all words with Levenshtein distance of atmost 3)
        int a;
        string t;
        cin >> a;
        cin >> t;

        vector<string> result;

        switch(a) {
            case 1: {
                result.push_back(to_string(dict.isPresent(t)));
                results.push_back(result); 
            }
            break;

            case 2: {
                dict.stringsWithPrefix(t, result);
                results.push_back(result);
            }
            break;

            case 3: {
                dict.stringsWithin3Edits(t, result);
                results.push_back(result);
            }
            break;

            default:
            break;

        }
    }

    // print result
    for (auto vs: results) {
        if (vs.size() == 1 && (vs[0] == "0" || vs[0] == "1")) {
            cout << vs[0] << '\n';
        }
        else {
            cout << vs.size() << '\n';
            for (auto s: vs) {
                cout << s << '\n';
            }
        }
    }

}