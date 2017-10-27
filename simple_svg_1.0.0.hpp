
/*******************************************************************************
*  The "New BSD License" : http://www.opensource.org/licenses/bsd-license.php  *
********************************************************************************

Copyright (c) 2010, Mark Turney
Copyright (c) 2017, Andreas Gocht
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

******************************************************************************/

#ifndef SIMPLE_SVG_HPP
#define SIMPLE_SVG_HPP

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <iostream>

namespace svg
{
// Utility XML/String Functions.
template <typename T>
std::string attribute(std::string const &attribute_name, T const &value, std::string const &unit = "")
{
    std::stringstream ss;
    ss << attribute_name << "=\"" << value << unit << "\" ";
    return ss.str();
}
std::string elemStart(std::string const &element_name)
{
    return "\t<" + element_name + " ";
}
std::string elemEnd(std::string const &element_name)
{
    return "</" + element_name + ">\n";
}
std::string emptyElemEnd()
{
    return "/>\n";
}

// Quick optional return type.  This allows functions to return an invalid
//  value if no good return is possible.  The user checks for validity
//  before using the returned value.
template <typename T> class optional
{
public:
    optional<T>(T const &type) : valid(true), type(type)
    {
    }
    optional<T>() : valid(false), type(T())
    {
    }
    T *operator->()
    {
        // If we try to access an invalid value, an exception is thrown.
        if (!valid)
            throw std::exception();

        return &type;
    }
    // Test for validity.
    bool operator!() const
    {
        return !valid;
    }
    optional &operator=(optional other)
    {
        valid = other.valid;
        type = other.type;
        return *this;
    }

private:
    bool valid;
    T type;
};

struct Dimensions
{
    Dimensions() : width(0), height(0)
    {
    }

    Dimensions(double width, double height) : width(width), height(height)
    {
    }
    Dimensions(double combined) : width(combined), height(combined)
    {
    }

    Dimensions &operator=(Dimensions other)
    {
        width = other.width;
        height = other.height;
        return *this;
    }

    double width;
    double height;
};

struct Point
{
    Point() : x(0), y(0)
    {
    }
    Point(double x, double y) : x(x), y(y)
    {
    }
    Point &operator=(Point other)
    {
        x = other.x;
        y = other.y;
        return *this;
    }
    double x;
    double y;
};
optional<Point> getMinPoint(std::vector<Point> const &points)
{
    if (points.empty())
        return optional<Point>();

    Point min = points[0];
    for (unsigned i = 0; i < points.size(); ++i)
    {
        if (points[i].x < min.x)
            min.x = points[i].x;
        if (points[i].y < min.y)
            min.y = points[i].y;
    }
    return optional<Point>(min);
}
optional<Point> getMaxPoint(std::vector<Point> const &points)
{
    if (points.empty())
        return optional<Point>();

    Point max = points[0];
    for (unsigned i = 0; i < points.size(); ++i)
    {
        if (points[i].x > max.x)
            max.x = points[i].x;
        if (points[i].y > max.y)
            max.y = points[i].y;
    }
    return optional<Point>(max);
}

// Defines the dimensions, scale, origin, and origin offset of the document.
struct Layout
{
    enum Origin
    {
        TopLeft,
        BottomLeft,
        TopRight,
        BottomRight
    };

    Layout() : scale(1), origin(TopLeft)
    {
    }
    Layout(Dimensions const &dimensions, Origin origin) : dimensions(dimensions), scale(1), origin(origin)
    {
    }
    Layout(Dimensions const &dimensions, Origin origin, double scale, Point const &origin_offset)
        : dimensions(dimensions), scale(scale), origin(origin), origin_offset(origin_offset)
    {
    }

