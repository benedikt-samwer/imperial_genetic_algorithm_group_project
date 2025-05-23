# Palusznium‑Rush Ilmenite Optimiser

*A self‑contained C++17 + OpenMP toolkit that designs mineral‑processing circuits with a genetic‑algorithm core, plus Python helpers for visualisation.*

---

## 1  Problem in a nutshell

We must configure a circuit of identical separation units so that two valuable minerals—**palusznium (P)** and **gormanium (G)**—are recovered profitably while punishing waste entrainment and oversized equipment.
The design space (both topology **and** unit volumes) is combinatorial, so we use a **genetic algorithm** (GA) to search it.

Full background → *Problem Statement for Genetic Algorithms Project 2025.pdf*.

---

## 2  Repository layout

```
.
├── CMakeLists.txt            # top‑level build
├── include/                  # public headers used by src/
│   ├── CCircuit.h            # circuit class
│   ├── CUnit.h               # single separation unit
│   ├── CSimulator.h          # helper for testing/plotting
│   ├── Genetic_Algorithm.h   # GA interface
│   └── …
├── src/                      # implementation (.cpp files)
│   ├── CCircuit.cpp
│   ├── CUnit.cpp
│   ├── CSimulator.cpp
│   ├── Genetic_Algorithm.cpp
│   └── main.cpp              # CLI entry point
├── docs/                     # PDF + Markdown design docs
├── plotting/                 # **generated** on first run
│   ├── circuit_results.csv   # GA output (append‑only)
│   ├── plot.py               # matplotlib helper → png/pdf
│   └── cplot.cpp             # minimal C++ visualiser (optional)
├── tests/                    # GoogleTest unit‑tests & CTest driver
├── rng_examples/             # tiny demos comparing RNG quality
├── hooks/                    # pre‑commit & install helper for git hooks
└── parameters.txt            # runtime GA settings (human‑readable)
```

> **Tip:** The project builds out‑of‑tree; `build/` is ignored by git.

---

## 3  Build & run

### 3.1 Prerequisites

| Tool                       | Minimum        | Tested on                   |
| -------------------------- | -------------- | --------------------------- |
| **CMake**                  | 3.12           | 3.27                        |
| **C++ compiler**           | C++17 + OpenMP | GCC 9, Clang 14, MSVC 19.36 |
| **Python (visualisation)** | 3.8            | 3.11                        |

Install Python deps with `pip install -r requirements.txt` (matplotlib + pandas).

### 3.2  Build

The project ships with a convenience **Makefile** that wraps the CMake build under the hood.

```bash
make build      # configure + compile (Release) to build/
make clean      # remove the build directory
```

The first invocation generates `build/bin/Optimizer` (and unit‑test binaries).

### 3.3  Run

```bash
make run        # executes the optimiser, plots, appends CSV
```

Need more cores? → `OMP_NUM_THREADS=8 make run`

Want a different GA mode? Edit **`parameters.txt`** (`mode = d | h | c`)
– remember: *continuous‑only (`c`) is dev/test only; it will not yield profitable solutions.*

```bash
OMP_NUM_THREADS=8 make run        # force 8 OpenMP threads
MODE=d            make run        # override mode in parameters.txt (d/h/c)
```

Internally this calls the optimiser and then `plotting/main.py -f` to create `output/flowchart.png`.

> Rendering requires **Graphviz**, **Pillow** and **pandas**; install once with `pip install -r plotting/requirements.txt`.

| Mode | Search dimension treated as variable | Primary purpose |
|------|--------------------------------------|-----------------|
| `d`  | **connections** only                 | explore profitable flowsheets |
| `c`  | **β-volumes** only (connections frozen) – **DEV-ONLY**. ⚠️ This mode does *not* find profitable solutions; it is kept for unit-testing kinetics & cost functions |
| `h`  | alternates *d* ↔ *c*                 | end-to-end optimisation |

---

## 4  `parameters.txt` — full reference

Every run-time option is in `parameters.txt` so you can tune the optimiser without recompiling.

| Key                         | Type / Range      | Default      | Description                                                       |
| --------------------------- | ----------------- | ------------ | ----------------------------------------------------------------- |
| **num\_units**              | integer ≥ 2       | 6            | Number of separation units *(vector length = 2·n + 1)*            |
| **mode**                    | `d` \| `c` \| `h` | `h`          | GA operating mode: discrete, continuous (**dev‑only**), or hybrid |
| **max\_iterations**         | integer           | 100          | GA generations per optimisation call                              |
| **population\_size**        | integer           | 600          | Individuals per generation                                        |
| **elite\_count**            | integer           | 2            | Best genomes copied unchanged each generation                     |
| **tournament\_size**        | integer           | 3            | k‑way tournament selection pressure                               |
| **crossover\_probability**  | 0–1               | 0.9          | Chance two parents cross                                          |
| **mutation\_probability**   | 0–1               | 0.08         | Per‑gene mutation chance (all modes)                              |
| **mutation\_step\_size**    | integer ≥ 1       | 3            | Max ± step for discrete "creep"                                   |
| **use\_inversion**          | bool              | true         | Enable contiguous slice reversal (discrete)                       |
| **inversion\_probability**  | 0–1               | 0.2          | Chance *per child* that inversion occurs                          |
| **use\_scaling\_mutation**  | bool              | true         | Enable multiplicative tweak for β genes                           |
| **scaling\_mutation\_prob** | 0–1               | 0.3          | Probability a child gets scaling mutation                         |
| **scaling\_mutation\_min**  | >0                | 0.7          | Lower bound of scaling factor                                     |
| **scaling\_mutation\_max**  | >1                | 1.3          | Upper bound of scaling factor                                     |
| **convergence\_threshold**  | real ≥ 0          | 0.1          | Δfitness below which a change is deemed “no improvement”          |
| **stall\_generations**      | integer           | 50           | Stop if no improvement for this many generations                  |
| **verbose**                 | bool              | true         | Print progress every 10 generations                               |
| **log\_results**            | bool              | false        | Append CSV copy of every generation to `log_file`                 |
| **log\_file**               | filename          | `ga_run.log` | Only used if `log_results = true`                                 |
| **random\_seed**            | integer \| −1     | 42           | ≥ 0 → deterministic RNG, −1 → random seed                         |

> **Tip:** change a value, save the file, re‑run `./run.sh` — no rebuild is needed.

---

## 5  Interpreting output  Analysing results

### 5  Analysing results

#### 5.1 CSV format

Every optimiser run appends to `plotting/circuit_results.csv`:

```
[int vector …] , [Σ concentration flow/unit , Σ tailings flow/unit , …]
```

#### 5.2 Auto‑generated flowchart

Running `./run.sh` (or `python plotting/main.py -f`) produces `output/flowchart.png`:

* directed graph of the circuit with blue/red edge labels (concentrate / tails flow)
* beneath it: a table showing the integer vector laid out by unit

Open the PNG directly, or embed it in documentation.

---

## 6  Developers’ guide

* **Unit kinetics** – edit `src/CUnit.cpp` (`CUnit::process`).
* **Economic model** – tune coefficients in `src/CCircuit.cpp` (`get_economic_value`).
* **GA extensions** – new operators live in `src/Genetic_Algorithm.cpp` (see the three `optimize` overloads).
* **Unit tests** – add cases in `tests/`; they build and run automatically with `./build.sh test` or `ctest`.
* **Git hooks** – `hooks/install.sh` installs clang‑format, static‑analysis and pre‑commit checks.

---

## 7  Licence & citation

The code is released under the **MIT Licence** (see `LICENSE`).

---

*Happy circuit hunting!* 🚀
