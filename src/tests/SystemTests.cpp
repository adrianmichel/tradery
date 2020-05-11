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
#include <set>
#include <CppUnitTest.h>
#include <sstream>

#include <misc.h>
#include <switch.h>

#include "TestDataPath.h"
#include "TestLogger.h"

#include "..\tradery\System.h"
#include "..\tradery\SourceGenerator.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;


constexpr auto TEST_CASES = "testcases";
constexpr auto CONFIG_COMMON = "config_common.txt";
constexpr auto SYMBOLS_FILE = "symbols.txt";
constexpr auto CONFIG_OPT = "configfile ";
constexpr auto SYMBOLS_TO_CHARTS_FILE = "symbolstochartfile.txt";
constexpr auto SESSION_PARENT_DIR = "tradery";
constexpr auto POSITION_SIZING_DIR = "possizing";

#include <fstream>
#include <iterator>
#include <string>
#include <algorithm>

#include <tokenizer.h>

class Options {
private:
	std::string m_options;

public:
/*	template< typename T > void add(const std::string& name, const T& value) {
		Assert::IsFalse(name.empty(), L"Option name must not be empty");
		std::string prefix = name.size() == 1 ? "-" : "--";
		m_options += " "s + prefix + name + "\"" + std::to_string(value) + "\" ";
	}
*/
	void add(const std::string& name, const std::string& value) {
		Assert::IsFalse(name.empty(), L"Option name must not be empty");
		std::string prefix = name.size() == 1 ? "-" : "--";
		m_options += " "s + prefix + name + "  \"" + value + "\" ";
	}

	void add(const std::string& name, const fs::path& value) {
		add( name, ws2s( value.c_str() ));
	}

	void add(const std::string& name, const Path& value) {
		add(name, ws2s(value.getPath().c_str()));
	}

	Options& operator+=(const Options& options) {
		m_options += options.m_options;
		return *this;
	}

	const std::string& to_string() const { return m_options; }
};

class TestDescription {
private:
		Options m_options;
		std::string m_expectedOutputPath;
		std::string m_systemFile;
		std::string m_runnableId;

public:
	TestDescription(const std::string& file, const Path& testDataPath ) {
		std::ifstream f(file);

		if (f.fail()) {
			throw std::exception(("could not open file "s + file).c_str());
		}

		std::string line;

		fs::path p(file);

		Path currentDir(ws2s(p.parent_path().c_str()));

		while (getline(f, line)) {
			boost::trim(line);
			if (!line.empty()) {
				Tokenizer tok(line, "=");

				if (tok.size() != 2) {
					throw std::exception(("test property error: "s + line).c_str());
				}

				std::string key = boost::trim_copy(tok[0]);
				std::string value = boost::trim_copy(tok[1]);

				auto system = [&testDataPath, &value, this]() {
					std::wstring systemPath = testDataPath.makePath("systems", value).c_str();
					m_options.add("u", ws2s(systemPath));
				};

				Switch< std::string, std::function< void()>> properties{
					{
						{ "possizingFile", [&]() { m_options.add(CONFIG_OPT, ws2s(testDataPath.makePath(POSITION_SIZING_DIR, value)));  } },
						{ "slippage", [&]() { m_options.add("Q", value); } },
						{ "commission", [&]() {  m_options.add("M", value); } },
						{ "symbolsFile", [&]() { m_options.add("y", ws2s(testDataPath.makePath("symbols", value)));
																	m_options.add("symchartfile", ws2s(testDataPath.makePath("symbols", value))); }},
						{ "start", [&]() {  m_options.add("R", value); }},
						{ "end", [&]() {  m_options.add("T", value); }},
						{ "expectedresultsDir", [&]() { m_expectedOutputPath = ws2s(currentDir.makePath(value)); }},
						{ "system", system},
						{ "threads",[&]() { m_options.add("threads", value); } }
					}
				};

				auto f = properties[key];
				if (f) {
					(*f)();
				}
				else {
					Assert::Fail((L"unknown property: "s + s2ws(line)).c_str());
				}
			}
		}
	}

	const Options& options() const { return m_options;  }
	std::string expectedOutputDir() const{ return m_expectedOutputPath; }
	const std::string& systemFile() const { return m_systemFile; }
	const std::string& runnableId() const { return m_runnableId; }
};

bool binaryCompareFiles(const std::string& p1, const std::string& p2) {
	std::ifstream f1(p1, std::ifstream::binary | std::ifstream::ate);
	std::ifstream f2(p2, std::ifstream::binary | std::ifstream::ate);

	if (f1.fail() ){
		throw std::exception(("could not open file: "s + p1).c_str());
	}

	if (f2.fail()) {
		throw std::exception(("could not open file: "s + p2).c_str());
	}

	if (f1.tellg() != f2.tellg()) {
		return false; //size mismatch
	}

	//seek back to beginning and use std::equal to compare contents
	f1.seekg(0, std::ifstream::beg);
	f2.seekg(0, std::ifstream::beg);
	return std::equal(std::istreambuf_iterator<char>(f1.rdbuf()),
		std::istreambuf_iterator<char>(),
		std::istreambuf_iterator<char>(f2.rdbuf()));
}

