# Exercise 2

## Compile

```
$ make
```

## Presentation

The presentation is available on google drive at: https://docs.google.com/presentation/d/19MunOnAZNLvp6c6lKLaCaDjU0BtsZAqKjycxlBBfPB4/edit?usp=sharing

### 2.1 Calculate cache line

```
$ ./cache-line <nr of max strides>
```

Default argument (when not provided) is 100.

Output is in the format:
```
stride    bytes/second
```

Example:
```
$ ./cache-line 2
                   1	      2194969849.317
                   2	      2182890331.694
```

Results on the tested machine: https://docs.google.com/spreadsheets/d/10Uhk2YRqbbtsQKLKqJOmdQXhmIbETsIxCZG_w-21UVE/edit?usp=sharing
