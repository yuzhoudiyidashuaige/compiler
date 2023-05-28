#include "Ast.h"
#include "parsing.hpp"

void Int::json(string &s) {
    s.append("\n{\n");
    s.append("\"name\" : \"IntValue: " + to_string(this->value) + "\"\n");
    s.append("}");
}

void Float::json(string &s) {
    s.append("\n{\n");
    s.append("\"name\" : \"FloatValue: " + to_string(this->value) + "\"\n");
    s.append("}");
}

void Char::json(string &s) {
    s.append("\n{\n");
    s.append("\"name\" : \"CharValue: " + this->value.substr(1, this->value.length() - 2) + "\"\n");
    s.append("}");
}

void String::json(string &s) {
    s.append("\n{\n");
    s.append("\"name\" : \"StringValue: " + this->value.substr(1, this->value.length() - 2) + "\"\n");
    s.append("}");
}

void Identifier::json(string &s) {
    s.append("\n{\n");
    s.append("\"name\" : \"Identifier: " + this->name + "\"\n");
    s.append("}");
}

void getArrayElement::json(string &s) {
    s.append("\n{\n");
    s.append("\"name\" : \"getArrayElement\",\n");
    s.append("\"children\" : \n[");
    
    this->identifier.json(s);
    s.append(",");
    this->index.json(s);

    s.append("\n]\n");
    s.append("}");
}

void ArrayElementAssign::json(string &s) {
    s.append("\n{\n");
    s.append("\"name\" : \"ArrayElementAssign\",\n");
    s.append("\"children\" : \n[");
    
    this->identifier.json(s);
    s.append(",");
    this->index.json(s);
    s.append(",");
    this->value_Exp.json(s);

    s.append("\n]\n");
    s.append("}");
}

void FunctionCall::json(string &s) {
    s.append("\n{\n");
    s.append("\"name\" : \"FunctionCall\",\n");
    s.append("\"children\" : \n[");
    
    this->identifier.json(s);
    
    if (!this->args.empty())
        s.append(",");

    for (Ast* node : this->args) {
        node->json(s);
        if (node != this->args.back()) {
            s.append(",");
        }
    }

    s.append("\n]\n");
    s.append("}");
}

string getOpStr(int op) {
    string opStr;
    switch (op) {
    case PLUS : { opStr = "+"; break;}
    case MINUS : { opStr = "-"; break;}
    case MUL : { opStr = "*"; break;}
    case DIV : { opStr = "/"; break;}
    case OR : { opStr = "||"; break;}
    case AND : { opStr = "&&"; break;}
    case EQ : { opStr = "=="; break;}
    case NEQ : { opStr = "!="; break;}
    case LT : { opStr = "<"; break;}
    case GT : { opStr = ">"; break;}
    case LEQ : { opStr = "<="; break;}
    case GEQ : { opStr = ">="; break;}
    default: {
        cout << "ERROR" << op << endl;
    }
    }
    return opStr;
}

void BinaryOp::json(string &s) {
    s.append("\n{\n");
    s.append("\"name\" : \"BinaryOperation\",\n");
    s.append("\"children\" : \n[");
    
    this->left_arg.json(s);
    s.append(",");
    
    string opStr = getOpStr(this->op);

    s.append("\n{\n");
    s.append("\"name\" : \"" + opStr + "\"\n");
    s.append("}");

    s.append(",");
    this->right_arg.json(s);

    s.append("\n]\n");
    s.append("}");
}

void getAddr::json(string &s) {
    s.append("\n{\n");
    s.append("\"name\" : \"getAddr\",\n");
    s.append("\"children\" : \n[");
    
    this->identifier.json(s);

    s.append("\n]\n");
    s.append("}");
}

void getArrayAddr::json(string &s) {
    s.append("\n{\n");
    s.append("\"name\" : \"getArrayAddr\",\n");
    s.append("\"children\" : \n[");
    
    this->identifier.json(s);

    s.append(",");

    this->index.json(s);

    s.append("\n]\n");
    s.append("}");
}

void Assignment::json(string &s) {
    s.append("\n{\n");
    s.append("\"name\" : \"Assignment\",\n");
    s.append("\"children\" : \n[");
    
    this->id.json(s);
    s.append(",");
    this->exp_value.json(s);

    s.append("\n]\n");
    s.append("}");
}

void Block::json(string &s) {
    s.append("\n{\n");
    s.append("\"name\" : \"Block\",\n");
    s.append("\"children\" : \n[");
    for (Ast* node : this->StmtList) {
        node->json(s);
        if (node != this->StmtList.back()) {
            s.append(",");
        }
    }
    s.append("\n]\n");
    s.append("}");
}

void ExpStmt::json(string &s) {
    s.append("\n{\n");
    s.append("\"name\" : \"ExpStmt\",\n");
    s.append("\"children\" : \n[");
    this->exp.json(s);
    s.append("\n]\n");
    s.append("}");
}

void BreakStmt::json(string &s) {
    s.append("\n{\n");
    s.append("\"name\" : \"break\", \n");
    s.append("}");
}

void IfStmt::json(string &s) {
    s.append("\n{\n");
    s.append("\"name\" : \"IfOnlyStmt\",\n");
    s.append("\"children\" : \n[");
    
    this->exp.json(s);
    s.append(",");
    this->ifBlock.json(s);

    s.append("\n]\n");
    s.append("}");
}

void IfElseStmt::json(string &s) {
    s.append("\n{\n");
    s.append("\"name\" : \"IfElseStmt\",\n");
    s.append("\"children\" : \n[");
    
    this->exp.json(s);
    s.append(",");
    this->ifBlock.json(s);
    s.append(",");
    this->elseBlock.json(s);

    s.append("\n]\n");
    s.append("}");
}

void WhileStmt::json(string &s) {
    s.append("\n{\n");
    s.append("\"name\" : \"WhileStmt\",\n");
    s.append("\"children\" : \n[");
    
    this->exp.json(s);
    s.append(",");
    this->block.json(s);

    s.append("\n]\n");
    s.append("}");
}

void ReturnStmt::json(string &s) {
    s.append("\n{\n");
    s.append("\"name\" : \"ReturnStmt\",\n");
    s.append("\"children\" : \n[");
    this->exp.json(s);
    s.append("\n]\n");
    s.append("}");
}

void ReturnVoid::json(string &s) {
    s.append("\n{\n");
    s.append("\"name\" : \"ReturnVoid\",\n");
    s.append("\"children\" : \n[");
    s.append("\n]\n");
    s.append("}");
}


void VariableDeclaration::json(string &s) {
    s.append("\n{\n");
    s.append("\"name\" : \"VariableDeclaration\",\n");
    s.append("\"children\" : \n[");
    
    this->var_type.json(s);
    s.append(",");
    this->identifier.json(s);

    if (this->assignmentExp != nullptr) {
        s.append(",");
        this->assignmentExp->json(s);
    }

    s.append("\n]\n");
    s.append("}");
}

void FunctionDeclaration::json(string &s) {
    s.append("\n{\n");
    s.append("\"name\" : \"FunctionDeclaration\",\n");
    s.append("\"children\" : \n[");
    
    this->func_type.json(s);
    s.append(",");
    this->identifier.json(s);
    
    s.append(",");

    for (Ast* node : this->args) {
        node->json(s);
        s.append(",");
    }

    this->block.json(s);

    s.append("\n]\n");
    s.append("}");
}
