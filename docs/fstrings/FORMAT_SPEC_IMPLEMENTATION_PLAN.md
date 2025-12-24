# Format Specifier Implementation Plan

## Based on CPython Alignment Analysis

This document provides a step-by-step implementation plan for adding format specifier support to the C++ f-string parser, based on the CPython alignment analysis.

---

## Phase 1: Add Format Spec State Tracking

### Step 1.1: Extend FStringState Structure

**File:** `src/parser/tokenizer.hpp`

**Current:**
```cpp
struct FStringState {
    char quote;
    int quote_size;
    bool raw;
    int curly_brace_depth;
};
```

**Add:**
```cpp
struct FStringState {
    char quote;
    int quote_size;
    bool raw;
    int curly_brace_depth;
    int curly_brace_expr_start_depth;  // Depth when expression started
    bool in_format_spec;                // Are we in format spec mode?
};
```

**Initialization:**
```cpp
// In read_fstring_start(), after pushing state:
state.curly_brace_depth = 0;
state.curly_brace_expr_start_depth = -1;  // -1 means no active expression
state.in_format_spec = false;
```

### Step 1.2: Track Expression Start Depth

**File:** `src/parser/tokenizer.hpp`

**In next_token(), when encountering `{` in f-string:**

**Current:**
```cpp
if (c == '{') {
    if (/* is {{ */) {
        return read_fstring_middle();
    } else {
        state.curly_brace_depth++;
        position_++;
        column_++;
        return Token(TokenType::LBRACE, "{", line_, start_col);
    }
}
```

**Update to:**
```cpp
if (c == '{') {
    if (position_ + 1 < source_.length() && source_[position_ + 1] == '{') {
        // Literal {{ - handled in read_fstring_middle
        return read_fstring_middle();
    } else {
        // Start of expression
        if (state.curly_brace_depth == 0) {
            // This is the start of a new expression
            state.curly_brace_expr_start_depth = 0;
        }
        state.curly_brace_depth++;
        position_++;
        column_++;
        return Token(TokenType::LBRACE, "{", line_, start_col);
    }
}
```

---

## Phase 2: Handle Colon to Enter Format Spec Mode

### Step 2.1: Detect Colon at Expression Depth

**File:** `src/parser/tokenizer.hpp`

**In next_token(), after handling f-string context, before normal tokenization:**

**Add this check:**
```cpp
// Check for : at the expression start depth (format specifier)
if (!fstring_stack_.empty() && c == ':') {
    FStringState& state = fstring_stack_.back();
    
    // Only treat : as format spec if we're at the expression start depth
    if (state.curly_brace_depth > 0 && 
        state.curly_brace_depth - 1 == state.curly_brace_expr_start_depth) {
        // This is the format specifier colon
        state.in_format_spec = true;
        position_++;
        column_++;
        return Token(TokenType::COLON, ":", line_, start_col);
    }
}
```

**Explanation:**
- We check if we're at the same depth as when the expression started
- If so, this `:` is the format specifier delimiter
- Set `in_format_spec = true` and return the `:` token
- The parser will see this and call `parse_fstring_format_spec()`

---

## Phase 3: Tokenize Format Spec Text

### Step 3.1: Modify Format Spec Tokenization Logic

**File:** `src/parser/tokenizer.hpp`

**In next_token(), modify the f-string handling section:**

**Current:**
```cpp
// If we're inside an expression (depth > 0), tokenize normally
// Otherwise, read f-string middle (literal text)
if (state.curly_brace_depth > 0) {
    // We're inside an expression - fall through to normal tokenization
} else {
    return read_fstring_middle();
}
```

**Update to:**
```cpp
// Determine tokenization mode
if (state.curly_brace_depth > 0 && !state.in_format_spec) {
    // We're inside an expression - fall through to normal tokenization
} else {
    // We're in literal text or format spec mode
    return read_fstring_middle();
}
```

**Key insight:** When `in_format_spec == true`, we should read literal text (like `.2f`) as `FSTRING_MIDDLE`, not tokenize it as DOT, NUMBER, IDENTIFIER.

### Step 3.2: Handle `{` in Format Spec (Nested Expression)

**File:** `src/parser/tokenizer.hpp`

**In read_fstring_middle(), add special handling for `{` in format spec:**

**Add at the beginning of the while loop:**
```cpp
// If we're in format spec and encounter {, this is a nested expression
if (state.in_format_spec && !escaped && c == '{') {
    // Check if it's {{ (literal)
    if (position_ + 1 < source_.length() && source_[position_ + 1] == '{') {
        // Literal {{ - emit single {
        value += '{';
        position_ += 2;
        column_ += 2;
        continue;
    } else {
        // Nested expression in format spec
        // Emit what we have so far as FSTRING_MIDDLE
        // Then the caller will get { as LBRACE on next call
        break;
    }
}
```

### Step 3.3: Clear Format Spec Flag on `}` or `{`

**File:** `src/parser/tokenizer.hpp`

**In next_token(), when handling `{` in f-string:**

