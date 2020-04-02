#include <string>

#include "http_filter_config.h"
#include "http_filter.h"

namespace Envoy {
namespace Server {
namespace Configuration {

Http::FilterFactoryCb HttpSampleDecoderFilterConfig::createFilterFactoryFromProto(const Protobuf::Message& proto_config,
                                                      const std::string&,
                                                      FactoryContext& context){
    return createFilter(Envoy::MessageUtil::downcastAndValidate<const dosa::Dosa&>(
                            proto_config, context.messageValidationVisitor()),
                        context);                                              
}

Http::FilterFactoryCb HttpSampleDecoderFilterConfig::createFilter(const dosa::Dosa& proto_config, FactoryContext&) {
  Http::DosaConfigConstSharedPtr config =
      std::make_shared<Http::DosaConfig>(
          Http::DosaConfig(proto_config));

  return [config](Http::FilterChainFactoryCallbacks& callbacks) -> void {
    auto filter = new Http::HttpSampleDecoderFilter(config);
    callbacks.addStreamFilter(Http::StreamFilterSharedPtr{filter});
  };
}

/**
 * Static registration for this sample filter. @see RegisterFactory.
 */
static Registry::RegisterFactory<HttpSampleDecoderFilterConfig, NamedHttpFilterConfigFactory>
    register_;

} // namespace Configuration
} // namespace Server
} // namespace Envoy
