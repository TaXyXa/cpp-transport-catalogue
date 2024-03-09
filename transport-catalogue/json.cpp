#include "json.h"

#include<cmath>
#include <sstream>
#include <iostream>

using namespace std;

namespace json {

    bool Node::IsInt() const {
        return this->index() == 4;
    }
    bool Node::IsDouble() const {
        return this->index() == 4 || this->index() == 5;
    }
    bool Node::IsPureDouble() const {
        return this->index() == 5;
    }
    bool Node::IsBool() const {
        return this->index() == 3;
    }
    bool Node::IsString() const {
        return this->index() == 6;
    }
    bool Node::IsNull() const {
        return this->index() == 0;
    }
    bool Node::IsArray() const {
        return this->index() == 1;
    }
    bool Node::IsMap() const {
        return this->index() == 2;
    }

    int Node::AsInt() const {
        if (IsInt()) {
            return std::get<int>(*this);
        }
        else {
            throw std::logic_error("error_type");
        }
    }
    bool Node::AsBool() const {
        if (IsBool()) {
            return std::get<bool>(*this);
        }
        else {
            throw std::logic_error("error_type");
        }
    }
    double Node::AsDouble() const {
        if (IsInt()) {
            return std::get<int>(*this);
        }
        else if (IsPureDouble()) {
            return std::get<double>(*this);
        }
        else {
            throw std::logic_error("error_type");
        }
    }
    const std::string& Node::AsString() const {
        if (IsString()) {
            return std::get<string>(*this);
        }
        throw std::logic_error("error_type");
    }
    const Array& Node::AsArray() const {
        if (IsArray()) {
            return std::get<Array>(*this);
        }
        else {
            throw std::logic_error("error_type");
        }
    }
    const Dict& Node::AsMap() const {
        if (IsMap()) {
            return std::get<Dict>(*this);
        }
        else {
            throw std::logic_error("error_type");
        }
    }

    namespace {

        std::string TrimString(std::string& str) {
            std::string ans;
            for (auto a : str) {
                if (a != '\"')
                    ans.push_back(a);
            }
            return ans;
        }

        Node LoadNode(istream& input);

        Node LoadNull(istream& input) {
            std::string line;
            char c;
            while (input >> c) {
                if (c == ' ' || c == '"' || c == ',' || c == '}' || c == ']') {
                    input.putback(c);
                    break;
                }
                line.push_back(move(c));
            }
            if (line == "null") {
                return Node(nullptr);
            }
            throw ParsingError("String parsing error");
        }
        
        Node LoadBool(istream& input) {
            std::string line;
            char c;
            while (input >> c) {
                if (c == ' ' || c == '"' || c == ',' || c == '}' || c == ']' ) {
                    input.putback(c);
                    break;
                }
                line.push_back(move(c));
            }
            if (line == "true") {
                return Node{ true };
            }
            else if (line == "false"){
                return Node{ false };
            }
            throw ParsingError("String parsing error");
        }

