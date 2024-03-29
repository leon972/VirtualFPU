/* 
  File:   virtualfpu.h
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

#ifndef VIRTUALFPU_H
#define VIRTUALFPU_H

#include <map>
#include <string>
#include <map>
#include <stack>
#include <vector>
#include <iostream>
#include <functional>

namespace virtualfpu {

    using namespace std;

    /**
     * Available operators and functions
     */
    enum class Instruction {
        VALUE, PAR_OPEN, PAR_CLOSE, UNARY_MINUS, ADD, SUB, MUL, DIV, POW,DEF_FUNCTION, SQRT, SIN, COS, TAN, ASIN, ACOS, ATAN, ABS, EXP, LOG, LOG10, LOG2, SINH, COSH, TANH, ASINH, ACOSH, ATANH, SIGN
    };

    class VirtualFPUException : public std::exception {
    public:

        VirtualFPUException(const string& msg);

        virtual ~VirtualFPUException();

        const string getMessage() const;

        virtual const char * what() const noexcept override;

    private:

        string msg;


    };

    /**
     * RPN stack item
     */
    struct StackItem {
        Instruction instr;
        double value;
        string defVar;

        /**
         Converte da stringa ad operatore
         */

        void fromString(const string& opstr);

        StackItem *clone();

    };

    ostream& operator<<(ostream& s, const StackItem& item);

    /**
     * Mathematical expressions compiler and evaluator.
     * Converta the expression in a RPN (Reverse Polish Notation) before evaluation
     */
    class RPNCompiler {
    public:

        static const size_t DEFAULT_STACK_SIZE = 1024;





        /**
         * Create the compiler using a predefined RPN stack size 
         * 
         */
        RPNCompiler(size_t stackSize);

        /**
         * Create the compiler using the DEFAULT_STACK_SIZE
         */
        RPNCompiler();

        virtual ~RPNCompiler();

        /**
         * Compile a mathematical expression.
         * After the compilation a RPN stack is created internally and the evaluate method can be used to evalute the expression
         * @param statement example "4*(2.3*sin(1/(1+4.56)))/8, expressions can use user defined variable see the method defineVar
         */
        RPNCompiler& compile(const string& statement);

        /**
         * Evaluate the expression.Before calling this method the expression must be compiled using the compile method
         * @return 
         */
        double evaluate();


        /**
         * Max stack size
         * @return 
         */
        size_t getStackSize() const;

        /**
         * Actual number of instructions inside the internal stack
         * @return 
         */
        size_t stackLength() const;

        /**
         * Checks if the instructions stack is empty
         */
        bool stackIsEmpty() const;


        /**
         * Clear the instructions stack.After calling this method an expression must be compiled again before evaluating
         * @return 
         */
        void clearStack();


        /**
         * @return the RPN (Reverse Polish Notation) of the expression
         */
        string getRPNStack() const;


        double queryOutputRegister() const;

        /**
         * Define a custom variable.The variable can be used in the mathematical expression.
         * This method may be used to change the current value of an already defined variable.
         * If the variable was already defined before compilation, the expression can be evaluated using the new value
         * without compiling the expression again.
         * @param value variable value
         * @param name variable name
         * Example:
         * defineVar("pi",3.1415);
         * compile("cos(p1/2)")
         */
        void defineVar(const string &name, double value);

        /**
         * Undefine an existing custom variable
         */
        void undefVar(const string &name);

        /**
         * Define a custom function
         * @param name function name identified
         * @param fn the function
         */
        void defineFunction(const string &name, std::function<double(double) > fn);

        void undefFunction(const string &name);

        /**
         * Check if a variable is defined
         */
        bool isVarDefined(const string &name);

        /**
         * Chack if is a custom defined function
         * @param name
         * @return 
         */
        bool isFnDefined(const string &name);

        /**
         * Get the actual value of a custom defined variable
         * @param varName the variable symbol
         * @return the current value
         */
        double getVar(const string &varName);

        /**
         * Undefine all custom variables
         */
        void clearAllVariables();
        
        /**
         * Undefine all custom define functions
         */
        void clearAllCustomFunctions();


        const string& getLastCompiledStatement();


    protected:


        /**
         * Internal instruction stack
         */
        vector<StackItem*> *instrVector;

        /**
         * User defined variables
         */
        map<string, double> *defVars;

        /**
         * User defined functions
         */
        map<string, std::function<double(double) >> *defFunctions;

        /**
         * Current evaluation output
         */
        double output;

        string getToken(const string& statement, int fromIndex, int *nextIndex);

        double toDouble(const string& token);

        bool isNumber(const string& token);

        bool isOperator(const string& token);

        bool isOperator(const Instruction instr);

        /**
         * Check if the token is a function such as sin, cos
         * @param token
         * @return 
         */
        bool isFunction(const string& token);

        bool isFunction(const Instruction& instr);

        bool isFunction(const StackItem* item);

        bool isCustomFunction(const StackItem* item);

        void validateIndentifier(const string &name);

        /**       
         * @return a lower value means a lower precedence
         */
        int getOperatorPrecedence(const Instruction& instr) noexcept;

        static bool isBuiltinFunction(const Instruction& instr) noexcept;

    private:

        const int TK_NIL = 0;
        const int TK_NUM = 1;
        const int TK_OPERATOR = 2;
        const int TK_FUNCTION = 3;
        const int TK_OPEN_BRK = 4;
        const int TK_CLOSE_BRK = 5;
        const int TK_OTHER = 255;

        string last_compiled_statement;

        void init(size_t stackSize);

        StackItem* evaluateUnary(StackItem *operand, StackItem *operation);

        StackItem* evaluateCustomFn(StackItem *operand, StackItem *operation);

        double evaluateOperation(StackItem *op1, StackItem *op2, StackItem *operation);

        bool reduceStack(std::vector<StackItem*> &stack);

        double getValue(StackItem *operand);

        void addImpliedMul(stack<StackItem*> &temp, const int last);
        void addItemToTempStack(StackItem *item, stack<StackItem*> &stack, const int last);

        void throwError(const string &msg);

    };

};


#endif /* VIRTUALFPU_H */

