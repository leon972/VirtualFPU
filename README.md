# VirtualFPU

This is plain standard C++20 self contained mathematical expression compiler and evaluator.
Internally uses a RPN (Reverse Polish Notation) stack to evalute the expression.

# Usage example:

```
  RPNCompiler fpu;        
        
  fpu.compile("4*sin(-1.2)+(-1*(8/9+5/6))");
       
  std::cout<<fpu.evaluate()<<std::endl;   //prints -5.45038
  
```

# Features

All values are treated as C++ double (no template supported yet!)

- available operators:
  + addition, - subtraction, * multiplication, / division, - unary minus, ^ power
- available built-in functions
  sin,cos,tan,asin,acos,atan,sinh,cosh,acosh,atanh,exp (base-e exponential function),log (natural logarithm), log10 (base 10 loh),log2 (base 2 log),sign (signum)  
  
- custom defined variables
  Define one or more variables and use in the expression:
  
```
    RPNCompiler fpu;      

    fpu.defineVar("x", 1.67);
    fpu.defineVar("PI", M_PI);
    
    fpu.compile("tan(PI/4)+x^2");
    
    std::cout<<fpu.evaluate()<<std::endl;   //prints 3.7889
    
    //change the value of x and evaluate without compiling again  
    fpu.defineVar("x", 2);
    
    std::cout<<fpu.evaluate()<<std::endl;   //now prints 5                     

```
# Include in your program

Warning! A C++20 compliant compiler is required
This software has been tested using gcc11

- Add virtualfpu.cpp in your source dir
- Add virtualfpu.h in your headers files






  
  

