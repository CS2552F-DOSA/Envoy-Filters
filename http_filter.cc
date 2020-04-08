#include <string>

#include "http_filter.h"

#include "envoy/server/filter_config.h"

#include "extensions/filters/network/common/redis/codec.h"

namespace Envoy {
namespace Filter {

DosaConfig::DosaConfig(const dosa::Dosa& proto_config, Upstream::ClusterManager& cm):
  cm_(cm), cluster_(proto_config.cluster()) {}

// DosaConfig::DosaConfig(const dosa::Dosa& proto_config):
//   cluster_(proto_config.cluster()) {}

bool DosaEngine::isKeyInCache(std::string key){
  return true;
  return cache_.find(key) != cache_.end();
}

DosaEngine HttpSampleDecoderFilter::engine_ = DosaEngine();

HttpSampleDecoderFilter::HttpSampleDecoderFilter(
    DosaConfigConstSharedPtr config, Redis::DecoderFactory& factory)
    : config_(config), decoder_(factory.create(*this)){}

HttpSampleDecoderFilter::~HttpSampleDecoderFilter() {}

Network::FilterStatus HttpSampleDecoderFilter::onWrite(Buffer::Instance& data, bool){
  try {
    ENVOY_LOG(info, "Start decode data on onWrite");
    decoder_->decode(data);

    if(false){
      // Upstream::Host::CreateConnectionData info =
      //   config_->cm_.tcpConnForCluster(config_->cluster_, nullptr);
      // connection_ = std::move(info.connection_);
      // connection_->addConnectionCallbacks(*this);
      // connection_->connect();
      // connection_->write(data, false);
    }

    if(true){
      return Network::FilterStatus::Continue;
    } else {
      return Network::FilterStatus::Continue;
    }
  } catch (Redis::ProtocolError&) {
    // Redis::RespValue error;
    // error.type(Redis::RespType::Error);
    // error.asString() = "downstream protocol error";
    // encoder_->encode(error, encoder_buffer_);
    // callbacks_->connection().write(encoder_buffer_, false);
    // callbacks_->connection().close(Network::ConnectionCloseType::NoFlush);
    return Network::FilterStatus::StopIteration;
  }
}

Network::FilterStatus HttpSampleDecoderFilter::onData(Buffer::Instance& data, bool){
  try {
    ENVOY_LOG(info, "Start decode data on onData");
    decoder_->decode(data);

    if(false){
      // Upstream::Host::CreateConnectionData info =
      //   config_->cm_.tcpConnForCluster(config_->cluster_, nullptr);
      // connection_ = std::move(info.connection_);
      // connection_->addConnectionCallbacks(*this);
      // connection_->connect();
      // connection_->write(data, false);
    }

    if(true){
      return Network::FilterStatus::Continue;
    } else {
      return Network::FilterStatus::Continue;
    }
  } catch (Redis::ProtocolError&) {
    // Redis::RespValue error;
    // error.type(Redis::RespType::Error);
    // error.asString() = "downstream protocol error";
    // encoder_->encode(error, encoder_buffer_);
    // callbacks_->connection().write(encoder_buffer_, false);
    // callbacks_->connection().close(Network::ConnectionCloseType::NoFlush);
    return Network::FilterStatus::StopIteration;
  }
}

Network::FilterStatus HttpSampleDecoderFilter::onNewConnection(){
  return Network::FilterStatus::Continue;
}

void HttpSampleDecoderFilter::initializeReadFilterCallbacks(Network::ReadFilterCallbacks& callbacks){
  read_callbacks_ = &callbacks;
}

void HttpSampleDecoderFilter::onRespValue(Redis::RespValuePtr&& value) {
  ENVOY_LOG(info, "Dosa::decodeHeaders");
  ENVOY_LOG(info, "{}", value->toString());
  // pending_requests_.emplace_back(*this);
  // PendingRequest& request = pending_requests_.back();
  // CommandSplitter::SplitRequestPtr split = splitter_.makeRequest(std::move(value), request);
  // if (split) {
  //   // The splitter can immediately respond and destroy the pending request. Only store the handle
  //   // if the request is still alive.
  //   request.request_handle_ = std::move(split);
  // }
}



// void HttpSampleDecoderFilter::onDestroy() {}

/*
FilterHeadersStatus HttpSampleDecoderFilter::decodeHeaders(RequestHeaderMap& headers, bool) {
  ENVOY_STREAM_LOG(info, "Dosa::decodeHeaders: {}", *decoder_callbacks_, headers);
  ENVOY_STREAM_LOG(info, "Dosa::decodeHeaders: {}", *decoder_callbacks_, count_++);
  return FilterHeadersStatus::Continue;

  // if(copiedHeaders){
  //   copiedHeaders = Http::HeaderMapPtr{new Http::HeaderMapImpl(*headers)};
  //   decoder_callbacks_->continueDecoding();
  // }

  // if(decodeCacheCheck_){
  //   // The decodeData checked the cache
  //   if(decodeDoNotChange_){
  //     return FilterHeadersStatus::Continue;
  //   } else {
  //     // // Change the header to test database
  //     // std::string host = headers->Host()->value().c_str();
  //     // ASSERT(!host.empty());
  //     // host += "_test";
  //     // headers->Host()->value(host);
  //     return FilterHeadersStatus::Continue;
  //   }
  // } else {
  //   // We do not check the cache.
  //   return FilterHeadersStatus::StopIteration;
  // }
}

FilterDataStatus HttpSampleDecoderFilter::decodeData(Buffer::Instance&, bool) {
  // if(!decodeCacheCheck_){
  //   // TODO: Decode the data
  //   std::string key = "TODO:";s

  //   // Note: The order is important
  //   // decodeDoNotChange_ = !engine_.isKeyInCache(key);
  //   decodeDoNotChange_ = true;
  //   decodeCacheCheck_ = true;

  //   decoder_callbacks_->continueDecoding();
  // }

  // if(decodeDoNotChange_){
  //   if(copiedHeaders && copiedTrailers){
  //     // TODO: shadow the request to the test server
  //     Http::MessagePtr request(new Http::RequestMessageImpl(copiedHeaders));
  //     request->body().reset(new Buffer::OwnedImpl(*callbacks_->decodingBuffer()));
  //   } else {
  //     return FilterDataStatus::StopIterationAndBuffer;
  //   }
  // }

  return FilterDataStatus::Continue;
}

FilterTrailersStatus HttpSampleDecoderFilter::decodeTrailers(RequestTrailerMap&){
  // if(copiedTrailers){
  //   copiedTrailers = Http::HeaderMapPtr{new Http::HeaderMapImpl(*trailers)};
  //   decoder_callbacks_->continueDecoding();
  // }
  return FilterTrailersStatus::Continue;
}

Http::FilterHeadersStatus HttpSampleDecoderFilter::encodeHeaders(ResponseHeaderMap& headers, bool){
  ENVOY_STREAM_LOG(info, "Dosa::encodeHeaders: {}", *encoder_callbacks_, headers);
  ENVOY_STREAM_LOG(info, "Dosa::encodeHeaders: {}", *encoder_callbacks_, count_++);
  return FilterHeadersStatus::Continue;
}

Http::FilterDataStatus HttpSampleDecoderFilter::encodeData(Buffer::Instance&, bool){
  return FilterDataStatus::Continue;
}

Http::FilterTrailersStatus HttpSampleDecoderFilter::encodeTrailers(ResponseTrailerMap&){
  return FilterTrailersStatus::Continue;
}

FilterHeadersStatus HttpSampleDecoderFilter::encode100ContinueHeaders(ResponseHeaderMap&){
  return FilterHeadersStatus::Continue;
}

FilterMetadataStatus HttpSampleDecoderFilter::encodeMetadata(MetadataMap&){
  return FilterMetadataStatus::Continue;
}

void HttpSampleDecoderFilter::setDecoderFilterCallbacks(StreamDecoderFilterCallbacks& callbacks) {
  decoder_callbacks_ = &callbacks;
}

void HttpSampleDecoderFilter::setEncoderFilterCallbacks(StreamEncoderFilterCallbacks& callbacks) {
  encoder_callbacks_ = &callbacks;
}

void HttpSampleDecoderFilter::onSuccess(const AsyncClient::Request&, ResponseMessagePtr&&){
  return;
}

void HttpSampleDecoderFilter::onFailure(const AsyncClient::Request&, AsyncClient::FailureReason){
  return;
}
*/
} // namespace Filter
} // namespace Envoy
