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

using JsonValuePtr = std::shared_ptr<Json::Value>;

BASIC_EXCEPTION(ResponseException)

class _NULL {};

class TraderyCredentials {
 private:
  std::wstring m_userName;
  std::wstring m_password;

 public:
  TraderyCredentials() {}

  TraderyCredentials(const std::wstring& userName, const std::wstring& password)
      : m_userName(userName), m_password(password) {}

  const std::wstring& traderyPassword() const { return m_password; }
  const std::wstring& traderyUserName() const { return m_userName; }
  void setTraderyUserName(const std::wstring& userName) {
    m_userName = userName;
  }
  void setTraderyPassword(const std::wstring& password) {
    m_password = password;
  }
};

typedef std::shared_ptr<TraderyCredentials> TraderyCredentialsPtr;

class TraderyAuthToken : public TraderyCredentials {
 private:
  std::wstring m_sessionToken;

 public:
  TraderyAuthToken() {}

  TraderyAuthToken(const TraderyCredentials& credentials)
      : TraderyCredentials(credentials) {}

  TraderyAuthToken(const std::wstring& userName, const std::wstring& password)
      : TraderyCredentials(userName, password) {}

  TraderyAuthToken(const std::wstring& userName, const std::wstring& password,
                   const std::wstring& sessionToken)
      : TraderyCredentials(userName, password), m_sessionToken(sessionToken) {}

  const std::wstring& getTraderyAuthToken() const { return get(); }

  const std::wstring& get() const { return m_sessionToken; }

  void setTraderyAuthToken(const std::wstring& token) {}

  void set(const std::wstring& token) { m_sessionToken = token; }

  void clearTraderyAuthToken() { clear(); }

  void clear() { m_sessionToken.clear(); }
};

typedef std::shared_ptr<TraderyAuthToken> TraderyAuthTokenPtr;

#include "traderyapi.h"
//#include "sessions.h"

enum TraderyRequestErrorCode {
  // tradery errors (0-99)
  INVALID_SESSION_ID,
  RESPONSE_ERROR,
  REQUEST_CANCELED,
  UNKNOWN_ERROR,

  // curl and http errors
  CURL_ERROR = 100,
  HTTP_ERROR,
  OTHER_ERROR
};

BASIC_EXCEPTION(TraderyRequestExceptionBase)

class TraderyRequestException : public TraderyRequestExceptionBase {
 private:
  const TraderyRequestErrorCode m_code;

 public:
  TraderyRequestException(const std::wstring& message,
                          TraderyRequestErrorCode code = UNKNOWN_ERROR)
      : TraderyRequestExceptionBase(message), m_code(code) {}

  const TraderyRequestErrorCode code() const { return m_code; }
};

enum ResponseCode {
  code_success = 0,
  code_auth_failure = 1,
  code_parameter_error = 2,
  code_data_update_failure = 3,
  code_unsupported_api = 4,

  code_other_failure = 1000
};

enum AuthFailureSubcode { invalid_credentials = 0, invalid_session_id = 1 };

class Response {
 private:
  tradery::StringPtr m_response;

  int m_code;
  int m_subcode;
  std::wstring m_message;

  unsigned __int64 m_timeStamp;

  JsonValuePtr m_result;

 public:
  Response(tradery::StringPtr response);

  unsigned int code() const { return m_code; }
  bool hasSubcode() const { return m_subcode >= 0; }
  unsigned int subcode() const { return m_subcode; }
  const std::wstring& message() const { return m_message; }
  tradery::StringPtr response() const { return m_response; }
  unsigned __int64 timeStamp() const { return m_timeStamp; }
  JsonValuePtr result() const { return m_result; }
};

typedef std::shared_ptr<Response> ResponsePtr;

class TraderyRequest {
 private:
  ResponsePtr m_response;
  CurlHTTPRequest m_http;

 private:
  void TraderyRequest::processResponse(tradery::StringPtr response);

 protected:
  TraderyRequest();

  void get(const TraderyAPI& arguments);
  void get(const std::wstring& data);
  void post(const TraderyAPI& data);

 public:
  operator JsonValuePtr() const { return m_response->result(); }

  JsonValuePtr getResponseAsJson() const { return m_response->result(); }

  tradery::StringPtr getResponseAsString() const {
    return m_response->response();
  }

  ResponsePtr response() const { return m_response; }

  void cancel() { m_http.cancel(); }
};

typedef std::shared_ptr<TraderyRequest> TraderyRequestPtr;

class TraderyRequestGet : public TraderyRequest {
 public:
  TraderyRequestGet(const std::wstring& query);
  TraderyRequestGet(const TraderyAPI& args);
};

class TraderyRequestPost : public TraderyRequest {
 public:
  TraderyRequestPost(const TraderyAPI& api);
};

// session id
class TraderyConnectionId : public std::wstring {
 private:
  TraderyAuthToken& m_authToken;
  TraderyRequestPtr m_request;

  void saveCurrentToken() { m_authToken.set(*this); }

 public:
  TraderyConnectionId(TraderyAuthToken& authToken)
      : m_authToken(authToken), std::wstring(authToken.get()) {}

  bool isValid() const { return !__super::empty(); }

  void refresh();

  void invalidate() { __super::clear(); }

  void cancel() {
    if (m_request) m_request->cancel();
  }
};

class TraderyConnectionException {
 private:
  const std::wstring m_message;

 public:
  TraderyConnectionException(const std::wstring& message)
      : m_message(message) {}

  const std::wstring& message() const { return m_message; }
};

class TraderyConnection {
  TraderyConnectionId m_id;

  ResponsePtr m_response;
  TraderyRequestPtr m_request;

#pragma message( \
    todo         \
    "experimenting with serialized calls to tradery, to see if this helps with thread errors")
  // this a global mutex to ensure api calls to tradery are serialized
  static tradery::Mutex _mx;

 public:
  JsonValuePtr request(const TraderyAPI& args, unsigned int retries = 1);

  TraderyConnection(TraderyAuthToken& authToken) : m_id(authToken) {
    if (authToken.traderyPassword().empty() ||
        authToken.traderyUserName().empty())
      throw TraderyConnectionException("Tradery user name or password not set");
  }

  tradery::StringPtr responseAsString() const {
    if (m_response)
      return m_response->response();
    else
      return tradery::StringPtr();
  }

  unsigned __int64 responseTimeStamp() const {
    assert(m_response);

    if (m_response)
      return m_response->timeStamp();
    else
      return 0;
  }

  void cancel();
};

typedef std::shared_ptr<TraderyConnection> TraderyConnectionPtr;

const std::wstring& traderyServer();
void setTraderyServer(const std::wstring& server);
