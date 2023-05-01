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
#include <functional>



using namespace std;


namespace virtualfpu {

    static std::vector<Instruction> functionsOp = {
        Instruction::ABS, Instruction::ACOS, Instruction::ACOSH, Instruction::ASIN,
        Instruction::ASINH, Instruction::ATAN, Instruction::ATANH, Instruction::COS,
        Instruction::COSH, Instruction::EXP,
        Instruction::LOG, Instruction::LOG10, Instruction::LOG2,
        Instruction::SIGN, Instruction::SIN,
        Instruction::SINH, Instruction::SQRT, Instruction::TAN, Instruction::TANH
    };

    static std::map<Instruction, std::function<double(double) >> oneArgFunctions = {
        {Instruction::UNARY_MINUS, [](double val) {
                return -val;
            }},
        {Instruction::SIGN, [](double val) {
                if (val > 0) {
                    return 1.0;
                } else if (val < 0) {
                    return -1.0;
                } else {
                    return 0.0;
                }
            }},
        {Instruction::ABS, [](double val) {
                return fabs(val);
            }},
        {Instruction::COS, [](double val) {
                return cos(val);
            }},
        {Instruction::SIN, [](double val) {
                return sin(val);
            }},
        {Instruction::TAN, [](double val) {
                return tan(val);
            }},
        {Instruction::ACOS, [](double val) {
                return acos(val);
            }},
        {Instruction::ASIN, [](double val) {
                return asin(val);
            }},
        {Instruction::ATAN, [](double val) {
                return atan(val);
            }},
        {Instruction::COSH, [](double val) {
                return cosh(val);
            }},
        {Instruction::SINH, [](double val) {
                return sinh(val);
            }},
        {Instruction::TANH, [](double val) {
                return tanh(val);
            }},
        {Instruction::ASINH, [](double val) {
                return asinh(val);
            }},
        {Instruction::ACOSH, [](double val) {
                return acosh(val);
            }},
        {Instruction::ATANH, [](double val) {
                return atanh(val);
            }},
        {Instruction::EXP, [](double val) {
                return exp(val);
            }},
        {Instruction::LOG, [](double val) {
                return log(val);
            }},
        {Instruction::LOG10, [](double val) {
                return log10(val);
            }},
        {Instruction::LOG2, [](double val) {
                return log2(val);
            }},
        {Instruction::SQRT, [](double val) {
                return sqrt(val);
            }}
    };

    static std::map<Instruction, string> symToStr = {
        {Instruction::UNARY_MINUS, "[-]"},
        {Instruction::ADD, "+"},
        {Instruction::SUB, "-"},
        {Instruction::MUL, "*"},
        {Instruction::DIV, "/"},
        {Instruction::POW, "^"},
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
        {"^", Instruction::POW},
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

    ostream& operator<<(ostream& ostr, const StackItem & item) {

        if (item.instr == Instruction::VALUE) {
            if (!item.defVar.empty()) {
                ostr << item.defVar;
            } else {
                ostr << item.value;
            }
        } else if (symToStr.contains(item.instr)) {
            ostr << symToStr[item.instr];
        } else if (item.instr==Instruction::DEF_FUNCTION)
        {
            ostr<<(item.defVar != "" ? item.defVar:"<custom fn?>");
        }        
        else {
            ostr << "<?>";
        }

        return ostr;

    }

    void StackItem::fromString(const string & opstr) {

        if (strToSymbol.contains(opstr)) {
            instr = strToSymbol[opstr];

        } else {
            throw VirtualFPUException(opstr + ": invalid operator or function.");
        }

    }

    StackItem * StackItem::clone() {
        StackItem *s = new StackItem();
        s->value = this->value;
        s->instr = this->instr;
        s->defVar = this->defVar;
        return s;
    }

    ////////////////////// VirtualFPUException ////////////////////////////////////

    VirtualFPUException::VirtualFPUException(const string & msg) : msg(msg) {

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

        if (defVars) {

            defVars->clear();
            delete defVars;
            defVars = nullptr;

        }

        if (defFunctions) {
            defFunctions->clear();
            delete defFunctions;
            defFunctions = nullptr;
        }

    }

    bool RPNCompiler::isBuiltinFunction(const Instruction & instr) noexcept {
        return std::find(functionsOp.begin(), functionsOp.end(), instr) != functionsOp.end();
    }

    int RPNCompiler::getOperatorPrecedence(const Instruction & instr) noexcept {

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
            case Instruction::POW:
                return 6;
            case Instruction::UNARY_MINUS:
                return 7;
            case Instruction::DEF_FUNCTION:
                return 8;
            default:
                if (isFunction(instr)) {
                    return 8;
                } else {
                    return -1;
                }

        }

    }

