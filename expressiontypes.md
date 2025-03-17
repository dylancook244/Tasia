# Comprehensive List of Programming Control Structures and Expressions

## Loop Constructs

• **Traditional For Loops**
  - C-style: `for (init; condition; update) {}`
  - FORTRAN: `DO i=1,n,...`
  - BASIC: `FOR i = 1 TO n [STEP x]`
  - Pascal: `for i := 1 to n do`
  - Ada: `for I in 1..N loop`
  - PL/I: `DO I = 1 TO N BY 1;`

• **For-each / Enhanced For Loops**
  - Java: `for (Type item : collection) {}`
  - Python: `for item in iterable:`
  - JavaScript: `for (let item of array) {}`
  - PHP: `foreach ($array as $item) {}`
  - Ruby: `for item in collection do`
  - C#: `foreach (var item in collection) {}`
  - Kotlin: `for (item in collection) {}`
  - Swift: `for item in collection {}`
  - Groovy: `for (item in collection) {}`
  - Rust: `for item in collection {}`
  - Dart: `for (var item in collection) {}`

• **Iterator Loops**
  - JavaScript: `array.forEach(function(item) {})`
  - Ruby: `collection.each { |item| }`
  - Scala: `collection.foreach(item => {})`
  - Clojure: `(doseq [item coll] ...)`
  - Haskell: `map function list`
  - Elixir: `Enum.each(collection, fn item -> end)`

• **While Loops**
  - `while (condition) {}`
  - Python/Ruby: `while condition:`
  - Shell: `while [ condition ]; do`
  - VHDL: `while condition loop`
  - Lua: `while condition do`
  - COBOL: `PERFORM UNTIL condition`

• **Do-While / Repeat-Until**
  - C-family: `do { } while (condition);`
  - Pascal: `repeat ... until condition;`
  - Ruby: `begin ... end while condition`
  - Ada: `loop ... exit when condition; end loop;`
  - COBOL: `PERFORM ... UNTIL condition`
  - PL/I: `DO UNTIL (condition);`
  - Lua: `repeat ... until condition`

• **Until Loops**
  - Ruby: `until condition do ... end`
  - Bash: `until condition; do ... done`
  - Perl: `until (condition) {}`

• **Infinite Loops**
  - C-style: `for (;;) {}`
  - Most languages: `while (true) {}`
  - Ruby: `loop do ... end`
  - Rust: `loop {}`
  - Go: `for {}`

• **List Comprehensions**
  - Python: `[expr for item in iterable if condition]`
  - Haskell: `[f x | x <- xs, condition]`
  - Julia: `[expression for item in collection if condition]`
  - Scala: `for (i <- collection if condition) yield expr`
  - Groovy: `collection.findAll{condition}.collect{expression}`

• **Generator Expressions**
  - Python: `(expr for item in iterable if condition)`
  - JavaScript: `function* gen() { yield expr }`

• **Range-based Loops**
  - Python: `for i in range(start, stop, step):`
  - Ruby: `for i in start..end`
  - Kotlin: `for (i in start..end step s)`
  - R: `for (i in seq(from, to, by))` 

• **Numeric Loops**
  - APL: ` expr ⍳ count`
  - J: `for_i. i.n do. ... end.`
  - MATLAB: `for i = vector`
  - Julia: `for i = start:step:stop ... end`

• **Label-based Loops**
  - Java: `label: for (...) {}`
  - JavaScript: `label: for (...) {}`
  - Go: `Label: for {...}`
  - Ada: `Loop_Name: for ... loop`

• **Parallel Loops**
  - OpenMP: `#pragma omp parallel for`
  - HPF: `FORALL (i=1:n) A(i) = B(i)`
  - CUDA: `for (int i = threadIdx.x; i < n; i += blockDim.x)`
  - MPI: `for (i=rank; i<n; i+=size) ...`
  - Chapel: `forall i in 1..n do`
  - X10: `for (i in 1..n) async {}`

## Conditional Structures

