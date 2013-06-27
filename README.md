## SQL Parser

This can parse a reasonably large subset of SQL. I'm using lex and yacc as front-ends. Currently, it just checks for correctness, but I plan to have it compile to relational algebra. Compile with

```
> make
```

See some example tests with

```
> make test
```

Parse an arbitrary SQL file with

```
> ./parsesql.h <filename>
```