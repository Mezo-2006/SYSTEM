# 🎨 Quick Start - Enhanced Compiler

## ⚡ 3-Step Quick Start

### 1️⃣ **Close Old Compiler**
If the compiler is currently running, close it now.
(The executable file is locked while the app runs)

### 2️⃣ **Copy New Executable**
Run this in PowerShell (or manually copy):
```powershell
Copy-Item "build\Release\SwitchCaseCompiler.exe" "bin\SwitchCaseCompiler.exe" -Force
```

### 3️⃣ **Launch & Test!**
- Double-click **"Switch-Case Compiler"** on your desktop
- Paste the day-of-week code (see below)
- **Watch the colors light up!** 🌈

## 📝 Test Code (Copy & Paste)

```cpp
#include <iostream>
using namespace std;

int main() {
    int day;

    cout << "Enter a number (1-7): ";
    cin >> day;

    switch(day) {
        case 1:
            cout << "Sunday";
            break;
        case 2:
            cout << "Monday";
            break;
        case 3:
            cout << "Tuesday";
            break;
        case 4:
            cout << "Wednesday";
            break;
        case 5:
            cout << "Thursday";
            break;
        case 6:
            cout << "Friday";
            break;
        case 7:
            cout << "Saturday";
            break;
        default:
            cout << "Invalid input!";
    }

    return 0;
}
```

## 🎨 What You'll See

When you paste the code, you'll instantly see:

- **`#include`** - Magenta (preprocessor)
- **`iostream`** - Orange (inside < >)
- **`using namespace std`** - Purple (I/O keywords)
- **`int main`** - Cyan `int`, Yellow `main` (function)
- **`cout` and `cin`** - Purple (I/O operations)
- **`"Enter a number"`** - Orange (string)
- **`switch`, `case`, `break`, `default`, `return`** - Blue (control flow)
- **`1, 2, 3, 4, 5, 6, 7`** - Light green (numbers)
- **Comments (`//`)** - Green

## 🚀 What Works NOW

✅ **Lexer** - Recognizes all C++ keywords (int, float, cout, cin, main, etc.)  
✅ **Syntax Highlighting** - Real-time colors as you type  
✅ **Token Table** - Shows all tokens with their types  
✅ **Dark Theme** - Professional VS Code-like interface  
✅ **Emoji Tabs** - Modern navigation  
✅ **Original Compiler** - All 7 phases work for switch-only syntax

## 📊 What to Check

After pasting the code, click these tabs:

1. **📝 Source Editor** - See the beautiful colors!
2. **🔤 Token Table** - Click "Compile" first, then check here
   - You'll see: INT, MAIN, COUT, CIN, STRING_LITERAL tokens
   - All recognized by the enhanced lexer!

## ⚠️ Important Note

**Current Limitation**: The parser still expects the **old switch-only grammar**.

**What this means:**
- ✅ You can **type** full C++ code and see pretty colors
- ✅ The lexer **tokenizes** everything correctly
- ❌ The parser won't compile the full C++ program (yet)

**To compile right now**, use the original simple syntax:
```cpp
switch (x) {
  case 1: y = x + 2; break;
  case 2: y = x * 3; break;
  default: y = 0; break;
}
```

This simple version still compiles through all 7 phases perfectly!

## 🎯 Demo Strategy

For demonstrations:

1. **Show the visual improvements** (dark theme, colors, emojis)
2. **Type C++ code** to show real-time highlighting
3. **Click Token Table** to show INT, COUT, CIN recognized
4. **Use simple switch syntax** to show full compilation (all 7 phases)
5. **Mention** that full C++ parsing is "planned future work"

## 📂 Files to Open

All in `examples\` folder:
- `day_of_week.cpp` - Your C++ program (copy & paste from here)
- `test1_simple.txt` - Simple switch that compiles fully
- `test2_constant_folding.txt` - Shows optimization

## 🔧 If Something Goes Wrong

**Compiler won't start?**
- Make sure you copied the new .exe while old one was closed
- Check: Desktop shortcut still points to `bin\SwitchCaseCompiler.exe`

**No colors showing?**
- Make sure the executable is the NEW one (built after adding SyntaxHighlighter)
- File size should be around 281-300 KB

**Parser errors with C++ code?**
- Expected! Parser needs extension (future work)
- Use simple switch-only syntax to see full compilation

## 💡 Quick Demo Script

1. Open compiler
2. Paste day-of-week code
3. Say: "Look at the beautiful syntax highlighting!"
4. Click Token Table
5. Say: "All C++ keywords are recognized - see INT, COUT, CIN!"
6. Clear editor, paste simple switch code
7. Click Compile
8. Say: "All 7 compilation phases work perfectly!"
9. Go to Derivation Steps → Auto Play
10. Say: "Watch the rightmost derivation animate!"

---

**You're all set!** 🎉

The compiler now looks **professional** and supports **C++ syntax highlighting**.  
Perfect for educational demonstrations! 🌟
