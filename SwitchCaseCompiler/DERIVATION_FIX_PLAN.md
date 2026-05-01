# Derivation Generator Fix Plan

## Current Issues
1. The derivation is reconstructed from a parse tree built during recursive descent parsing
2. The parse tree construction order doesn't necessarily follow rightmost derivation order
3. Need to ensure strict rightmost expansion at every step

## Solution Approach

The current `performRightmostDerivation()` function already has the right structure:
- It maintains a mutable sentential form as a vector of symbols
- It scans right-to-left to find the rightmost non-terminal
- It validates the rightmost invariant
- It records full sentential forms at each step

The key issue is that the parse tree needs to be constructed in a way that supports rightmost derivation traversal.

## Implementation Strategy

We need to ensure that when we traverse the parse tree to generate the derivation:
1. We always expand the rightmost non-terminal first
2. For recursive productions (like case_list → case_clause case_list), we expand the rightmost occurrence first
3. We fully derive nested structures before moving to earlier non-terminals

The current implementation already does this correctly! The issue might be in how the parse tree is being built or traversed.

## Verification

The current code already:
- ✅ Maintains full sentential forms
- ✅ Scans right-to-left for rightmost NT
- ✅ Validates no NT exists to the right
- ✅ Records previousSententialForm and sententialForm
- ✅ Applies one production per step

The implementation looks correct. The user might be seeing issues because:
1. The parse tree structure doesn't match their expected grammar
2. The grammar rules in buildParseTree() need adjustment
3. Need to verify the actual output matches expectations

## Next Steps
1. Review the buildParseTree() implementation
2. Ensure grammar productions match the expected CFG
3. Test with sample input to verify output format
