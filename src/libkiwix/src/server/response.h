/*
 * Copyright 2019 Matthieu Gautier<mgautier@kymeria.fr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU  General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */


#ifndef KIWIXLIB_SERVER_RESPONSE_H
#define KIWIXLIB_SERVER_RESPONSE_H

#include <string>
#include <map>

#include <mustache.hpp>
#include "byte_range.h"
#include "etag.h"
#include "i18n_utils.h"

#include <zim/item.h>

extern "C" {
#include "microhttpd_wrapper.h"
}

namespace zim {
class Archive;
} // namespace zim

namespace kiwix {

class RequestContext;

class Response {
  public:
    enum Kind
    {
      STATIC_RESOURCE,
      ZIM_CONTENT,
      DYNAMIC_CONTENT
    };

  public:
    Response();
    virtual ~Response() = default;

    static std::unique_ptr<Response> build();
    static std::unique_ptr<Response> build_304(const ETag& etag);
    static std::unique_ptr<Response> build_416(size_t resourceLength);
    static std::unique_ptr<Response> build_redirect(const std::string& redirectUrl);

    MHD_Result send(const RequestContext& request, bool verbose, MHD_Connection* connection);

    void set_code(int code) { m_returnCode = code; }
    void set_kind(Kind k);
    Kind get_kind() const { return m_kind; }
    void set_etag_body(const std::string& id) { m_etag.set_body(id); }
    void add_header(const std::string& name, const std::string& value) { m_customHeaders[name] = value; }

    int getReturnCode() const { return m_returnCode; }

  private: // functions
    virtual MHD_Response* create_mhd_response(const RequestContext& request);
    MHD_Response* create_error_response(const RequestContext& request) const;

  protected: // data
    Kind m_kind = DYNAMIC_CONTENT;
    int m_returnCode;
    ByteRange m_byteRange;
    ETag m_etag;
    std::map<std::string, std::string> m_customHeaders;

    friend class ItemResponse;
};


class ContentResponse : public Response {
  public:
    ContentResponse(
      const std::string& content,
      const std::string& mimetype);

    static std::unique_ptr<ContentResponse> build(
      const std::string& content,
      const std::string& mimetype);

    static std::unique_ptr<ContentResponse> build(
      const std::string& template_str,
      kainjow::mustache::data data,
      const std::string& mimetype);

    const std::string& getContent() const { return m_content; }
    const std::string& getMimeType() const { return m_mimeType; }

  private:
    MHD_Response* create_mhd_response(const RequestContext& request);

    bool can_compress(const RequestContext& request) const;


  private:
    std::string m_content;
    std::string m_mimeType;
 };

class ContentResponseBlueprint
{
public: // functions
  ContentResponseBlueprint(const RequestContext* request,
                           int httpStatusCode,
                           const std::string& mimeType,
                           const std::string& templateStr,
                           bool includeKiwixResponseData = false);

  ~ContentResponseBlueprint();

  operator std::unique_ptr<Response>() const
  {
    return generateResponseObject();
  }

  std::unique_ptr<ContentResponse> generateResponseObject() const;

protected: // types
  class Data;

protected: //data
  const RequestContext& m_request;
  const int m_httpStatusCode;
  const std::string m_mimeType;
  const std::string m_template;
  const bool m_includeKiwixResponseData;
  std::unique_ptr<Data> m_data;
};

struct NewHTTP404Response : ContentResponseBlueprint
{
  NewHTTP404Response(const RequestContext& request,
                     const std::string& root,
                     const std::string& urlPath);
};

struct HTTPErrorResponse : ContentResponseBlueprint
{
  HTTPErrorResponse(const RequestContext& request,
                    int httpStatusCode,
                    const std::string& pageTitleMsgId,
                    const std::string& headingMsgId,
                    const std::string& cssUrl = "",
                    bool includeKiwixResponseData = false);

  HTTPErrorResponse& operator+(const ParameterizedMessage& errorDetails);
  HTTPErrorResponse& operator+=(const ParameterizedMessage& errorDetails);
};

struct HTTP404Response : HTTPErrorResponse
{
  explicit HTTP404Response(const RequestContext& request);
};

struct UrlNotFoundResponse : HTTP404Response
{
  explicit UrlNotFoundResponse(const RequestContext& request);
};

struct HTTP400Response : HTTPErrorResponse
{
  explicit HTTP400Response(const RequestContext& request);
};

struct HTTP500Response : ContentResponseBlueprint
{
  HTTP500Response(const RequestContext& request,
                  const std::string& root,
                  const std::string& urlPath,
                  const std::string& error = "");
};

class ItemResponse : public Response {
  public:
    ItemResponse(const zim::Item& item, const std::string& mimetype, const ByteRange& byterange);
    static std::unique_ptr<Response> build(const RequestContext& request, const zim::Item& item);

  private:
    MHD_Response* create_mhd_response(const RequestContext& request);

    zim::Item m_item;
    std::string m_mimeType;
};

struct BlockExternalLinkResponse : ContentResponseBlueprint
{
  BlockExternalLinkResponse(const RequestContext& request,
                            const std::string& root,
                            const std::string& externalUrl);
};

}

#endif //KIWIXLIB_SERVER_RESPONSE_H