    Layout &operator=(Layout other)
    {
        dimensions = other.dimensions;
        scale = other.scale;
        origin = other.origin;
        origin_offset = other.origin_offset;
        return *this;
    }
    Dimensions dimensions;
    double scale;
    Origin origin;
    Point origin_offset;
};

// Convert coordinates in user space to SVG native space.
double translateX(double x, Layout const &layout)
{
    if (layout.origin == Layout::BottomRight || layout.origin == Layout::TopRight)
        return layout.dimensions.width - ((x + layout.origin_offset.x) * layout.scale);
    else
        return (layout.origin_offset.x + x) * layout.scale;
}

double translateY(double y, Layout const &layout)
{
    if (layout.origin == Layout::BottomLeft || layout.origin == Layout::BottomRight)
        return layout.dimensions.height - ((y + layout.origin_offset.y) * layout.scale);
    else
        return (layout.origin_offset.y + y) * layout.scale;
}
double translateScale(double dimension, Layout const &layout)
{
    return dimension * layout.scale;
}

class Serializeable
{
public:
    Serializeable()
    {
    }
    virtual ~Serializeable(){};
    virtual std::string toString(Layout const &layout) const = 0;
};

class Color : public Serializeable
{
public:
    enum Defaults
    {
        Transparent = -1,
        Aqua,
        Black,
        Blue,
        Brown,
        Cyan,
        Fuchsia,
        Green,
        Lime,
        Magenta,
        Orange,
        Purple,
        Red,
        Silver,
        White,
        Yellow
    };

    Color() : transparent(false), red(0), green(0), blue(0)
    {
    }
    Color(int r, int g, int b) : transparent(false), red(r), green(g), blue(b)
    {
    }
    Color(Defaults color) : transparent(false), red(0), green(0), blue(0)
    {
        switch (color)
        {
        case Aqua:
            assign(0, 255, 255);
            break;
        case Black:
            assign(0, 0, 0);
            break;
        case Blue:
            assign(0, 0, 255);
            break;
        case Brown:
            assign(165, 42, 42);
            break;
        case Cyan:
            assign(0, 255, 255);
            break;
        case Fuchsia:
            assign(255, 0, 255);
            break;
        case Green:
            assign(0, 128, 0);
            break;
        case Lime:
            assign(0, 255, 0);
            break;
        case Magenta:
            assign(255, 0, 255);
            break;
        case Orange:
            assign(255, 165, 0);
            break;
        case Purple:
            assign(128, 0, 128);
            break;
        case Red:
            assign(255, 0, 0);
            break;
        case Silver:
            assign(192, 192, 192);
            break;
        case White:
            assign(255, 255, 255);
            break;
        case Yellow:
            assign(255, 255, 0);
            break;
        default:
            transparent = true;
            break;
        }
    }
    virtual ~Color()
    {
    }
    std::string toString(Layout const &) const
    {
        std::stringstream ss;
        if (transparent)
            ss << "transparent";
        else
            ss << "rgb(" << red << "," << green << "," << blue << ")";
        return ss.str();
    }

    Color &operator=(Color other)
    {
        transparent = other.transparent;
        red = other.red;
        green = other.green;
        blue = other.blue;
        return *this;
    }

private:
    bool transparent;
    int red;
    int green;
    int blue;

    void assign(int r, int g, int b)
    {
        red = r;
        green = g;
        blue = b;
    }
};

class Fill : public Serializeable
{
public:
    Fill() = default;
    Fill(Color::Defaults color) : color(color)
    {
    }
    Fill(Color color) : color(color)
    {
    }
    std::string toString(Layout const &layout) const
    {
        std::stringstream ss;
        ss << attribute("fill", color.toString(layout));
        return ss.str();
    }

    Fill &operator=(Fill other)
    {
        color = other.color;
        return *this;
    }

private:
    Color color;
};

class Stroke : public Serializeable
{
public:
    Stroke() : width(-1)
    {
    }
    Stroke(double width, Color color = Color::Transparent) : width(width), color(color)
    {
    }
    std::string toString(Layout const &layout) const
    {
        // If stroke width is invalid.
        if (width < 0)
            return std::string();

        std::stringstream ss;
        ss << attribute("stroke-width", translateScale(width, layout)) << attribute("stroke", color.toString(layout));
        return ss.str();
    }

    Stroke &operator=(Stroke other)
    {
        color = other.color;
        width = other.width;
        return *this;
    }

private:
    double width;
    Color color;
};

class Font : public Serializeable
{
public:
    Font() : size(12), family("Verdana")
    {
    }
    Font(double size, std::string const &family) : size(size), family(family)
    {
    }
    std::string toString(Layout const &layout) const
    {
        std::stringstream ss;
        ss << attribute("font-size", translateScale(size, layout)) << attribute("font-family", family);
        return ss.str();
    }

