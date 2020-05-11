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
#include "..\tradery\System.h"
#include "..\tradery\SourceGenerator.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace SourceGeneratorTests {
	TEST_CLASS(SourceGeneratorTests) {
		TEST_METHOD(SourceGeneratorTest) {
			TradingSystem system( "C:\\dev\\tradery_service\\src\\tests\\TestData\\systems\\SMA System with Pullback\\2aecf79f-a724-83ac-f212-17bf0597e120.h");

			TradingSystems systems{ { system } };

			SourceGenerator gen(systems);

			std::string x = gen.generate();

			Microsoft::VisualStudio::CppUnitTestFramework::Logger::WriteMessage(L"clazz");
		}
	};
}


