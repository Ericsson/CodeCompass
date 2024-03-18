#include <vector>

using namespace std;

// Flat

void flat_empty_inline() {}

void flat_empty()
{

}

int flat_regular(int a, int b)
{
    int aa = a * a;
    int ab2 = 2 * a * b;
    int bb = b * b;
    return aa + ab2 + bb;
}

// Single bump

void single_compound()
{
    {
        int z = 0;
    }
}

bool single_if_simple(bool a)
{
    if (a)
        a = false;
    return a;
}

int single_if_complex(int a)
{
    if (2ULL * a + static_cast<int>(1.0) < 12)
        a += 5 * (a << 3);
    return a;
}

int single_for_each(const vector<int>& v)
{
    int s = 0;
    for (int e : v)
        s += e;
    return s;
}

int single_for_loop(const vector<int>& v)
{
    int s = 0;
    for (size_t i = 0; i < v.size(); ++i)
        s += v[i];
    return s;
}

// Nested chain

void nested_chain_compound()
{
    {
        {
            {
                int z = 0;
            }
        }
    }
}

void nested_chain_if()
{
    if (0 == 0)
        if (true)
            if (false)
                flat_empty();
}

void nested_chain_compound_if()
{
    if (0 == 0)
    {
        if (true)
        {
            if (false)
            {
                flat_empty();
            }
        }
    }
}

void nested_chain_for(const vector<vector<vector<int>>>& array, int& sum)
{
    for (const auto& a0 : array)
        for (const auto& a1 : a0)
            for (const auto& a2 : a1)
                sum += a2;
}

void nested_chain_compound_for(const vector<vector<vector<int>>>& array, int& sum)
{
    for (const auto& a0 : array)
    {
        for (const auto& a1 : a0)
        {
            for (const auto& a2 : a1)
            {
                sum += a2;
            }
        }
    }
}

int nested_chain_mixed(int n)
{
    switch (n)
    {
    default:
        if (n < 0)
        {
            for (int i = 0; i < n; ++i)
            {
                try
                {
                    while (false)
                    {
                        do
                        {
                            return 0;
                        }
                        while (true);
                    }
                }
                catch (...) {}
            }
        }
    }
    return 0;
}

// Compare

void compare_level1()
{
    if (true)
    {
        long a = 7;
        long b = 3;
        long c = a * b;
    }
}

void compare_level2()
{
    if (true)
    {
        if (true)
        {
            long a = 7;
            long b = 3;
            long c = a * b;
        }
    }
}

void compare_level3()
{
    if (true)
    {
        if (true)
        {
            if (true)
            {
                long a = 7;
                long b = 3;
                long c = a * b;
            }
        }
    }
}

// Complex

int complex_two_levels(int i, int j)
{
    int m = i + 2 * j, n = -j;
    if (m + n < 42)
    {
        if (n < 0)
            m *= 8;
    }
    else
    {
        if (m > 0)
            n -= m * 2 - 1;
    }

    int s = 0;
    for (int k = m; k < n; ++k)
        s += (k % 6 == 0) ? k : n;

    return s;
}

int complex_three_levels_min(int a, int b)
{
    if (a > 0)
        if (b > 0)
            if (a + b < 100)
                return a + b;
    
    a ^= b;
    b ^= a;
    a ^= b;
    return 2 * a + b;
}

int complex_three_levels_max(int a, int b)
{
    if (a > 0)
        if (b > 0)
            if (a + b < 100)
            {
                a ^= b;
                b ^= a;
                a ^= b;
                return 2 * a + b;
            }
    
    return a + b;
}

// Nested lambda

void nested_lambda()
{
    auto a = []()
    {
        auto b = []()
        {
            auto c = []()
            {
                int z = 0;
            };
        };
    };
}

// Nested type

void nested_type()
{
    struct nested1
    {
        void method1()
        {
            struct nested2
            {
                void method2()
                {
                    struct nested3
                    {
                        void method3()
                        {
                            int z = 0;
                        }
                    };
                }
            };
        }
    };
}
