#include "parser.h"

void FuncNode::print(int l) {
    std::cout << nameNode << " : " << nameLex << " ";
    if(type->returnParam != nullptr) {
        type->returnParam->type->print(l);
    }
    std::cout << "\n";
    for(int j = 0; j < l; ++j) {
        std::cout << "     ";
    }
    std::cout << "Input\n";
    if(type->inputParam != nullptr) {
        type->inputParam->print(l);
    }
    for(int j = 0; j < l; ++j) {
        std::cout << "     ";
    }
    std::cout << "Local\n";
    if(type->localParam != nullptr) {
        type->localParam->print(l);
    }
    printTree(type->block, l+1, 0);
}

void ProcNode::print(int l) {
    std::cout << nameNode << " : " << nameLex << " ";
    std::cout << "\n";
    for(int j = 0; j < l; ++j) {
        std::cout << "     ";
    }
    std::cout << "Input\n";
    if(type->inputParam != nullptr) {
        type->inputParam->print(l);
    }
    for(int j = 0; j < l; ++j) {
        std::cout << "     ";
    }
    std::cout << "Local\n";
    if(type->localParam != nullptr) {
        type->localParam->print(l);
    }
    printTree(type->block, l+1, 0);
}

void SymbolTable::print(int l) {
    for(unsigned int i = 0; i < symbolsvec.size(); ++i) {
        for(int j = 0; j < l; ++j) {
            std::cout << "     ";
        }
        std::cout << std::setw(8) << std::left << symbolsvec[i]->lex.lexem << " | ";
        std::cout << std::setw(8) << std::left << SymbolTypeToStr((int)symbolsvec[i]->_class) << " | ";
        if(symbolsvec[i]->type != nullptr)
            symbolsvec[i]->type->print(l);
        //std::cout << " " <<(int)((SymbolVar*)symbolsvec[i])->mot;
        std::cout << "\n";
    }
}

void DescriptorRecord::print(int l) {
    std::cout << "record\n";
    l++;
    for(unsigned int i = 0; i < rName.size(); ++i) {
        for(int j = 0; j < l; ++j) {
            std::cout << "     ";
        }
        std::cout << "val" << i << " ";
        std::cout << rName[i]->lex.lexem << " ";
        rName[i]->type->print(l);
        std::cout << "\n";
    }
}

void DescriptorArray::print(int l) {
    std::cout << "array\n";
    l = l + 1;
    for(unsigned int i = 0; i < indices.size(); ++i) {
        for(int j = 0; j < l; ++j) {
            std::cout << "     ";
        }
        std::cout << "index" << i << " ";
        if(!isOpen)
            indices[i]->print(l);
        std::cout << "\n";
    }
    for(int j = 0; j < l; ++j) {
        std::cout << "     ";
    }
    std::cout << "type : ";
    baseType->print(l);

}

void DescriptorLimited::print(int l) {
    std::cout << "limit\n";
    l = l + 1;
    for(int j = 0; j < l; ++j) {
        std::cout << "     ";
    }
    std::cout << "min : ";
    if(_min != nullptr)
        printTree(_min, 0, 0);
    std::cout << "\n";
    for(int j = 0; j < l; ++j) {
        std::cout << "     ";
    }
    std::cout << "max : ";
    if(_max != nullptr)
        printTree(_max, 0, 0);
    std::cout << "\n";
    for(int j = 0; j < l; ++j) {
        std::cout << "     ";
    }
    std::cout << "type : ";
    baseType->print(l);

}

std::string SymbolTypeToStr(int st) {
	switch(st) {
	case (int)SymbolTypes::types:
	{
		return "types";
	}
	case (int)SymbolTypes::consts:
	{
		return "consts";
	}
	case (int)SymbolTypes::vars:
	{
		return "vars";
	}
	case (int)SymbolTypes::func:
	{
		return "func";
	}
	case (int)SymbolTypes::proc:
	{
		return "proc";
	}
	}
	return "";
}

void printTree(TreeNode *root, int l, int rot) {
	bool if_print = false;
	if(root->children.size() == 0) {
		if(l != 0) {
			for(int j = 0; j < l-1; ++j) {
				std::cout << "     ";
			}
			if(rot == 0) {
				//std::cout << "\\____";
				std::cout << "\xc0\xc4\xc4\xc4\xc4";
			} else {
				std::cout << "\xda\xc4\xc4\xc4\xc4";
				//std::cout << "/----";
			}
		}
		root->print(l);
		std::cout << std::endl;
	}

	for (unsigned int k = 0; k < root->children.size(); ++ k) {
		if(k < root->children.size()/2 && root->children[k] != nullptr){
			printTree(root->children[k], l+1, 1);
			std::cout << std::endl;
		}
		if(k >= root->children.size()/2 && !if_print) {
			if(l != 0) {
				for(int j = 0; j < l-1; ++j) {
					std::cout << "     ";
				}
				if(rot == 0) {
					//std::cout << "\\____";
					std::cout << "\xc0\xc4\xc4\xc4\xc4";
				} else {
					std::cout << "\xda\xc4\xc4\xc4\xc4";
					//std::cout << "/----";
				}
			}
			root->print(l);
			std::cout << std::endl;
			if_print = true;
		}
		if(k >= root->children.size()/2  && root->children[k] != nullptr) {
			printTree(root->children[k], l+1, 0);
			std::cout << std::endl;
		}
	}
}
