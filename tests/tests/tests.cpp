/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: leonardo
 *
 * Created on 29 aprile 2023, 13:09
 */

#include <cstdlib>
#include <iostream>
#include <exception>
#include <string>
#include <vector>
#include <map>
#include "virtualfpu.h"
#include "tests.h"

using namespace std;
using namespace virtualfpu;

/*
 * 
 */
int main(int argc, char** argv) {

    try {

        tests::print_test_title("RPN Compiler tests");

        RPNCompiler fpu;     
        
        fpu.defineVar("g",10);      
        fpu.compile("g*g-2");
        cout<<fpu.getRPNStack()<<endl;
        cout<<fpu.evaluate()<<endl;

        fpu.compile("5*(3+7)+10");
        cout << fpu.getRPNStack() << endl;
        cout << fpu.evaluate() << endl;
        tests::expect_true("5,3,7,+,*,10,+,"s == fpu.getRPNStack(), "RPN stack display error");

        fpu.compile("1+1");
        tests::expect_true(fpu.evaluate() == 2, "1+1");
        tests::expect_true(fpu.getLastCompiledStatement() == "1+1"s, "Error last compiled statement");
        tests::expect_num(fpu.evaluate(), 2.0, "1+1 error", "1+1 OK");

        map<string, double> statements = {
            {"2*(3*(2*(7-2*(3-2))))", 60},
            {"5*(3+7)+10", 60},
            {"1/(2+7-8+4+5)", 0.1},
            {"-1*(-5)", 5},
            {"(5+6)/(7-8)", -11},
            {"5*(2+6)-7*(11-4*(6-7))", -65},
            {"sin(cos(-1.233453223+2))/(1-sin(1.2))", 9.70584},
            {"-sqrt(9)*(8+2)", -30},
            {"1+2+3+4+5+7*8/9", 21.222222},
            {"-3*(-2*(2-3)*(8-3))", -30},
            {"-1*(-2*(-6*(1*(8+9/6))))", -114},
            {"sin((2-3)*1.222)", -0.939785},
            {"2", 2},
            {"sin(0.89)", 0.777072},
            {"1/(4-1/(2-3))", 0.2},
            {"2+5*6/8-3", 2.75},
            {"1+1+1+1+1+1", 6},
            {"3*((2-5))", -9},
            {"-(5+2)", -7},
            {"-3*(-2*(4/2))-2)",10},
            {"(7-2)/(1+1)",2.5}
        };

        for (auto const& [key, val] : statements) {
            fpu.compile(key);
            tests::expect_num(fpu.evaluate(), val, key, key, 0.00001);
        }
        
        tests::print_test_title("Test compiler error detection");
        tests::expect_throw([&](){fpu.compile("4**6");},"error not detected","error detected",true);
        tests::expect_throw([&](){fpu.compile("4*(2-(2-2)");},"error not detected","error detected",true);
        tests::expect_throw([&](){fpu.compile("akjs4*(2-(2-2)");},"error not detected","error detected",true);
        
        tests::print_test_title("Test custom variables");
        
        fpu.clearStack();
        tests::expect_equals(fpu.getRPNStack(),""s,"Stack not cleared"s);
        fpu.defineVar("g",9.81);
        tests::expect_true(fpu.isVarDefined("g"),"var defined not detected"s,"OK var defined"s);
        tests::expect_num(fpu.getVar("g"),9.81,"var value not valid"s,"OK set var"s);
        tests::expect_num(fpu.getVar("g"),9.81,"var value not valid"s,"OK set var"s);
        fpu.compile("g*g-2");
        tests::expect_num(fpu.evaluate(),94.2361,"error evaluating using custom variable"s);
        


    } catch (std::exception &e) {
        tests::print_fail(string(e.what()));
    }
}

