#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>

using namespace std;

// Mutex for printing and shared memory protection
mutex print_mutex;
mutex shared_mutex;

// Shared variable
atomic<long long> shared_atomic(0);
long long shared_no_protect = 0;

// Function to multiply part of matrix
void multiplyElement(const vector<vector<int>>& A, const vector<vector<int>>& B, vector<vector<int>>& C, int row, int col, int m) {
    int sum = 0;
    for (int i = 0; i < m; ++i) {
        sum += A[row][i] * B[i][col];
    }
    C[row][col] = sum;

    lock_guard<mutex> lock(print_mutex);
    cout << "[" << row << "," << col << "]=" << sum << endl;
}

// Shared memory increment without protection
void increment_no_protect(int times) {
    for (int i = 0; i < times; ++i) {
        shared_no_protect++;
    }
}

// Shared memory increment with mutex
void increment_with_mutex(int times) {
    for (int i = 0; i < times; ++i) {
        lock_guard<mutex> lock(shared_mutex);
        shared_no_protect++;
    }
}

// Shared memory increment atomically
void increment_atomic(int times) {
    for (int i = 0; i < times; ++i) {
        shared_atomic++;
    }
}

int main() {
    const int n = 4, m = 4, k = 4;
    vector<vector<int>> A(n, vector<int>(m));
    vector<vector<int>> B(m, vector<int>(k));
    vector<vector<int>> C(n, vector<int>(k, 0));

    // Fill A and B
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < m; ++j)
            A[i][j] = rand() % 10;
    for (int i = 0; i < m; ++i)
        for (int j = 0; j < k; ++j)
            B[i][j] = rand() % 10;

    // Parallel matrix multiplication
    vector<thread> threads;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < k; ++j) {
            threads.emplace_back(multiplyElement, cref(A), cref(B), ref(C), i, j, m);
        }
    }
    for (auto& t : threads) t.join();

    // Experiment with shared variable (race condition)
    int times = 10000000; // reduce for testing
    thread t1(increment_no_protect, times);
    thread t2(increment_no_protect, times);
    t1.join();
    t2.join();
    cout << "Without protection, shared_no_protect = " << shared_no_protect << " (expected: " << times * 2 << ")\n";

    shared_no_protect = 0;
    thread t3(increment_with_mutex, times);
    thread t4(increment_with_mutex, times);
    t3.join();
    t4.join();
    cout << "With mutex, shared_no_protect = " << shared_no_protect << "\n";

    shared_atomic = 0;
    auto start = chrono::high_resolution_clock::now();
    thread t5(increment_atomic, times);
    thread t6(increment_atomic, times);
    t5.join();
    t6.join();
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;
    cout << "With atomic, shared_atomic = " << shared_atomic << ", time = " << duration.count() << "s\n";

    return 0;
}