• **If-Then**
  - C-family: `if (condition) {}`
  - Python/Ruby: `if condition:`
  - BASIC: `IF condition THEN`
  - Pascal: `if condition then`
  - COBOL: `IF condition THEN`
  - Lisp: `(if condition then-expr)`

• **If-Then-Else**
  - C-family: `if (condition) {} else {}`
  - Pascal: `if condition then ... else ...`
  - Lisp: `(if condition then-expr else-expr)`
  - F#: `if condition then ... else ...`
  - Erlang: `if condition -> ...; true -> ... end`

• **If-ElseIf-Else**
  - C-family: `if (condition1) {} else if (condition2) {} else {}`
  - Python: `if condition1: ... elif condition2: ... else:`
  - Ruby: `if condition1 ... elsif condition2 ... else ... end`
  - Swift: `if condition1 {} else if condition2 {} else {}`

• **Nested If**
  - All languages support nesting of if statements

• **Switch/Case**
  - C-family: `switch (var) { case val1: ... break; default: ... }`
  - Pascal: `case var of val1: ...; else ... end;`
  - Ruby: `case var when val1 ... else ... end`
  - Bash: `case var in pattern1) ... ;; esac`
  - Erlang: `case Var of Pattern1 -> ...; _ -> ... end`

• **Pattern Matching**
  - Haskell: `case expr of pattern1 -> ...; pattern2 -> ...`
  - Scala: `expr match { case pattern1 => ...; case _ => ... }`
  - Rust: `match expr { pattern1 => ..., _ => ... }`
  - F#: `match expr with | pattern1 -> ... | _ -> ...`
  - OCaml: `match expr with pattern1 -> ... | _ -> ...`
  - Elixir: `case expr do pattern1 -> ...; _ -> ... end`

• **Guard Clauses**
  - Swift: `guard condition else { return }`
  - Haskell: `f x | condition = ...`
  - Elixir: `def fun(x) when condition do ... end`

• **Ternary Operator**
  - C-family: `condition ? true_expr : false_expr`
  - Python: `true_expr if condition else false_expr`
  - Ruby: `condition ? true_expr : false_expr`
  - Scala: `if (condition) trueExpr else falseExpr`

• **Null Coalescing**
  - C#: `var ?? default`
  - Swift/Kotlin: `var ?: default`
  - PHP: `$var ?? $default`
  - JavaScript: `var ?? default` or older `var || default`

• **Optional Chaining**
  - JavaScript: `obj?.prop?.method?.()`
  - Swift: `object?.property?.method?()`
  - Kotlin: `object?.property?.method?()`
  - TypeScript: `obj?.prop?.method?.()`

• **When Expression**
  - Kotlin: `when (x) { condition -> ...; else -> ... }`
  - Scala: `x match { case condition => ...; case _ => ... }`

• **Cond**
  - Lisp: `(cond (test1 result1) (test2 result2) (t default))`
  - Clojure: `(cond test1 result1 :else default)`

## Other Control Structures

• **Exception Handling**
  - Try-Catch: `try { ... } catch (Exception e) { ... }`
  - Python: `try: ... except Exception as e: ... finally: ...`
  - Ruby: `begin ... rescue Exception => e ... ensure ... end`
  - JavaScript: `try { ... } catch (e) { ... } finally { ... }`
  - Java: `try { ... } catch (Exception e) { ... } finally { ... }`
  - Go: `defer func() { if r := recover(); r != nil { ... } }()`
  - Ada: `begin ... exception when ... => ... end;`

• **With Statement / Context Managers**
  - Python: `with resource as r: ...`
  - Pascal: `with object do ...`
  - JavaScript: `with (obj) { ... }` (deprecated)
  - C#: `using (resource) { ... }`
  - F#: `use resource = ... in ...`

• **Goto Statements**
  - C-family: `goto label;`
  - BASIC: `GOTO line`
  - FORTRAN: `GO TO n`
  - Assembly: `JMP addr`
  - COBOL: `GO TO paragraph-name`

• **Break and Continue**
  - Break: `break;` or `break label;`
  - Continue: `continue;` or `continue label;`
  - Python: `break` and `continue`
  - Ruby: `break` and `next`
  - Perl: `last` and `next`

