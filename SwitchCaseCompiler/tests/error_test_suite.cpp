// ERROR TEST SUITE - Testing all error conditions
// Each section should produce specific error types

// ============================================================================
// LEXICAL ERRORS
// ============================================================================

// Test 1: Invalid characters
#include <iostream>
int main() {
    int x@ = 5;  // Invalid character @
    switch (x) {
        case 1: break;
    }
    return 0;
}

// Test 2: Unterminated string
#include <iostream>
int main() {
    string msg = "unterminated string;  // Missing closing quote
    switch (1) {
        case 1: break;
    }
    return 0;
}

// ============================================================================
// SYNTAX ERRORS
// ============================================================================

// Test 3: Missing semicolon
#include <iostream>
int main() {
    int x = 5  // Missing semicolon
    switch (x) {
        case 1: break;
    }
    return 0;
}

// Test 4: Missing break statement
#include <iostream>
int main() {
    int x = 5;
    switch (x) {
        case 1: x = 10;  // Missing break
        case 2: x = 20; break;
    }
    return 0;
}

// Test 5: Invalid switch syntax
#include <iostream>
int main() {
    int x = 5;
    switch x {  // Missing parentheses
        case 1: break;
    }
    return 0;
}

// ============================================================================
// SEMANTIC ERRORS
// ============================================================================

// Test 6: Undeclared variable
#include <iostream>
int main() {
    switch (undeclared_var) {  // Variable not declared
        case 1: break;
    }
    return 0;
}

// Test 7: Duplicate case values
#include <iostream>
int main() {
    int x = 5;
    switch (x) {
        case 1: x = 10; break;
        case 1: x = 20; break;  // Duplicate case
        default: x = 0; break;
    }
    return 0;
}

// Test 8: Assignment to undeclared variable
#include <iostream>
int main() {
    int x = 5;
    switch (x) {
        case 1: 
            undeclared = 10;  // Assigning to undeclared variable
            break;
    }
    return 0;
}

// Test 9: Using uninitialized variable
#include <iostream>
int main() {
    int x;  // Declared but not initialized
    int y = x + 5;  // Using uninitialized variable
    switch (y) {
        case 1: break;
    }
    return 0;
}

// Test 10: Type mismatch (if supported)
#include <iostream>
int main() {
    string text = "hello";
    switch (text) {  // Switch on string (may not be supported)
        case "hello": break;
    }
    return 0;
}