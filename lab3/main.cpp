#include <iostream>
#include <syncstream>
#include <thread>
#include <latch>
#include <chrono>
#include <vector>
#include <array>

using namespace std;
using namespace std::chrono_literals;

constexpr int NT = 4;

latch La(5), Lb(6), Lc(4), Ld(8), Le(9), Lf(6), Lg(6), Lh(9), Li(7), Lj(6), Lk(4);

latch& L(char x)
{
    switch (x)
    {
    case 'a': return La;
    case 'b': return Lb;
    case 'c': return Lc;
    case 'd': return Ld;
    case 'e': return Le;
    case 'f': return Lf;
    case 'g': return Lg;
    case 'h': return Lh;
    case 'i': return Li;
    case 'j': return Lj;
    default:  return Lk;
    }
}

void wait_deps(char x)
{
    switch (x)
    {
    case 'c': case 'e': La.wait(); break;
    case 'g': case 'd': Lb.wait(); break;
    case 'h': case 'j': Lc.wait(); Lg.wait(); break;
    case 'i':           Le.wait(); Ld.wait(); break;
    case 'f':           Lj.wait(); break;
    case 'k':           Lh.wait(); Li.wait(); break;
    default: break;
    }
}

void f(char x, int i)
{
    this_thread::sleep_for(1s);
    osyncstream(cout) << "Set " << x << ": action " << i << " done.\n";
}

struct Task
{
    char set;
    int idx;
};

const array<vector<Task>, NT> schedule = { {
    { { 'a',1 },{ 'a',5 },{ 'b',4 },{ 'c',1 },{ 'g',1 },{ 'g',5 },{ 'h',1 },{ 'h',5 },{ 'h',9 },
     { 'j',4 },{ 'e',5 },{ 'e',9 },{ 'd',4 },{ 'd',8 },{ 'i',1 },{ 'i',5 },{ 'f',5 },{ 'k',3 } },
    { { 'a',2 },{ 'b',1 },{ 'b',5 },{ 'c',2 },{ 'g',2 },{ 'g',6 },{ 'h',2 },{ 'h',6 },{ 'j',1 },
     { 'j',5 },{ 'e',6 },{ 'd',1 },{ 'd',5 },{ 'f',1 },{ 'i',2 },{ 'i',6 },{ 'f',6 },{ 'k',4 } },
    { { 'a',3 },{ 'b',2 },{ 'b',6 },{ 'c',3 },{ 'g',3 },{ 'e',2 },{ 'h',3 },{ 'h',7 },{ 'j',2 },
     { 'j',6 },{ 'e',7 },{ 'd',2 },{ 'd',6 },{ 'f',2 },{ 'i',3 },{ 'i',7 },{ 'k',1 } },
    { { 'a',4 },{ 'b',3 },{ 'e',1 },{ 'c',4 },{ 'g',4 },{ 'e',3 },{ 'h',4 },{ 'h',8 },{ 'j',3 },
     { 'e',4 },{ 'e',8 },{ 'd',3 },{ 'd',7 },{ 'f',3 },{ 'i',4 },{ 'f',4 },{ 'k',2 } },
} };

void worker(int id)
{
    for (const Task& t : schedule[id])
    {
        wait_deps(t.set);
        f(t.set, t.idx);
        L(t.set).count_down();
    }
}

int main()
{
    osyncstream(cout) << "Computation started.\n";
    auto start = chrono::steady_clock::now();
    {
        vector<jthread> threads;
        for (int id = 0; id < NT; ++id)
            threads.emplace_back(worker, id);
    }
    chrono::duration<double> dur = chrono::steady_clock::now() - start;
    osyncstream(cout) << "Computation finished. Computation time is "
        << dur.count() << " seconds.\n";
    return 0;
}