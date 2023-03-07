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

        VirtualFPU fpu(VirtualFPU::DEFAULT_STACK_SIZE);

        fpu.defineVar(string("g"), 9.82);

        //5,9,8,+,4,6,*,*,7,+,*
        //fpu.compile("5*(((9+8)*(4*6))+7)");

      //  fpu.compile("-3*(g+8)");
        
       //  fpu.compile("1/3+2+7*5");
        fpu.compile("sin(2)");

        //fpu.compile("6.55-2.2+1+2+3-4");

        cout << fpu.getRPNStack() << endl;
        
        cout<<"RESULT="<<fpu.evaluate()<<endl;

    } catch (VirtualFPUException &ex) {

        cerr << ex.getMessage() << endl;
    }

    // cout<<fpu.evaluate()<<endl;

    return 0;

}

