import csv
import matplotlib.pyplot as plt
import os

os.makedirs("output", exist_ok=True)

def read_units_vs_time(filename):
    """
    Read the CSV file containing units and execution times.
    """
    units = []
    time_discrete = []
    time_continuous = []

    with open(filename, 'r') as file:
        reader = csv.reader(file)
        for row in reader:
            units.append(int(row[0]))
            time_discrete.append(float(row[1]))
            time_continuous.append(float(row[2]))

    return units, time_discrete, time_continuous


def plot_units_vs_time(units, time_d, time_c):
    """
    Plot the execution time against the number of units.
    """
    plt.figure(figsize=(8, 6))
    plt.plot(units, time_d, marker='o', label='Discrete')
    plt.plot(units, time_c, marker='s', label='Continuous')
    plt.title('Execution Time vs Units')
    plt.xlabel('Units')
    plt.ylabel('Execution Time (s)')
    plt.xticks(units)
    plt.grid(True)
    plt.legend()
    plt.tight_layout()
    plt.savefig('output/time_vs_units.png', dpi=300)


def read_parallel_data(filename):
    """
    Read the CSV file containing thread counts and execution times.
    """
    threads = []
    time_discrete = []
    time_continuous = []

    with open(filename, 'r') as file:
        reader = csv.reader(file)
        for row in reader:
            threads.append(int(row[0]))
            time_discrete.append(float(row[1]))
            time_continuous.append(float(row[2]))
    
    return threads, time_discrete, time_continuous


def calculate_efficiency(threads, runtimes):
    """
    Calculate the parallel efficiency based on the number of threads and runtimes.
    """
    baseline = runtimes[0]  # baseline is the runtime with 1 thread
    efficiency = [(baseline / (rt * t)) * 100 for t, rt in zip(threads, runtimes)]
    return efficiency


def plot_performance(threads_fixed, time_d, time_c):
    """
    Plot the execution time and parallel efficiency against the number of threads.
    """
    eff_d = calculate_efficiency(threads_fixed, time_d)
    eff_c = calculate_efficiency(threads_fixed, time_c)

    fig, axs = plt.subplots(1, 2, figsize=(12, 5))

    # Execution time vs threads
    axs[0].plot(threads_fixed, time_d, marker='o', label='Discrete')
    axs[0].plot(threads_fixed, time_c, marker='s', label='Continuous')
    axs[0].set_title('Execution Time vs Threads')
    axs[0].set_xlabel('Threads')
    axs[0].set_ylabel('Execution Time (s)')
    axs[0].set_xticks(threads_fixed)
    axs[0].legend()
    axs[0].grid(True)

    # Parallel efficiency vs threads
    axs[1].plot(threads_fixed, eff_d, marker='o', label='Discrete')
    axs[1].plot(threads_fixed, eff_c, marker='s', label='Continuous')
    axs[1].set_title('Parallel Efficiency vs Threads')
    axs[1].set_xlabel('Threads')
    axs[1].set_ylabel('Parallel Efficiency (%)')
    axs[1].set_xticks(threads_fixed)
    axs[1].legend()
    axs[1].grid(True)

    plt.tight_layout()
    plt.savefig("output/parallel_efficiency.png", dpi=300)


if __name__ == '__main__':
    # plot units vs time
    filename_1 = 'time_vs_units.csv'
    units, time_d, time_c = read_units_vs_time(filename_1)
    plot_units_vs_time(units, time_d, time_c)

    # plot parallel efficiency 
    filename_2 = 'parallel_efficiency.csv'
    threads, time_d, time_c = read_parallel_data(filename_2)
    plot_performance(threads, time_d, time_c)
    