    Font &operator=(Font other)
    {
        size = other.size;
        family = other.family;
        return *this;
    }

private:
    double size;
    std::string family;
};

class Shape : public Serializeable
{
public:
    Shape() = default;
    Shape(Fill const &fill, Stroke const &stroke) : fill(fill), stroke(stroke)
    {
    }
    virtual ~Shape()
    {
    }
    virtual std::string toString(Layout const &layout) const = 0;
    virtual void offset(Point const &offset) = 0;

protected:
    Fill fill;
    Stroke stroke;
};
template <typename T> std::string vectorToString(std::vector<T> collection, Layout const &layout)
{
    std::string combination_str;
    for (unsigned i = 0; i < collection.size(); ++i)
        combination_str += collection[i].toString(layout);

    return combination_str;
}

class Circle : public Shape
{
public:
    Circle() : Shape(), radius(0)
    {
    }
    Circle(Point const &center, double diameter, Fill const &fill, Stroke const &stroke)
        : Shape(fill, stroke), center(center), radius(diameter / 2)
    {
    }
    std::string toString(Layout const &layout) const
    {
        std::stringstream ss;
        ss << elemStart("circle") << attribute("cx", translateX(center.x, layout))
           << attribute("cy", translateY(center.y, layout)) << attribute("r", translateScale(radius, layout))
           << fill.toString(layout) << stroke.toString(layout) << emptyElemEnd();
        return ss.str();
    }
    void offset(Point const &offset)
    {
        center.x += offset.x;
        center.y += offset.y;
    }

    Circle &operator=(Circle other)
    {
        center = other.center;
        radius = other.radius;
        fill = other.fill;
        stroke = other.stroke;
        return *this;
    }

private:
    Point center;
    double radius;
};

class Elipse : public Shape
{
public:
    Elipse() : radius_width(1), radius_height(1)
    {
    }
    Elipse(Point const &center, double width, double height, Fill const &fill, Stroke const &stroke)
        : Shape(fill, stroke), center(center), radius_width(width / 2), radius_height(height / 2)
    {
    }
    std::string toString(Layout const &layout) const
    {
        std::stringstream ss;
        ss << elemStart("ellipse") << attribute("cx", translateX(center.x, layout))
           << attribute("cy", translateY(center.y, layout)) << attribute("rx", translateScale(radius_width, layout))
           << attribute("ry", translateScale(radius_height, layout)) << fill.toString(layout) << stroke.toString(layout)
           << emptyElemEnd();
        return ss.str();
    }
    void offset(Point const &offset)
    {
        center.x += offset.x;
        center.y += offset.y;
    }

    Elipse &operator=(Elipse other)
    {
        center = other.center;
        radius_width = other.radius_width;
        radius_height = other.radius_height;
        fill = other.fill;
        stroke = other.stroke;
        return *this;
    }

private:
    Point center;
    double radius_width;
    double radius_height;
};

class Rectangle : public Shape
{
public:
    Rectangle() : width(1), height(1)
    {
    }
    Rectangle(Point const &edge, double width, double height, Fill const &fill, Stroke const &stroke)
        : Shape(fill, stroke), edge(edge), width(width), height(height)
    {
    }
    std::string toString(Layout const &layout) const
    {
        std::stringstream ss;
        ss << elemStart("rect") << attribute("x", translateX(edge.x, layout))
           << attribute("y", translateY(edge.y, layout)) << attribute("width", translateScale(width, layout))
           << attribute("height", translateScale(height, layout)) << fill.toString(layout) << stroke.toString(layout)
           << emptyElemEnd();
        return ss.str();
    }
    void offset(Point const &offset)
    {
        edge.x += offset.x;
        edge.y += offset.y;
    }

    Rectangle &operator=(Rectangle other)
    {
        edge = other.edge;
        width = other.width;
        height = other.height;
        fill = other.fill;
        stroke = other.stroke;
        return *this;
    }

private:
    Point edge;
    double width;
    double height;
};

class Line : public Shape
{
public:
    Line() = default;
    Line(Point const &start_point, Point const &end_point, Stroke const &stroke)
        : Shape(Fill(), stroke), start_point(start_point), end_point(end_point)
    {
    }
    std::string toString(Layout const &layout) const
    {
        std::stringstream ss;
        ss << elemStart("line") << attribute("x1", translateX(start_point.x, layout))
           << attribute("y1", translateY(start_point.y, layout)) << attribute("x2", translateX(end_point.x, layout))
           << attribute("y2", translateY(end_point.y, layout)) << stroke.toString(layout) << emptyElemEnd();
        return ss.str();
    }
    void offset(Point const &offset)
    {
        start_point.x += offset.x;
        start_point.y += offset.y;

        end_point.x += offset.x;
        end_point.y += offset.y;
    }

