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
#include <traderyconnection.h>
#include <traderyapi.h>
#include <stringformat.h>

#define DEFAULT_TRADERY_SERVER "www.tradery.com"

std::wstring _traderyServer(DEFAULT_TRADERY_SERVER);

const std::wstring& traderyServer() { return _traderyServer; }

void setTraderyServer(const std::wstring& server) { _traderyServer = server; }

Response::Response(tradery::StringPtr response)
    : m_code(-1), m_subcode(-1), m_response(response), m_timeStamp(0) {
  try {
    assert(response);

    std::wistringstream is(*response);

    Json::Value json;

    is >> json;

    if (json.isObject()) {
      Json::Value jsonResponse = json["response"];

      if (!jsonResponse.isNull()) {
        Json::Value code = jsonResponse["code"];
        assert(code.isInt());

        m_code = code.asInt();

        Json::Value timeStamp(jsonResponse["timestamp"]);
        assert(timeStamp.isInt());

        m_timeStamp = timeStamp.asInt();

        if (!jsonResponse["subcode"].isNull()) {
          assert(jsonResponse["subcode"].isInt());
          m_subcode = jsonResponse["subcode"].asInt();
        }

        if (!jsonResponse["message"].isNull()) {
          assert(jsonResponse["message"].isString());
          m_message = jsonResponse["message"].asString();
        }
      } else {
#pragma message( \
    __TODO__     \
    "handle response error - the json response should always have a \"response\" element")
      }

      m_result = new Json::Value(json["result"]);
    } else
      throw ResponseException(std::wstring("Invalid response: ") << *response);
  } catch (const std::runtime_error& e) {
    throw ResponseException(tradery::format("Error: \"%1%\", response: \"%2%\"",
                                            e.what(), *response));
  }
}

TraderyRequest::TraderyRequest() : m_http(traderyServer(), false) {}

void TraderyRequest::processResponse(tradery::StringPtr response) {
  try {
    m_response = new Response(response);

    LOG(log_debug, _T( "response: " ) << *response);

    switch (m_response->code()) {
      case code_success:
        break;
      case code_auth_failure: {
        switch (m_response->subcode()) {
          case invalid_credentials:
            throw TraderyRequestException(m_response->message());
          case invalid_session_id:
            // ignore server message, the app knows better what to say.
            throw TraderyRequestException(
                "Could not authenticate Tradery user. Check your Tradery user "
                "name and password and try again.",
                INVALID_SESSION_ID);
        }
      } break;
      case code_data_update_failure:
        throw TraderyRequestException(m_response->message());
        break;
      case code_unsupported_api:
        throw TraderyRequestException(m_response->message());
        break;
      default:
        throw TraderyRequestException(m_response->message());
        break;
    }
  } catch (const ResponseException& e) {
    throw TraderyRequestException(e.message());
  }
}

void TraderyRequest::get(const TraderyAPI& data) {
  try {
    tradery::StringPtr response(m_http.get(data.object(), data));

    processResponse(response);
  } catch (const InternetException& e) {
  // there has been some curl or http error
#pragma message(__TODO__ "handle exception and errors(curl, http, json")

    if (e.errorType() == REQUEST_CANCELED)
      throw TraderyRequestException(e.message(), REQUEST_CANCELED);

    if (e.curlError())
      throw TraderyRequestException(e.message(), CURL_ERROR);
    else if (e.httpError())
      throw TraderyRequestException(e.message(), HTTP_ERROR);
    else
      throw TraderyRequestException(e.message(), OTHER_ERROR);
  }
}

void TraderyRequest::get(const std::wstring& data) {
  tradery::StringPtr response;

  try {
    response = m_http.get(data);

    processResponse(response);

  } catch (const InternetException& e) {
  // there has been some curl or http error
#pragma message(__TODO__ "handle exception and errors(curl, http, json")

    if (e.errorType() == REQUEST_CANCELED)
      throw TraderyRequestException(e.message(), REQUEST_CANCELED);

    if (e.curlError())
      throw TraderyRequestException(e.message(), CURL_ERROR);
    else if (e.httpError())
      throw TraderyRequestException(e.message(), HTTP_ERROR);
    else
      throw TraderyRequestException(e.message(), OTHER_ERROR);
  }
}

void TraderyRequest::post(const TraderyAPI& data) {
  tradery::StringPtr response;
  try {
    response = m_http.post(data.object(), data);

    processResponse(response);

  } catch (const InternetException& e) {
  // there has been some curl or http error
#pragma message(__TODO__ "handle exception and errors(curl, http, json")

    if (e.errorType() == REQUEST_CANCELED)
      throw TraderyRequestException(e.message(), REQUEST_CANCELED);

    if (e.curlError())
      throw TraderyRequestException(e.message(), CURL_ERROR);
    else if (e.httpError())
      throw TraderyRequestException(e.message(), HTTP_ERROR);
    else
      throw TraderyRequestException(e.message(), OTHER_ERROR);
  }
}

TraderyRequestGet::TraderyRequestGet(const TraderyAPI& args) {
  __super::get(args);
}

TraderyRequestGet::TraderyRequestGet(const std::wstring& query) {
  __super::get(query);
}

TraderyRequestPost::TraderyRequestPost(const TraderyAPI& data) {
  LOG(log_debug, *data.toString());
  __super::post(data);
}

void TraderyConnectionId::refresh() {
  try {
    AuthenticateAPI auth(m_authToken.traderyUserName(),
                         m_authToken.traderyPassword());
    m_request = TraderyRequestPtr(new TraderyRequestPost(auth));
    JsonValuePtr root(m_request->getResponseAsJson());

    assert(root);

    Json::Value idValue = (*root)["id"];

    std::wstring::operator=(idValue.asString());

    saveCurrentToken();

    // handle autherror result
  } catch (const TraderyRequestException& e) {
    throw TraderyConnectionException(e.message());
  }
}

tradery::Mutex TraderyConnection::_mx;

JsonValuePtr TraderyConnection::request(const TraderyAPI& api,
                                        unsigned int retries) {
  tradery::Lock lock(_mx);
  if (!m_id.isValid()) m_id.refresh();

  if (!m_id.isValid())
    throw TraderyConnectionException(
        "Could not authenticate the tradery account");

  TraderyAPI tempAPI(api);
  tempAPI.setId(m_id);

  try {
    m_request = TraderyRequestPtr(new TraderyRequestPost(tempAPI));
    m_response = m_request->response();

    // convert to json
    return m_response->result();
  } catch (const TraderyRequestException& e) {
    if (e.code() == INVALID_SESSION_ID) {
      m_id.invalidate();
      if (retries > 0)
        return request(tempAPI, --retries);
      else {
        std::wstring errorMessage("Invalid Tradery session id: ");
        errorMessage += e.message();
        LOG(log_error, errorMessage);
        // could not get a valid session id (auth error likely)
        throw TraderyConnectionException(errorMessage);
      }
    } else {
      LOG(log_error, e.message());
      throw TraderyConnectionException(e.message());
    }
  }
}

void TraderyConnection::cancel() {
  m_id.cancel();
  if (m_request) m_request->cancel();
}
