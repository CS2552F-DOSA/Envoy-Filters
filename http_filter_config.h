#pragma once

#include <string>

#include "envoy/registry/registry.h"

#include "http_filter.pb.h"
#include "http_filter.pb.validate.h"
#include "http_filter.h"

namespace Envoy {
namespace Server {
namespace Configuration {

class HttpSampleDecoderFilterConfig: public NamedNetworkFilterConfigFactory {
public:
  Network::FilterFactoryCb createFilterFactoryFromProto(const Protobuf::Message& config,
                                                      FactoryContext& context) override;

  ProtobufTypes::MessagePtr createEmptyConfigProto() override {
    return ProtobufTypes::MessagePtr{new dosa::Dosa()};
  }

  std::string name() const override { return "dosa"; }

private:
  Network::FilterFactoryCb createFilter(const dosa::Dosa& proto_config, FactoryContext&);
};

} // Configuration
} // Server
} // Envoy