    Line &operator=(Line other)
    {
        start_point = other.start_point;
        end_point = other.end_point;
        fill = other.fill;
        stroke = other.stroke;
        return *this;
    }

private:
    Point start_point;
    Point end_point;
};

class Polygon : public Shape
{
public:
    Polygon() = default;
    Polygon(Fill const &fill, Stroke const &stroke) : Shape(fill, stroke)
    {
    }
    Polygon(Stroke const &stroke) : Shape(Color::Transparent, stroke)
    {
    }
    Polygon &operator<<(Point const &point)
    {
        points.push_back(point);
        return *this;
    }
    std::string toString(Layout const &layout) const
    {
        std::stringstream ss;
        ss << elemStart("polygon");

        ss << "points=\"";
        for (unsigned i = 0; i < points.size(); ++i)
            ss << translateX(points[i].x, layout) << "," << translateY(points[i].y, layout) << " ";
        ss << "\" ";

        ss << fill.toString(layout) << stroke.toString(layout) << emptyElemEnd();
        return ss.str();
    }
    void offset(Point const &offset)
    {
        for (unsigned i = 0; i < points.size(); ++i)
        {
            points[i].x += offset.x;
            points[i].y += offset.y;
        }
    }

    Polygon &operator=(Polygon other)
    {
        points = other.points;
        fill = other.fill;
        stroke = other.stroke;
        return *this;
    }

private:
    std::vector<Point> points;
};

class Polyline : public Shape
{
public:
    Polyline() = default;
    Polyline(Fill const &fill, Stroke const &stroke) : Shape(fill, stroke)
    {
    }
    Polyline(Stroke const &stroke) : Shape(Color::Transparent, stroke)
    {
    }
    Polyline(std::vector<Point> const &points, Fill const &fill = Fill(), Stroke const &stroke = Stroke())
        : Shape(fill, stroke), points(points)
    {
    }
    Polyline &operator<<(Point const &point)
    {
        points.push_back(point);
        return *this;
    }
    std::string toString(Layout const &layout) const
    {
        std::stringstream ss;
        ss << elemStart("polyline");

        ss << "points=\"";
        for (unsigned i = 0; i < points.size(); ++i)
            ss << translateX(points[i].x, layout) << "," << translateY(points[i].y, layout) << " ";
        ss << "\" ";

        ss << fill.toString(layout) << stroke.toString(layout) << emptyElemEnd();
        return ss.str();
    }
    void offset(Point const &offset)
    {
        for (unsigned i = 0; i < points.size(); ++i)
        {
            points[i].x += offset.x;
            points[i].y += offset.y;
        }
    }
    std::vector<Point> points;

    Polyline &operator=(Polyline other)
    {
        points = other.points;
        fill = other.fill;
        stroke = other.stroke;
        return *this;
    }
};

class Text : public Shape
{
public:
    Text() = default;
    Text(Point const &origin, std::string const &content, Fill const &fill, Font const &font, Stroke const &stroke)
        : Shape(fill, stroke), origin(origin), content(content), font(font)
    {
    }
    std::string toString(Layout const &layout) const
    {
        std::stringstream ss;
        ss << elemStart("text") << attribute("x", translateX(origin.x, layout))
           << attribute("y", translateY(origin.y, layout)) << fill.toString(layout) << stroke.toString(layout)
           << font.toString(layout) << ">" << content << elemEnd("text");
        return ss.str();
    }
    void offset(Point const &offset)
    {
        origin.x += offset.x;
        origin.y += offset.y;
    }

