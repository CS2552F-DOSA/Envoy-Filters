#pragma once

#include <string>

#include "envoy/server/instance.h"

#include "server/config/network/http_connection_manager.h"

namespace Envoy {
namespace Server {
namespace Configuration {

class HttpSampleDecoderFilterConfig: public HttpFilterConfigFactory {
public:
  HttpFilterFactoryCb tryCreateFilterFactory(HttpFilterType type, const std::string& name,
                                            const Json::Object& json_config,
                                            const std::string& stats_prefix,
                                            Server::Instance& server) override;
};

typedef std::shared_ptr<HttpSampleDecoderFilterConfig> HttpSampleDecoderFilterConfigSharedPtr;

} // Configuration
} // Server
} // Envoy

