/* 
  File:   virtualfpu.cpp
  Author: Leonardo Berti
 
  Mathematical expression interpreter and compiler 
 
   (A C++20 compliant compiler is required)

 MIT License

 Copyright (c) 2014-2024 Leonardo Berti (leonardo.berti[at]ymail.com)

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 
 */

#include "virtualfpu.h"
#include <cstring>
#include <map>
#include <stdexcept>
#include <cctype>
#include <stack>
#include <sstream>
#include <iostream>
#include <ostream>
#include <vector>
#include <cmath>



using namespace std;


namespace virtualfpu {

    static std::vector<Instruction> functionsOp = {
        Instruction::ABS, Instruction::ACOS, Instruction::ACOSH, Instruction::ASIN,
        Instruction::ASINH, Instruction::ATAN, Instruction::ATANH, Instruction::COS,
        Instruction::COSH, Instruction::EXP,
        Instruction::LOG, Instruction::LOG10, Instruction::LOG2,
        Instruction::MUL, Instruction::SIGN, Instruction::SIN,
        Instruction::SINH, Instruction::SQRT, Instruction::TAN, Instruction::TANH
    };

    static std::map<Instruction, string> symToStr = {
        {Instruction::UNARY_MINUS, "[-]"},
        {Instruction::ADD, "+"},
        {Instruction::SUB, "-"},
        {Instruction::MUL, "*"},
        {Instruction::DIV, "/"},
        {Instruction::SQRT, "sqrt"},
        {Instruction::COS, "cos"},
        {Instruction::SIN, "sin"},
        {Instruction::TAN, "tan"},
        {Instruction::ASIN, "asin"},
        {Instruction::ACOS, "acos"},
        {Instruction::ATAN, "atan"},
        {Instruction::ABS, "abs"},
        {Instruction::EXP, "exp"},
        {Instruction::LOG, "log"},
        {Instruction::LOG10, "log10"},
        {Instruction::LOG2, "log2"},
        {Instruction::SINH, "sinh"},
        {Instruction::COSH, "cosh"},
        {Instruction::TANH, "tanh"},
        {Instruction::ASINH, "asinh"},
        {Instruction::ACOSH, "acosh"},
        {Instruction::ATANH, "atanh"},
        {Instruction::SIGN, "sign"}
    };

    static std::map<string, Instruction> strToSymbol = {
        {"[-]", Instruction::UNARY_MINUS},
        {"(", Instruction::PAR_OPEN},
        {")", Instruction::PAR_CLOSE},
        {"+", Instruction::ADD},
        {"-", Instruction::SUB},
        {"*", Instruction::MUL},
        {"/", Instruction::DIV},
        {"sqrt", Instruction::SQRT},
        {"cos", Instruction::COS},
        {"sin", Instruction::SIN},
        {"tan", Instruction::TAN},
        {"asin", Instruction::ASIN},
        {"acos", Instruction::ACOS},
        {"atan", Instruction::ATAN},
        {"abs", Instruction::ABS},
        {"exp", Instruction::EXP},
        {"log", Instruction::LOG},
        {"log10", Instruction::LOG10},
        {"log2", Instruction::LOG2},
        {"sinh", Instruction::SINH},
        {"cosh", Instruction::COSH},
        {"tanh", Instruction::TANH},
        {"asinh", Instruction::ASINH},
        {"acosh", Instruction::ACOSH},
        {"atanh", Instruction::ATANH},
        {"sign", Instruction::SIGN}

    };

    /////////////////////// StackItem ////////////////////////////////////////////

    ostream& operator<<(ostream& ostr, const StackItem& item) {

        if (item.instr == Instruction::VALUE) {
            if (!item.defVar.empty()) {
                ostr << item.defVar;
            } else {
                ostr << item.value;
            }
        } else if (symToStr.contains(item.instr)) {
            ostr << symToStr[item.instr];
        } else {
            ostr << "<?>";
        }

        return ostr;

    }

    void StackItem::fromString(const string& opstr) {

        if (strToSymbol.contains(opstr)) {
            instr = strToSymbol[opstr];

        } else {
            throw VirtualFPUException(opstr + ": invalid operator or function.");
        }

    }

    StackItem* StackItem::clone() {
        StackItem *s = new StackItem();
        s->value = this->value;
        s->instr = this->instr;
        s->defVar = this->defVar;
        return s;
    }

