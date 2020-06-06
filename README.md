Submit your feedback on the issues page, or email it to info@adrianmichel.com.

Refer to the [Disclaimer](#Disclaimer) and [License](#License) sections for important 
information regarding the use of this library.

# Table of Contents
   * [Tradery Engine](#tradery-engine)
   * [Installation and test](#installation-and-test)
   * [Objectives](#objectives)
   * [What is new](#what-is-new)
   * [The Visual Studio Tradery solution](#the-visual-studio-tradery-solution)
   * [Session](#session)
   * [Configuration](#configuration)
   * [The Systems Test Project](#the-systems-test-project)
     * [Single-threaded vs multi-threaded test cases](#single-threaded-vs-multi-threaded-test-cases)
   * [Running the application from the command line](#running-the-application-from-the-command-line)
   * [Plugins](#plugins)
   * [Trading system definition language](#trading-system-definition-language)
   * [Back testing realism](#back-testing-realism)
     * [Slippage](#slippage)
   * [Market Data](#market-data)
     * [Data Format](#data-format)
     * [Data Location](#data-location)
     * [Imperfect Data](#imperfect-data)
     * [Charting](#charting)
   * [Logging](#logging)
   * [Security](#security)
   * [Disclaimer](#disclaimer)
   * [License](#license)
   * [3rd party components and applications](#3rd-party-components-and-applications)


# Trading Engine
This project contains an updated version of the Trading Engine (TE) previously used on the tradery.com back end.

The purpose of the TE is to run user defined algorithmic trading systems over historical market data of selected symbols with
specified configuration parameters.

The main functions of the TE are:

1. Trading system back-testing
2. Generation of trading signals
2. Trading system optimization (work in progress)

# Installation and test
TE is provided as a Visual Studio 2019 solution.

Here are the steps to install, build, test and run TE.

1. Install the latest version of Visual Studio 2019 Community Edition. Make sure to select C++, SDK, 32 and 64 bit libraries.
2. Install a recent git client.
2. Clone the tradery repository to a local directory, for instance "<some_path>\tradery".
4. Checkout "master" branch.
3. Open a console window (cmd.exe, or PowerShell), cd to the root of the installation directory, "tradery" in our case, 
and run "bootstrap.bat" from there. This will download and build 3rd party libraries that the TE depends on.
4. Optionally, go to https://www.advsofteng.com/download.html and download the evaluation version of the charting library for C++ Windows. The TE will build and run
with or without this library, but when the library is present, TE will generate an equity curve chart for each session. 
Extract the files from the downloaded ChartDirector zip file under the "tradery\external" directory, so the directory structure will be "tradery\external\ChartDirector\..."
5. Start Visual Studio 2019 and open the tradery solution  "tradery\src\tradery.sln"
9. Select the desired debug/release, 32/64 bit combination at the top of the Visual Studio window. Please note
that the debug version for either 32 or 64 bit will run much slower than the release counterpart, so for the first run
which is intended to ensure that everything works as expected, it is recommended to use "release".
6. Open the Solution Explorer window.
9. Right click on the solution icon and click on "Build solution". You will see a fair number of warnings in the output window, some of which are inevitable,
and others that will be fixed in future commits, but none of which will cause any issues with the application.
11. Open the Test Explorer window by clicking on "Test/Test Explorer" on the Visual Studio menu bar
11. In the Test Explorer Window click on the double green arrow (Run all tests). This will start a build if necessary, and then start all available tests.
12. Tests will run for a few minutes if release version was selected, longer for debug, after which the test icon should turn to a green check mark indicating
that all tests have passed.

# Objectives
Here is a list of initial design goals, and how they were achieved. 

In order to build an online trading simulation platform, issues like performance, scalability, stability, massive concurrent use etc needed to be considered.

| |Objective | Implementation |
|----|-------|--------------|
|1| High performance for real-time use |- written in C++<br>- multi-threading and multi-processing<br>- use of data caches<br>- compiled system code<br>  |
|2| Realistic simulation| support for predefined or user-definable parameters such as slippage, commission algorithms, position size, account size, position size limits etc|
|3| Support for trading systems from the most trivial to the most complex| C++ based system definition language<br>with many trading specific constructs|
|4| Support for various types of market data (e.g. bar, tick)| internal abstraction of data type|
|5| Diversification by asset|Support for multi-symbol runs |
|6| Diversification by system |Support for multi-system runs |
|7| Ability to handle large datasets| efficient data and memory management|
|8| Easy to learn and use script-like language| abstraction of complex trading functions |
|9| Support for a variety of common or less common statistical and technical indicators| use of TALib technical indicators library|
|10| Potential real trading use|Trading signal generation |
|11| Ability to objectively assess the performance and risk of a trading system|Generation of a variety of simulation statistical information|

While there are many other programming languages available, C++ was chosen for its flexibility, large number of third party 
libraries available for any conceivable functionality, and not least, its ability to generate some 
of the most efficient code.
 
# What is new
This new version contains many enhancements, internal as well as in terms of usability.

- Ported to Visual Studio 2019.
- Updated to use recent C++ language features.
- Generates both 32 and 64 bit binaries.
- Contains integration tests for validation and regression testing.
- Can be run standalone command line utility.
- Contains a large number of fixes and enhancements.
- Required 3rd party dependencies are automatically downloaded and built using the bootstrap.bat script.
- Tedious environment configuration parameters are automatically generated so that only relevant parameters need to be set by the user.
- It automatically detects and takes into account whether the charting library is available when building and running.

This project is work in progress and more updates will be committed based on your feedback.

# The Visual Studio Tradery solution
The Visual Studio "tradery" solution contains the following projects:

* **core** - implements the various trading language constructs, and coordinates the multi-processing aspects of running the trading simulation sessions.
* **datasource** - dll, reads files containing historical data for various market symbols.
* **fileplugins** - dll, contains various plugins implementations (datasource, symbolssource, statistics, charting, commission, slippage).
* **filesymbols** - static lib, code for handling lists of symbols.
* **include** - no target, container for common include files used across different projects.
* **misc** - dll, various general utility classes and functions.
* **miscwin** - dll, various windows dependent utility classes and functions.
* **optimizer** - exe, trading system optimization utility, work in progress (not functional at this time).
* **plugin** - static library, plugin related utility classes.
* **runtimeproj** - dll, used at runtime to generate "runnable" plugins containing user system code, not built as part of the solution.
* **systemsplugin1, 2** - dlls, sample systems built to ensure there are no system syntax and structure regressions - will be added to the test suite in the future.
* **tests** - unit and integration tests.
* **tradery** - static library, contains the entry point to the TE, provides high level APIs used to run sessions.
* **traderyconsole** - exe, tradery.exe command line utility

Projects are set to use the most recent set of C++ specs supported by the Visual Studio compiler.

# Session
In the context of the TE, "Tradery session", "session" or "run" refer to one run of the TE over one set of input parameters resulting in
 either success and a set of output data, or a failure.

# Configuration
To run a successful session, the TE must receive various input parameters such as build environment paths (lib, include and path locations), 
session specific environment paths (system code file location, data location, output directory), position sizing, and others.

These parameters are passed as one string with the format:
```
-short_param_name1 [arg1] --long_param_name2 [arg2] ...
```

Where short parameter names are one character long preceded by "-" and long parameter names are two or more characters long preceded by "--". 

Names are, depending on parameter, optionally followed by one argument each. 

The argument string should be surrounded by quote marks if it contains spaces.

Some parameters are optional, others are mandatory, and yet others can appear more than once.

A configuration string can be passed to the console utilty on the command line, or programmatically which is the case of tests for instance. 

Some or all of these parameters can be stored in configuration files whose path can then be passed as a configuration argument, which makes it easier to reuse
sets of parameters that do not change frequently between sessions such as environment parameters.

An example of a configuration string pointing to two configuration files.
```
--configfile "c:\tradery\some_config_file.txt" --configfile "c:\tradery\some_other_config_file.txt"
```
Configuration files containing various environment parameters are generated automatically during a build under "tradery\config" and are used by tests 
and can be used when running tradery.exe from the command line. They are different for 32 and 64 bit.

When running tests, all other parameters are automatically generated at runtime and programmatically passed to the TE.

When running the tradery.exe console application, all parameters need to be specified on the command line. The environment configuration file can be passed
using the --configfile parameters, but other parameters need to be explicitly passed by the user (they can also be stored in configuration files)

# The Systems Test Project
To view the available tests expand the green test icon in the Test Explorer window.

The main test is called "Systems Tests" and its purpose is to verify that all components of the TE work properly. 

Technically speaking this is an "integration test" which validates the application end-to-end, as opposed to "unit tests" which verify that various classes or individual components
perform as expected when run in isolation.

Both types of tests can be used to ensure the application performs as expected in the case of a new installation for example, 
and/or to quickly detect regression bugs in case of new code changes.

"Systems Test" is a data driven test: test cases are not hard coded but they are defined by files contained in the "tradery\src\Tests\TestData" directory. 
Data driven tests allows adding, modifying or removing test cases without having to modify and/or recompile the code.

Test case files contain sample market data, trading system code, configuration parameters and expected results. "Systems Test" understands their structure and
format and uses them to create and run sessions.

Each test case run will generate a set of session result files which are stored in the session output directory, 
by default located under <windows_user_directory>\AppData\Local\Temp\tradery (such as c:\)

In general, the output directory can be set in the session configuration parameters.

The test will also generate log statements that are sent to the Visual Studio test output window, which can be used to debug the application in case of failure, or 
simply examine what happens during a run.

Under the output directory there will be a number of subdirectories with UUID names (for instance "37076bbc-0c54-4262-b44d-6254bdbdf65e"), each associated with a session,
 containing files such as the the trading system code binaries, the list of generated trades, trading signals, statistics, charts etc and others.

At the end of a each test case session, the Systems Test compares the output files with a set of files containing the expected results, 
located in the TestData directory, and if they are identical, it declares the test a success.

The reference (expected) output data was obtained by running the exact same test cases on a local, functional version of the old tradery.com online service.

### Single-threaded vs multi-threaded test cases
There are currently several test cases that are run once using with one thread, and again with multiple threads. 

The results generated by the former are fully deterministic: each run will generate the exact same output data.
 This is useful to verify that the internal logic is valid and that the output data is generated correctly.

In the case of multi-threaded runs, some of the results depend on thread timings and are not guaranteed identical between sessions, which 
is in fact allowed by design, to provide a level of randomness that could make tests more realistic. 
Therefore only the results that are guaranteed deterministic are used for multi-threaded test validation.

Together, these two types of test provide a comprehensive validation of the entire TE functionality.

Please note that the output directories and data can take a fair amount of storage each (up to several hundred Mb per session), 
and since each test session creates new directories and new data, the amount of storage used can grow quite a bit after a while.


# Running the application from the command line
The solution project traderyconsole generates the tradery.exe command line utilty in the solution output directory tradery\src\bin\<Debug|Release>\<x86|x64>.

This utility provides the same functionality and results as the "Systems Test" described previously, with the difference that 
the user needs to specify all the session parameters such as the trading system code file and data location, and others.

To run a sample tradery.exe session, open a console windows, cd to one of the output directories (for instance "\<repo_root\>\src\bin\release\x64", 
assuming this combination was previously built) and run the following sample command line, which is identical functionally to one of the test cases in "Systems Tests".

```
tradery.exe -b  "c:\Users\<user_name>\AppData\Local\Temp\<temp_directory>"  -a  "<repo_root>\src\tests\TestData\data"  --configfile   "<repo_root>\src\tests\TestData\config\config_common.txt"  --configfile   "<repo_root>\src\..\config\env_<x86|x64>.txt"  -i  "${SessionPath}"  --configfile   "<repo_root>\src\tests\TestData\possizing\possizing1.txt"  -Q  "0.2"  -M  "5"  -y  "<repo_root>\src\tests\TestData\symbols\symbols1.txt"  --symchartfile  "<repo_root>\src\tests\TestData\symbols\symbols1.txt"  -R  "1/12/2016"  -T  "6/12/2016"  -u  "<repo_root>\src\tests\TestData\systems\SMA System with Pullback.txt"  --threads  "4"
```

A few comments regarding this command line:
1. the "-b" parameter sets the session output path, in this example to a directory under Temp (same as the test) - make sure to replace the parts between <> with actual values.
2. all other directories are located under the "repo_root" directory (this is where the project repository was initially cloned) - make sure to substitute the string between \<> with actual values.
3. the configuration files containing environment variables are different for 32 and 64 bit versions, so replace the value between \<> with "x86" for 32 bit and "x64" for 64 bit.
4. the --threads parameter specifies how many parallel threads should the session be run in, potentially increasing the processing speed,
as the application will attempt to run each thread on a separate processor.


To get more information about the various parameters supported by tradery.exe run:

```
tradery.exe --help
```

which will list all the supported arguments with a brief description for each of them.

It is also possible to read parameters from one or more configuration files instead of having to create long command lines containing all
necessary parameters.


# Plugins
The TE uses a plugin based architecture, where most of the work is performed by components (plugins) discovered and loaded dynamically at runtime, 
based on availability and configuration parameters.

Plugins are implemented as DLLs exporting certain APIs that allow them to be discovered and used.

Each plugin is identified by a UUID.

The work is coordinated by a core module, which receives commands via API calls, and distributes and schedules 
tasks to be performed to the different plugins.

Typical plugins:

- data source, which loads data to be processed,
- symbols source, which retrieves lists of symbols to be processed,
- runnable, which contains user code as compiled binary components.
- commission and slippage

Most plugins are statically generated, meaning they are compiled beforehand as they do not change often, and loaded at runtime by the TE.

Runnable plugins however, which contains the compiled user code, can be different from one session to the next, 
and so they are dynamically generated at the start of each session.

The TE accomplishes this by using an internal code generator which takes the user trading system, translates it into the complete runnable plugin C++ project, 
and then builds the runnable plugin binary.

# Trading system definition language
The trading system definition language (TSDL) used by the TE is based on C++, although with many simplifying primitives that should allow 
users of any programming skill level to write their own code.

The user code has a specific structure and syntax that is translated internally into C++ code, which is then compiled to generate 
the runnable binary.

Here's an example of a typical system:

``` C++
void run() {
  installTimeBasedExit(2);

  Series SMAclose2 = closeSeries().SMA(2);
  Series SMAclose5 = closeSeries().SMA(5);
  Series SMAclose30 = closeSeries().SMA(30);
  double lc = 0.93;
  double sc = 0.93;

  for (Index bar = 1; bar < barsCount(); bar++) {
    applyAutoStops(bar);

    for (Position pos = getFirstOpenPosition(); pos;
         pos = getNextOpenPosition()) {
      double posPrice = pos.getEntryPrice();

      if (open(bar + 1) > posPrice AND pos.isLong())
        sellAtMarket(bar + 1, pos, "Sell at Market");

      if (open(bar + 1) < posPrice AND pos.isShort())
        coverAtMarket(bar + 1, pos, "Cover at Market");
    }

    if (SMAclose5[bar] < lc * SMAclose30[bar]) {
      for (Index j = 1; j < 31; j++) {
        double lf = 0.75 - (j * 0.005);
        double lp = SMAclose2[bar] * lf;
        buyAtLimit(bar + 1, lp, 1000, "BuyAtLimit");
      }
    }

    if (SMAclose5[bar] > sc * SMAclose30[bar]) {
      for (Index j = 1; j < 31; j++) {
        double sf = 1.25 + (j * 0.005);
        double sp = SMAclose2[bar] * sf;
        shortAtLimit(bar + 1, sp, 1000, "ShortAtLimit");
      }
    }
  }
}

```

The solution projects systemsplugin1 and systemsplugin2 contain samples that illustrate various
techniques and APIs used to create trading systems.

# Back testing realism
By default, trading systems will run assuming ideal market behavior which can lead to unrealistic, usually overly optimistic results.

For more realistic simulations, settings for position sizing, commission costs and slippage need to be used.

### Slippage
Slippage introduces an uncertainty factor used to model more realistic market behavior. Generally, slippage acts against the investor.

For example, in case of a buy at limit order, in the ideal case, if the price reaches the limit level or lower, 
the order would always be filled. However, in reality it is possible that the limit or lower price was reached for 
a very short period of time, and due to lack of liquidity, only a limited number of shares traded at that price, which then went up above the limit.
 In that case, the order may not have been filled, resulting in a discrepancy between simulated and real results.

Slippage, in its simplest form, is implemented as a percentage of the price which is added to or subtracted from the limit, stop or other
parameters of the orders in the direction that would make the behavior less favorable to the trader, making it harder to fill these orders.

In the example above, if slippage is 0.5%, then the price would have to drop by 0.5% under the limit price for the 
order to be filled. The result of this can be easily seen by running a simulation with and without slippage, leading to drastically different results:
most systems' performance drops significantly, sometimes turning a wildly profitable system into a losing one.

Use values between 0-1%, depending on how liquid the symbols are.

# Market Data
The project contains several data for a few symbols that is several years old, used for tests only.

To run the TE on other symbols and for more recent periods of time, the user will have to provide their own data.

A free tool that can be used for 
that purpose can be found at https://www.yloader.com, and the source code at https:://github.com/adrianmichel/yloader.

Go to \<repo_root>\tests\TestData\data to view the sample market data format and structure expected by the trading engine.

### Data Format
Market data consists of CSV files containing bar price and volume numbers, one per symbol. By default data is assumed to 
contain End Of Day (EOD) values, but intra-day data could be potentially used as well (no testing has been done for that).

Data also needs to be dividend and split adjusted, or the systems will yield inaccurate results.

Each line consists of data in the following format:
```
Date,Open,High,Low,Close,Volume
```
where Date is in `mm/dd/yyyy` format.

Example:
```
1/27/2020,73.550003,74.169998,73.169998,73.849998,19565100
1/28/2020,73.540001,73.970001,73.309998,73.870003,10883800
1/29/2020,73.940002,74.540001,73.879997,74.489998,17079100
2/2/2020,74.709999,75.010002,74.300003,74.610001,15304800
2/3/2020,74.669998,74.879997,74.190002,74.260002,12190400
2/4/2020,74.089996,74.720001,73.709999,74.690002,13317700
```

Data needs to be dividend and split adjusted, although in principle, if dividend and split information could be provided separately, the data could 
also be unadjusted.

### Data Location
Data files are located under the directory specified in the configuration file by the argument -a or --datasourcepath.

As the number of data files can be quite large, they are organized in a
directory tree structure, where the first level consists of directories named after the first character of the symbol, and the second level 
are directories named after the second
character of the symbol name (if any).

For instance, if the symbols are MIB, MSB, MSFT, TI, TIBX, the structure is:
```
<data_dir>
    - M (d)
        - I (d)
            - MIB.csv (f)
        - S (d)
            - MSB.csv (f)
            - MSFT.csv (f)
    - T (d)
        - I (d)
            - TI.csv (f)
            - TIBX.csv (f)
```

### Imperfect Data
Many free and even paying data sources provide imperfect historical data.

Data can contain:
- missing bars
- wrong price values
- wrong volume values

TE can be configured to trigger warnings or errors if it detects invalid data, however, not all bad data can be detected, 
so make sure that data is valid before feeding it to the TE or it may generate misleading results (GIGO).

## Charting
TE can use the optional ChartDirector library to generate charts (see _3rd Party Components_ section for details). 

If the library is present at the location indicated in the installation section, the project will link with that library during build 
and generate an equity curve chart for each session, which can be quite useful to assess the performance and risk of a system at a glance.

If you decide to use this library, make sure you comply fully with the ChartDirector license and terms of use.

TE generates the equity curve and associated statistics chart, price/volume charts for each symbol, as well
as user defined charts based on various indicators or other time series. For the latter, TSDL provides a number of primitives that allow users to define additional panes parallel to
the main symbol price pane, and what to display in these panes and how.

TE generates directly only the equity curve chart, which is a png file found in the session directory upon a successful run.

To generate the symbol price and additional user defined charts, the TE shares the work with the module that will actually display these charts. 

First, during a session, TE saves a number of files 
with all the information describing the charts, panes, values and other attributes as XML files under the _session/charts_ directory. 

After the session is complete, these files are used by the UI module to actually display the charts to the user.

The reason for this split is that ChartDirector has the ability to generate financial charts and manipulate them at viewing time based on user commands (zoom in/out, scroll etc), 
and after each command, a new chart is generated, corresponding to the area being displayed and its level of zoom.

# Logging
The configuration contains a parameter for specifying the name of the session log file. Each session will generate a log file
inside the session directory.

# Security
This project was originally run as a Windows service on a server for the tradery.com online application. 

As it allows running user defined C++ code on the server, security is a very important aspect.

The first line of defense is to impose certain limitations on users' system scripts. For example, no preprocessor commands can be
used (#define, #include etc), there is no access to system functions, and other features of the C++ language are hidden or disabled as well.

However, the assumption should be that these limitations won't be enough to stop determined and inventive hackers, who could and likely 
will come up with ways to circumvent them.

The second line of defense is to ensure that if a weakness has been found, exploiting it won't give access to any critical data and resources.

This is best done by running the executable on a dedicated server or VM with no access to critical data, either locally or over the network, and/or from an account with 
very limited permissions, just enough to allow it to read the data and generate the output files.

# Disclaimer
This project allows users to define algorithmic trading systems as code, to back-test them and to generate trading signals.

**Many trading systems that seem profitable on paper will lose money when used for trading with real money, therefore Adrian Michel recommends you do not use any of the sample
 trading systems included in this project, or the data generated by running the project built binaries, 
for any trading or investment decisions with real money.**

**Any action you take upon the information you acquire from this project or by running the binaries generated by it, is at your own risk, and 
Adrian Michel will not be liable for any losses or damages in connection with their use**.

# License
   Copyright (C) 2018-2020 Adrian Michel

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this software except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

# 3rd party components and applications
Tradery Trading Engine uses several 3rd party components:

- **TA-Lib**, a technical analysis indicators library https://ta-lib.org
- **boost**, a set of portable C++ libraries: https://www.boost.org
- **JSON for modern C++**, a JSON C++ library: https://github.com/nlohmann/json
- **tree.hh**, a tree class: http://tree.phi-sci.com/
- **Chart Director**, optional, a charting library: https://www.advsofteng.com. This is a commercial package that requires a paid license for commercial use. The evaluation version will create 
charts with a footer text indicating that they were generated with an unlicensed version of the software. If you have a license code and want to set it in the TE, 
look for the line containing "Chart::setLicenseCode" in the solution, copy the code as indicated, and uncomment the line.

Also, the following applications have been used during development:

* **Visual Studio 2019 Community Edition**
* **Git Command Line Client for Linux**: https://git-scm.com/downloads.
* **VirtualBox**: https://www.virtualbox.org, hosted on Windows
* **Ubuntu** running as guest on VirtualBox, used for general file and git operations.
