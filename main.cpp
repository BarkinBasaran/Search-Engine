#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <locale>
#include <map>
using namespace std;

struct DocumentItem {
    string documentName;
    int count;
};

struct WordItem {
    string word;
    vector<DocumentItem> documents;

    // Add a new document or update the count if the document already exists
    void addDocument(const string& docName, int count) {
        for (auto& doc : documents) {
            if (doc.documentName == docName) {
                doc.count += count;
                return;
            }
        }
        // If the document is not found, add a new DocumentItem
        documents.push_back({docName, count});
    }
};

struct Node {
    WordItem data;
    int height; // Height of the node
    Node *lchild, *rchild;
}*root = nullptr;




// Function prototypes
int height(Node *p);
Node *LLRotation(Node *p);
Node *LRRotation(Node *p);
Node *RRRotation(Node *p);
Node *RLRotation(Node *p);
Node *RInsert(Node *p, const WordItem& key);
void updateNodeData(Node* node, const WordItem& key);
void Inorder(Node *p);
Node *Search(const string& key);
Node *inPre(Node *p);
Node *inSucc(Node *p);
int BF(Node *p); // Balance factor
Node *deleteNode(Node *p, const string& key);
void processFile(const string& fileName, Node*& root);
void generateDot(Node* node, ofstream& file);
void generateDotFile(Node* root, const string& filename);
string escapeDot(const string& str);



string escapeDot(const string& str) {
    string new_str;
    for (char c : str) {
        switch (c) {
            case '"': new_str += "\\\""; break;
            case '\\': new_str += "\\\\"; break;
            case '\n': new_str += "\\n"; break;
            case '{': new_str += "\\{"; break;
            case '}': new_str += "\\}"; break;
            default:
                if (static_cast<unsigned char>(c) < 128)
                    new_str += c;
                else
                    new_str += "?";  // Replace non-ASCII characters
                break;
        }
    }
    return new_str;
}

void generateDot(Node* node, ofstream& file) {
    if (node == nullptr) {
        return;
    }

    string nodeLabel = escapeDot(node->data.word);
    string label = "\"" + nodeLabel + "\\n";

    for (const auto& doc : node->data.documents) {
        label += "{" + escapeDot(doc.documentName) + ", " + to_string(doc.count) + "}\\n";
    }
    label += "\"";

    file << "    \"" << nodeLabel << "\" [label=" << label << "];\n";

    if (node->lchild != nullptr) {
        file << "    \"" << nodeLabel << "\" -> \"" << escapeDot(node->lchild->data.word) << "\";\n";
        generateDot(node->lchild, file);
    }
    if (node->rchild != nullptr) {
        file << "    \"" << nodeLabel << "\" -> \"" << escapeDot(node->rchild->data.word) << "\";\n";
        generateDot(node->rchild, file);
    }
}


void generateDotFile(Node* root, const string& filename) {
    ofstream file(filename);
    file << "digraph AVLTree {\n";
    file << "    rankdir=TB;\n";
    generateDot(root, file);
    file << "}\n";
    file.close();
    system("dot -Tpng avl_tree.dot -o avl_tree.png");
}


//"dot -Tpng avl_tree.dot -o avl_tree.png" for terminal command
int main() {
    locale::global(std::locale("en_US.UTF-8")); // Set locale to UTF-8

    int numFiles;
    cout << "Enter number of input files: ";
    cin >> numFiles;
    cin.ignore();

    for (int i = 0; i < numFiles; ++i) {
        string fileName;
        cout << "Enter " << (i + 1) << ". file name: ";
        getline(cin, fileName);
        processFile(fileName, root);
    }

    generateDotFile(root, "avl_tree.dot");

    string query;
    while (true) {
        cout << "Enter queried words in one line: ";
        getline(cin, query);
        if (query == "ENDOFINPUT") {
            break;
        } else if (query.substr(0, 7) == "REMOVE ") {
            string wordToRemove = query.substr(7);
            root = deleteNode(root, wordToRemove);
            cout << wordToRemove << " has been REMOVED" << endl;
            generateDotFile(root, "avl_tree.dot");
        } else {
            istringstream iss(query);
            string word;
            map<string, map<string, int>> documentWordCounts;  // Maps document to word counts
            bool foundAny = false;

            while (iss >> word) {
                transform(word.begin(), word.end(), word.begin(), ::tolower);
                Node* node = Search(word);
                if (node) {
                    foundAny = true;
                    for (const auto& doc : node->data.documents) {
                        documentWordCounts[doc.documentName][word] += doc.count;
                    }
                }
            }

            if (!foundAny) {
                cout << "No document contains the given query" << endl;
            } else {
                for (const auto& doc : documentWordCounts) {
                    cout << "in Document " << doc.first << ", ";
                    bool firstWord = true;
                    for (const auto& wordCount : doc.second) {
                        if (!firstWord) {
                            cout << ", ";
                        }
                        cout << wordCount.first << " found " << wordCount.second << " times";
                        firstWord = false;
                    }
                    cout << ".\n";
                }
            }
        }
    }

    return 0;
}

int height(Node *p) {
    if (!p) return 0;
    return p->height;
}

Node* LLRotation(Node* p) {
    Node* pl = p->lchild;
    Node* plr = pl->rchild;

    // Perform rotation
    pl->rchild = p;
    p->lchild = plr;

    // Update heights
    p->height = max(height(p->lchild), height(p->rchild)) + 1;
    pl->height = max(height(pl->lchild), height(pl->rchild)) + 1;

    // Return new root
    return pl;
}