    ////////////////////// VirtualFPUException ////////////////////////////////////

    VirtualFPUException::VirtualFPUException(const string& msg) : msg(msg) {

    }

    VirtualFPUException::~VirtualFPUException() {
    }

    const string VirtualFPUException::getMessage() const {
        return msg;
    }

    const char * VirtualFPUException::what() const noexcept {
        return msg.c_str();
    }

    ////////////////////// VirtualFPU //////////////////////////////////////////////

    RPNCompiler::RPNCompiler() : instrVector(0) {
        init(DEFAULT_STACK_SIZE);
    }

    RPNCompiler::~RPNCompiler() {

        clearStack();

        if (instrVector) {
            delete instrVector;
            instrVector = nullptr;
        }

        if (defVars && !defVars->empty()) {

            defVars->clear();
            delete defVars;
            defVars = nullptr;

        }

    }
    
    bool RPNCompiler::isBuiltinFunction(const Instruction& instr) noexcept
    {
        return std::find(functionsOp.begin(),functionsOp.end(),instr)!=functionsOp.end();
    }

    int RPNCompiler::getOperatorPrecedence(const Instruction& instr) noexcept {

        switch (instr) {
            case Instruction::VALUE:
                return 0;
            case Instruction::ADD:
                return 2;
            case Instruction::SUB:
                return 3;
            case Instruction::MUL:
                return 4;
            case Instruction::DIV:
                return 5;
            case Instruction::UNARY_MINUS:
                return 6;
            default:
                if (isFunction(instr)) {
                    return 8;
                } else {
                    return -1;
                }

        }

    }

