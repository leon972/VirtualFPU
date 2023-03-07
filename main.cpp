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

        VirtualFPU fpu;

        fpu.defineVar(string("g"), 9.82);
        
        cout<<fpu.getVar("g")<<endl;

        //5,9,8,+,4,6,*,*,7,+,*
        //fpu.compile("5*(((9+8)*(4*6))+7)");

      //  fpu.compile("-3*(g+8)");
        
       //  fpu.compile("1/3+2+7*5");
        fpu.compile("6*g");

        //fpu.compile("6.55-2.2+1+2+3-4");

        cout << fpu.getRPNStack() << endl;
        
        cout<<"RESULT="<<fpu.evaluate()<<endl;
        
        fpu.defineVar("x",2);
        fpu.compile("x*x");
        cout << fpu.getRPNStack() << endl;
        cout<<fpu.evaluate()<<endl;
        
        for (double x=-10;x<=10;x+=0.5)
        {
            fpu.defineVar("x",x);
            cout<<x<<" -> "<<fpu.evaluate()<<endl;
        }
        

    } catch (VirtualFPUException &ex) {

        cerr << ex.getMessage() << endl;
    }

    // cout<<fpu.evaluate()<<endl;

    return 0;

}

