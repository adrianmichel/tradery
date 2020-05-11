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

#include "stdafx.h"
#include "TraderyProcess.h"
#include "RunnablePluginBuilder.h"
#include "BuildErrorsParser.h"
#include "SourceGenerator.h"

void build_path(string& path, const std::vector<std::string> paths,
                const std::string& type) {
  for_each(paths.begin(), paths.end(),
           [&](const std::string& p) { path += type + "\\\"" + p + "\\\" "; });
}

#if defined( _WIN64)
#define TARGET "X64"
#else
#define TARGET "X86"
#endif

#if defined( _DEBUG )
#define CONFIGURATION "debug"
#define CFG_STRING "CFG=Debug"
#else
#define CONFIGURATION "release"
#define CFG_STRING ""
#endif

RunnablePluginBuilder::RunnablePluginBuilder( const Configuration& config, bool& _cancel)
    : _exitCode(0) {

  const TradingSystems& systems = config.getSystems();
  SourceGenerator gen(systems);
  std::string source = gen.generate();
  std::string sessionPath = config.getSessionPath();
  std::string sourceFile = ws2s(Path{ sessionPath }.makePath("defines.h").c_str());

  {
    std::ofstream ofs(sourceFile);
    if (ofs.is_open()) {
      ofs << source;
    }
    else {
      throw RunnablePluginBuilderException("Could not save system source file: "s + sourceFile);
    }
  }

  // a local txt errors file. This is just for trace purposes, to see the
  // actuall compiler errors a sanitized html file will be generated
  std::string errorsFile = tradery::ws2s( Path{ sessionPath }.makePath( "errs.txt" ).c_str() );

  string libpath;
  string includepath;

  build_path(libpath, config.libPath(), "/LIBPATH:");
  build_path(includepath, config.includePaths(), "/I ");

  std::ostringstream _cmdLine;
  _cmdLine << "/B /f \"" << addFSlash(config.projectPath())
           << "makefile.mak\" "
           << "INCLUDEPATH=\"" << includepath << "\" "
           << "LIBPATH=\"" << libpath << "\" "
           << "OUTDIR=\"" << removeFSlash(sessionPath) << "\" "
           << "INTDIR=\""
           << (addFSlash(config.outputPath()) + "common\\" CONFIGURATION "\\" TARGET )
           << "\" "
           << "PROJDIR=\"" << config.projectPath() << "\" "
           << "BUILDERRORSFILE=\"" << errorsFile << "\" "
           << "TOOLSPATH=\"" << config.toolsPath() << "\" "
           << "TARGET=" TARGET << " "
           << CFG_STRING << " "
           << " /X \"c:\\dev\\make_output.txt\""
      ;

  LOG(log_debug, config.getSessionId(), " make cmd line:\n", _cmdLine.str());

  Environment env(*config.getEnvironment());

  env.add("TEMP", config.outputPath());

  LOG(log_debug, config.getSessionId(), "environment:\n", env.toString());

  // log the entire nmake command line for diagnostic purposes
  std::ostringstream cmd;
  cmd << "\""s << ws2s( Path(config.toolsPath()).makePath("nmake.exe").c_str() ) << "\" " << _cmdLine.str();
  LOG(log_debug, "build command line: ", cmd.str() );

  const ProcessResult pr(process(config, _cancel, addFSlash(config.toolsPath()) + "nmake.exe",
        _cmdLine.str(), std::make_shared< std::string >(addFSlash(config.outputPath())).get(), env));

  _exitCode = pr.exitCode();
  LOG(log_debug, config.getSessionId(), " [RunnablePluginBuilder constr] - exit code: ", _exitCode);

  ifstream ifs(errorsFile.c_str());

  if (ifs) {
    LOG(log_debug, config.getSessionId(), " reading parsed errors");
    BuildErrorsParser parser(ifs, config);

    ifs.close();

    ofstream ofs(config.errorsFile().c_str());
    if (ofs) {
      LOG(log_debug, config.getSessionId(), " writing parsed errors");
      parser.write(ofs, true, 5);
    }
    else {
      throw RunnablePluginBuilderException("Could not open errors file for writing: "s + config.errorsFile());
    }
  }
}
