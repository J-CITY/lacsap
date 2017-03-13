#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED
#include <algorithm>
#include "scanner.h"

class TreeNode {
public:
    std::string nameNode = "";
    TreeNode(){}
    std::vector<TreeNode*> children;
    virtual void print() = 0;
};

class FunctionalNode:public TreeNode {
public:
    int operation_type;
    int id;
    std::string nameLex;
    FunctionalNode(std::string _chN, std::string _name, int _t, int _id) {
        nameNode = _chN;
        operation_type = _t;
        nameLex = _name;
        id = _id;
    }
    void print() {
        std::cout << nameLex << ":" << nameNode;
    }
};

class BufferNode:public TreeNode {
public:
    std::string nameLex = "";
    BufferNode(std::string in) {
        nameNode = in;
        nameLex = in;
    }
    void print() {
        std::cout << nameNode;
    }
};

class Parser {
public:
    Parser(std::string _filePath);
    void Parse();
    void print();
private:
    Scanner scanner;
    TreeNode *root = nullptr;
    bool inCycle = false;

    TreeNode *getIdent();
    TreeNode *factor(void);
    TreeNode *term();
    TreeNode *expression();
    TreeNode *condition();

    TreeNode *statement();
    TreeNode *getVarBlock(bool ifNeedNext);
    TreeNode *block();
    void program();

    bool expect(int symacc);
    bool accept(int symacc);
    void error(std::string s);
    void printTree(TreeNode *root, int l);
};


#endif // PARSER_H_INCLUDED
