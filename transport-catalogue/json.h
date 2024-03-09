#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace json {

    class Node;
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    // Эта ошибка должна выбрасываться при ошибках парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

    class Node : public Value {
    public:
        
        using Value::Value;
        Node(std::string value);

        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsBool() const;
        bool IsString() const;
        bool IsNull() const;
        bool IsArray() const;
        bool IsMap() const;

        int AsInt() const;
        bool AsBool() const;
        double AsDouble() const;
        const std::string& AsString() const;
        const Array& AsArray() const;
        const Dict& AsMap() const;

        const Value& GetValue() const;
    };

    class Document {
    public:
        explicit Document(Node root);

        const Node& GetRoot() const;

    private:
        Node root_;
    };

    Document Load(std::istream& input);

    void Print(const Document& doc, std::ostream& output);
    
    bool operator==(const Document& lhv, const Document& rhv);
    
    bool operator!=(const Document& lhv, const Document& rhv);
       
    bool operator==(const Node& lhv, const Node& rhv);

    bool operator!=(const Node& lhv, const Node& rhv);

    struct PrintContext {
        std::ostream& out;
        int indent_step = 4;
        int indent = 0;

        void PrintIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        // Возвращает новый контекст вывода с увеличенным смещением
        PrintContext Indented() const {
            return { out, indent_step, indent_step + indent };
        }
    };

    void PrintValue(std::nullptr_t nptr, PrintContext& out);

    void PrintValue(const Array& value, PrintContext& out);

    void PrintValue(const Dict& value, PrintContext& out);

    void PrintValue(const int& value, PrintContext& out);

    void PrintValue(const bool& value, PrintContext& out);

    void PrintValue(const double& value, PrintContext& out);

    void PrintValue(const std::string& value, PrintContext& out);

    void PrintNode(const Node& node, std::ostream& out);

    void PrintNode(const Node& node, PrintContext& output);
}  // namespace json