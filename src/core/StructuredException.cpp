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

#include "structuredexception.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

using namespace std;

using namespace tradery;

static void my_translator(unsigned code, EXCEPTION_POINTERS *info) {
  switch (code) {
    case EXCEPTION_ACCESS_VIOLATION:
      throw AccessViolationExceptionImpl(*info);
    case EXCEPTION_INT_DIVIDE_BY_ZERO:
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:
      _clearfp();
      throw DivideByZeroExceptionImpl(*info);
    default:
      throw StructuredException(UNKNOWN_STRUCTURED_ERROR, *info);
  }
}

StructuredException::StructuredException(ErrorCode code, EXCEPTION_POINTERS const &info) throw()
    : CoreException(code) {
  EXCEPTION_RECORD const &exception = *(info.ExceptionRecord);
  address_ = exception.ExceptionAddress;
  // code_ = exception.ExceptionCode;
}

void StructuredException::install() throw() {
  int _cw;
  _cw = _controlfp(0, 0);

  // Set the exception masks OFF, turn exceptions on.
  int cwx = _cw & ~(EM_ZERODIVIDE);

  // Set the control word.
  _controlfp(cwx, MCW_EM);

  _set_se_translator(my_translator);
}

std::string StructuredException::what() const throw() {
  return "Unspecified Structured Exception";
}

void const *StructuredException::where() const throw() { return address_; }

//
//  AccessViolation::
//
AccessViolationExceptionImpl::AccessViolationExceptionImpl(EXCEPTION_POINTERS const &info) throw()
    : StructuredException(ACCESS_VIOLATION_ERROR, info) {}

std::string AccessViolationExceptionImpl::what() const throw() {
  return "Access violation exception";
}

//
//  DivideByZero::
//
DivideByZeroExceptionImpl::DivideByZeroExceptionImpl(EXCEPTION_POINTERS const &info) throw()
    : StructuredException(DIVIDE_BY_ZERO_ERROR, info) {}

std::string DivideByZeroExceptionImpl::what() const throw() {
  return "Divide by zero exception";
}
