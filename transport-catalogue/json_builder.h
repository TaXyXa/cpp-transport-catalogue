#pragma once
#include "json.h"
#include <list>

namespace json {

	class Builder {
	private:
		class After;
		class AfterKey;
		class AfterDictValue;
		class AfterArrayValue;
	public:
		Builder();

		AfterKey Key(std::string key);

		After Value(Node::Value value);

		AfterDictValue StartDict();

		AfterArrayValue StartArray();

		After EndDict();

		After EndArray();

		Node Build();
	private:

		Node root_;
		std::list<Node*> nodes_stack_;

		void AddValue(Node::Value value, bool is_cont);

		class After {
		public:
			After(Builder& builder)
				:builder_(&builder)
			{}

			AfterKey Key(std::string key) {
				return builder_->Key(std::move(key));
			}

			After Value(Node::Value value) {
				return builder_->Value(std::move(value));
			}

			AfterDictValue StartDict() {
				return builder_->StartDict();
			}

			AfterArrayValue StartArray() {
				return builder_->StartArray();
			}

			After EndDict() {
				return builder_->EndDict();
			}

			After EndArray() {
				return builder_->EndArray();
			}

			Node Build() {
				return builder_->Build();
			}

		private:
			Builder* builder_;
		};

		class AfterKey : public After {
		public:
			AfterKey(After builder)
				:After(builder)
			{}

			AfterKey Key(std::string key) = delete;

			AfterDictValue Value(Node::Value value) {
				return After::Value(std::move(value));
			}

			After EndDict() = delete;

			After EndArray() = delete;

			Node Build() = delete;

		};

		class AfterDictValue : public After {
		public:
			AfterDictValue(After builder)
				:After(builder)
			{}

			After Value(Node::Value value) = delete;

			After StartDict() = delete;

			After StartArray() = delete;

			After EndArray() = delete;

			Node Build() = delete;

		};

		class AfterArrayValue : public After {
		public:
			AfterArrayValue(After builder)
				:After(builder)
			{}

			AfterArrayValue Value(Node::Value value) {
				return After::Value(std::move(value));
			}

			AfterKey Key(std::string key) = delete;

			After EndDict() = delete;

			Node Build() = delete;

		};

	};
}
