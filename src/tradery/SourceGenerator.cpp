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
#include "SourceGenerator.h"
#include <resourcewrapper.h>
#include "Resource.h"
#include "System.h"
#include "res\cfoot.h"
#include "res\chead.h"
#include "res\init.h"

/*
sample file

#pragma once
using namespace tradery;


<<< HEADER >>>

//*********************************************
#undef SYSTEM_ID
#define SYSTEM_ID "9259a413-071b-9e23-0bad-c98562716758"


#pragma message( "#systemName=SMA System with Pullback" )
#pragma message( "#className=System_9259a413_071b_9e23_0bad_c98562716758" )
class System_9259a413_071b_9e23_0bad_c98562716758 : public BarSystem<
System_9259a413_071b_9e23_0bad_c98562716758 >
{
public: System_9259a413_071b_9e23_0bad_c98562716758(const std::vector< t_string
>* params = 0) : BarSystem< System_9259a413_071b_9e23_0bad_c98562716758
>(Info("9259a413-071b-9e23-0bad-c98562716758", _TT("SMA System with Pullback"),
_TT("Inspired by OscillatorPullBack by .toe in Wealthlab.")), "385")
{}
private:
#include <9259a413-071b-9e23-0bad-c98562716758.h>
};

#define PLUGIN_INIT_METHOD \
virtual void init() \
{ \
                insert< System_9259a413_071b_9e23_0bad_c98562716758 >(); \
}

<< < FOOTER >> >
*/

SourceGenerator::SourceGenerator(const TradingSystems& systems)
    : systems(systems) {}

SourceGenerator::~SourceGenerator() {}

std::string SourceGenerator::generate() {
  std::string header = HEADER;
  LOG(log_info, "----------------- header:\n", header);
  std::string footer = FOOTER;
  LOG(log_info, "----------------- footer:\n", footer);
  std::string init = INIT;
  LOG(log_info, "----------------- init:\n", init);

  std::string code;
  std::string inserts;

  code += header;

  //	std::cout << "-----------------" << std::endl << *code;

  for (auto system : systems ){
    code += system.generateClass();
    inserts += system.generateInsert();
  }
  // std::cout << "-----------------" << std::endl << *code;

  code += boost::replace_all_copy(std::string(init), MACRO(INSERTS), inserts);
  // std::cout << "-----------------" << std::endl << *code;
  code += footer;
  // std::cout << "-----------------" << std::endl << *code;

  return code;
}
