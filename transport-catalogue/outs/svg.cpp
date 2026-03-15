#include "svg.h"

#include <cstdint>
#include <cmath>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace svg {

    using namespace std::literals;

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        RenderObject(context);

        context.out << std::endl;
    }
    
    std::string RemoveExtraZero (const std::string& input) {
        size_t last_no_zero = input.find_last_not_of('0');
        if (last_no_zero == 1 || last_no_zero == std::string::npos) {
            return input.substr(0, 1);
        } else {
            return input.substr(0, last_no_zero + 1);
        }  
    }

    // ---------- Circle ------------------

    Circle& Circle::SetCenter(Point&& center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\""sv;
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    //-----------Polyline------------------

    Polyline& Polyline::AddPoint(Point point) {
        polyline_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\""sv;
        bool first = true;
        for (auto& point : polyline_) {
            if (!first) {
                out << " ";
            }
            else {
                first = false;
            }
            double x = point.x;
            double y = point.y;

            out << x << "," << y;
        }
        out << "\"";
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    //-------------Text---------------------

    Text& Text::SetPosition(Point pos) {
        position_ = pos;
        return *this;
    }

    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size) {
        font_size_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = font_family;
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = font_weight;
        return *this;
    }

    Text& Text::SetData(std::string data) {
        std::string shield_data;
        for (char& simbol : data) {
            switch (simbol)
            {
            case '\\':
                shield_data.insert(shield_data.size(), "&quot;"s);
                break;
            case '\'':
                shield_data.insert(shield_data.size(), "&apos;"s);
                break;
            case '<':
                shield_data.insert(shield_data.size(), "&lt;"s);
                break;
            case '>':
                shield_data.insert(shield_data.size(), "&gt;"s);
                break;
            case '&':
                shield_data.insert(shield_data.size(), "&amp;"s);
                break;
            default:
                shield_data.push_back(simbol);
            }
        }
        data_ = shield_data;
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text";
        RenderAttrs(context.out);
        out << " x=\""sv << position_.x << "\" y=\""sv << position_.y << "\" dx=\""sv
            << offset_.x << "\" dy=\""sv << offset_.y << "\" font-size=\""sv << font_size_;
        if (font_family_ != "") {
            out << "\" font-family=\""sv << font_family_;
        }
        if (font_weight_ != "") {
            out << "\" font-weight=\"" << font_weight_;
        }
        out << "\">"sv << data_ << "</text>"sv;
    }

    void ObjectContainer::Render([[maybe_unused]] std::ostream& out) const {

    }

    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.emplace_back(std::move(obj));
    }

    void Document::Render(std::ostream& out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">" << std::endl;
        for (auto& obj : objects_) {
            RenderContext context(out, 2, 2);
            obj->Render(context);
        }
        out << "</svg>" << std::endl;
    }

    std::ostream& operator<<(std::ostream& out, const StrokeLineCap& line_cap) {
        switch (line_cap) {
        case StrokeLineCap::BUTT:
            out << "butt";
            return out;
        case StrokeLineCap::ROUND:
            out << "round";
            return out;
        case StrokeLineCap::SQUARE:
            out << "square";
            return out;
        }
        return out;
    }

    std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& line_join) {
        switch (line_join) {
        case StrokeLineJoin::ARCS:
            out << "arcs";
            return out;
        case StrokeLineJoin::BEVEL:
            out << "bevel";
            return out;
        case StrokeLineJoin::MITER:
            out << "miter";
            return out;
        case StrokeLineJoin::MITER_CLIP:
            out << "miter-clip";
            return out;
        case StrokeLineJoin::ROUND:
            out << "round";
            return out;
        }
        return out;
    }

}  // namespace svg