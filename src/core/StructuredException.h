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

/**
 * Base class for structured exceptions
 *
 * This class is used by specific exceptions that are not handled by the C++
 * exception system, mainly division by 0.
 *
 * This class is not relevant from an API standpoint, and it would have been
 * hidden normally, but because of certain implementation issues related to
 * structured exceptions, this was not possible. A user should only call methods
 * of its base class CoreExceptions.
 *
 * @see AccessViolationException
 * @see DivideByZeroException
 * @see CoreException
 */
class StructuredException : public CoreException {
 public:
  StructuredException(ErrorCode code, EXCEPTION_POINTERS const &) throw();
  static void install() throw();
  virtual std::string what() const throw();
  void const *where() const throw();

 private:
  void const *address_;
  // unsigned code_;
};

/**
 * Structured exception thrown in case of an access violation exception
 *
 * This exception is never thrown in simlib presently and it is here for future
 * use only
 *
 * @see CoreException
 * @see ErrorCode
 */
class AccessViolationExceptionImpl : public StructuredException,
                                     public AccessViolationException {
 public:
  AccessViolationExceptionImpl(EXCEPTION_POINTERS const &) throw();
  virtual std::string what() const throw();

  virtual std::string message() const { return what(); }
};

/**
 * Thrown in case of a divizion by zero error
 *
 * @see CoreException
 * @see ErrorCode
 */
class DivideByZeroExceptionImpl : public StructuredException,
                                  public DivideByZeroException {
 public:
  DivideByZeroExceptionImpl(EXCEPTION_POINTERS const &) throw();
  virtual std::string what() const throw();
  virtual std::string message() const { return what(); }
};
