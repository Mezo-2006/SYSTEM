// COMPREHENSIVE TEST SUITE FOR SWITCH-CASE COMPILER
// Testing all language features, edge cases, and error conditions

// ============================================================================
// TEST 1: BASIC SWITCH STATEMENT
// ============================================================================
#include <iostream>
int main() {
    int x = 5;
    switch (x) {
        case 1: x = 10; break;
        case 5: x = 50; break;
        default: x = 0; break;
    }
    return 0;
}

// ============================================================================
// TEST 2: COMPLEX ARITHMETIC EXPRESSIONS
// ============================================================================
#include <iostream>
int main() {
    int a = 3;
    int b = 4;
    int result = (a + b) * (a - b) + a * b / 2;
    switch (result) {
        case -1: result = 100; break;
        case 5: result = 200; break;
        case 7: result = 300; break;
        default: result = 999; break;
    }
    return 0;
}

// ============================================================================
// TEST 3: NESTED EXPRESSIONS WITH CONSTANTS
// ============================================================================
#include <iostream>
int main() {
    int x = 2;
    int y = 3;
    int z = ((x + 1) * (y - 1)) + (5 * 2) - (8 / 4);
    switch (z) {
        case 12: z = z + 10; break;
        case 14: z = z * 2; break;
        case 16: z = (z + 5) * (z - 3); break;
        default: z = 0; break;
    }
    return 0;
}

// ============================================================================
// TEST 4: MULTIPLE VARIABLE DECLARATIONS
// ============================================================================
#include <iostream>
int main() {
    int first = 1;
    int second = 2;
    int third = 3;
    int fourth = first + second + third;
    switch (fourth) {
        case 6: 
            first = 100;
            second = 200;
            third = 300;
            break;
        case 7:
            first = second * third;
            break;
        default:
            first = 0;
            second = 0;
            third = 0;
            break;
    }
    return 0;
}

// ============================================================================
// TEST 5: SWITCH ON EXPRESSION RESULT
// ============================================================================
#include <iostream>
int main() {
    int base = 5;
    switch (base * 2 + 1) {
        case 11:
            base = base + 10;
            break;
        case 21:
            base = base * 3;
            break;
        default:
            base = 999;
            break;
    }
    return 0;
}