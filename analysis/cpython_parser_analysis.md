# CPython Parser Architecture Analysis

## Overview

CPython uses a **PEG (Parsing Expression Grammar)** parser since Python 3.9, replacing the older LL(1) parser. The parser is generated from a grammar file (`Grammar/python.gram`) using the **pegen** parser generator.

## Key Architectural Differences

### 1. Parser Generation vs Hand-Written

| Aspect | CPython | cpp_python |
|--------|---------|------------|
| Parser Type | Generated PEG parser | Hand-written recursive descent |
| Grammar File | `Grammar/python.gram` | Embedded in code |
| Memoization | Selective (with `memo` marker) | None |
| Left Recursion | Supported via special handling | Not supported |
| Backtracking | Full PEG backtracking | Limited |

### 2. Expression Precedence (CPython Grammar)

```
expression → disjunction ('if' disjunction 'else' expression)? | lambdef
disjunction → conjunction ('or' conjunction)*
conjunction → inversion ('and' inversion)*
inversion → 'not' inversion | comparison
comparison → bitwise_or (compare_op bitwise_or)*
bitwise_or → bitwise_xor ('|' bitwise_xor)*
bitwise_xor → bitwise_and ('^' bitwise_and)*
bitwise_and → shift_expr ('&' shift_expr)*
shift_expr → sum (('<<'|'>>') sum)*
sum → term (('+'|'-') term)*
term → factor (('*'|'/'|'//'|'%'|'@') factor)*
factor → ('+'|'-'|'~') factor | power
power → await_primary ('**' factor)?
primary → atom (trailer)*
atom → NAME | NUMBER | STRING | '(' ... ')' | '[' ... ']' | '{' ... '}'
```

### 3. Key CPython Grammar Features

#### Lookahead Operators
- `&e` - Positive lookahead (succeed if e parses, don't consume)
- `!e` - Negative lookahead (fail if e parses, don't consume)
- `~` - Cut operator (commit to current alternative)
- `&&e` - Eager parse (no backtracking)

#### Grammar Actions
CPython uses inline actions to directly generate AST nodes:
```
sum[expr_ty]:
    | a=sum '+' b=term { _PyAST_BinOp(a, Add, b, EXTRA) }
    | a=sum '-' b=term { _PyAST_BinOp(a, Sub, b, EXTRA) }
    | term
```

### 4. Statement Rules

#### Simple Statements
```
simple_stmt (memo):
    | assignment
    | &"type" type_alias
    | e=star_expressions { _PyAST_Expr(e, EXTRA) }
    | &'return' return_stmt
    | &('import' | 'from') import_stmt
    | &'raise' raise_stmt
    | &'pass' pass_stmt
    | &'del' del_stmt
    | &'yield' yield_stmt
    | &'assert' assert_stmt
    | &'break' break_stmt
    | &'continue' continue_stmt
    | &'global' global_stmt
    | &'nonlocal' nonlocal_stmt
```

#### Compound Statements
```
compound_stmt:
    | &('def' | '@' | 'async') function_def
    | &'if' if_stmt
    | &('class' | '@') class_def
    | &('with' | 'async') with_stmt
    | &('for' | 'async') for_stmt
    | &'try' try_stmt
    | &'while' while_stmt
    | match_stmt
```

### 5. Tokenization

CPython separates tokenization from parsing:
- Tokenizer: `Parser/tokenizer.c`
- Tokens defined in: `Grammar/Tokens`
- Special handling for: indentation, encoding, async/await, f-strings

## Recommendations for cpp_python

### Performance Optimizations

1. **Implement Memoization**
   - Add a memoization cache for expensive rules
   - Use selective memoization (not all rules need it)
   - Key rules to memoize: `expression`, `disjunction`, `conjunction`, `inversion`, `factor`

2. **Use Lookahead for Disambiguation**
   - Add positive lookahead before expensive alternatives
   - Example: `&'if'` before `if_stmt` to avoid unnecessary parsing

3. **Implement Cut Operator**
   - Once a unique prefix is matched, commit to that alternative
   - Prevents unnecessary backtracking

### Template-Based Architecture

1. **Parser Combinators as Templates**
   ```cpp
   template<typename... Parsers>
   struct Sequence { ... };
   
   template<typename... Parsers>
   struct Choice { ... };
   
   template<typename Parser>
   struct Optional { ... };
   
   template<typename Parser>
   struct ZeroOrMore { ... };
   ```

2. **Compile-Time Grammar Definition**
   - Use constexpr for grammar rules
   - Template metaprogramming for parser generation

3. **Type-Safe AST Construction**
   - Use templates to ensure correct AST node types
   - Compile-time validation of grammar rules

### Missing Features to Add

1. **Pattern Matching (match/case)** - Python 3.10+
2. **Type Parameters** - Python 3.12+
3. **Exception Groups (try*/except*)** - Python 3.11+
4. **Positional-only parameters** - Python 3.8+
5. **Assignment expressions (:=)** - Python 3.8+
6. **F-string improvements** - Various versions