    RPNCompiler& RPNCompiler::compile(const string& statement) {

        const string err = "Syntax error:";

        clearStack();

        last_compiled_statement = statement;

        int idx = 0;
        int next = 0;

        const int lu = statement.length();

        const int TK_NIL = 0;
        const int TK_NUM = 1;
        const int TK_OPERATOR = 2;
        const int TK_FUNCTION = 3;
        const int TK_OPEN_BRK = 4;
        const int TK_CLOSE_BRK = 5;
        const int TK_OTHER = 255;

        if (lu == 0) {
            throwError(err + "expression is empty");
        }

        string token = "";

        int state = 0;

        stack<StackItem*> temp;

        //last token type processed
        int last = TK_NIL;

        //convert from infix to postfix notation (RPN)
        while (idx < lu) {

            //estrae il token
            token = getToken(statement, idx, &next);

            if (isNumber(token)) {

                if (last == TK_NUM) {
                    ostringstream ss;
                    ss << "Found two consecutive numbers at position " << idx;
                    throwError(ss.str());
                }

                StackItem *s = new StackItem();

                s->instr = Instruction::VALUE;
                s->value = toDouble(token);

                last = TK_NUM;

                instrVector->push_back(s);

            } else if (token == "(") {

                if (last == TK_CLOSE_BRK) {
                    ostringstream ss;
                    ss << "Invalid bracket " << token << " at index " << idx << " (missing operator or function)";
                    throwError(ss.str());
                }

                last = TK_OPEN_BRK;

                StackItem *s = new StackItem();
                s->instr = Instruction::PAR_OPEN;
                s->value = 0;

                temp.push(s);

            } else if (token == ")") {

                if (last == TK_OPEN_BRK) {
                    ostringstream ss;
                    ss << "Empty brackets at index " << idx;
                    throwError(ss.str());
                }

                last = TK_CLOSE_BRK;

                /**  if (temp.empty()) {

                      ostringstream ss;
                      ss << err << ") not expected at position " << idx;
                      throwError(ss.str());
                  }*/

                if (!temp.empty()) {

                    bool matchingParFound = false;

                    for (;;) {

                        StackItem *s = temp.top();

                        if (s->instr == Instruction::PAR_OPEN) {
                            matchingParFound = true;
                            temp.pop();
                            //  if (!temp.empty() && isFunction(temp.top()->instr)) {
                            break;
                            //  }
                            // break;
                        } else {
                            instrVector->push_back(s);
                            temp.pop();
                        }

                        if (temp.empty()) {
                            break;
                        }
                    }

                    /**     if (!matchingParFound) {

                             ostringstream ss;
                             ss << "Missing matching bracket at index " << idx;
                             throwError(ss.str());
                         }*/

                }


            } else if (isOperator(token)) {

                if ((last == TK_OPERATOR || last == TK_FUNCTION) && token != "-") {

                    ostringstream ss;

                    ss << "Invalid operator " << token << " at index " << idx;
                    //due operatori successivi
                    throwError(ss.str());
                }

                StackItem *opItem = new StackItem();
                opItem->fromString(token);

                if (!temp.empty()) {

                    if (opItem->instr == Instruction::SUB && (temp.top()->instr == Instruction::PAR_OPEN || isOperator(temp.top()->instr)) && last != TK_NUM && last != TK_CLOSE_BRK) {
                        opItem->instr = Instruction::UNARY_MINUS;
                    }

                    for (;;) {

                        //gestione precedenza operatori
                        StackItem* topOp = temp.top();

                        if (topOp->instr == Instruction::PAR_OPEN) {
                            break;
                        }

                        if (getOperatorPrecedence(topOp->instr) >= getOperatorPrecedence(opItem->instr)) {

                            instrVector->push_back(topOp);
                            temp.pop();

                        } else {
                            break;
                        }

                        if (temp.empty()) break;
                    }

                } else {

                    if (opItem->instr == Instruction::SUB && last != TK_NUM && last != TK_CLOSE_BRK) {
                        //se lo stack è vuoto ed è un meno allora è un meno unario
                        opItem->instr = Instruction::UNARY_MINUS;
                    }

                }

                temp.push(opItem);

                if ((last == TK_OPEN_BRK || last == TK_NIL) && opItem->instr != Instruction::UNARY_MINUS) {
                    ostringstream ss;
                    ss << "Unexpected operator " << token << " at index " << idx;
                    throwError(ss.str());
                }

                last = TK_OPERATOR;

            } else if (isFunction(token)) {
                if (last == TK_FUNCTION) {

                    ostringstream ss;

                    ss << "Invalid function sequence " << token << " at index " << idx;
                    //due operatori successivi
                    throwError(ss.str());
                }


                StackItem *opItem = new StackItem();
                opItem->fromString(token);

                if (!temp.empty()) {

                    for (;;) {

                        //gestione precedenza operatori
                        StackItem* topOp = temp.top();

                        if (topOp->instr == Instruction::PAR_OPEN) {
                            break;
                        }

                        if (getOperatorPrecedence(topOp->instr) > getOperatorPrecedence(opItem->instr)) {

                            instrVector->push_back(topOp);
                            temp.pop();

                        } else {
                            break;
                        }

                        if (temp.empty()) break;
                    }

                }

                temp.push(opItem);


                last = TK_FUNCTION;


            } else if (isVarDefined(token)) {

                //variable defined in the lookup table

                StackItem *s = new StackItem();
                s->instr = Instruction::VALUE;
                s->value = getVar(token);
                s->defVar = token;
                instrVector->push_back(s);
                last = TK_NUM;

            } else {

                ostringstream ss;

                ss << "Invalid token " << token << " at index " << idx;

                throwError(ss.str());
            }

            idx = next;
        }

        while (!temp.empty()) {

            StackItem *item = temp.top();

            if (item->instr == Instruction::PAR_OPEN) {
                throwError("Unclosed bracket found in expression.");
            }

            instrVector->push_back(item);
            temp.pop();
        }

        return *this;
    }

    const string& RPNCompiler::getLastCompiledStatement() {
        return last_compiled_statement;
    }

    bool RPNCompiler::isOperator(const string& token) {

        StackItem item;

        try {
            item.fromString(token);
            return isOperator(item.instr);
        } catch (VirtualFPUException &ex) {
            return false;
        }
    }

    bool RPNCompiler::isOperator(const Instruction instr) {
        return instr == Instruction::MUL || instr == Instruction::DIV || instr == Instruction::SUB || instr == Instruction::ADD || instr == Instruction::UNARY_MINUS;
    }

    bool RPNCompiler::isFunction(const string& token) {

        StackItem item;

        try {
            item.fromString(token);
            return isFunction(item.instr);
        } catch (VirtualFPUException &ex) {
            return false;
        }

    }

    bool RPNCompiler::isFunction(const Instruction& instr) {
        return RPNCompiler::isBuiltinFunction(instr);
    }

