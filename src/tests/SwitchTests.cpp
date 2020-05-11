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
#include <switch.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace SwitchTests {
	TEST_CLASS(SwitchTests)	{
		TEST_METHOD(SwitchTestWithDefault)	{
			Switch< int, int > es{
				{
					{1, 2}, // case 1
					{2, 3}, // case 2
					{3, 3}, // case 3 (same value as 2)
					{4, 10} // case 4
				},
				20 		// default value
			};

			Assert::AreEqual(2, *es[1]);
			Assert::AreEqual(3, *es[2]);
			Assert::AreEqual(3, *es[3]);
			Assert::AreEqual(10, *es[4]);
			// default values
			Assert::AreEqual(20, *es[10]);
			Assert::AreEqual(20, *es[1000]);

		}

		TEST_METHOD(SwitchTestWithoutDefault) {
			Switch< int, int > es{
				{
					{1, 2}, // case 1
					{2, 3}, // case 2
					{3, 3}, // case 3 (same value as 2)
					{4, 10} // case 4
				}
			};

			Assert::AreEqual(2, *es[1]);
			Assert::AreEqual(3, *es[2]);
			Assert::AreEqual(3, *es[3]);
			Assert::AreEqual(10, *es[4]);
			// default values
			Assert::IsFalse(es[10].has_value());
			Assert::IsFalse(es[10].has_value());
		}
	};
}


