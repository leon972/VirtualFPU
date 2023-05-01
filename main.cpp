/* 
 * File:   main.cpp
 * Author: Proprietario
 *
 * Created on 8 febbraio 2014, 16.03
 */

#include <cstdlib>
#include <iostream>
#include "virtualfpu.h"


using namespace std;
using namespace virtualfpu;

/**
 * 
 */
int main(int argc, char** argv) {

    try {

        RPNCompiler fpu;

        fpu.compile("4*sin(-1.2)+(-1*(8/9+5/6))");

        std::cout << fpu.evaluate() << std::endl; //prints -5.45038


    } catch (VirtualFPUException &ex) {

        cerr << ex.getMessage() << endl;
    }
    

    return 0;

}

