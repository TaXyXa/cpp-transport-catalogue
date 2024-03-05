#pragma once

#include <iostream>

#include "geo.h"
#include "json.h"
#include "request_handler.h"

namespace json {

	struct Requests {
		int id;
		std::string type;
		std::string name;
	};

	class Reader {
	public:
		
		Reader(RequestHandler& request_handler);

		void SetInfo(std::istream& input);

		void PrintInfo(std::ostream& output);

	private:
		RequestHandler& request_handler_;
		std::vector<Requests> requests_;
	};

}