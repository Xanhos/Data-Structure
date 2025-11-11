//
// Created by y.grallan on 10/11/2025.
// Copyright (c) 2025 Yann Grallan All rights reserved.
//

#pragma once
#include <utility>
#include <memory>

template<typename Ty_, typename PointType_>
concept IsAPoint = requires(Ty_ a)
{
    a.x;
    std::same_as<std::remove_cvref_t<decltype(a.x)>, PointType_>;

    a.y;
    std::same_as<std::remove_cvref_t<decltype(a.y)>, PointType_>;
};

template<typename Ty_, typename Data_ = void>
struct Point
{
    Ty_ x;
    Ty_ y;
    std::unique_ptr<Data_> data;

    Point() = default;

    template<typename D = Data_,typename T = Ty_>
    requires (!std::is_same_v<std::remove_cvref_t<D>, Point>)
    explicit Point(D&& data_ ,T&& x_ = 0, T&& y_ = 0) noexcept
        : x(std::forward<T>(x_)), y(std::forward<T>(y_)), data(std::make_unique<Data_>(std::forward<Data_>(data_))) {}

    void swap(Point& first, Point& second) noexcept
    {
        using std::swap;
        swap(first.x, second.x);
        swap(first.y, second.y);
        swap(first.data, second.data);
    }

    Point(const Point& other) : x(other.x), y(other.y)
    {
        if (other.data)
        {
            data = std::make_unique<Data_>(*other.data);
        }
    }

    Point& operator=(Point other)
    {
        swap(*this, other);
        return *this;
    }

};



template<typename Ty_>
struct Point<Ty_, void> {
    Ty_ x;
    Ty_ y;

    template<typename T = Ty_>
    explicit Point(T&& x_ = 0, T&& y_ = 0) noexcept
        : x(std::forward<T>(x_)), y(std::forward<T>(y_)) {}

    void swap(Point& first, Point& second) noexcept
    {
        using std::swap;
        swap(first.x, second.x);
        swap(first.y, second.y);
    }
};



template<typename Ty_>
struct Rectangle {
    Ty_ x;
    Ty_ y;
    Ty_ width;
    Ty_ height;

    explicit Rectangle(Ty_ x = 0, Ty_ y = 0, Ty_ w = 0, Ty_ h = 0) noexcept
        : x(x), y(y), width(w), height(h) {}

    template<IsAPoint<Ty_> Point>
    bool contains(const Point& p) const {
        return (p.x >= x - width && p.x <= x + width &&
                p.y >= y - height && p.y <= y + height);
    }

    void swap(Rectangle& first, Rectangle& second) noexcept
    {
        using std::swap;
        swap(first.x, second.x);
        swap(first.y, second.y);
        swap(first.width, second.width);
        swap(first.height, second.height);
    }
};


template<typename Ty_, typename Data_ = void,size_t QuadLimits_ = 16ull>
class QuadTree
{
    using UsedPoint = Point<Ty_,Data_>;

    Rectangle<Ty_> boundary = Rectangle<Ty_>{0,0,1,1};
    std::vector<UsedPoint> points;

    std::unique_ptr<QuadTree> northWest;
    std::unique_ptr<QuadTree> northEast;
    std::unique_ptr<QuadTree> southWest;
    std::unique_ptr<QuadTree> southEast;



    template<typename Func>
    auto do_for_all_children(Func&& func) const -> decltype(func(northWest))
    {
        using ReturnValue = std::invoke_result_t<Func, decltype(northWest)>;
        if constexpr (std::is_same_v<ReturnValue, void>)
        {
            if (northWest) func(northWest);
            if (northEast) func(northEast);
            if (southWest) func(southWest);
            if (southEast) func(southEast);
            return;
        }
        else if constexpr (std::is_same_v<ReturnValue, bool>)
        {
            if (northWest && northEast && southWest && southEast)
                return  func(southEast) && func(southWest) && func(northEast) && func(northWest);
            return false;
        }
        else
        {
            ReturnValue value = 0;
            if (northWest) value += func(northWest);
            if (northEast) value += func(northEast);
            if (southWest) value += func(southWest);
            if (southEast) value += func(southEast);
            return value;
        }
    }

