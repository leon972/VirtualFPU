/**
 * Tests suite helpers
 */

#ifndef TESTS_H
#define TESTS_H

#include <cstdlib>
#include <iostream>
#include <string>
#include <functional>
#include <cmath>
#include <sstream>

#define SEP "==================================================="
#define NUM_TOLERANCE 0.000001

namespace tests {
    using namespace std;

    void print_fail(const string& msg, bool terminate = false) noexcept {
        cerr << "FAIL!:" << msg << endl;
        if (terminate) {
            exit(0);
        }
    }

    void print_success(const string& msg) noexcept {
        cout << "SUCCESS:" << msg << endl;
    }

    void print_test_title(const string& title) {
        cout << SEP << endl << title << endl << SEP << endl << endl;
    }

    void expect_true(bool value, const string& fail_msg, const string& success_msg = ""s) noexcept {
        if (!value) {
            print_fail(fail_msg, true);
        } else if (success_msg != ""s) {
            print_success(success_msg);
        }
    }

    void expect_false(bool value, const string& fail_msg, const string& success_msg = ""s) {
        expect_true(!value, fail_msg, success_msg);
    }

    template<typename N>
    void expect_num(N value, N expected, const string& fail_msg, const string& success_msg = ""s, double tolerance = NUM_TOLERANCE) noexcept {
        if ((abs(static_cast<double> (value) - static_cast<double> (expected)) > tolerance)) {
            stringstream ss;
            ss << fail_msg << " (actual value:" << value << " expected:" << expected << ")" << endl;
            print_fail(ss.str(), true);
        } else if (success_msg != ""s) {
            print_success(success_msg);
        }
    }

    template<typename T>
    void expect_equals(T value, T expected, const string& fail_msg, const string& success_msg = ""s) noexcept {
        if (value != expected) {
            stringstream ss;

            ss << fail_msg << " (actual value:" << value << " expected:" << expected << ")" << endl;
            print_fail(ss.str(), true);
        } else if (success_msg != ""s) {
            print_success(success_msg);
        }
    }

    void expect_throw(std::function<void() > f, const string& fail_msg, const string& success_msg = ""s,bool show_exception_msg = false) {
        try {
            f();
            print_fail(fail_msg, true);
        } catch (std::exception &e) {
            if (show_exception_msg) {
                cerr << e.what() << endl;
            }
            if (success_msg!="")
            {
                print_success(success_msg);
            }
        }
    }

    void expect_nothrow(std::function<void() > f, const string& fail_msg) {

        try {
            f();
        } catch (std::exception &e) {
            cerr << e.what() << endl;
            print_fail(fail_msg, true);
        }
    }

}

#endif /* TESTS_H */

