# GUI Professional Redesign — Implementation Guide

## 📍 WHERE TO FIND & EDIT FILES

### **File Locations:**
```
src/gui/
├── MainWindow.cpp          ← UPDATED with new stylesheet loader
├── MainWindow.h            ← No changes needed
├── modern_style.qss        ← NEW — professional stylesheet
├── DerivationViewer.cpp    ← No changes needed
├── DerivationViewer.h      ← No changes needed
├── ParseTreeView.cpp       ← No changes needed
├── ParseTreeView.h         ← No changes needed
├── SyntaxHighlighter.cpp   ← No changes needed
├── SyntaxHighlighter.h     ← No changes needed
├── ThreeColumnView.cpp     ← No changes needed
├── ThreeColumnView.h       ← No changes needed
├── OptimizationPanel.cpp   ← No changes needed
├── OptimizationPanel.h     ← No changes needed
└── resources/
    ├── resources.qrc       ← UPDATED to include stylesheet
    └── cpp_icon.svg        ← No changes needed
```

---

## 🎨 DESIGN SYSTEM APPLIED

### **Color Palette:**
| Element | Color | Usage |
|---------|-------|-------|
| Background Base | #0f1117 | Main window background |
| Surface | #1a1d27 | Panels, cards, tables |
| Elevated | #22263a | Hover states, active tabs |
| Overlay | #2c3150 | Modals, focus states |
| **Non-terminal** | #7c9ef5 | Grammar symbols (blue) |
| **Terminal** | #56d4a0 | Leaf nodes (teal) |
| **Rule Labels** | #f0a04b | R1–R46 labels (amber) |
| **Active/Focus** | #a78bfa | Buttons, active elements (purple) |
| **Success** | #4caf82 | Match, valid (green) |
| **Error** | #e05c6a | Mismatch, invalid (red) |

### **Typography:**
- **UI Font:** Segoe UI, Inter (system-ui fallback)
- **Code Font:** JetBrains Mono, Fira Code
- **Font Sizes:** 10px (small) → 11px (body) → 12px (headings)
- **Font Weight:** 400 (regular) / 500 (medium) / 600 (bold)

### **Spacing & Radius:**
- **Padding:** 6px–12px (buttons & inputs)
- **Border Radius:** 4px (small) → 6px (medium) → 8px (large)
- **Scrollbar Width:** 10px (sleek)

---

## ✅ WHAT'S RESTYLED

### Components Completely Redesigned:
1. **Tabs** — Modern bottom border indicator, semantic colors
2. **Text Editors** — Dark background, focused blue border, monospace font
3. **Buttons** — Purple accent, hover brightness effect
4. **Tables** — Alternating rows, subtle gridlines, clean headers
5. **Checkboxes** — Semantic colors (green when checked)
6. **Progress Bars** — Purple fill, rounded corners
7. **Scrollbars** — Thin, elegant, semi-transparent
8. **Tooltips** — Dark style with consistent colors
9. **All Labels** — Proper semantic colors (NT/Term/Rule/Error)

### Components Preserved (No Logic Changes):
- All parsing/derivation logic
- Data structures & algorithms
- Grammar rules R0–R46
- FIRST/FOLLOW computation
- Parse tree rendering logic
- Three-column trace view
- Optimization algorithm

---

## 🚀 HOW TO BUILD & RUN

### Step 1: Update CMakeLists.txt (if needed)
Add resource file to your Qt project:
```cmake
set(PROJECT_SOURCES
    # ... existing sources ...
    src/gui/resources/resources.qrc
)
```

### Step 2: Rebuild
```powershell
cd c:\Users\Mizo\OneDrive - Arab Academy for Science and Technology\Desktop\systems 2\SwitchCaseCompiler
cmake --build build --config Release
```

### Step 3: Run
```powershell
.\build\Release\SwitchCaseCompiler.exe
```

---

## 🎯 QUICK CUSTOMIZATION GUIDE

### Change Button Colors
Edit `modern_style.qss` line ~120:
```qss
QPushButton {
    background-color: #a78bfa;  /* Change this color */
    color: #ffffff;
}
```

### Change Primary Accent (NT color)
Find all `#7c9ef5` and replace. Or create CSS variable:
```qss
:root {
    --color-primary: #7c9ef5;
}
```

### Add Dark/Light Theme Toggle
Create two QSS files:
- `modern_style_dark.qss` (current)
- `modern_style_light.qss` (light theme)

Add toggle button to load them:
```cpp
void MainWindow::toggleTheme(bool isDark) {
    QFile file(isDark ? ":/styles/modern_style_dark.qss" : ":/styles/modern_style_light.qss");
    if (file.open(QFile::ReadOnly)) {
        qApp->setStyleSheet(file.readAll());
        file.close();
    }
}
```

---

## 📋 FILES TO MODIFY

### Already Updated:
- ✅ `MainWindow.cpp` — Now loads QSS stylesheet
- ✅ `modern_style.qss` — New professional stylesheet
- ✅ `resources.qrc` — Includes stylesheet resource

### Optional Enhancements:
- [ ] Add `modern_style_light.qss` for light theme
- [ ] Create theme toggle button
- [ ] Customize SyntaxHighlighter colors
- [ ] Add custom palette for semantic tokens

---

## 🔧 TROUBLESHOOTING

**QSS Not Loading?**
1. Check resources.qrc is included in CMakeLists.txt
2. Rebuild project (`cmake --build build --config Release`)
3. Verify path: `:/styles/modern_style.qss`

**Colors Look Wrong?**
1. Ensure Fusion style is loaded: `qApp->setStyle(QStyleFactory::create("Fusion"));`
2. Check color hex values in `.qss` file
3. Try disabling Windows accent colors (Settings > Personalization)

**Fonts Not Loading?**
1. Ensure system has "Segoe UI" or "Inter" installed
2. Fallback to system-ui: `font-family: 'Segoe UI', system-ui, sans-serif;`
3. For monospace: `font-family: 'JetBrains Mono', monospace;`

---

## 📚 REFERENCE LINKS

- **Qt Stylesheet Syntax:** https://doc.qt.io/qt-6/stylesheet-syntax.html
- **Color Palette Tool:** https://coolors.co
- **Modern CSS Design:** https://material.io/design

---

## 🎬 NEXT STEPS

1. **Build & Test:** Follow build instructions above
2. **Customize:** Edit colors in `modern_style.qss`
3. **Add Interactivity:** Create theme toggle or animation effects
4. **Polish:** Add custom icons, shadows, and transitions

Your GUI will now look **professional, modern, and semantic** while keeping all compiler logic intact!