    bool RPNCompiler::isNumber(const string& token) {

        int idx = 0;
        const int lu = token.length();

        bool hasSign = false;
        bool hasDigit = false;
        bool decPoint = false;
        bool decExpected = false;
        bool trailing = false;

        while (idx < lu) {
            char ch = token[idx];

            if (isdigit(ch)) {
                if (ch == '0' && !hasDigit) {
                    decExpected = true;
                }

                if (trailing) {
                    return false;
                }

                hasDigit = true;
            } else if (ch == '-') {
                if (hasDigit || hasSign || trailing) {
                    return false;
                }

                hasSign = true;
            } else if (ch == '.') {
                if (!hasDigit || decPoint || trailing) {
                    return false;
                }

                decPoint = true;
            } else if (ch == ' ') {
                if (hasDigit || decPoint || hasSign) {
                    trailing = true;
                }
            } else {
                return false;
            }

            ++idx;
        }

        if (!hasDigit) {
            return false;
        }

        return true;

    }

    double RPNCompiler::toDouble(const string& token) {
        double r = 0;

        istringstream ss(token);

        ss >> r;

        if (ss.rdstate() & std::istringstream::failbit) {
            throwError(string("Error parsing double value:") + token);
        }

        return r;

    }

    string RPNCompiler::getToken(const string& statement, int fromIndex, int *nextIndex) {

        const size_t lu = statement.length();

        if (fromIndex > lu) return "";

        int idx = fromIndex;

        ostringstream ss;

        int state = 0;

        while (idx < lu) {

            char ch = statement[idx];

            if (ch == '(' || ch == ')' || ch == '+' || ch == '-' || ch == '/' || ch == '*') {

                if (state == 1) {
                    break;
                } else {
                    ++idx;
                    ss << ch;
                    break;
                }

            } else if (ch == ' ') {
                //skip
                ++idx;
                if (state == 1) {
                    break;
                }
            } else if (isalnum(ch) || ch == '.' || ch == ',') {

                state = 1;
                ss << ch;
                ++idx;

            } else {

                //simbolo non valido
                ss << ch;
                ++idx;
                break;

            }
        }

        *nextIndex = idx;
        return ss.str();
    }

    bool RPNCompiler::reduceStack(std::vector<StackItem*> &stack) {
        auto lu = stack.size();
        if (lu > 0) {
            int i = lu - 1;
            StackItem *item = stack[i];
            if (item->instr != Instruction::VALUE) {
                --i;
                if (i < 0) {
                    throwError("Invalid stack:found operation without operand.Reached end of stack");
                }
                StackItem *op1 = stack[i];
                if (op1->instr != Instruction::VALUE) {
                    throwError("Invalid stack:found operation without operand.");
                }

                if (isFunction(item->instr) || item->instr == Instruction::UNARY_MINUS) {

                    evaluateUnary(op1, item);
                    op1->defVar = ""s;
                    delete stack[lu - 1];
                    stack.pop_back();
                    return true;

                } else {

                    switch (item->instr) {

                        case Instruction::ADD:
                        case Instruction::SUB:
                        case Instruction::DIV:
                        case Instruction::MUL:
                        {
                            --i;
                            if (i < 0) {
                                throwError("Invalid stack:missing second operand");
                            }
                            StackItem *op2 = stack[i];
                            if (op2->instr != Instruction::VALUE) {
                                throwError("Invalid stack:value expected.");
                            }

                            op2->value = evaluateOperation(op2, op1, item);
                            op2->defVar = ""s;

                            delete stack[lu - 1];
                            delete stack[lu - 2];
                            stack.pop_back();
                            stack.pop_back();
                        }
                            return true;
                        default:
                            throwError("Unhandled instruction");
                            break;
                    }
                }
            }
        }
        return false;
    }

