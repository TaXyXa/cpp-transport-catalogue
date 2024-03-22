#pragma once
#include "json.h"
#include <list>

namespace json {

	class Builder;
	class AfterDictValue;
	class AfterStartDict;
	class AfterArrayValue;
	class AfterStartArray;


	class AfterKey {
	public:
		AfterKey(Builder& builder);

		AfterDictValue Value(Node::Value&& value);

		AfterStartDict StartDict();

		AfterStartArray StartArray();

	private:
		Builder* builder_ = nullptr;
	};

	class AfterDictValue {
	public:
		AfterDictValue(Builder& builder);

		AfterKey Key(std::string&& key);

		Builder& EndDict();

	private:
		Builder* builder_ = nullptr;
	};

	class AfterStartDict {
	public:
		AfterStartDict(Builder& builder);

		AfterKey Key(std::string&& key);

		Builder& EndDict();

	private:
		Builder* builder_ = nullptr;
	};

	class AfterArrayValue {
	public:
		AfterArrayValue(Builder& builder);

		AfterArrayValue Value(Node::Value&& value);

		AfterStartDict StartDict();

		AfterStartArray StartArray();

		Builder& EndArray();

	private:
		Builder* builder_ = nullptr;
	};

	class AfterStartArray {
	public:

		AfterStartArray(Builder& builder);

		AfterArrayValue Value(Node::Value&& value);

		AfterStartDict StartDict();

		AfterStartArray StartArray();

		Builder& EndArray();

	private:
		Builder* builder_ = nullptr;
	};

	class Builder {
	public:

		Builder();

		AfterKey Key(std::string&& key);

		Builder& Value(Node::Value&& value);

		AfterStartDict StartDict();

		AfterStartArray StartArray();

		Builder& EndDict();

		Builder& EndArray();

		Node Build();

	private:
		Node root_;
		std::list<Node*> nodes_stack_;
	};
}


