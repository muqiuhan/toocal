<div align="center">

<img src=".github/logo.png" height="150px">

# Toocal

*An minimalist persistent key/value store in OCaml*

</div>

## INTRODUCTION

<div align="center">

```
+----------+    +-------------------+    +---------+
| Database | -> | Data access layer | -> | SSD/HDD |
+----------+    +-------------------+    +---------+
```

Underlying data structure is B-Tree, and the page size is 4KB.

</div>

## REFERENCE
- [Bolt: An embedded key/value database for Go.](https://github.com/boltdb/bolt)
- [LibraDB is a simple, persistent key/value store written in pure Go for learning purposes.](https://github.com/amit-davidson/LibraDB)
- [Wikipedia: NoSQL](https://en.wikipedia.org/wiki/NoSQL)
- [Wikipedia: Hash Table](https://en.wikipedia.org/wiki/Hash_table)
- [Wikipedia: B+ Tree](https://en.wikipedia.org/wiki/B%2B_tree)
- [Wikipedia: B Tree](https://en.wikipedia.org/wiki/B-tree)

## LICENSE
The MIT License (MIT)

Copyright (c) 2022 Muqiu Han

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.