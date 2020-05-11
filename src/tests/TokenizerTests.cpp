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

#include "pch.h"
#include <CppUnitTest.h>
#include <tokenizer.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#define TOKEN1 "abc"s
#define TOKEN2 "def"s
#define TOKEN3 "xyz"s
#define TOKEN4 "123"s

namespace TokenizerTests {
	TEST_CLASS(TokenizerTests) {
		TEST_METHOD(TokenizerTest1) {
			Sep sep(",");
			std::string str(TOKEN1 "," TOKEN2 "," TOKEN3 );
			std::vector < std::string> expected{ {TOKEN1, TOKEN2, TOKEN3} };
			Tokenizer tok(str, sep);

			Assert::IsTrue(expected == tok);
		}

		TEST_METHOD(TokenizerTest2) {
			Sep sep(", \t;");
			std::string str( "   " TOKEN1 ",\t;" TOKEN2 ";;;," TOKEN3);
			std::vector < std::string> expected{ {TOKEN1, TOKEN2, TOKEN3} };
			Tokenizer tok(str, sep);

			Assert::IsTrue(expected == tok);
		}

		TEST_METHOD(TokenizerTest3 ) {
			Sep sep(",");
			std::string str(TOKEN1 "," TOKEN2 "," TOKEN3 ",,," );
			std::vector < std::string> expected{ {TOKEN1, TOKEN2 "," TOKEN3 ",,," } };

			Tokenizer tok(str, sep, 2 );

			Assert::IsTrue(expected == tok);
		}

		TEST_METHOD(TokenizerTest4) {
			Sep sep(",; \t");
			std::string str(TOKEN1 ",  ;;" TOKEN2 ", \t\t;" TOKEN3 ",,," TOKEN4 ";;;" );
			std::vector < std::string> expected{ {TOKEN1, TOKEN2, TOKEN3 ",,," TOKEN4 ";;;" } };

			Tokenizer tok(str, sep, 3);

			Assert::IsTrue(expected == tok);
		}

	};
}


