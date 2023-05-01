/* 
 * File:   main.cpp
 * Author: Proprietario
 *
 * Created on 8 febbraio 2014, 16.03
 */

#include <cstdlib>
#include <iostream>
#include "virtualfpu.h"
#include <cmath>

using namespace virtualfpu;

/**
 * 
 */
int main(int argc, char** argv) {

    try {

        RPNCompiler fpu;        
        
        fpu.compile("4*sin(-1.2)+(-1*(8/9+5/6))");
        
        std::cout<<fpu.evaluate()<<std::endl;   //print -5.45038
        
        
        fpu.defineVar("x", 2);
    fpu.defineVar("PI", M_PI);
        
         fpu.compile("tan(PI/4)+x^2");
         
          std::cout<<fpu.evaluate()<<std::endl;

    } catch (VirtualFPUException &ex) {

        std::cerr << ex.getMessage() << std::endl;
    }   

    return 0;

}

