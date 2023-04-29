/* 
 * File:   virtualfpu.h
 * Author: Leonardo Berti
 * Copyright (c) 2014
 * Created on 8 febbraio 2014, 16.03
 * 
 * FPU virtuale per l'interpretazione e compilazione in notazione RPN
 * (Reverse Polish Notation)
 * di espressioni matematiche
 */

#ifndef VIRTUALFPU_H
#define VIRTUALFPU_H

#include <map>
#include <string>
#include <map>
#include <stack>
#include <vector>
#include <iostream>

namespace virtualfpu {

    using namespace std;

    /**
     *Set di istruzioni disponibili
     */
    enum Instruction {
        VALUE, PAR_OPEN, PAR_CLOSE, UNARY_MINUS, ADD, SUB, MUL, DIV, SQRT, SIN, COS
    };

    /**
     * Eccezione virtual FPU
     * @param msg
     */
    class VirtualFPUException {
    public:

        VirtualFPUException(const string& msg);

        virtual ~VirtualFPUException();

        const string getMessage() const;

    private:

        string msg;


    };

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
     * FPU virtuale per calcoli matematici.
     * Utilizza architettura stack-based per la valutazione di espressioni matematiche
     * L'espressione viene convertita in notazione polacca inversa e valutata usando uno stack
     * @param stackSize
     */
    class RPNCompiler {
    public:

        static const size_t DEFAULT_STACK_SIZE = 1024;


        /**
         * Restituisce la precedenza dell'operatore
         */
        static int getOperatorPrecedence(const Instruction& instr);

        /**
         * Compila l'espressione creando lo stack RPN usato in seguito per la valutazione del valore
         * dell'espressione
         */
        RPNCompiler& compile(const string& statement);

        /**
         * Valuta l'espressione compilata in precedenza
         * @return 
         */
        double evaluate();


        /**
         * 
         * @param stackSize dimensione stack in "word" virtuali
         */
        RPNCompiler(size_t stackSize);

        RPNCompiler();

        virtual ~RPNCompiler();


        /**
         * Dimensione massima dello stack
         * @return 
         */
        size_t getStackSize() const;

        /**
         * Numero di istruzioni presenti nello stack
         * @return 
         */
        size_t stackLength() const;

        /**
         * Determina se lo stack è vuoto
         */
        bool stackIsEmpty() const;


        /**
         * Resetta lo stack
         * @return 
         */
        void clearStack();


        /**
         *Restituisce la rappresentazione RPN
         */
        string getRPNStack() const;


        /**
         * Restituisce lo stato del registro di output
         * @return 
         */
        double queryOutputRegister() const;

        /**
         * Definisce una variabile
         * se la variabile è già esistente ne cambia il valore
         * @param name nome della variabile che puo' essere usata nell'espressione
         * @param value
         */
        void defineVar(const string &name, double value);

        /**
         * Rimuove una variabile
         */
        void undefVar(const string &name);

        /**
         * Determina se una variabile è definita
         * @param name
         * @return 
         */
        bool isVarDefined(const string &name);


        /**
         * Acquisisce il valore di una variabile
         * @param varName
         * @return 
         */
        double getVar(const string &varName);

        /**
         *Resetta tutte le variabili
         */
        void clearAllVariables();


    protected:

        

        //vettore usato come delegato dello stack, si usa un vettore per poter
        //esaminarne velocemente il contenuto
        vector<StackItem*> *instrVector;

     //   stack<StackItem *, vector<StackItem*> > *instrStack;

        /**
         Definizione variabili
         */
        map<string, double> *defVars;

        /**
         * Output della valutazione dell'espressione
         * @param stackSize
         */
        double output;


        /**
         * Acquisisce un token
         * @param statement
         * @param fromIndex
         * @param nextIndex
         * @return 
         */
        string getToken(const string& statement, int fromIndex, int *nextIndex);

        /**
         * Converte il token in double
         * @param stackSize
         */
        double toDouble(const string& token);

        /**
         * Determina se è un valore numerico
         * @param token
         * @return 
         */
        bool isNumber(const string& token);

        /**
         * Determina se è un operatore
         * @param token
         * @return 
         */
        bool isOperator(const string& token);
        
        bool isOperator(const Instruction instr);
        
        bool isFunction(const string& token);



    private:

        /**
         * inizializza lo stack e le altre strutture, invocato dal costruttore
         * @param stackSize
         * @return 
         */
        void init(size_t stackSize);

        StackItem* evaluateUnary(StackItem *operand, StackItem *operation);

        double evaluateOperation(StackItem *op1, StackItem *op2, StackItem *operation);

        bool reduceStack(std::vector<StackItem*> &stack);
        
        double getValue(StackItem *operand);

    };


}; //fine namespace


#endif /* VIRTUALFPU_H */

