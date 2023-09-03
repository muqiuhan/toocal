<div align="center">

# WiscoDB

*WiscKey Key-Value storage engine implemented by OCaml*

![OCaml5.0](https://img.shields.io/badge/OCaml5.0.0-%23EC6813)


![](https://github.com/muqiuhan/wiscodb/workflows/Linux/badge.svg)
![](https://github.com/muqiuhan/wiscodb/workflows/Windows/badge.svg)
![](https://github.com/muqiuhan/wiscodb/workflows/MacOS/badge.svg)

</div>

## Introduction
WiscKey is a persistent LSM-tree-based key-value store with a performance-oriented data layout that separates keys from values to minimize I/O amplifi- cation. The design of WiscKey is highly SSD optimized, leveraging both the sequential and random performance characteristics of the device.

- Original Paper: https://www.usenix.org/system/files/conference/fast16/fast16-papers-lu.pdf

## Usage
> ...

## Dependencies
- [Alcotest: A lightweight and colourful test framework](https://github.com/mirage/alcotest)
- [Core: Jane Street Capital's standard library overlay](https://github.com/janestreet/core)
- [Core_unix: Unix-specific portions of Core](https://github.com/janestreet/core_unix)

## Reference
- [WiscKey: Key-value database based on the WiscKey paper](https://github.com/adambcomer/WiscKey)
- [WiscKey: Separating Keys from Values in SSD-conscious Storage](https://www.usenix.org/system/files/conference/fast16/fast16-papers-lu.pdf)
- [The Log-Structured Merge-Tree (LSM-Tree)](https://www.cs.umb.edu/~poneil/lsmtree.pdf)

## [LICENSE](./LICENSE)
Copyright 2022 Muqiu Han

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.