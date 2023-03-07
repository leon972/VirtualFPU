/** Author: Leonardo Berti
 * Copyright (c) 2014
 * Created on 8 febbraio 2014, 16.03
 * 
 * FPU virtuale per l'interpretazione e compilazione in notazione RPN
 * (Reverse Polish Notation)
 * di espressioni matematiche
 * 
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
#include <list>


using namespace std;


namespace virtualfpu {

    /////////////////////// StackItem ////////////////////////////////////////////

    /**
     * Output operator per lo stack item
     * invia allo stream di output (es. stdout)
     * la rappresetnazione dell'elemento 
     */
    ostream& operator<<(ostream& ostr, const StackItem& item) {

        switch (item.instr) {
            case VALUE:
                ostr << item.value;
                break;
            case UNARY_MINUS:
                ostr << "[-]";
                break;
            case ADD:
                ostr << "+";
                break;
            case SUB:
                ostr << "-";
                break;
            case MUL:
                ostr << "*";
                break;
            case DIV:
                ostr << "/";
                break;
            case SQRT:
                ostr << "sqrt";
                break;
            case COS:
                ostr << "cos";
                break;
            case SIN:
                ostr << "sin";
                break;

            default:
                ostr << "???";
                break;

        }

        return ostr;

    }

    void StackItem::fromString(const string& opstr) {

        if (opstr == "+") {
            instr = ADD;
        } else if (opstr == "-") {
            instr = SUB;
        } else if (opstr == "/") {
            instr = DIV;
        } else if (opstr == "*") {
            instr = MUL;
        } else if (opstr == "cos") {
            instr = COS;
        } else if (opstr == "sin") {
            instr = SIN;
        } else if (opstr == "sqrt") {
            instr = SQRT;
        } else if (opstr == "(") {
            instr = PAR_OPEN;
        } else if (opstr == ")") {
            instr = PAR_CLOSE;
        } else {
            throw VirtualFPUException(opstr + ": invalid operator or function.");
        }

    }

    StackItem* StackItem::clone() {
        StackItem *s = new StackItem();
        s->value = this->value;
        s->instr = this->instr;
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

    ////////////////////// VirtualFPU //////////////////////////////////////////////

    /**
     * 
     * @param stackSize dimensione stack in "word" virtuali
     */
    VirtualFPU::VirtualFPU(size_t stackSize) : instrStack(0) {
        init(stackSize);
    }

    VirtualFPU::VirtualFPU() : instrStack(0) {
        init(DEFAULT_STACK_SIZE);
    }

    VirtualFPU::~VirtualFPU() {

        if (instrStack) {

            clearStack();
            delete instrStack;
            instrStack = 0;

        }

        if (instrVector) {
            delete instrVector;
        }

        if (defVars && !defVars->empty()) {

            defVars->clear();
            delete defVars;
            defVars = 0;

        }

    }

    /**
     * Restituisce la precedenza dell'operatore
     */
    int VirtualFPU::getOperatorPrecedence(const Instruction& instr) {

        switch (instr) {
            case VALUE:
                return 0;
            case ADD:
            case SUB:
                return 1;
            case MUL:
            case DIV:
                return 2;
            case COS:
            case SIN:
            case SQRT:
                return 3;
            case UNARY_MINUS:
                return 4;
            default:
                return -1;

        }

    }

    /**
     * Compila l'espressione creando lo stack RPN usato in seguito per la valutazione del valore
     * dell'espressione
     */
    VirtualFPU& VirtualFPU::compile(const string& statement) {

        const string err = "Syntax error:";

        clearStack();

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
            throw VirtualFPUException(err + "expression is empty");
        }

        string token = "";

        int state = 0;

        stack<StackItem*> temp;

        //tipo ultimo token processato
        int last = TK_NIL;

        //parser
        //converte la forma infissa dell'espressione nella forma postfissa
        //"reverse polish notation"
        while (idx < lu) {

            //estrae il token
            token = getToken(statement, idx, &next);

            if (isNumber(token)) {

                StackItem *s = new StackItem();

                s->instr = VALUE;
                s->value = toDouble(token);

                last = TK_NUM;

                instrVector->push_back(s);

            } else if (token == "(") {

                if (last == TK_CLOSE_BRK) {
                    ostringstream ss;
                    ss << "Invalid bracket " << token << " at index " << idx << " (missing operator or function)";
                    throw VirtualFPUException(ss.str());
                }

                last = TK_OPEN_BRK;

                StackItem *s = new StackItem();
                s->instr = PAR_OPEN;
                s->value = 0;

                temp.push(s);

            } else if (token == ")") {

                if (last == TK_OPEN_BRK) {
                    ostringstream ss;
                    ss << "Empty brackets at index " << idx;
                    throw VirtualFPUException(ss.str());
                }

                last = TK_CLOSE_BRK;

                if (temp.empty()) {

                    ostringstream ss;
                    ss << err << ") not expected at position " << idx;
                    throw VirtualFPUException(ss.str());
                }

                bool matchingParFound = false;

                for (;;) {

                    StackItem *s = temp.top();

                    if (s->instr == PAR_OPEN) {
                        temp.pop();
                        matchingParFound = true;
                        break;
                    }

                    instrVector->push_back(temp.top());
                    temp.pop();

                    if (temp.empty()) {
                        break;
                    }
                }

                if (!matchingParFound) {

                    ostringstream ss;
                    ss << "Missing matching bracket at index " << idx;
                    throw new VirtualFPUException(ss.str());
                }


            } else if (isOperator(token)) {

                if (last == TK_OPERATOR) {

                    ostringstream ss;

                    ss << "Invalid operator " << token << " at index " << idx;
                    //due operatori successivi
                    throw VirtualFPUException(ss.str());
                }

                StackItem *opItem = new StackItem();
                opItem->fromString(token);

                if (!temp.empty()) {

                    if (opItem->instr == SUB && temp.top()->instr == PAR_OPEN && last != TK_NUM) {
                        opItem->instr = UNARY_MINUS;
                    }

                    for (;;) {

                        //gestione precedenza operatori
                        StackItem* topOp = temp.top();

                        if (topOp->instr == PAR_OPEN) {
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

                } else {

                    if (opItem->instr == SUB && last != TK_NUM) {
                        //se lo stack è vuoto ed è un meno allora è un meno unario
                        opItem->instr = UNARY_MINUS;
                    }

                }

                temp.push(opItem);

                if ((last == TK_OPEN_BRK || last == TK_NIL) && opItem->instr != UNARY_MINUS) {
                    ostringstream ss;
                    ss << "Unexpected operator " << token << " at index " << idx;
                    throw VirtualFPUException(ss.str());
                }

                last = TK_OPERATOR;

            } else if (isVarDefined(token)) {

                StackItem *s = new StackItem();

                s->instr = VALUE;
                s->value = getVar(token);

                instrVector->push_back(s);

                last = TK_NUM;

            } else {

                ostringstream ss;

                ss << "Invalid token " << token << " at index " << idx;

                throw VirtualFPUException(ss.str());
            }

            idx = next;
        }

        while (!temp.empty()) {

            StackItem *item = temp.top();

            if (item->instr == PAR_OPEN) {
                throw VirtualFPUException("Unclosed bracket found in expression.");
            }

            instrVector->push_back(item);
            temp.pop();
        }

        return *this;
    }

    /**
     *Determina se il token è un operatore valido
     */
    bool VirtualFPU::isOperator(const string& token) {

        StackItem item;

        try {
            item.fromString(token);
            return true;
        } catch (VirtualFPUException &ex) {
            //non è un operatore o funzione
            return false;
        }
    }

    /**
     * Determina se è un numero
     */
    bool VirtualFPU::isNumber(const string& token) {

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

    double VirtualFPU::toDouble(const string& token) {
        double r = 0;

        istringstream ss(token);

        ss >> r;

        if (ss.rdstate() & std::istringstream::failbit) {
            throw VirtualFPUException(string("Error parsing double value:") + token);
        }

        return r;

    }

    string VirtualFPU::getToken(const string& statement, int fromIndex, int *nextIndex) {

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

    bool VirtualFPU::reduceStack(std::vector<StackItem*> &stack) {
        auto lu = stack.size();
        if (lu > 0) {
            int i = lu - 1;
            StackItem *item = stack[i];
            if (item->instr != VALUE) {
                --i;
                if (i < 0) {
                    throw VirtualFPUException("Invalid stack:found operation without operand.Reached end of stack");
                }
                StackItem *op1 = stack[i];
                if (op1->instr != VALUE) {
                    throw VirtualFPUException("Invalid stack:found operation without operand.");
                }
                switch (item->instr) {
                    case UNARY_MINUS:
                    case COS:
                    case SIN:
                    case SQRT:
                        evaluateUnary(op1, item);
                        delete stack[lu - 1];
                        stack.pop_back();
                        return true;
                    case ADD:
                    case SUB:
                    case DIV:
                    case MUL:
                    {
                        --i;
                        if (i < 0) {
                            throw VirtualFPUException("Invalid stack:missing second operand");
                        }
                        StackItem *op2 = stack[i];
                        if (op2->instr != VALUE) {
                            throw VirtualFPUException("Invalid stack:value expected.");
                        }

                        op2->value = evaluateOperation(op2, op1, item);

                        delete stack[lu - 1];
                        delete stack[lu - 2];
                        stack.pop_back();
                        stack.pop_back();
                    }
                        return true;
                    default:
                        throw VirtualFPUException("Unhandled instruction");
                        break;
                }
            }
        }
        return false;
    }

    /**
     * Valuta l'espressione compilata in precedenza
     * @return 
     */
    double VirtualFPU::evaluate() {

        //6 7 - 7 3 - * 2 -

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

            if (executeStack.size() == 1 && executeStack[0]->instr == VALUE) {

                double r = executeStack[0]->value;
                delete executeStack[0];
                return r;
            } else {
                throw new VirtualFPUException("Error evaluating expression.");
            }
        } catch (VirtualFPUException &e) {
            stringstream ss;
            ss << "Error:" << e.getMessage();
            for (int i=0;i<executeStack.size();++i)
            {
                delete executeStack[i];
            }
            throw new VirtualFPUException(ss.str());
        }


        // try {
        /**  while (
                  .size()>1) {
                
              for (auto it=executeList.begin();it!=executeList.end();++it)
              {
                  StackItem *op1=*it;
                  if (op1->instr!=VALUE)
                  {
                      throw new VirtualFPUException("Value was expected ad first list element");
                  }
                  StackItem *op2=*it;
                  if (op2->instr!=VALUE)
                  {
                      StackItem *r=evaluateUnary(op1,op2);                        
                      executeList.remove(op1);
                      executeList.remove(op2);
                        
                  }
                    
              }
                
              op1 = executeList.top();
              executeList.pop();
              if (op1->instr != VALUE) {
                  throw VirtualFPUException(string("Expected value for instruction operand 1"));
              }
              if (executeList.empty()) {
                  throw VirtualFPUException(string("Unexpected end of stack"));
              }
              op2 = executeList.top();
              executeList.pop();
              if (op2->instr != VALUE) {
                  StackItem *r=evaluateUnary(op1,op2);
                  delete op1;op1=nullptr;
                  delete op2;op2=nullptr;
                  executeList.push(r);

              } else {
                  if (executeList.empty()) {
                      throw VirtualFPUException(string("Unexpected end of stack"));
                  }
                  op3 = executeList.top();
                  executeList.pop();
                  StackItem *r=evaluateOperation(op1,op2,op3);
                  delete op1;op1=nullptr;
                  delete op2;op2=nullptr;
                  delete op3;op3=nullptr;
                  executeList.push(r);                                     

              }                
          }
      } catch (VirtualFPUException &e) {
          throw VirtualFPUException("Error evaluating expression:"s + e.getMessage());
      }*/

        /** StackItem* finalResult=executeList.top();
         executeList.pop(); //now stack must be empty
         double r=finalResult->value;
         delete finalResult;finalResult=nullptr;
         return r;        */



    }

    StackItem* VirtualFPU::evaluateUnary(StackItem *operand, StackItem *operation) {

        switch (operation->instr) {
            case UNARY_MINUS:
                operand->value = -operand->value;
                break;
            case SIN:
                operand->value = sin(operand->value);
                break;
            case COS:
                operand->value = cos(operand->value);
                break;
            case SQRT:
                operand->value = sqrt(operand->value);
                break;
            default:
                throw VirtualFPUException("Unsupported function");
        }

        return operand;

    }

    /**
     * op1 <op> op2
     * @param op1
     * @param op2
     * @param operation
     * @return 
     */
    double VirtualFPU::evaluateOperation(StackItem *op1, StackItem *op2, StackItem *operation) {

        switch (operation->instr) {
            case ADD:
                return op1->value + op2->value;
            case SUB:
            case UNARY_MINUS:
                return op1->value - op2->value;
            case MUL:
                return op1->value * op2->value;
            case DIV:
                return op1->value / op2->value;
            default:
                throw VirtualFPUException("Unsupported function for two operands");
        }

    }

    /**
     * Dimensione massima dello stack
     * @return 
     */
    size_t VirtualFPU::getStackSize() const {
        return stackSize;
    }

    /**
     * Numero di istruzioni presenti nello stack
     * @return 
     */
    size_t VirtualFPU::stackLength() const {
        return instrStack->size();
    }

    /**
     * Determina se lo stack è vuoto
     */
    bool VirtualFPU::stackIsEmpty() const {
        return instrStack->empty();
    }

    /**
     * Resetta lo stack
     * @return 
     */
    void VirtualFPU::clearStack() {

        if (instrStack && !instrStack->empty()) {

            while (!instrStack->empty()) {
                delete instrStack->top();
                instrStack->pop();
            }

        }
    }

    /**
     * Restituisce la rappresentazione RPN
     * dell'espressione
     */
    string VirtualFPU::getRPNStack() const {

        ostringstream ss;

        for (vector<StackItem*>::iterator it = instrVector->begin(); it != instrVector->end(); it++) {

            StackItem* instr = *it;

            StackItem& t = (*instr);

            ss << t << ',';

        }

        return ss.str();
    }

    /**
     * Restituisce lo stato del registro di output
     * @return 
     */
    double VirtualFPU::queryOutputRegister() const {
        return output;
    }

    /**
     * Definisce una variabile
     * se la variabile è già esistente ne cambia il valore
     * @param name nome della variabile che puo' essere usata nell'espressione
     * @param value
     */
    void VirtualFPU::defineVar(const string &name, double value) {

        if (name == "") {
            throw new VirtualFPUException(string("Variabile name not set"));
        }

        if (name.find(' ') != string::npos) throw VirtualFPUException(string("Invalid variabile name:space is not allowed."));

        const size_t lu = name.length();

        if (!isalpha(name[0])) {
            throw VirtualFPUException(string("Variabile name must start with a letter."));
        }

        for (size_t i = 0; i < lu; i++) {

            if (!isalnum((name[i]))) {
                throw VirtualFPUException(string("Invalid variabile name:" + name));
            }
        }

        (*defVars)[name] = value;
    }

    /**
     * Rimuove una variabile
     */
    void VirtualFPU::undefVar(const string &name) {

        if (defVars->find(name) != defVars->end()) {
            defVars->erase(name);
        }
    }

    /**
     * Determina se una variabile è definita
     * @param name
     * @return 
     */
    bool VirtualFPU::isVarDefined(const string &name) {
        return defVars->find(name) != defVars->end();
    }

    /**
     * Acquisisce il valore di una variabile
     * @param varName
     * @return 
     */
    double VirtualFPU::getVar(const string &varName) {

        try {

            return (*defVars)[varName];

        } catch (std::out_of_range &ex) {
            throw new VirtualFPUException(string("Variabile ") + varName + string(" is not defined!"));
        }

    }

    /**
     *Resetta tutte le variabili
     */
    void VirtualFPU::clearAllVariables() {

        defVars->clear();

    }

    /**
     * inizializza lo stack e le altre strutture, invocato dal costruttore
     * @param stackSize
     * @return 
     */
    void VirtualFPU::init(size_t stackSize) {

        if (stackSize <= 0) {
            throw VirtualFPUException("Invalid stack size on init");
        }

        instrVector = new vector<StackItem*>();

        instrStack = new stack<StackItem*, vector<StackItem*> >(*instrVector);

        this->stackSize = stackSize;

        defVars = new map<string, double>();

    }

}; //fine namespace