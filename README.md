# KessLang

## The language I am making after my cat!


### KessLang is a customizable language.
#### Don't like the reserved keywords/syntax? You can change that in the config easily!

### Comments.

```
!! This is a basic comment.
```

### Variables.

```
?var = "SomeVar";               !! A normal string variable!
?var1 = 0x5;                    !! A hex variable!
?&CONSTVAR = "DoNotChangeMe!";  !! A constant string variable.
?CONSTVAR = "ThisWontWork!";    !! This line will raise an error.
```

### Print statements.

```
print "Hello, World!";    
print ?someVar;           !! Prints a variable.
print 0x5;
```

### String index stuff.

```
?str = "Hello!";
print ?str[0];    !! This should print 'H'.
```


### Including files from the KessLang standard library.

```
stdinc ("stdinfo")
```

### Including multiple files from KessLang standard library.

```
stdinc (
    "stdinfo",
    "stdkess"
)
```