        Node LoadNumber(std::istream& input) {
            using namespace std::literals;

            std::string parsed_num;

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            // Считывает одну или более цифр в parsed_num из input
            auto read_digits = [&input, read_char] {
                if (!std::isdigit(input.peek())) {
                    throw ParsingError("A digit is expected"s);
                }
                while (std::isdigit(input.peek())) {
                    read_char();
                }
            };

            if (input.peek() == '-') {
                read_char();
            }
            // Парсим целую часть числа
            if (input.peek() == '0') {
                read_char();
                // После 0 в JSON не могут идти другие цифры
            } else {
                read_digits();
            }

            bool is_int = true;
            // Парсим дробную часть числа
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            // Парсим экспоненциальную часть числа
            if (int ch = input.peek(); ch == 'e' || ch == 'E') {
                read_char();
                if (ch = input.peek(); ch == '+' || ch == '-') {
                    read_char();
                }
                read_digits();
                is_int = false;
            }

            try {
                if (is_int) {
                    // Сначала пробуем преобразовать строку в int
                    try {
                        return Node(std::stoi(parsed_num));
                    } catch (...) {
                        // В случае неудачи, например, при переполнении,
                        // код ниже попробует преобразовать строку в double
                    }
                }
                double wtf = std::stod(parsed_num);
                return Node(wtf);
            } catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        Node LoadString(istream& input) {
            using namespace std::literals;
            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true) {
                if (it == end) {
                    // Поток закончился до того, как встретили закрывающую кавычку?
                    throw ParsingError("String parsing error");
                }
                const char ch = *it;
                if (ch == '"') {
                    // Встретили закрывающую кавычку
                    ++it;
                    break;
                }
                else if (ch == '\\') {
                    // Встретили начало escape-последовательности
                    ++it;
                    if (it == end) {
                        // Поток завершился сразу после символа обратной косой черты
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
                    // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
                    switch (escaped_char) {
                    case 'n':
                        s.push_back('\n');
                        break;
                    case 't':
                        s.push_back('\t');
                        break;
                    case 'r':
                        s.push_back('\r');
                        break;
                    case '"':
                        s.push_back('"');
                        break;
                    case '\\':
                        s.push_back('\\');
                        break;
                    default:
                        // Встретили неизвестную escape-последовательность
                        throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                }
                else if (ch == '\n' || ch == '\r') {
                    // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                    throw ParsingError("Unexpected end of line"s);
                }
                else {
                    // Просто считываем очередной символ и помещаем его в результирующую строку
                    s.push_back(ch);
                }
                ++it;
            }

            if (s == "true") {
                return Node{ true };
            }
            else if (s == "false") {
                return Node{ false };
            }

            return Node(move(s));
        }

        Node LoadArray(istream& input) {
            Array result;
            string bufer;
            char c;
            for (; input >> c;) {
                if (c == ']') {
                    break;
                }
                if (c == '[' || c == '{' || c == 'n' || c == 't' || c == 'f' || c == '"' ) {
                    input.putback(c);
                    result.push_back(LoadNode(input));
                    continue;
                }
                if (c != ',') {
                    bufer.push_back(c);
                }
                else if (!bufer.empty()) {
                    istringstream input_buf(bufer);
                    result.push_back(LoadNode(input_buf));
                    bufer.clear();
                }
            }
            if (!bufer.empty()) {
                istringstream input_buf(bufer);
                result.push_back(LoadNode(input_buf));
            }
            else if (result.empty() && c != ']') {
                throw ParsingError("smthg wrong"s);
            }
            return Node(move(result));
        }
        
        Node LoadDict(istream& input) {
            Dict result;
            string key;
            string value;
            bool iskey = true;
            char c;
            for (; input >> c;) {
                if (c == '}') {
                    break;
                }
                if (iskey) {
                    if (c == ',') {
                        continue;
                    }
                    if (c == ':') {
                        iskey = false;
                    }
                    else {
                        key = c;
                        key += LoadString(input).AsString();
                        key += '\"';
                    }
                } else {
                    if (c == ',') {
                        iskey = true;
                        istringstream input_value(value);
                        key = TrimString(key);
                        result.insert({ move(key), LoadNode(input_value) });
                        key.clear();
                        value.clear();
                    }
                    else if (c == '[' || c == '{' || c == 'n' || c == 't' || c == 'f' || c == '"') {
                        iskey = true;
                        input.putback(c);
                        key = TrimString(key);
                        result.insert({ move(key), LoadNode(input) });
                    } 
                    else {
                        value.push_back(c);
                    }
                }
            }
            if (!key.empty()) {
                istringstream input_key(key);
                istringstream input_value(value);
                key = LoadNode(input_key).AsString();
                result.insert({ move(key), LoadNode(input_value) });
            }
            else if (result.empty() && c != '}') {
                throw ParsingError("Unexpected end of line"s);
            }
            return Node(move(result));
        }

        Node LoadNode(istream& input) {
            try {
                char c;
                input >> c;

                if (c == '[') {
                    return LoadArray(input);
                }
                else if (c == '{') {
                    return LoadDict(input);
                }
                else if (c == '"') {
                    return LoadString(input);
                }
                else if (c == 'n') {
                    input.putback(c);
                    return LoadNull(input);
                }
                else if (c == 't' || c == 'f') {
                    input.putback(c);
                    return LoadBool(input);
                }
                else {
                    input.putback(c);
                    return LoadNumber(input);
                }
            }
            catch (const std::exception& e) {
                throw ParsingError("Parsing error");
            }
        }

    }  // namespace

    Node::Node(std::string value) : Value(value) {
        if (value == "null") {
            Value(nullptr);
        }
        else {
            Value(value);
        }
    }

    const Value& Node::GetValue() const {
        return *this;
    }

    Document::Document(Node root)
        : root_(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }

    void Print(const Document& doc, std::ostream& output) {
        PrintNode(doc.GetRoot(), output);
    }

    void PrintValue([[maybe_unused]] std::nullptr_t nptr, PrintContext& out) {
        out.out << "null"sv;
    }

    void PrintValue(const Array& value, PrintContext& out) {
        bool nofirst = false;
        out.out << '[' << std::endl;
        for (const auto& node : value) {
            if (nofirst) {
                out.out << ',' << std::endl;
            }
            else {
                nofirst = true;
            }
            PrintContext output = out.Indented();
            output.PrintIndent();
            PrintNode(node, output);
        }
        out.PrintIndent();
        out.out << std::endl;
        out.PrintIndent();
        out.out << ']';
    }

    void PrintValue(const Dict& value, PrintContext& out) {
        out.out << '{';
        out.out << std::endl;
        bool nofirst = false;
        for (const auto& [key, node] : value) {
            if (nofirst) {
                out.out << ',' << std::endl;
            }
            else {
                nofirst = true;
            }
            PrintContext output = out.Indented();
            output.PrintIndent();
            PrintValue(key, output);
            out.out << ':' << ' ';
            PrintNode(node, output);
        }
        out.out << std::endl;
        out.PrintIndent();
        out.out << '}';
    }

    void PrintValue(const int& value, PrintContext& out) {
        out.out << value;
    }

    void PrintValue(const bool& value, PrintContext& out) {
        if (value) {
            out.out << "true";
        }
        else {
            out.out << "false";
        }
    }

    void PrintValue(const double& value, PrintContext& out) {
        out.out << value;
    }
    
    void PrintValue(const std::string& value, PrintContext& out) {
        using namespace std::literals;
        std::string ans;
        ans.push_back('\"');
        for (const char& c : value) {
            switch (c) {
            case '\\':
                ans.push_back('\\');
                ans.push_back('\\');
                break;
            case '\n':
                ans.push_back('\\');
                ans.push_back('n');
                break;
            case '\r':
                ans.push_back('\\');
                ans.push_back('r');
                break;
            case '\t':
                ans.push_back('\\');
                ans.push_back('t');
                break;
            case '\"':
                ans.push_back('\\');
                ans.push_back('\"');
                break;
            default:
                ans.push_back(c);
            }
        }
        ans.push_back('\"');
        out.out << ans;
    }

    void PrintNode(const Node& node, std::ostream& out) {
        PrintContext output = PrintContext({ out });
        std::visit(
            [&output](const auto& value) { PrintValue(value, output); },
            node.GetValue());
    }

    void PrintNode(const Node& node, PrintContext& output) {
        std::visit(
            [&output](const auto& value) { PrintValue(value, output); },
            node.GetValue());
    }

    bool operator==(const Document& lhv, const Document& rhv) {
        return lhv.GetRoot() == rhv.GetRoot();
    }

    bool operator!=(const Document& lhv, const Document& rhv) {
        return !(lhv == rhv);
    }
    
    bool operator==(const Node& lhv, const Node& rhv) {
        return lhv.GetValue().index() == rhv.GetValue().index() && lhv.GetValue() == rhv.GetValue();
    }

    bool operator!=(const Node& lhv, const Node& rhv) {
        return !(lhv == rhv);
    }

}  // namespace json