    double RPNCompiler::evaluate() {


        if (!instrVector || instrVector->empty()) {
            throw VirtualFPUException("Compile an expression before evaluating");
        }

        auto rpnEval = instrVector;
        auto executeStack = vector<StackItem*>{};
        try {
            for (auto it = rpnEval->begin(); it < rpnEval->end(); ++it) {
                StackItem *si = *it;
                executeStack.push_back(si->clone());
                while (reduceStack(executeStack)) {
                }
            }

            if (executeStack.size() == 1 && executeStack[0]->instr == Instruction::VALUE) {

                double r = executeStack[0]->value;
                delete executeStack[0];
                return r;
            } else {
                throw VirtualFPUException("Error evaluating expression " + last_compiled_statement);
            }
        } catch (VirtualFPUException &e) {
            stringstream ss;
            ss << "Error:" << e.getMessage();
            for (int i = 0; i < executeStack.size(); ++i) {
                delete executeStack[i];
            }
            throw VirtualFPUException(ss.str());
        }
    }

    StackItem* RPNCompiler::evaluateUnary(StackItem *operand, StackItem *operation) {

        const double val = getValue(operand);

        switch (operation->instr) {
            case Instruction::UNARY_MINUS:
                operand->value = -val;
                break;
            case Instruction::SIN:
                operand->value = sin(val);
                break;
            case Instruction::COS:
                operand->value = cos(val);
                break;
            case Instruction::TAN:
                operand->value = tan(val);
                break;
            case Instruction::SQRT:
                operand->value = sqrt(val);
                break;
            default:
                throwError("Unsupported function");
        }

        return operand;

    }

    double RPNCompiler::getValue(StackItem *operand) {
        return operand->defVar.empty() ? operand->value : getVar(operand->defVar);
    }

    double RPNCompiler::evaluateOperation(StackItem *op1, StackItem *op2, StackItem *operation) {
                        

        switch (operation->instr) {
            case Instruction::ADD:
                return getValue(op1) + getValue(op2);
            case SUB:
            case UNARY_MINUS:

                return getValue(op1) - getValue(op2);

            case MUL:

                return getValue(op1) * getValue(op2);

                break;
            case DIV:
                return getValue(op1) / getValue(op2);
            default:
                throwError("Unsupported function for two operands");
                return 0.0;
        }

    }

    size_t RPNCompiler::getStackSize() const {
        return instrVector->size();
    }

    size_t RPNCompiler::stackLength() const {
        return instrVector->size();
    }

    bool RPNCompiler::stackIsEmpty() const {
        return instrVector->empty();
    }

    void RPNCompiler::clearStack() {

        if (instrVector && instrVector->size() > 0) {

            for (int i = 0; i < instrVector->size(); ++i) {
                delete instrVector->at(i);
            }

            instrVector->clear();
        }
    }

    string RPNCompiler::getRPNStack() const {

        ostringstream ss;

        for (vector<StackItem*>::iterator it = instrVector->begin(); it != instrVector->end(); it++) {

            StackItem* instr = *it;

            StackItem& t = (*instr);

            ss << t << ',';

        }

        return ss.str();
    }

    double RPNCompiler::queryOutputRegister() const {
        return output;
    }

    void RPNCompiler::defineVar(const string &name, double value) {

        if (name == "") {
            throwError(string("Variabile name not set"));
        }

        if (name.find(' ') != string::npos) throw VirtualFPUException(string("Invalid variabile name:space is not allowed."));

        const size_t lu = name.length();

        if (!isalpha(name[0])) {
            throwError(string("Variabile name must start with a letter."));
        }

        for (size_t i = 0; i < lu; i++) {

            if (!isalnum((name[i]))) {
                throwError(string("Invalid variabile name:" + name));
            }
        }

        (*defVars)[name] = value;
    }

    void RPNCompiler::undefVar(const string &name) {

        if (defVars->find(name) != defVars->end()) {
            defVars->erase(name);
        }
    }

    bool RPNCompiler::isVarDefined(const string &name) {
        return defVars->find(name) != defVars->end();
    }

    double RPNCompiler::getVar(const string &varName) {

        if (!defVars->count(varName)) {
            throwError(string("Variabile ") + varName + string(" is not defined!"));
        }
        return defVars->at(varName);

    }

    void RPNCompiler::clearAllVariables() {

        defVars->clear();

    }

    void RPNCompiler::init(size_t stackSize) {

        if (stackSize <= 0) {
            throwError("Invalid stack size on init");
        }

        instrVector = new vector<StackItem*>();


        defVars = new map<string, double>();

    }

    void RPNCompiler::throwError(const string &msg) {
        stringstream ss;
        ss << msg << " expr:" << last_compiled_statement;
        throw VirtualFPUException(ss.str());
    }

}; //end namespace