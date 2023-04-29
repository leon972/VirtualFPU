/**
 * Tests suite helpers
 */

#ifndef TESTS_H
#define TESTS_H

#include <cstdlib>
#include <iostream>
#include <string>
#include <functional>

#define SEP "==================================================="

namespace tests {
    using namespace std;

    void print_test_title(const string& title) {
        cout << SEP << endl << title << SEP << endl << endl;
    }

    void expect_true(bool value, const string& fail_msg) {
        if (!value) {
            print_fail(fail_msg);
        }
    }

    void expect_false(bool value, const string& fail_msg) {
        expect_true(!value, fail_msg);
    }

    void print_fail(const string& msg) {
        cerr << "FAIL:" << msg << endl;
    }

    void expect_throw(std::function<void() > f, const string& fail_msg, bool show_exception_msg) {
        try {
            f();
            print_fail(fail_msg);
        } catch (std::exception &e) {
            cerr << e.what() << endl;
        }
    }

    void expect_nothrow(std::function<void() > f, const string& fail_msg) {
        
        try {
            f();
        } catch (std::exception &e) {
            cerr << e.what() << endl;
            print_fail(fail_msg);
        }
    }

}

#endif /* TESTS_H */