std::set< std::string > fileToSet(const std::string& p, std::set< int > ignoreColumns = std::set< int >{}) {
	std::ifstream f(p, std::ifstream::binary);

	if (f.fail())
		throw std::exception("could not open file");

	std::string line;
	std::set< std::string > s;

	while (getline(f, line)) {
		Tokenizer tok(line, ",");
		for( auto i = ignoreColumns.crbegin(); i != ignoreColumns.crend(); ++i){
			tok.erase(std::next( tok.begin(), *i));
		}
		line = std::accumulate(std::next(tok.begin()), tok.end(), tok[0], [](std::string a, std::string b) { return std::move(a) + ',' + b; });
		s.insert(line);
	}

	return s;
}

bool contentCompareFiles(const std::string& p1, const std::string& p2) {
	std::string fileName = ws2s( fs::path{ p1 }.filename() );

	// for signals csv files ignore certain dynamically generated fields 7 and 9
	if (fileName == DEFAULT_RAW_SIGNALS_CSV_FILE || fileName == DEFAULT_SIGNALS_CSV_FILE) {
		// remove the signal id column which changes between runs and is not relevant anyway
		// with respect to the signal data
		std::set< std::string > s1(fileToSet(p1, { 7, 9 }));
		std::set< std::string > s2(fileToSet(p2, { 7, 9 }));

		return s1 == s2;
	}
	else {
		std::set< std::string > s1(fileToSet(p1));
		std::set< std::string > s2(fileToSet(p2));

		return s1 == s2;
	}
}

namespace SystemsTests
{
	TEST_CLASS(SystemsTests)
	{
	private:
		void validateResults(fs::path& expectedResultsDir, const std::string& sessionPath ){
			LOG( log_debug, "validating results - expected dir: ", ws2s( expectedResultsDir.c_str() ), ", sessionPath: ", sessionPath );
			Assert::IsTrue(fs::exists(expectedResultsDir), (L"expected results dir doesn't exist: "s + expectedResultsDir.c_str()).c_str());
			Assert::IsTrue(fs::exists(expectedResultsDir), (L"session dir doesn't exist: "s + s2ws( sessionPath ) ).c_str());
			for (auto x : fs::directory_iterator(expectedResultsDir)) {
				auto file = Path(sessionPath).makePath(x.path().filename());
				std::string p1 = ws2s(x.path().c_str());
				std::string p2 = ws2s(file.c_str());
				if (!binaryCompareFiles(p1, p2)) {
					if (!contentCompareFiles(p1, p2)) {
						Assert::Fail( s2ws( tradery::format("files are not equal: ", p1, ", ", p2 )).c_str());
					}
				}
			}
		}

#if defined( _WIN64)
#define TARGET x64
#else
#define TARGET x86
#endif

#define ENV_CONFIG_FILE( target ) "env_" EXPAND( target ) ".txt"

		void makeOptions( const TestDescription& td, const TestDataPath& testDataPath ) const {
			// this is a simulation test data directory

			std::optional< Path > op = Path::create_tmp_dir(SESSION_PARENT_DIR);
			if (!op) {
				Assert::Fail(L"Could not create temporary output directory");
			}

			Options options;

			TestPath envConfigPath{ EXPAND( CONFIG_FILE ) };

			fs::path commonConfigPath = testDataPath.makePath("config", CONFIG_COMMON);

			// create output path option
			std::string outputPath = ws2s(op->getPath().c_str());
			options.add("b", outputPath);
			options.add("a", TestDataPath{}.makePath("data"));
			options.add(CONFIG_OPT, ws2s(commonConfigPath.c_str()));
			options.add(CONFIG_OPT, ws2s(envConfigPath.getPath().c_str()));
			options.add("i", "${SessionPath}"s);

			options += td.options();

			setConfig(std::make_shared<Configuration>( options.to_string()));
			LOG(log_debug, "after cmd line processing");
		}

		void forAllTests( const fs::path simulationsPath, std::function< void( const fs::path& ) >f) {
			for (auto p : fs::directory_iterator(simulationsPath)) {
				if (fs::is_directory(p)) {
					// for all test description files inside the current test directory
					for (auto testDescription : fs::directory_iterator(p)) {
						if (!fs::is_directory(testDescription)) {
							f(testDescription.path());
						}
					}
				}
			}
		}
	public:

		TEST_METHOD(SimulationTest)
		{
			try {
				const TestDataPath testDataPath;
				auto f = [this, &testDataPath](const fs::path& testDescription)->void {
					TestDescription td(ws2s(testDescription.c_str()), testDataPath);
					makeOptions(td, testDataPath);
					std::string sessionPath = getConfig().getSessionPath();
/*					std::string symlink( ws2s( Path{ sessionPath }.makePath("link_to_test_case.txt").c_str() ) );
					try {
						fs::create_hard_link(testDescription, fs::path{ symlink });
					}
					catch (const fs::filesystem_error& e) {
						std::string error = e.what();
						Assert::Fail( L"Could not create hard link");
					}
					*/
					int nRetCode = run();


					Assert::IsTrue(nRetCode == 0, L"session failed");

					fs::path expected_results = td.expectedOutputDir();
					const std::string& session_path = getConfig().getSessionPath();
					validateResults(expected_results, session_path);

				};
				// run test cases from all test directories
				fs::path simulationsPath = testDataPath.makePath(TEST_CASES);
				forAllTests(simulationsPath, f);
			}
			catch (ConfigurationException & e) {
				Assert::Fail(s2ws("configuration exception: "s + e.what()).c_str());
			}
		}
	};
}

TEST_MODULE_INITIALIZE(methodName) {
	tradery::Log::log().addLogger(std::make_shared< TestLogger>(tradery::LogConfig(log_debug)));
}
