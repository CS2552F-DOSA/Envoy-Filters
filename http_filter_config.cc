#include <string>

#include "http_filter_config.h"
#include "http_filter.h"

namespace Envoy {
namespace Server {
namespace Configuration {

Http::FilterFactoryCb HttpSampleDecoderFilterConfig::tryCreateFilterFactory(HttpFilterType,
                                                          const std::string& name,
                                                          const Json::Object& json_config,
                                                          const std::string&,
                                                          Server::Instance& server){
    return createFilter(Envoy::MessageUtil::downcastAndValidate<const sample::Decoder&>(
                            proto_config, context.messageValidationVisitor()),
                        context);                                              
}

Http::FilterFactoryCb createFilter(const sample::Decoder& proto_config, FactoryContext&) {
  Http::DosaConfigConstSharedPtr config =
      std::make_shared<Http::DosaConfigConstSharedPtr>(
          Http::DosaConfigConstSharedPtr(proto_config));

  return [config](Http::FilterChainFactoryCallbacks& callbacks) -> void {
    auto filter = new Http::HttpSampleDecoderFilter(config);
    callbacks.addStreamFilter(Http::StreamFilterSharedPtr{filter});
  };
}

/**
 * Static registration for this sample filter. @see RegisterFactory.
 */
static RegisterHttpFilterConfigFactory<HttpSampleDecoderFilterConfig, NamedHttpFilterConfigFactory>
    register_;

} // namespace Configuration
} // namespace Server
} // namespace Envoy