    RPNCompiler & RPNCompiler::compile(const string & statement) {

        const string err = "Syntax error:";

        clearStack();

        last_compiled_statement = statement;

        int idx = 0;
        int next = 0;

        const int lu = statement.length();

        /**   const int TK_NIL = 0;
           const int TK_NUM = 1;
           const int TK_OPERATOR = 2;
           const int TK_FUNCTION = 3;
           const int TK_OPEN_BRK = 4;
           const int TK_CLOSE_BRK = 5;
           const int TK_OTHER = 255;*/

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
                            break;
                        } else {
                            instrVector->push_back(s);
                            temp.pop();
                        }

                        if (temp.empty()) {
                            break;
                        }
                    }
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

                if (last == TK_NUM) {
                    addImpliedMul(temp, last);
                    last = TK_OPERATOR;
                }


                StackItem *opItem = new StackItem();
                opItem->fromString(token);

                addItemToTempStack(opItem, temp, last);

                last = TK_FUNCTION;


            } else if (isVarDefined(token)) {

                if (last == TK_NUM) {
                    addImpliedMul(temp, last);
                    last = TK_OPERATOR;
                }

                //variable defined in the lookup table

                StackItem *s = new StackItem();
                s->instr = Instruction::VALUE;
                s->value = getVar(token);
                s->defVar = token;
                instrVector->push_back(s);


                last = TK_NUM;

            } else if (isFnDefined(token)) {
                if (last == TK_FUNCTION) {

                    ostringstream ss;

                    ss << "Invalid function sequence " << token << " at index " << idx;
                    //due operatori successivi
                    throwError(ss.str());
                }

                if (last == TK_NUM) {
                    addImpliedMul(temp, last);
                    last = TK_OPERATOR;
                }


                StackItem *opItem = new StackItem();
                opItem->instr = Instruction::DEF_FUNCTION;
                opItem->defVar = token;

                addItemToTempStack(opItem, temp, last);

                last = TK_FUNCTION;

            }
            else {

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

    void RPNCompiler::addItemToTempStack(StackItem *opItem, stack<StackItem*> &temp, const int last) {

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

    }

    void RPNCompiler::addImpliedMul(stack<StackItem*> &temp, const int last) {
        StackItem *mulItem = new StackItem();
        mulItem->instr = Instruction::MUL;
        mulItem->value = 0;
        mulItem->defVar = "";
        addItemToTempStack(mulItem, temp, last);

    }

    const string & RPNCompiler::getLastCompiledStatement() {
        return last_compiled_statement;
    }

    bool RPNCompiler::isOperator(const string & token) {

        StackItem item;

        try {
            item.fromString(token);
            return isOperator(item.instr);
        } catch (VirtualFPUException &ex) {
            return false;
        }
    }

    bool RPNCompiler::isOperator(const Instruction instr) {
        return instr == Instruction::MUL || instr == Instruction::DIV || instr == Instruction::SUB || instr == Instruction::ADD || instr == Instruction::UNARY_MINUS || instr == Instruction::POW;
    }

    bool RPNCompiler::isFunction(const string & token) {

        StackItem item;

        try {
            item.fromString(token);
            return isFunction(item.instr) || defFunctions->contains(token);
        } catch (VirtualFPUException &ex) {
            return false;
        }

    }

    bool RPNCompiler::isFunction(const Instruction & instr) {
        return RPNCompiler::isBuiltinFunction(instr);
    }

    bool RPNCompiler::isFunction(const StackItem* item) {
        return isFunction(item->instr) || (item->defVar != "" && defFunctions->contains(item->defVar));
    }

    bool RPNCompiler::isCustomFunction(const StackItem* item) {
        return item->defVar != "" && defFunctions->contains(item->defVar);
    }

    bool RPNCompiler::isNumber(const string & token) {

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

    double RPNCompiler::toDouble(const string & token) {
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
        bool last_num = false;
        bool last_alpha = false;

        while (idx < lu) {

            char ch = statement[idx];

            if (ch == '(' || ch == ')' || ch == '+' || ch == '-' || ch == '/' || ch == '*' || ch == '^') {

                last_num = last_alpha = false;

                if (state == 1) {
                    break;
                } else {
                    ++idx;
                    ss << ch;
                    break;
                }

            } else if (ch == ' ') {

                last_num = last_alpha = false;

                //skip
                ++idx;
                if (state == 1) {
                    break;
                }
            } else if (isalpha(ch)) {

                if (last_num && !last_alpha) {
                    break;
                }

                last_num = false;
                last_alpha = isalpha(ch);

                state = 1;
                ss << ch;
                ++idx;

            } else if (ch == '.') {
                last_num = false;
                last_alpha = false;

                state = 1;
                ss << ch;
                ++idx;
            } else if (isdigit(ch)) {
                last_num = true;
                last_alpha = false;
                state = 1;
                ss << ch;
                ++idx;
            } else {

                last_num = false;
                last_alpha = false;

                //invalid
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

                } else if (item->instr == Instruction::DEF_FUNCTION) {
                    evaluateCustomFn(op1, item);
                    op1->defVar = ""s;
                    delete stack[lu - 1];
                    stack.pop_back();
                    return true;

                }
                else {

                    switch (item->instr) {

                        case Instruction::ADD:
                        case Instruction::SUB:
                        case Instruction::DIV:
                        case Instruction::MUL:
                        case Instruction::POW:
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
                            return false;
                            break;
                    }
                }
            }
        }

        return false;

    }//end reduceStack

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

    StackItem * RPNCompiler::evaluateUnary(StackItem *operand, StackItem * operation) {
        std::function<double(double) > fn = oneArgFunctions[operation->instr];
        if (!fn) {
            throwError("Cannot find the built-in one arg function "s + symToStr[operation->instr]);
            return nullptr;
        }
        operand->value = fn(getValue(operand));
        operand->defVar = "";
        return operand;
    }

    StackItem* RPNCompiler::evaluateCustomFn(StackItem *operand, StackItem * operation) {

        std::function<double(double) > fn = defFunctions->at(operation->defVar);
        if (!fn) {
            throwError("Cannot find custom function "s + operation->defVar);
        }
        operand->value = fn(getValue(operand));
        operand->defVar = "";
        return operand;
    }

    double RPNCompiler::getValue(StackItem * operand) {
        return operand->defVar.empty() ? operand->value : getVar(operand->defVar);
    }

    double RPNCompiler::evaluateOperation(StackItem *op1, StackItem *op2, StackItem * operation) {

        switch (operation->instr) {
            case Instruction::ADD:
                return getValue(op1) + getValue(op2);
            case Instruction::SUB:
            case Instruction::UNARY_MINUS:

                return getValue(op1) - getValue(op2);

            case Instruction::MUL:

                return getValue(op1) * getValue(op2);

                break;
            case Instruction::DIV:
                return getValue(op1) / getValue(op2);
            case Instruction::POW:
                return pow(getValue(op1), getValue(op2));
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

    void RPNCompiler::validateIndentifier(const string &name) {
        if (name == "") {
            throwError(string("Identifier name not set"));
        }

        if (name.find(' ') != string::npos) throw VirtualFPUException("Invalid identifier name "s + name + ":space is not allowed."s);

        const size_t lu = name.length();

        if (!isalpha(name[0])) {
            throwError(string("Variabile name must start with a letter."));
        }

        for (size_t i = 0; i < lu; i++) {

            if (!isalnum((name[i]))) {
                throwError(string("Invalid variabile name:" + name));
            }
        }

    }

    void RPNCompiler::defineVar(const string &name, double value) {

        validateIndentifier(name);

        if (!defVars->contains(name) && defFunctions->contains(name)) {
            throw VirtualFPUException("Variable name "s + name + " conflicts with an already defined function");
        }

        (*defVars)[name] = value;
    }

    void RPNCompiler::undefVar(const string & name) {

        if (defVars->find(name) != defVars->end()) {
            defVars->erase(name);
        }
    }

    void RPNCompiler::defineFunction(const string &name, std::function<double(double) > fn) {
        validateIndentifier(name);

        if (!defFunctions->contains(name) && defVars->contains(name)) {
            throw VirtualFPUException("Function name "s + name + " conflicts with an already defined variable");
        }

        (*defFunctions)[name] = fn;

    }

    void RPNCompiler::undefFunction(const string &name) {
        if (defFunctions->find(name) != defFunctions->end()) {
            defFunctions->erase(name);
        }
    }

    bool RPNCompiler::isVarDefined(const string & name) {
        return defVars->find(name) != defVars->end();
    }

    bool RPNCompiler::isFnDefined(const string &name) {
        return defFunctions->find(name) != defFunctions->end();
    }

    double RPNCompiler::getVar(const string & varName) {

        if (!defVars->count(varName)) {
            throwError(string("Variabile ") + varName + string(" is not defined!"));
        }
        return defVars->at(varName);

    }

    void RPNCompiler::clearAllVariables() {

        defVars->clear();

    }

    void RPNCompiler::clearAllCustomFunctions() {
        defFunctions->clear();
    }

    void RPNCompiler::init(size_t stackSize) {

        if (stackSize <= 0) {
            throwError("Invalid stack size on init");
        }

        instrVector = new vector<StackItem*>();


        defVars = new map<string, double>();
        defFunctions = new map<string, std::function<double(double) >>();

    }

    void RPNCompiler::throwError(const string & msg) {
        stringstream ss;
        ss << msg << " expr:" << last_compiled_statement;
        throw VirtualFPUException(ss.str());
    }

}; //end namespace