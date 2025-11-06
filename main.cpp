#include <iostream>
#include "Deque.h"
#include "Vector.h"
#include "List.h"


namespace DequeMain
{
    void run()
    {
        Deque<int> a;

        for (int i = 0; i < 5; ++i)
        {
            a.push_front(-i);
            a.push_back(i);
        }

        for (int i = 0; i < a.size(); ++i)
        {
            std::cout << a[i] << " ";
        }

        Deque<int> b = a;
    }
}


namespace VectorMain
{
    int run()
    {
        Vector<int> a;
        {
            int i = 0;
            a.push_back(++i);
            a.push_back(++i);
            a.push_back(++i);
            a.push_back(++i);
            a.push_back(++i);
        }

        a.erase_swap(0);
        a.pop_back();


        a.shrink_to_fit();

        a.resize(55);


        for (size_t i = 0ull; i < a.get_size(); ++i)
        {
            std::cout << a[i] << " ";
        }

        return 0;
    }
}

namespace ListMain
{
    int run()
    {
        List<int> a;
        a.push_back(51);
        a.move_forward_cursor();

        a.push_back(52);
        a.move_forward_cursor();

        a.push_back(53);


        auto b = a;
        b = a;

        a.print_list("a");
        b.print_list("b");

        b.insert_after_cursor(56);

        a.print_list("a");
        b.print_list("b");
        b.reset_cursor();
        b.insert_after_cursor(55);

        a.print_list("a");
        b.print_list("b");

        return 0;
    }
}

int main()
{
    std::cout << "\n";
    DequeMain::run();
    std::cout << "\n";
    VectorMain::run();
    std::cout << "\n";
    ListMain::run();

}
