/*
	 Copyright (C) 2018-2020 Adrian Michel

	 Licensed under the Apache License, Version 2.0 (the "License");
	 you may not use this file except in compliance with the License.
	 You may obtain a copy of the License at

			 http://www.apache.org/licenses/LICENSE-2.0

	 Unless required by applicable law or agreed to in writing, software
	 distributed under the License is distributed on an "AS IS" BASIS,
	 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	 See the License for the specific language governing permissions and
	 limitations under the License.
*/

#pragma once

#include <map>
#include <initializer_list>


template< typename K, typename V> class Switch {
public:
	using M = std::map< K, V >;
private:
	std::map< K, V > m_m;
	std::optional< V > m_def;

public:
	Switch(std::initializer_list< typename M::value_type > init, const V& def)
		: m_m(init), m_def(def) {
	}

	Switch(std::initializer_list< typename M::value_type > init)
		: m_m(init) {
	}

	std::optional< V > operator[](K k) const {
		M::const_iterator i = m_m.find(k);

		return i != m_m.end() ? i->second : m_def;
	}
};
