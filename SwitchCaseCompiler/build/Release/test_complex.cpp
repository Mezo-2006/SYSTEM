#include <iostream>
int main() {
    int a = 3;
    int b = 4;
    int result = (a + b) * (a - b);
    switch (result) {
        case -7: result = 100; break;
        case 5: result = 200; break;
        default: result = 999; break;
    }
    return 0;
}