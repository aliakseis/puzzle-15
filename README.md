# puzzle-15
Optimal implementation of the Sliding 15 Puzzle game with Iterative Deepening A* AI Solver using Template Metaprogramming

Qt and Console versions

![00076-617612536](https://github.com/aliakseis/puzzle-15/assets/11851670/c0e1d559-88ce-446d-af33-08add9ea103b)

# Optimal 15-Puzzle Solver

## Overview

This project implements a highly optimized solver for the classic **15-puzzle** (4×4 sliding puzzle).

The solver searches for an **optimal solution** (minimum number of moves) using an iterative deepening strategy combined with:

* Manhattan distance guidance
* Linear conflict detection
* Corner and edge conflict analysis
* Parity normalization
* Aggressive branch pruning
* Compile-time generated move dispatch

The implementation is heavily optimized for performance and minimizes runtime overhead through extensive use of C++ templates.

---

## Puzzle Representation

The puzzle consists of a 4×4 grid:

```text
+-----> Y
|
|
|
V
X
```

Each tile is represented by an integer:

```cpp
using Cell = unsigned int;
```

The empty space is represented by:

```cpp
0
```

Board state is stored as:

```cpp
Cell m_cells[DIMENSION][DIMENSION];
```

along with the current blank position:

```cpp
int m_emptyX;
int m_emptyY;
```

---

## Solvability Check

Before attempting a search, the solver validates the input configuration.

### Duplicate Detection

A bit mask tracks all previously seen tiles:

```cpp
bitMask |= 1 << value;
```

Duplicate tiles immediately invalidate the puzzle.

### Parity Calculation

The solver computes puzzle parity using inversion counting:

```cpp
parity += GetBitsNumber(bits);
```

For even-width boards (such as the 15-puzzle), the blank row contributes to parity:

```cpp
parity += i / DIMENSION;
```

This determines whether the puzzle belongs to the solvable state space.

---

## Manhattan Distance Guidance

A key heuristic is based on Manhattan distance.

The helper:

```cpp
GetManhattanChange(...)
```

computes whether moving a tile:

* decreases distance to its goal
* increases distance
* leaves it unchanged

Instead of recomputing Manhattan distance for the whole board, the solver evaluates only the moved tile.

This provides an extremely cheap heuristic update.

---

## Goal State Detection

The goal state is verified using compile-time recursion:

```cpp
FinalPosition<X,Y>::Is(...)
```

The compiler generates the entire verification chain.

This avoids loops and allows aggressive optimization.

---

## Search Strategy

The algorithm performs iterative deepening:

```cpp
while (...)
{
    ++nDerivation;
}
```

The search first attempts:

```cpp
edge = true
```

which only follows moves that strictly improve the board.

If no solution exists at that depth, the allowed derivation budget is increased.

This behavior closely resembles:

* IDA*
* Depth-first branch-and-bound
* Cost-bounded heuristic search

---

## Compile-Time Move Generation

Moves are generated through template dispatch:

```cpp
DispatchStep<X,Y,edge>
```

and

```cpp
DistributeStep(...)
```

Possible directions are:

* Up
* Down
* Left
* Right

The implementation avoids runtime switch statements and dynamically generates move trees during compilation.

---

## Move Encoding

Moves are stored compactly as:

```cpp
(emptyXOffset & 3) + (emptyYOffset & 2)
```

Each move occupies a single byte.

The solution path is reconstructed backward during recursion and reversed at completion.

---

## Linear Conflict Heuristic

The solver extends Manhattan distance using the well-known **Linear Conflict** heuristic.

### Horizontal Conflicts

```cpp
HasNoHorizontalLinearConflict<row>()
```

detects tiles that:

1. belong in the same target row
2. appear in reversed order

Such configurations require additional moves beyond Manhattan distance.

### Vertical Conflicts

```cpp
HasNoVerticalLinearConflict<col>()
```

performs the same test for columns.

These tests provide stronger pruning than Manhattan distance alone.

---

## Specialized Conflict Detection

Several optimized conflict detectors are implemented.

### Doublet Conflicts

```cpp
DoubletHasNoHorizontalLinearConflict()
DoubletHasNoVerticalLinearConflict()
```

Checks interactions between two important tiles.

### Triplet Conflicts

```cpp
TripletHasNoHorizontalLinearConflict()
TripletHasNoVerticalLinearConflict()
```

Checks three-tile ordering constraints.

These are tailored specifically for the 15-puzzle and eliminate many unproductive branches.

---

## Corner Analysis

The solver contains dedicated corner logic:

```cpp
HasCornerTiles()
```

This detects situations where corner-adjacent tiles block each other.

Special handling of corners significantly improves search efficiency because corner errors are expensive to repair later.

---

## Last-Move Recognition

```cpp
HasLastMoves()
```

Identifies positions that have entered the final solving phase.

Once the board reaches this stage, different pruning rules become applicable.

This allows the search to become more aggressive near the solution.

---

## Multi-Tile Conflict Evaluation

```cpp
MultiTilesTest(...)
```

Performs a sophisticated aggregate conflict analysis.

The routine combines:

* corner constraints
* linear conflicts
* doublet conflicts
* triplet conflicts
* final-stage tile placement checks

The result is used as a lower-bound estimate.

Branches exceeding the remaining derivation budget are discarded immediately.

---

## Single-Conflict Fast Pass

```cpp
HasSingleConflictPass()
```

Provides a very fast pruning path for low-derivation states.

It verifies:

* corner conditions
* horizontal conflicts
* vertical conflicts
* first-row constraints

without invoking the full conflict analysis.

This significantly reduces search overhead near optimal solutions.

---

## Symmetry Normalization

The solver supports both:

* top-left blank convention
* bottom-right blank convention

Internally, all puzzles are transformed into a common representation:

```cpp
Solver(bool bTopLeftBlank)
```

This removes the need for duplicate solving logic.

After the solution is found, move directions are transformed back into the original coordinate system.

---

## Performance Characteristics

The solver achieves high performance through:

### Template Metaprogramming

Most move-generation logic is resolved at compile time.

### Incremental Heuristics

Manhattan updates are computed only for moved tiles.

### Aggressive Pruning

Conflict analysis removes large portions of the search tree.

### No Dynamic Allocation During Search

The board is stored directly in fixed-size arrays.

### Cache-Friendly Design

All critical structures fit into small memory footprints.

---

## Algorithm Summary

1. Validate puzzle configuration.
2. Compute parity.
3. Normalize board orientation.
4. Perform iterative deepening search.
5. Use Manhattan-distance changes for move ordering.
6. Apply linear-conflict pruning.
7. Apply corner and multi-tile conflict pruning.
8. Continue increasing derivation depth until a solution is found.
9. Reconstruct and return the optimal move sequence.

---

## Features

* Optimal solution generation
* Full solvability validation
* Manhattan distance heuristic
* Linear conflict heuristic
* Corner conflict detection
* Multi-tile pruning
* Compile-time move dispatch
* Iterative deepening search
* Symmetry normalization
* Low memory consumption
* High-performance C++ implementation

---

## Typical Use

```cpp
int length = Solve(board, resultMoves);
```

Where:

* `board` contains the 16 tile values (`0` = blank)
* `resultMoves` receives the optimal move sequence
* return value is the number of moves

---

## Complexity

Worst-case complexity remains exponential, as expected for optimal 15-puzzle solving.

However, the implemented pruning techniques reduce the practical search space by several orders of magnitude compared to naive depth-first or breadth-first approaches.

The solver is designed specifically for efficient optimal solving of the classical 15-puzzle.
