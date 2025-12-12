# Applied Data Science and Machine Learning Assessment

**Imperial College London**

**MSc Applied Computational Science and Engineering (ACSE)**

**Module:** Applied Data Science and Machine Learning (2024/25)

---

## Palusznium-Rush Ilmenite Optimiser

This repository contains the solution for the **Genetic Algorithms** group project (Part 3 of the Applied Data Science and Machine Learning module). The goal was to design and implement a computational tool to optimize the configuration of a mineral processing circuit for the extraction of two valuable minerals: **Palusznium (P)** and **Gormanium (G)**.

The project implements a **Genetic Algorithm (GA)** from scratch in **C++17** to explore the combinatorial search space of circuit topologies and continuous unit parameters.

### 1. Problem Overview

The challenge involves configuring a circuit of separation units to maximize profit.
-   **Inputs**: A feed containing Palusznium, Gormanium, and Waste.
-   **Components**: A set of identical separation units (e.g., flotation cells).
-   **Decisions**:
    -   **Topology**: How units are connected (feed, concentrate, tailings streams).
    -   **Sizing**: The volume/residence time of each unit (in continuous/hybrid modes).
-   **Objective**: Maximize Net Present Value (NPV) or a profit function based on recovery rates, purity grades, and operating costs (penalizing large volumes).

### 2. Implementation Details

The solution is a high-performance C++ application that combines circuit simulation with an evolutionary optimization strategy.

#### Core Components
-   **Circuit Simulator (`CCircuit`, `CUnit`)**:
    -   Models the steady-state mass balance of the system.
    -   Calculates recovery rates and grades for Palusznium and Gormanium.
    -   Evaluates the economic performance (Profit/Loss).
    -   Handles validity checks to ensure circuits are physically realizable (e.g., no self-loops, all units reachable).

-   **Genetic Algorithm (`Genetic_Algorithm`)**:
    -   **Representation**:
        -   *Discrete*: Integer vector representing stream destinations (circuit topology).
        -   *Continuous*: Real-valued vector representing unit volumes ($\tau$).
    -   **Operators**:
        -   *Selection*: Tournament selection to preserve diversity while applying selection pressure.
        -   *Crossover*: Single and multi-point crossover to combine efficient substructures.
        -   *Mutation*: Random stream redirection and volume scaling to explore new areas of the search space.
        -   *Elitism*: Preserves the best performing individuals across generations.
    -   **Parallelization**: Utilizes **OpenMP** to evaluate the fitness of the population in parallel, significantly speeding up convergence.

#### Optimization Modes
The solver supports three distinct modes of operation:
1.  **Discrete (`d`)**: Optimizes only the circuit topology (connections) with fixed unit volumes.
2.  **Continuous (`c`)**: Optimizes only the unit volumes for a fixed topology (primarily for sensitivity analysis).
3.  **Hybrid (`h`)**: Simultaneously optimizes both the circuit structure and the unit volumes. This was the primary solution strategy to find the global optimum.

### 3. Repository Structure

```
.
├── src/                    # C++ source code
│   ├── main.cpp            # Entry point and CLI
│   ├── Genetic_Algorithm.cpp # GA implementation
│   ├── CSimulator.cpp      # Simulation logic
│   ├── CCircuit.cpp        # Circuit graph and economic model
│   └── CUnit.cpp           # Unit operation physics
├── include/                # Header files
├── plotting/               # Python visualization tools
├── tests/                  # Unit tests (GoogleTest)
├── parameters.txt          # Runtime configuration
└── CMakeLists.txt          # Build configuration
```

### 4. Getting Started

#### Prerequisites
-   **C++ Compiler**: C++17 compliant (GCC, Clang, MSVC).
-   **CMake**: Version 3.12+.
-   **Python 3**: For visualization scripts (requires `matplotlib`, `graphviz`, `pandas`).
-   **OpenMP**: Recommended for parallel execution.

#### Build and Run
The project uses a `Makefile` wrapper for CMake.

1.  **Build the optimizer**:
    ```bash
    make build
    ```

2.  **Run the optimization**:
    ```bash
    make run
    ```
    This will compile the code (if needed), run the genetic algorithm, and execute the visualization scripts.

3.  **Run Tests**:
    ```bash
    ./run_tests.sh
    ```

### 5. Configuration

The algorithm's behavior can be tuned via `parameters.txt` without recompiling:

-   `mode`: `d` (discrete), `c` (continuous), or `h` (hybrid).
-   `num_units`: Number of separation units in the circuit.
-   `population_size`, `max_iterations`: GA hyperparameters.
-   `mutation_probability`, `crossover_probability`: Evolution rates.

### 6. Results & Visualization

Upon completion, the tool outputs:
1.  **Console Summary**: Best fitness, circuit vector, and detailed recovery/grade metrics.
2.  **CSV Log**: `plotting/circuit_results.csv` containing the history of the run.
3.  **Flowchart**: A visual representation of the best circuit found, generated in `plotting/output/flowchart.png`.

### 7. License

This project is licensed under the MIT License.