    template<typename Func>
    auto do_for_all_children(Func&& func) -> decltype(func(northWest))
    {
        using ReturnValue = std::invoke_result_t<Func, decltype(northWest)>;
        if constexpr (std::is_same_v<ReturnValue, void>)
        {
            if (northWest) func(northWest);
            if (northEast) func(northEast);
            if (southWest) func(southWest);
            if (southEast) func(southEast);
            return;
        }
        else if constexpr (std::is_same_v<ReturnValue, bool>)
        {
            if (northWest && northEast && southWest && southEast)
                return  func(southEast) && func(southWest) && func(northEast) && func(northWest);
            return false;
        }
        else
        {
            ReturnValue value = 0;
            if (northWest) value += func(northWest);
            if (northEast) value += func(northEast);
            if (southWest) value += func(southWest);
            if (southEast) value += func(southEast);
            return value;
        }
    }

    [[nodiscard]] bool is_divided() const noexcept
    {
        return do_for_all_children([](const std::unique_ptr<QuadTree>& child){return child != nullptr;});
    }

    void divide()
    {
        Ty_ x(boundary.x), y(boundary.y), w(boundary.width / 2), h(boundary.height / 2);
        northWest = std::make_unique<QuadTree>(Rectangle<Ty_>(x, y, w, h));
        northEast = std::make_unique<QuadTree>(Rectangle<Ty_>(x + w, y, w, h));
        southWest = std::make_unique<QuadTree>(Rectangle<Ty_>(x, y + h, w, h));
        southEast = std::make_unique<QuadTree>(Rectangle<Ty_>(x + w, y + h, w, h));

        for (auto& point: points)
        {
            insert(std::move(point));
        }
        points.clear();
    }


public:

    QuadTree() = default;

    explicit QuadTree(const Rectangle<Ty_>& rectangle) noexcept  : boundary(rectangle){}

    QuadTree(const QuadTree& other) : boundary(other.boundary)
    {
        for (const auto & point: other.points)
        {
            insert(UsedPoint(point));
        }
    }

    QuadTree& operator=(QuadTree other)
    {
        swap(*this, other);
        return *this;
    }

    void swap(QuadTree& first, QuadTree& second) noexcept
    {
        using std::swap;
        swap(first.boundary, second.boundary);
        swap(first.points, second.points);
        swap(first.northWest, second.northWest);
        swap(first.northEast, second.northEast);
        swap(first.southEast, second.southEast);
        swap(first.southWest, second.southWest);
    }

    bool insert(UsedPoint&& point)
    {
        if (!boundary.contains(point))
        {
            return false;
        }


        if (size() >= QuadLimits_ && !is_divided())
        {
            divide();
        }


        if (!is_divided())
        {
            points.push_back(std::move(point));
            return true;
        }




        if (northWest && northWest->contains(point))
        {
            return northWest->insert(std::move(point));
        }
        if (northEast && northEast->contains(point))
        {
            return northEast->insert(std::move(point));
        }
        if (southWest && southWest->contains(point))
        {
            return southWest->insert(std::move(point));
        }
        if (southEast && southEast->contains(point))
        {
            return southEast->insert(std::move(point));
        }

        return false;

    }

    [[nodiscard]] size_t size() const noexcept
    {
        size_t result = points.size();

        result += do_for_all_children([](const std::unique_ptr<QuadTree>& child){return child->size();});

        return result;

    }

    template<IsAPoint<Ty_> Point>
    [[nodiscard]] bool contains(const Point& point) const noexcept
    {
        return boundary.contains(point);
    }

    template<IsAPoint<Ty_> Point>
    [[nodiscard]] std::optional<std::reference_wrapper<UsedPoint>> get_at(const Point& point) noexcept
    {
        if (!contains(point))
        {
            return std::nullopt;
        }

        for (auto & owned_point: points)
        {
            if (point.x == owned_point.x && point.y == owned_point.y)
            {
                return owned_point;
            }
        }

        if (auto return_point = northWest->get_at(point)) return return_point;
        if (auto return_point = northEast->get_at(point)) return return_point;
        if (auto return_point = southWest->get_at(point)) return return_point;
        if (auto return_point = southEast->get_at(point)) return return_point;

        return std::nullopt;
    }

};