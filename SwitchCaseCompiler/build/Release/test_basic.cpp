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