Node* RRRotation(Node* p) {
    Node* pr = p->rchild;
    Node* prl = pr->lchild;

    // Perform rotation
    pr->lchild = p;
    p->rchild = prl;

    // Update heights
    p->height = max(height(p->lchild), height(p->rchild)) + 1;
    pr->height = max(height(pr->lchild), height(pr->rchild)) + 1;

    // Return new root
    return pr;
}

Node* LRRotation(Node* p) {
    p->lchild = RRRotation(p->lchild);
    return LLRotation(p);
}

Node* RLRotation(Node* p) {
    p->rchild = LLRotation(p->rchild);
    return RRRotation(p);
}

Node* RInsert(Node* node, const WordItem& key) {
    if (node == nullptr) {
        Node* newNode = new Node();
        newNode->data = key;
        newNode->height = 1;  // New node is initially added at leaf
        newNode->lchild = newNode->rchild = nullptr;
        return newNode;
    }

    if (key.word < node->data.word) {
        node->lchild = RInsert(node->lchild, key);
    } else if (key.word > node->data.word) {
        node->rchild = RInsert(node->rchild, key);
    } else {
        // Update the existing node with the new data
        updateNodeData(node, key);
        return node;
    }

    // Update height of this ancestor node
    node->height = max(height(node->lchild), height(node->rchild)) + 1;

    // Get the balance factor of this ancestor node to check whether this node became unbalanced
    int balance = BF(node);

    // If this node becomes unbalanced, then there are 4 cases

    // Left Left Case
    if (balance > 1 && key.word < node->lchild->data.word) {
        return LLRotation(node);
    }

    // Right Right Case
    if (balance < -1 && key.word > node->rchild->data.word) {
        return RRRotation(node);
    }

    // Left Right Case
    if (balance > 1 && key.word > node->lchild->data.word) {
        node->lchild = RRRotation(node->lchild);
        return LLRotation(node);
    }

    // Right Left Case
    if (balance < -1 && key.word < node->rchild->data.word) {
        node->rchild = LLRotation(node->rchild);
        return RRRotation(node);
    }

    // Return the (unchanged) node pointer
    return node;
}

void updateNodeData(Node* node, const WordItem& key) {
    // Here you need to define how to update the node's data when the word already exists
    // For example, you might want to merge the document lists or update counts
    // This is just a placeholder implementation
    for (const auto& newDoc : key.documents) {
        bool found = false;
        for (auto& existingDoc : node->data.documents) {
            if (existingDoc.documentName == newDoc.documentName) {
                existingDoc.count += newDoc.count;
                found = true;
                break;
            }
        }
        if (!found) {
            node->data.documents.push_back(newDoc);
        }
    }
}

void Inorder(Node *p) {
    if (p) {
        Inorder(p->lchild);
        cout << "Word: " << p->data.word << " | Height: " << p->height << " | Balance Factor: " << BF(p) << " | Documents: ";
        for (const auto& doc : p->data.documents) {
            cout << "{" << doc.documentName << ", " << doc.count << "} ";
        }
        cout << endl;
        Inorder(p->rchild);
    }
}

Node *Search(const string& key) {
    Node *t = root;
    while (t != nullptr) {
        // Compare words instead of integers
        if (key == t->data.word)
            return t;
        else if (key < t->data.word)
            t = t->lchild;
        else
            t = t->rchild;
    }
    return nullptr;
}

int BF(Node *p) {
    if (!p) return 0;
    int leftHeight = p->lchild ? p->lchild->height : 0;
    int rightHeight = p->rchild ? p->rchild->height : 0;
    return leftHeight - rightHeight;
}

// Function to find the inorder predecessor of a node
Node *inPre(Node *p) {
    while (p && p->rchild != nullptr) {
        p = p->rchild;
    }
    return p;
}

// Function to find the inorder successor of a node
Node *inSucc(Node *p) {
    while (p && p->lchild != nullptr) {
        p = p->lchild;
    }
    return p;
}

Node *deleteNode(Node *p, const string& key) {
    if (p == nullptr) {
        return nullptr;
    }

    if (p->lchild == nullptr && p->rchild == nullptr) {
        if (p == root) {
            root = nullptr;
        }
        delete p;
        return nullptr;
    }

    if (key < p->data.word) {
        p->lchild = deleteNode(p->lchild, key);
    } else if (key > p->data.word) {
        p->rchild = deleteNode(p->rchild, key);
    } else {
        if (height(p->lchild) > height(p->rchild)) {
            Node *q = inPre(p->lchild);
            p->data = q->data;
            p->lchild = deleteNode(p->lchild, q->data.word);
        } else {
            Node *q = inSucc(p->rchild);
            p->data = q->data;
            p->rchild = deleteNode(p->rchild, q->data.word);
        }
    }

    p->height = max(height(p->lchild), height(p->rchild)) + 1;

    int balance = BF(p);

    if (balance > 1 && BF(p->lchild) >= 0) {
        return LLRotation(p);
    } else if (balance > 1 && BF(p->lchild) < 0) {
        return LRRotation(p);
    } else if (balance < -1 && BF(p->rchild) <= 0) {
        return RRRotation(p);
    } else if (balance < -1 && BF(p->rchild) > 0) {
        return RLRotation(p);
    }

    return p;
}
void processFile(const string& fileName, Node*& root) {
    ifstream file(fileName);
    string word;

    while (file >> word) {
        // Convert to lowercase
        transform(word.begin(), word.end(), word.begin(), ::tolower);

        // Create or update the WordItem
        WordItem item;
        item.word = word;
        item.addDocument(fileName, 1);

        // Insert or update the AVL tree
        root = RInsert(root, item);
    }

    file.close();
}
