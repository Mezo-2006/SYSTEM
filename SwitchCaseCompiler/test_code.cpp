#include <iostream>
using namespace std;

int main() {
    int category;
    int  item;

    cout << "Select Category:\n";
    cout << "1. Drinks\n2. Food\n";
    cin >> category;

    switch(category) {

        case 1: // Drinks
            cout << "Select Drink:\n1. Coffee\n2. Juice\n";
            cin >> item;

            switch(item) {
                case 1:
                    cout << "You chose Coffee\n";
                    break;
                case 2:
                    cout << "You chose Juice\n";
                    break;
                default:
                    cout << "Invalid drink choice\n";
            }
            break;

        case 2: // Food
            cout << "Select Food:\n1. Burger\n2. Pizza\n";
            cin >> item;

            switch(item) {
                case 1:
                    cout << "You chose Burger\n";
                    break;
                case 2:
                    cout << "You chose Pizza\n";
                    break;
                default:
                    cout << "Invalid food choice\n";
            }
            break;

        default:
            cout << "Invalid category\n";
    }

    return 0;
}