    Text &operator=(Text other)
    {
        origin = other.origin;
        content = other.content;
        font = other.font;
        return *this;
    }

private:
    Point origin;
    std::string content;
    Font font;
};

// Sample charting class.
class LineChart : public Shape
{
public:
    LineChart() : scale(1)
    {
    }
    LineChart(Dimensions margin, double scale, Stroke const &axis_stroke)
        : axis_stroke(axis_stroke), margin(margin), scale(scale)
    {
    }
    LineChart &operator<<(Polyline const &polyline)
    {
        if (polyline.points.empty())
            return *this;

        polylines.push_back(polyline);
        return *this;
    }
    std::string toString(Layout const &layout) const
    {
        if (polylines.empty())
            return "";

        std::string ret;
        for (unsigned i = 0; i < polylines.size(); ++i)
            ret += polylineToString(polylines[i], layout);

        return ret + axisString(layout);
    }
    void offset(Point const &offset)
    {
        for (unsigned i = 0; i < polylines.size(); ++i)
            polylines[i].offset(offset);
    }

    LineChart &operator=(LineChart other)
    {
        axis_stroke = other.axis_stroke;
        margin = other.margin;
        scale = other.scale;
        polylines = other.polylines;
        return *this;
    }

private:
    Stroke axis_stroke;
    Dimensions margin;
    double scale;
    std::vector<Polyline> polylines;

    optional<Dimensions> getDimensions() const
    {
        if (polylines.empty())
            return optional<Dimensions>();

        optional<Point> min = getMinPoint(polylines[0].points);
        optional<Point> max = getMaxPoint(polylines[0].points);
        for (unsigned i = 0; i < polylines.size(); ++i)
        {
            if (getMinPoint(polylines[i].points)->x < min->x)
                min->x = getMinPoint(polylines[i].points)->x;
            if (getMinPoint(polylines[i].points)->y < min->y)
                min->y = getMinPoint(polylines[i].points)->y;
            if (getMaxPoint(polylines[i].points)->x > max->x)
                max->x = getMaxPoint(polylines[i].points)->x;
            if (getMaxPoint(polylines[i].points)->y > max->y)
                max->y = getMaxPoint(polylines[i].points)->y;
        }

        return optional<Dimensions>(Dimensions(max->x - min->x, max->y - min->y));
    }
    std::string axisString(Layout const &layout) const
    {
        optional<Dimensions> dimensions = getDimensions();
        if (!dimensions)
            return "";

        // Make the axis 10% wider and higher than the data points.
        double width = dimensions->width * 1.1;
        double height = dimensions->height * 1.1;

        // Draw the axis.
        Polyline axis(Color::Transparent, axis_stroke);
        axis << Point(margin.width, margin.height + height) << Point(margin.width, margin.height)
             << Point(margin.width + width, margin.height);

        return axis.toString(layout);
    }
    std::string polylineToString(Polyline const &polyline, Layout const &layout) const
    {
        Polyline shifted_polyline = polyline;
        shifted_polyline.offset(Point(margin.width, margin.height));

        std::vector<Circle> vertices;
        for (unsigned i = 0; i < shifted_polyline.points.size(); ++i)
            vertices.push_back(
                Circle(shifted_polyline.points[i], getDimensions()->height / 30.0, Color::Black, Stroke(1)));

        return shifted_polyline.toString(layout) + vectorToString(vertices, layout);
    }
};

class Document
{
public:
    Document() = default;
    Document(std::string const &file_name, Layout layout) : file_name(file_name), layout(layout)
    {
    }

    Document &operator<<(Shape const &shape)
    {
        body_nodes_str += shape.toString(layout);
        return *this;
    }
    std::string toString() const
    {
        std::stringstream ss;
        ss << "<?xml " << attribute("version", "1.0") << attribute("standalone", "no")
           << "?>\n<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" "
           << "\"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n<svg "
           << attribute("width", layout.dimensions.width, "px") << attribute("height", layout.dimensions.height, "px")
           << attribute("xmlns", "http://www.w3.org/2000/svg") << attribute("version", "1.1") << ">\n"
           << body_nodes_str << elemEnd("svg");
        return ss.str();
    }
    bool save() const
    {
        std::ofstream ofs(file_name.c_str());
        if (!ofs.good())
            return false;

        ofs << toString();
        ofs.close();
        return true;
    }

    Document &operator=(Document other)
    {
        file_name = other.file_name;
        layout = other.layout;
        body_nodes_str = other.body_nodes_str;
        return *this;
    }

private:
    std::string file_name;
    Layout layout;

    std::string body_nodes_str;
};
}

#endif