**Add:**
```cpp
if (c == '{') {
    if (/* is {{ */) {
        return read_fstring_middle();
    } else {
        // Start of expression
        if (state.in_format_spec) {
            // Nested expression in format spec
            state.in_format_spec = false;  // ⭐ Clear flag
        }
        if (state.curly_brace_depth == 0) {
            state.curly_brace_expr_start_depth = 0;
        }
        state.curly_brace_depth++;
        position_++;
        column_++;
        return Token(TokenType::LBRACE, "{", line_, start_col);
    }
}
```

**In next_token(), when handling `}` in f-string:**

**Add:**
```cpp
if (c == '}') {
    if (/* is }} */) {
        return read_fstring_middle();
    } else if (state.curly_brace_depth > 0) {
        state.curly_brace_depth--;
        if (state.curly_brace_depth == 0) {
            // End of top-level expression
            state.in_format_spec = false;  // ⭐ Clear flag
            state.curly_brace_expr_start_depth = -1;
        }
        position_++;
        column_++;
        return Token(TokenType::RBRACE, "}", line_, start_col);
    }
}
```

---

## Phase 4: Testing

### Test Case 1: Simple Format Spec
```python
f"{x:.2f}"
```

**Expected tokens:**
```
FSTRING_START("")
LBRACE
IDENTIFIER("x")
COLON              ← in_format_spec = true
FSTRING_MIDDLE(".2f")
RBRACE             ← in_format_spec = false
FSTRING_END
```

### Test Case 2: Nested Expression in Format Spec
```python
f"{x:{width}}"
```

**Expected tokens:**
```
FSTRING_START("")
LBRACE
IDENTIFIER("x")
COLON              ← in_format_spec = true
FSTRING_MIDDLE("")
LBRACE             ← in_format_spec = false, depth++
IDENTIFIER("width")
RBRACE             ← depth--
FSTRING_MIDDLE("")
RBRACE             ← in_format_spec = false
FSTRING_END
```

### Test Case 3: Complex Format Spec
```python
f"{x:>10.2f}"
```

**Expected tokens:**
```
FSTRING_START("")
LBRACE
IDENTIFIER("x")
COLON              ← in_format_spec = true
FSTRING_MIDDLE(">10.2f")
RBRACE             ← in_format_spec = false
FSTRING_END
```

### Test Case 4: Multiple Nested Expressions
```python
f"{x:{width}.{prec}}"
```

**Expected tokens:**
```
FSTRING_START("")
LBRACE
IDENTIFIER("x")
COLON              ← in_format_spec = true
FSTRING_MIDDLE("")
LBRACE             ← in_format_spec = false
IDENTIFIER("width")
RBRACE
FSTRING_MIDDLE(".")
LBRACE             ← in_format_spec = false (already false)
IDENTIFIER("prec")
RBRACE
FSTRING_MIDDLE("")
RBRACE
FSTRING_END
```

---

## Phase 5: Edge Cases

### Edge Case 1: Colon in Nested Expression
```python
f"{d[x:y]:.2f}"
```

**Expected behavior:**
- First `:` (in `x:y`) is inside brackets, depth > expr_start_depth, so it's a slice, not format spec
- Second `:` (before `.2f`) is at expr_start_depth, so it's format spec

**Solution:** Check `curly_brace_depth - 1 == curly_brace_expr_start_depth` to ensure we're at the right level.

### Edge Case 2: Colon in Dictionary Literal
```python
f"{{a: b}:.2f}"  # This is invalid Python, but we should handle gracefully
```

**Expected behavior:** The `{` and `}` are literal (escaped), so the `:` is just literal text.

### Edge Case 3: Empty Format Spec
```python
f"{x:}"
```

**Expected tokens:**
```
FSTRING_START("")
LBRACE
IDENTIFIER("x")
COLON
FSTRING_MIDDLE("")  ← Empty string
RBRACE
FSTRING_END
```

---

## Implementation Checklist

### Tokenizer Changes
- [ ] Add `curly_brace_expr_start_depth` to `FStringState`
- [ ] Add `in_format_spec` to `FStringState`
- [ ] Initialize new fields in `read_fstring_start()`
- [ ] Track expression start depth when `{` is encountered
- [ ] Detect `:` at expression depth and set `in_format_spec = true`
- [ ] Modify tokenization logic to read literal text when `in_format_spec == true`
- [ ] Handle `{` in format spec (nested expression)
- [ ] Clear `in_format_spec` when `{` or `}` is encountered
- [ ] Handle `}}` correctly in format spec

### Parser Changes
- [ ] Verify `parse_fstring_format_spec()` handles `FSTRING_MIDDLE` correctly
- [ ] Verify nested expressions in format specs work
- [ ] Add error handling for invalid format specs

### Testing
- [ ] Test simple format specs (`.2f`, `>10`, etc.)
- [ ] Test nested expressions in format specs
- [ ] Test multiple nested expressions
- [ ] Test edge cases (colon in slices, empty format spec)
- [ ] Test all existing f-string tests still pass

---

## Expected Outcome

After implementation:
- ✅ `f"{x:.2f}"` → Works
- ✅ `f"{x:>10}"` → Works
- ✅ `f"{x:{width}}"` → Works
- ✅ `f"{x:{width}.{prec}}"` → Works
- ✅ All basic f-string features still work

**Alignment with CPython: 95%+**
