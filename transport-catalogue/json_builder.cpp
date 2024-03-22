#include "json_builder.h"
#include "json.h"

#include <utility>
#include <map>
#include <vector>
#include <list>
#include <stdexcept>


namespace json {

	Builder::Builder()
	{
		nodes_stack_.push_back(&root_);
	}

	AfterKey Builder::Key(std::string&& key) {
		if (!nodes_stack_.empty()) {
			Node* curent_node = *nodes_stack_.rbegin();
			if (curent_node->IsDict()) {
				Node node{ nullptr };
				auto iter = curent_node->AsDictNoConst().emplace(key, node);
				nodes_stack_.emplace_back(&iter.first->second);
				AfterKey ret{ *this };
				return ret;
			}
		}
		throw std::logic_error("error Key use");
	}

	Builder& Builder::Value(Node::Value&& value) {
		if (!nodes_stack_.empty()) {
			Node* curent_node = *nodes_stack_.rbegin();
			if (curent_node->IsArray()) {
				Node node{ nullptr };
				node.GetValueNoConst() = value;
				curent_node->AsArrayNoConst().push_back(node);
				return *this;
			}
			else if (curent_node->IsNull()) {
				curent_node->GetValueNoConst() = value;
				nodes_stack_.pop_back();
				return *this;
			}
		}
		throw std::logic_error("error value use");
	}

	AfterStartDict Builder::StartDict() {
		if (!nodes_stack_.empty()) {
			Node* curent_node = *nodes_stack_.rbegin();
			Dict dict_node;
			if (curent_node->IsNull()) {
				curent_node->GetValueNoConst() = dict_node;
				AfterStartDict ret{ *this };
				return ret;
			}
			else if (curent_node->IsArray()) {
				auto iter = curent_node->AsArrayNoConst().end();
				Node* new_dict = &(*curent_node->AsArrayNoConst().insert(iter, dict_node));
				nodes_stack_.push_back(new_dict);
				AfterStartDict ret{ *this };
				return ret;
			}
		}
		throw std::logic_error("error StartDict use");

	}

	AfterStartArray Builder::StartArray() {
		if (!nodes_stack_.empty()) {
			Node* curent_node = *nodes_stack_.rbegin();
			Array array_node;
			if (curent_node->IsNull()) {
				curent_node->GetValueNoConst() = array_node;
				AfterStartArray ret{ *this };
				return ret;
			}
			else if (curent_node->IsArray()) {
				auto iter = curent_node->AsArrayNoConst().end();
				Node* new_dict = &(*curent_node->AsArrayNoConst().insert(iter, array_node));
				nodes_stack_.push_back(new_dict);
				AfterStartArray ret{ *this };
				return ret;
			}
		}

		throw std::logic_error("error StartArray use");
	}

	Builder& Builder::EndDict() {
		if (!nodes_stack_.empty()) {
			Node* curent_node = *nodes_stack_.rbegin();
			if (curent_node->IsDict()) {
				nodes_stack_.pop_back();
				return *this;
			}
		}
		throw std::logic_error("error EndDict use");

	}

	Builder& Builder::EndArray() {
		if (!nodes_stack_.empty()) {
			Node* curent_node = *nodes_stack_.rbegin();
			if (curent_node->IsArray()) {
				nodes_stack_.pop_back();
				return *this;
			}
		}
		throw std::logic_error("error EndArray use");
	}

	Node Builder::Build() {
		if (nodes_stack_.empty()) {
			return std::move(root_);
		}
		throw std::logic_error("error Build use");
	}

	AfterKey::AfterKey(Builder& builder)
		:builder_(&builder)
	{}

	AfterDictValue AfterKey::Value(Node::Value&& value) {
		return builder_->Value(std::move(value));
	}

	AfterStartDict AfterKey::StartDict() {
		return builder_->StartDict();
	}

	AfterStartArray AfterKey::StartArray() {
		return builder_->StartArray();
	}

	AfterDictValue::AfterDictValue(Builder& builder)
		:builder_(&builder)
	{}

	AfterKey AfterDictValue::Key(std::string&& key) {
		return builder_->Key(std::move(key));
	}

	Builder& AfterDictValue::EndDict() {
		return builder_->EndDict();
	}

	AfterStartDict::AfterStartDict(Builder& builder)
		:builder_(&builder)
	{}

	AfterKey AfterStartDict::Key(std::string&& key) {
		return builder_->Key(std::move(key));
	}

	Builder& AfterStartDict::EndDict() {
		return builder_->EndDict();
	}

	AfterStartArray::AfterStartArray(Builder& builder)
		:builder_(&builder)
	{}

	AfterArrayValue AfterStartArray::Value(Node::Value&& value) {
		return builder_->Value(std::move(value));
	}

	AfterStartDict AfterStartArray::StartDict() {
		return builder_->StartDict();
	}

	AfterStartArray AfterStartArray::StartArray() {
		return builder_->StartArray();
	}

	Builder& AfterStartArray::EndArray() {
		return builder_->EndArray();
	}

	AfterArrayValue::AfterArrayValue(Builder& builder)
		:builder_(&builder)
	{}

	AfterArrayValue AfterArrayValue::Value(Node::Value&& value) {
		return builder_->Value(std::move(value));
	}

	AfterStartDict AfterArrayValue::StartDict() {
		return builder_->StartDict();
	}

	AfterStartArray AfterArrayValue::StartArray() {
		return builder_->StartArray();
	}

	Builder& AfterArrayValue::EndArray() {
		return builder_->EndArray();
	}


}
