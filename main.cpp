#include <iostream>
#include <string>
#include <format>

#include "Deque.h"
#include "Vector.h"
#include "List.h"
#include "HashMap.h"
#include "QuadTree.h"
#include "Colony.h"

namespace DequeMain
{
    void run()
    {
        Deque<int> b;

        {
            Deque<int> a;
            a.push_back(5);

            std::cout << a.front() << "\n";
            std::cout << a.back() << "\n";

            for (int i = 0; i < 5; ++i)
            {
                a.push_front(-i);
                a.push_back(i);
            }

            for (const int & element : a)
            {
                std::cout << element << " ";
            }
            std::cout << "\n";
            b = a;
        }


        for (const int & element : b)
        {
            std::cout << element << " ";
        }
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


        for (const auto&  a_ : a)
        {
            std::cout << a_  << " ";
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

namespace HashMapMain
{
    void run()
    {
        using namespace std::string_literals;
        std::cout << "\n\n----- Closed Hash map -----\n\n";

        ClosedHashMap::HashMap<std::string, int> closedHashMap;
        std::vector<std::string> keys;

        for (int i = 0; i < 30; ++i)
        {
            closedHashMap.insert(std::to_string(i),i * i );
            keys.push_back(std::to_string(i));
        }


        for (const auto& key: keys)
        {
            std::cout << closedHashMap.find(key) << ' ';
        }
        closedHashMap.rehash();
        std::cout << "\n";
        closedHashMap.remove("1"s);
        for (const auto& [key,value] : closedHashMap)
        {
            std::cout << "Key : " << key << ' '  << "Value : "<< value << '\n';
        }


        std::cout << "\n\n----- Open Hash map -----\n\n";


        OpenHashMap::HashMap<std::string, int> openHashMap;
        keys.clear();

        for (int i = 0; i < 30; ++i)
        {
            openHashMap.insert(std::to_string(i), i);
            keys.push_back(std::to_string(i));
        }
        std::cout << "\n";
        std::cout << "\n";
        for (const auto& key: keys)
        {
             std::cout << openHashMap.find(key) << ' ';
        }

        std::cout << "\n";
        std::cout << "\n";

        for (const auto& [key,value]: openHashMap)
        {
            std::cout << "Key : " << key << ' '  << "Value : "<< value << '\n';
        }
        openHashMap.remove("15"s);


    }
}

namespace QuadTreeMain
{
    struct Player
    {
        std::string name;
        int hp;
    };

    void run()
    {
        QuadTree<float, Player> quad{Rectangle<float>(0,0,100,100)};
        auto size = quad.size();

        constexpr int point_number = 100;
        for (int i = 0; i < point_number; ++i)
        {
            for (int x = 0; x < 17; ++x)
            {
                quad.insert(Point<float, Player>(Player{"Player " + std::to_string(i), i * 5},static_cast<float>(i),static_cast<float>(i)));
            }
        }

        std::cout << "Get all point \n\n";

        for (int i = 0; i < quad.size(); ++i)
        {
            if (const auto point =  quad.get_at(Point<float>(i,i)))
            {
                std::cout << std::format("Player name : {}, has : {} hp\n", point->get().data->name, point->get().data->hp);
            }
        }

        std::cout << "\n\nGet queried point \n\n";


        for (auto & queries_point: quad.queries_points(Rectangle<float>(25,25,50,50)))
        {
            std::cout << std::format("Player name : {}, has : {} hp\n", queries_point.get().data->name, queries_point.get().data->hp);
        }

        auto quad_b = quad;

    }
}

namespace ColonyMain
{
    void run()
    {
        Colony<int> colony;

        for (int i = 0; i < 50; ++i)
        {
            colony.insert_back(i);
        }

        colony.remove(5);


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
    std::cout << "\n";
    HashMapMain::run();
    std::cout << "\n";
    QuadTreeMain::run();
    std::cout << "\n";
    ColonyMain::run();



    return 0;

}
