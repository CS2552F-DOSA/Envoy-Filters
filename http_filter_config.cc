#include <string>

#include "http_filter_config.h"
#include "http_filter.h"

namespace Envoy {
namespace Server {
namespace Configuration {

Network::FilterFactoryCb HttpSampleDecoderFilterConfig::createFilterFactoryFromProto(
                                                      const Protobuf::Message& proto_config,
                                                      FactoryContext& context){
    return createFilter(Envoy::MessageUtil::downcastAndValidate<const dosa::Dosa&>(
                            proto_config, context.messageValidationVisitor()),
                        context);                                              
}

Network::FilterFactoryCb HttpSampleDecoderFilterConfig::createFilter(const dosa::Dosa& proto_config, FactoryContext& context) {
  Filter::DosaConfigConstSharedPtr config =
      std::make_shared<Filter::DosaConfig>(
          Filter::DosaConfig(proto_config, context.clusterManager()));
  // Filter::DosaConfigConstSharedPtr config =
  //     std::make_shared<Filter::DosaConfig>(
  //         Filter::DosaConfig(proto_config));

  return [config](Network::FilterManager& filter_manager) -> void {
    Extensions::NetworkFilters::Common::Redis::DecoderFactoryImpl factory;
    auto filter = new Filter::HttpSampleDecoderFilter(config, factory);
    filter_manager.addFilter(Network::FilterSharedPtr{filter});
  };
}

/**
 * Static registration for this sample filter. @see RegisterFactory.
 */
// static Registry::RegisterFactory<HttpSampleDecoderFilterConfig, NamedHttpFilterConfigFactory>
//     register_;

static Registry::RegisterFactory<HttpSampleDecoderFilterConfig, NamedNetworkFilterConfigFactory>
    registered_;

} // namespace Configuration
} // namespace Server
} // namespace Envoy
