#pragma once

#include <string>

#include "envoy/registry/registry.h"
#include "http_filter.h"
#include "http_filter.pb.h"
#include "http_filter.pb.validate.h"

namespace Envoy {
namespace Server {
namespace Configuration {

class HttpSampleDecoderFilterConfig : public NamedHttpFilterConfigFactory {
 public:
  Http::FilterFactoryCb createFilterFactoryFromProto(
      const Protobuf::Message& config, const std::string& stat_prefix,
      FactoryContext& context) override;

  ProtobufTypes::MessagePtr createEmptyConfigProto() override {
    return ProtobufTypes::MessagePtr{new dosa::Dosa()};
  }

  std::string name() const override { return "dosa"; }

 private:
  Http::FilterFactoryCb createFilter(const dosa::Dosa& proto_config,
                                     FactoryContext&);
};

}  // namespace Configuration
}  // namespace Server
}  // namespace Envoy
