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

	Builder::AfterKey Builder::Key(std::string key) {
		if (!nodes_stack_.empty()) {
			Node* curent_node = *nodes_stack_.rbegin();
			if (curent_node->IsDict()) {
				Node node{ nullptr };
				auto iter = curent_node->AsDictNoConst().emplace(std::move(key), node);
				nodes_stack_.push_back(&iter.first->second);
				return After(*this);
			}
		}
		throw std::logic_error("error Key use");
	}

	Builder::After Builder::Value(Node::Value value) {
		AddValue(std::move(value), false);
		return *this;
	}

	Builder::AfterDictValue Builder::StartDict() {
		AddValue(Dict{}, true);
		return After(*this);
	}

	Builder::AfterArrayValue Builder::StartArray() {
		AddValue(Array{}, true);
		return After(*this);
	}

	Builder::After Builder::EndDict() {
		if (!nodes_stack_.empty()) {
			Node* curent_node = *nodes_stack_.rbegin();
			if (curent_node->IsDict()) {
				nodes_stack_.pop_back();
				return *this;
			}
		}
		throw std::logic_error("error EndDict use");
	}

	Builder::After Builder::EndArray() {
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

	void Builder::AddValue(Node::Value value, bool is_cont) {
		Node* curent_node = *nodes_stack_.rbegin();
		if (!nodes_stack_.empty()) {
			if (curent_node->IsArray()) {
				auto iter = curent_node->AsArrayNoConst().end();
				Node new_node{nullptr};
				new_node.GetValueNoConst() = std::move(value);
				Node* new_node_ptr = &(*curent_node->AsArrayNoConst().insert(iter, new_node));
				if (is_cont) {
					nodes_stack_.emplace_back(new_node_ptr);
				}
				return;
			}
			else if (curent_node->IsNull()) {
				curent_node->GetValueNoConst() = std::move(value);
				if (!is_cont) {
					nodes_stack_.pop_back();
				}
				return;
			}
		}
		throw std::logic_error("error value use");
	}

}
