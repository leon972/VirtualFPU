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

static void testStatements(RPNCompiler &fpu, const map<string, double> &statements) {
    for (auto const& [key, val] : statements) {
        fpu.compile(key);
        tests::expect_num(fpu.evaluate(), val, key, key, 0.00001);
    }
}

/*
 * 
 */
int main(int argc, char** argv) {

    try {

        tests::print_test_title("RPN Compiler tests");

        RPNCompiler fpu;

        // fpu.compile("2.1*(1+1)");
        //  cout<<fpu.evaluate()<<endl;

        fpu.compile("4sin(2.1)");
        cout << fpu.getRPNStack() << endl;
        cout << fpu.evaluate() << endl;
        cout << fpu.evaluate() << endl;

        cout << "-------------------------" << endl;

        fpu.defineVar("g", 10);
        fpu.compile("g*g-2");
        cout << fpu.getRPNStack() << endl;
        cout << "RESULT=" << fpu.evaluate() << endl;

        fpu.compile("5*(3+7)+10");
        cout << fpu.getRPNStack() << endl;
        cout << fpu.evaluate() << endl;
        tests::expect_true("5,3,7,+,*,10,+,"s == fpu.getRPNStack(), "RPN stack display error");

        fpu.compile("1+1");
        tests::expect_true(fpu.evaluate() == 2, "1+1");
        tests::expect_true(fpu.getLastCompiledStatement() == "1+1"s, "Error last compiled statement");
        tests::expect_num(fpu.evaluate(), 2.0, "1+1 error", "1+1 OK");

        map<string, double> statements = {
            {"2^2", 4},
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
            {"-3*(-2*(4/2))-2)", 10},
            {"(7-2)/(1+1)", 2.5},
            {"3^2/9", 1},
            {"3^2^2", 81},
            {"1-2.56^(sin(8/9))", 1 - 2.0746557603876212},
            {"4sin(2.3)-5cos(2.2)/6sin(1.1)", 3.419884621292166},
            {"4 + 5*sin(cos(12sqrt(8+32+5)))", 5.846170068808339},
            {"4*sin(-1.2)+(-1*(8/9+5/6))",4*sin(-1.2)+(-1*(8.0/9.0+5.0/6.0))}

        };

        testStatements(fpu, statements);

        tests::print_test_title("Test compiler error detection");
        tests::expect_throw([&]() {
            fpu.compile("4**6");
        }, "error not detected", "error detected", true);
        tests::expect_throw([&]() {
            fpu.compile("4*(2-(2-2)");
        }, "error not detected", "error detected", true);
        tests::expect_throw([&]() {
            fpu.compile("akjs4*(2-(2-2)");
        }, "error not detected", "error detected", true);

        tests::print_test_title("Test custom variables");

        fpu.clearStack();
        tests::expect_equals(fpu.getRPNStack(), ""s, "Stack not cleared"s);
        fpu.defineVar("g", 9.81);
        tests::expect_true(fpu.isVarDefined("g"), "var defined not detected"s, "OK var defined"s);
        tests::expect_num(fpu.getVar("g"), 9.81, "var value not valid"s, "OK set var"s);
        tests::expect_num(fpu.getVar("g"), 9.81, "var value not valid"s, "OK set var"s);
        fpu.compile("g*g-2");
        tests::expect_num(fpu.evaluate(), 94.2361, "error evaluating using custom variable"s, "OK calc with defned var");

        fpu.undefVar("g");
        tests::expect_false(fpu.isVarDefined("g"), "Error undefine var", "OK undefine var");

        fpu.defineVar("x", 0);
        fpu.defineVar("y", 0);

        fpu.compile("(x+y)*(x-y)");

        for (double x = -10; x < 10; x += 0.5) {
            for (double y = x; y < x + 10; y += 0.5) {
                fpu.defineVar("x", x);
                fpu.defineVar("y", y);
                const auto value = (x + y)*(x - y);
                const auto evaluated = fpu.evaluate();
                cout << "x=" << x << " y=" << y << " evaluated=" << fpu.evaluate() << " actual value=" << value << endl;
                tests::expect_num(evaluated, value, "failed calc with x,y");
            }
        }

        double x = 3.45;
        double y = -1.2;
        fpu.defineVar("x", x);
        fpu.defineVar("y", y);
        fpu.compile("sin((x+y)/2)*cos((x-y)/2)");
        tests::expect_num(fpu.evaluate(), sin((x + y) / 2) * cos((x - y) / 2), "failed to evalute using def var x,y", "OK expression x,y");
        fpu.compile("3*x*x*x-2*y*y/x");
        tests::expect_num(fpu.evaluate(), 3 * x * x * x - 2 * y * y / x, "failed to evalute using def var x,y", "OK expression x,y");

        fpu.defineVar("x", 3);
        fpu.compile("3.4*x^4-1*x^3+2*x^2-x-1");
        tests::expect_num(fpu.evaluate(), 262.4, "Error evaluating polynomial expression");

        fpu.compile("2x^2/(4x-x^3.1)");
        tests::expect_num(fpu.evaluate(), -0.992538005594048, "Error");

        tests::print_test_title("BUILTIN FUNCTIONS");

        fpu.defineVar("x", 1.67);
        fpu.defineVar("PI", M_PI);

        statements = {
            // {"2.1*tan(sin(1.22*2)-cos(2.1)+asin(0.7)-acos(0.89)+log(12)-log10(122)+x*log2(64)-sinh(12)+cosh(1)-tanh(2.5))", 2.1*tan(sin(1.22*2)-cos(2.1)+asin(0.7)-acos(0.89)+log(12)-log10(122)+x*log2(64)-sinh(12)+cosh(1)-tanh(2.5))},
            {"sin(PI*0.3)", sin(M_PI * 0.3)},
            {"tan(PI/4)", tan(M_PI / 4)},
            {"cos(PI/7)*sin(PI/6)", cos(M_PI / 7) * sin(M_PI / 6)},
            {"log10(1000000)", 6},
            {"log(123)", log(123)},
            {"log2(123)", log2(123)},
            {"sign(7-8)", -1.0},
            {"sign(5-(10/2))", 0.0},
            {"asin(sin(1.2))", 1.2},
            {"acos(sin(1.2))", acos(sin(1.2))},
            {"atan(tan(PI/8))", M_PI / 8},
            {"sinh(3.2)-cosh(8.9)+tanh(3.1)", sinh(3.2) - cosh(8.9) + tanh(3.1)},
            {"asinh(3.2)-acosh(8.9)+atanh(3.1)", asinh(3.2) - acosh(8.9) + atanh(3.1)},
            {"4exp(2.3)", 4 * exp(2.3)},
            {"sqrt(cos(x)^2+sin(x)^2)",1},
            {"sqrt(abs(-9*9))",9},
            {"abs(11)+abs(-11)+sign(7-7)",22}

        };

        testStatements(fpu, statements);

        cout << "TESTS SUCCESS!" << endl;


    } catch (std::exception &e) {
        tests::print_fail(string(e.what()));
    }
}

