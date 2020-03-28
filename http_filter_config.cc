#include <string>

#include "envoy/registry/registry.h"

#include "http_filter_config.h"
#include "http_filter.h"

namespace Envoy {
namespace Server {
namespace Configuration {

const std::string EXTAUTH_HTTP_FILTER_SCHEMA(R"EOF(
{
  "$schema": "http://json-schema.org/schema#",
  "type" : "object",
  "properties" : {
    "cluster" : {"type" : "string"}
  },
  "required" : ["cluster"],
  "additionalProperties" : false
}
)EOF");

Http::FilterFactoryCb HttpSampleDecoderFilterConfig::tryCreateFilterFactory(HttpFilterType type,
                                                          const std::string& name,
                                                          const Json::Object& json_config,
                                                          const std::string& stats_prefix,
                                                          Server::Instance& server){
    if (name != "dosa") {
      return nullptr;
    }
    json_config.validateSchema(EXTAUTH_HTTP_FILTER_SCHEMA);

    Http::DosaConfigConstSharedPtr config(new Http::ExtAuthConfig{
        server.clusterManager(), json_config.getString("cluster")});
    return [config](Http::FilterChainFactoryCallbacks& callbacks) -> void {
      callbacks.addStreamFilter(Http::StreamFilterSharedPtr{new Http::ExtAuth(config)});
    };                                                        
}

/**
 * Static registration for this sample filter. @see RegisterFactory.
 */
static RegisterHttpFilterConfigFactory<HttpSampleDecoderFilterConfig>
    register_;

} // namespace Configuration
} // namespace Server
} // namespace Envoy
