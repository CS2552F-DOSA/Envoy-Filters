#include <string>

#include "http_filter.h"

#include "envoy/server/filter_config.h"

#include "extensions/filters/network/common/redis/codec.h"
#include "common/buffer/buffer_impl.h"

namespace Envoy {
namespace Filter {

DosaConfig::DosaConfig(const dosa::Dosa& proto_config, Upstream::ClusterManager& cm):
  cm_(cm), cluster_(proto_config.cluster()) {}

bool DosaEngine::isKeyInCache(std::string key){
  return true;
  return cache_.find(key) != cache_.end();
}

DosaEngine HttpSampleDecoderFilter::engine_ = DosaEngine();

HttpSampleDecoderFilter::HttpSampleDecoderFilter(
    DosaConfigConstSharedPtr config, Redis::DecoderFactory& factory)
    : config_(config), decoder_(factory.create(*this)), filter_state_(FilterState::RequestNotRead){}

HttpSampleDecoderFilter::~HttpSampleDecoderFilter() {}

Network::FilterStatus HttpSampleDecoderFilter::onWrite(Buffer::Instance& data, bool){
  if(filter_state_ == FilterState::Set ||
      filter_state_ == FilterState::RequestNotRead){
    filter_state_ = FilterState::RequestNotRead;
    // TODO: decide if a new connection will invoke filters or not
    // return Network::FilterStatus::Continue;
  }
  if(filter_state_ == FilterState::Get){
    try {
      ENVOY_LOG(info, "Start decode data on onWrite");
      filter_state_ = FilterState::ResponseReading;
      decoder_->decode(data, false);
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

  while(filter_state_ == FilterState::ResponseReading)
    return Network::FilterStatus::StopIteration;

  // TODO: compare the data and return the real shit
  return Network::FilterStatus::Continue;
}

Network::FilterStatus HttpSampleDecoderFilter::onData(Buffer::Instance& data, bool){
  ENVOY_LOG(info, "onData is called");
  if(filter_state_ == FilterState::RequestNotRead){
    try {
      ENVOY_LOG(info, "Start decode data on onData");
      filter_state_ = FilterState::RequestReading;
      decoder_->decode(data, false);

      // read_callbacks_->connection().write()
    } catch (Redis::ProtocolError&) {
      // Redis::RespValue error;
      // error.type(Redis::RespType::Error);
      // error.asString() = "downstream protocol error";
      // encoder_->encode(error, encoder_buffer_);
      // read_callbacks_->connection().write(encoder_buffer_, false);
      read_callbacks_->connection().close(Network::ConnectionCloseType::NoFlush);
      return Network::FilterStatus::StopIteration;
    }
  }

  // Wait until the examnation ends
  while(filter_state_ == FilterState::RequestReading){}
    // return Network::FilterStatus::StopIteration;

  ENVOY_LOG(info, "onData is called here again");
  if(filter_state_ == FilterState::Get){
    // The command is a get
    return Network::FilterStatus::Continue;
  } else {
    // The command is a set
    if(true){
      Upstream::Host::CreateConnectionData info =
        config_->cm_.tcpConnForCluster(config_->cluster_, nullptr);
      connection_ = std::move(info.connection_);
      // connection_->addConnectionCallbacks(*this);
      connection_->connect();
      connection_->write(data, false);

      // TODO: wait until the data is received and send.
      // read_callbacks_->connection().write(data, false);
      // callbacks_->connection().close(Network::ConnectionCloseType::NoFlush);
    }
    return Network::FilterStatus::Continue;
  }
  return Network::FilterStatus::Continue;
}

Network::FilterStatus HttpSampleDecoderFilter::onNewConnection(){
  return Network::FilterStatus::Continue;
}

void HttpSampleDecoderFilter::initializeReadFilterCallbacks(Network::ReadFilterCallbacks& callbacks){
  read_callbacks_ = &callbacks;
}

void HttpSampleDecoderFilter::onRespValue(Redis::RespValuePtr&& value) {
  ENVOY_LOG(info, "Dosa::decodeHeaders, {}", count_++);
  ENVOY_LOG(info, "{}", value->toString());

  if(value->type() == Redis::RespType::Array){
    // This is a request
    std::vector<Redis::RespValue>& request = value->asArray();
    std::string request_method = request[0].toString();
    // ENVOY_LOG(info, "Dosa::decodeHeaders, {}", request_method.compare("\"SET\""));
    if(request_method == "\"SET\"" ||
      request_method == "\"GET\""){
        filter_state_ = (request_method == "\"GET\"")? FilterState::Get:FilterState::Set;
        // key_ = request[1];
        // value_ = request[2];
        read_callbacks_->continueReading();
      }
  } else if(value->type() == Redis::RespType::SimpleString){
    // response_resp_ = Redis::RespValue(value);
    return;
  } else {
    return;
  }
  // pending_requests_.emplace_back(*this);
  // PendingRequest& request = pending_requests_.back();
  // CommandSplitter::SplitRequestPtr split = splitter_.makeRequest(std::move(value), request);
  // if (split) {
  //   // The splitter can immediately respond and destroy the pending request. Only store the handle
  //   // if the request is still alive.
  //   request.request_handle_ = std::move(split);
  // }
}

} // namespace Filter
} // namespace Envoy