• **Return Statement**
  - C-family: `return [value];`
  - Python/Ruby: `return value`
  - Lisp: `(return value)`
  - Rust: `expr` (last expression is return value)
  - Scala: Last expression is return value

• **Yield Statements**
  - Python: `yield value`
  - Ruby: `yield value`
  - JavaScript: `yield value` or `yield* generator`
  - C#: `yield return value` or `yield break`

• **Defer Statements**
  - Go: `defer func()`
  - Swift: `defer { ... }`
  - Zig: `defer { ... }`

• **Asynchronous Programming**
  - JavaScript: `async function() { await promise; }`
  - C#: `async Task Method() { await task; }`
  - Python: `async def func(): await coroutine`
  - Kotlin: `suspend fun func() { coroutine.await() }`
  - Rust: `async fn func() -> Future { .await }`

• **Parallel Processing**
  - Java: `CompletableFuture.runAsync(() -> {})`
  - C#: `Parallel.For(0, n, i => {})`
  - JavaScript: `Promise.all([...])`
  - Go: `go func() {}`
  - Rust: `spawn(|| {})`

• **Monadic Constructs**
  - Haskell: `do { x <- action; return expr }`
  - Scala: `for { x <- action } yield expr`
  - F#: `computation { let! x = action; return expr }`

• **Pipeline Operators**
  - F#: `value |> function1 |> function2`
  - Elixir: `value |> function1() |> function2()`
  - R: `value %>% function1() %>% function2()`
  - Julia: `value |> function1 |> function2`
  - Hack: `value |> function1() |> function2()`

• **Pattern Guards**
  - Haskell: `case expr of pattern | guard -> ...`
  - Erlang: `fun(X) when guard -> ...`
  - Elixir: `def fun(x) when guard -> ...`

• **Query Expressions**
  - C# LINQ: `from x in collection where condition select expr`
  - F#: `query { for x in xs do where condition select expr }`
  - JavaScript: `collection.filter().map()`

• **Coroutines / Generators**
  - Python: `def gen(): yield value`
  - JavaScript: `function* gen() { yield value }`
  - C#: `IEnumerable<T> Method() { yield return value; }`
  - Kotlin: `suspend fun method() { ... }`
  - Lua: `coroutine.create(function() end)`

• **Continuations**
  - Scheme: `call-with-current-continuation`
  - Ruby: `callcc { |cont| ... }`
  - Standard ML: `callcc`

• **Locks and Synchronized Blocks**
  - Java: `synchronized (obj) { ... }`
  - C#: `lock (obj) { ... }`
  - Python: `with lock: ...`
  - Go: `mutex.Lock(); defer mutex.Unlock()`

• **Block Expressions**
  - Ruby: `do |params| ... end`
  - Rust: `{ let x = 1; x + 2 }` (returns value)
  - Kotlin: `run { ... }` or `let { it -> ... }`
  - Swift: `{ ... }()` (immediately invoked)

• **Macros**
  - Lisp: `(defmacro name (args) body)`
  - Rust: `macro_rules! name { (pattern) => { ... } }`
  - Elixir: `defmacro name(args) do ... end`
  - C/C++: `#define MACRO(args) replacement`

• **Aspect-Oriented Programming**
  - AspectJ: `@Before("pointcut") void method() {}`
  - Spring: `@Around("execution(* *(..))")`

• **Contract Programming**
  - Eiffel: `require ... ensure ...`
  - D: `in { assert } out { assert } body {}`
  - Ada: `Pre => condition, Post => condition`

• **Pattern-Driven Control**
  - AWK: `pattern { action }`
  - Prolog: `head :- body.`
  - XSLT: `<xsl:template match="pattern">`

• **Reactive Programming**
  - Rx: `observable.filter().map().subscribe()`
  - Svelte: `$: computed = expression`
  - React: `useEffect(() => {}, [deps])`

• **Logic Programming**
  - Prolog: `condition1, condition2 -> action; alternative.`
  - Mercury: `if condition then ... else ... endif`
  - Datalog: `head :- literal1, literal2.`