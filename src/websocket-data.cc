
/*
@author Jiang W
@time   2020.6.14
@email  aflyingwolf@126.com
**/

#include "websocket-data.h"
#include "sha1.h"
#include "base64.h"
#include <arpa/inet.h>
#include <sstream>
#include <unistd.h>
#include <sys/ioctl.h>
namespace server {

WebSocketData::WebSocketData(EventLoop *loop, int connfd)
    : loop_(loop),
      channel_(new Channel(loop, connfd)),
      fd_(connfd),
      error_(ERROR_NO),
      state_(STATE_HEAD),
      readPacket_(new Packet()),
      writePacket_(new Packet()),
      is_final_(false),
      uuid_(gen_uuid()){
  channel_->setReadHandler(bind(&WebSocketData::handleRead, this));
  channel_->setConnHandler(bind(&WebSocketData::handleConn, this));
}

WebSocketData::~WebSocketData() {
  LOG_I << "connect " << uuid_ << " leaving.";
  if (fd_ > 0) {
    close(fd_);
  }   
}
void WebSocketData::handleClose() {
  LOG_D << "remove from poller.";
  loop_->removeFromPoller(channel_);
  channel_->holder_.reset();
  //loop_->quit();
}

void WebSocketData::newEvent() {
  channel_->setEvents(DEFAULT_EVENT);
  loop_->addToPoller(channel_, DEFAULT_EXPIRED_TIME);
}

void WebSocketData::handleRead() {
  __uint32_t &events_ = channel_->getEvents();
  do {
    if (state_ == STATE_HEAD) {
      int ret = readAndParseHead();
      if (ret == 0) {
        ret = sendRespond();
        if (ret != 0) {
          state_ = STATE_ERROR;
        } else {
          state_ = STATE_START;
        }
      } else {
        state_ = STATE_ERROR;
      }
    }

    if (state_ == STATE_START) {
      int ret = processStart();
      if (ret != 0) {
        state_ = STATE_ERROR;
      } else {
        state_ = STATE_DATA;
      }
    } else if (state_ == STATE_DATA) {
      int ret = readAndParsePacket();
      if (ret != 0) {
        state_ = STATE_ERROR;
      } else {
        ret = processPacket();
        if (ret != 0) {
          state_ = STATE_ERROR;
        } else if (is_final_) {
          state_ = STATE_FINISH;
        }
      }
    }

    if (state_ == STATE_FINISH) {
      int ret = processEnd();
      if (ret != 0) {
        state_ = STATE_ERROR;
      } else {
        state_ = STATE_CLOSE;
      }
    }

    if (state_ == STATE_ERROR) {
      processError(error_);
    }
  } while(false);
}

void WebSocketData::handleConn() {
  if (state_ != STATE_ERROR && state_ != STATE_CLOSE) {
    __uint32_t &events_ = channel_->getEvents();
    events_ |= EPOLLIN;
    loop_->updatePoller(channel_);
  } else {
    loop_->runInLoop(bind(&WebSocketData::handleClose, shared_from_this()));
  }
}

void WebSocketData::sendErrorRespond(int err_code) {
  std::string request;
  request += "HTTP/1.1 ";
  request += std::to_string(err_code);
  request += " Switching Protocols\r\n";
  request += "Connection: upgrade\r\n";
  request += "Sec-WebSocket-Accept: ";
  std::string server_key = headers_["Sec-WebSocket-Key"];
  server_key += MAGIC_KEY;

  SHA1 sha;
  unsigned int message_digest[5];
  sha.Reset();
  sha << server_key.c_str();

  sha.Result(message_digest);
  for (int i = 0; i < 5; i++) {
    message_digest[i] = htonl(message_digest[i]);
  }   
  server_key = base64_encode(reinterpret_cast<const unsigned char*>(message_digest),20);
  server_key += "\r\n";
  request += server_key;
  request += "Upgrade: websocket\r\n\r\n";
  writeData(request); 
}

void WebSocketData::processError(ERROR_TYPE error_type) {
  if (error_type == ERROR_PARSE_HEAD) {
    sendErrorRespond(400);
  }
}
int WebSocketData::readAndParseHead() {
  std::string inBuffer;
  bool needRead = true;
  ssize_t nread = 0;
  ssize_t readSum = 0;
  while (true) {
    const int MAX_BUFF = 4096; 
    char buff[MAX_BUFF];
    if ((nread = read(fd_, buff, MAX_BUFF)) < 0) {
      if (errno == EINTR) {
        continue;
      } else if (errno == EAGAIN) {
        break;
      } else {
        LOG_E << "read Http head error!";
        error_ = ERROR_PARSE_HEAD;
        return -1;
      }
    } else if (nread == 0) {
      LOG_E << "maybe client has closed.";
      error_ = ERROR_CLIENT_CLOSE; 
      return -1;
    }
    readSum += nread;
    inBuffer += std::string(buff, buff + nread);
  }
  return parseHead(inBuffer); 
}

int WebSocketData::parseHead(std::string &head) {
  bool isReachHeadTail = false;
  int head_size = head.size();
  if (head[head_size-1] == '\n' && head[head_size-2] == '\r' && 
      head[head_size-3] == '\n' && head[head_size-4] == '\r') {
    isReachHeadTail = true;
  }
  if (isReachHeadTail == false) {
    LOG_E << "not reach whole http head.";
    error_ = ERROR_PARSE_HEAD;
    return -1;
  }
  std::istringstream s(head);
  std::string request;

  std::getline(s, request);
  if (request[request.size()-1] == '\r') {
    request.erase(request.end()-1);
  } else {
    LOG_E << "this line tail not correct.";
    error_ = ERROR_PARSE_HEAD;
    return -1; 
  }
  std::string header;
  std::string::size_type end;
  while (std::getline(s, header) && header != "\r") {
    if (header[header.size()-1] != '\r') {
      LOG_E << "not should reach there.";
      continue; //end
    } else {
      header.erase(header.end()-1);   //remove last char
    }   

    end = header.find(": ", 0);
    if (end != std::string::npos) {
      std::string key = header.substr(0, end);
      std::string value = header.substr(end+2);
      headers_[key] = value;
    }
  }
  return 0;
}

int WebSocketData::sendRespond() {
  std::string request;
  request += "HTTP/1.1 101 Switching Protocols\r\n";
  request += "Connection: upgrade\r\n";
  request += "Sec-WebSocket-Accept: ";
  std::string server_key = headers_["Sec-WebSocket-Key"];
  server_key += MAGIC_KEY;

  SHA1 sha;
  unsigned int message_digest[5];
  sha.Reset();
  sha << server_key.c_str();

  sha.Result(message_digest);
  for (int i = 0; i < 5; i++) {
    message_digest[i] = htonl(message_digest[i]);
  }  
  server_key = base64_encode(reinterpret_cast<const unsigned char*>(message_digest),20);
  server_key += "\r\n";
  request += server_key;
  request += "Upgrade: websocket\r\n\r\n";
  return writeData(request);  
}

int WebSocketData::writeData(std::string &data) {
  int bytes_left; 
  int written_bytes = 0;
  const char *buffer = data.c_str();
  const char *ptr = buffer ;
  bytes_left = data.size(); 
  while (bytes_left > 0) {
    written_bytes = write(fd_, ptr, bytes_left); 
    if (written_bytes <= 0) {
      if (errno == EINTR) {   
        LOG_W << "write data to socket encounter EINTR , continue";
        continue;
      } else if (errno == EAGAIN) {
        LOG_W << "write data to socket encounter EAGAIN, continue-" <<  bytes_left;
        usleep(200);
        continue;
      } else {
        LOG_E << "write Error.";
        error_ = ERROR_WRITE;
        return -1;
      }   
    }   
    bytes_left -= written_bytes;
    ptr += written_bytes;
  }   
  return 0;
}

int WebSocketData::writeData(std::shared_ptr<Packet> packet) {
  int bytes_left; 
  int written_bytes = 0;
  const char *buffer = packet->buffer();
  const char *ptr = buffer;
  bytes_left = packet->len_; 
  while (bytes_left > 0) {
    written_bytes = write(fd_, ptr, bytes_left); 
    if (written_bytes <= 0) {
      if (errno == EINTR) {   
        LOG_W << "write data to socket encounter EINTR , continue";
        continue;
      } else if (errno == EAGAIN) {
        LOG_W << "write data to socket encounter EAGAIN, continue-" << bytes_left;
        usleep(200);
        continue;
      } else {
        LOG_E << "write Error.";
        error_ = ERROR_WRITE;
        return -1; 
      }   
    }   
    bytes_left -= written_bytes;
    ptr += written_bytes;
  }   
  return 0;
}

int WebSocketData::readAndParsePacket() {
  std::shared_ptr<Packet> packet = readPacket_;
  int bytes = 0;
  ioctl(fd_, FIONREAD, &bytes);
  int rbytes = 0;

  if (packet->state_ == PACKET_INIT) {
    packet->reset();
    if (bytes < 2) {
      return 0;
    }
    
    char head[2];
    rbytes = read(fd_, head, 2);
    if (rbytes != 2) {
      LOG_E << "not should this error.";
      error_ = ERROR_READ;
      return -1;
    }
    packet->head_size_ += 2;
    bytes -= 2;
    packet->fin_ = (unsigned char)head[0] >> 7;
    packet->opcode_ = head[0] & 0x0f;
    packet->mask_ = (unsigned char)head[1] >> 7;
    packet->body_size_ = head[1] & 0x7f;
    if (packet->body_size_ < 126) {
      packet->state_ = PACKET_MASK; 
    } else if (packet->body_size_ == 126) {
      packet->state_ = PACKET_HEAD_126;
    } else if (packet->body_size_ == 127) {
      packet->state_ = PACKET_HEAD_127;
    }
  }

  if (packet->state_ == PACKET_HEAD_126) {
    if (bytes < 2) {
      return 0;
    }
    uint16_t length = 0;
    char len_buffer[2];
    rbytes = read(fd_, len_buffer, 2);
    if (rbytes != 2) {
      LOG_E << "not should this error.";
      error_ = ERROR_READ;
      return -1;
    }
    packet->head_size_ += 2;
    bytes -= 2;
    memcpy(&length, len_buffer, 2);
    packet->body_size_ = ntohs(length);
    packet->state_ = PACKET_MASK;
  }
  
  if (packet->state_ == PACKET_HEAD_127) {
    if (bytes < 8) {
      return 0;
    }
    char len_buffer[8];
    rbytes = read(fd_, len_buffer, 8);
    if (rbytes != 8) {
      LOG_E << "not should this error.";
      error_ = ERROR_READ;
      return -1;
    }
    packet->head_size_ += 8;
    bytes -= 8;
    if (len_buffer[0] != 0 && len_buffer[1] != 0 &&
        len_buffer[2] != 0 && len_buffer[3] != 0) {
      LOG_E << "body len too big, error.";
      error_ = ERROR_PARSE_DATA;
      return -1;
    }
    uint32_t length = 0;
    memcpy(&length, len_buffer + 4, 4);
    packet->body_size_ = ntohl(length);
    packet->state_ = PACKET_MASK;
    
  }

  if (packet->state_ == PACKET_MASK) {
    if (packet->mask_ != 1) {
      packet->state_ = PACKET_DATA;
    } else {
      if (bytes < 4) {
        return 0;
      }
      char mask_buffer[4];
      rbytes = read(fd_, mask_buffer, 4);

      if (rbytes != 4) {
        LOG_E << "not should this error.";
        error_ = ERROR_READ;
        return -1;
      }

      packet->head_size_ += 4;
      bytes -= 4;

      for (int i = 0; i < 4; i++) {
        packet->masking_key_[i] = mask_buffer[i];
      }
      packet->state_ = PACKET_DATA;
    }
  }

  if (packet->state_ == PACKET_DATA) {
    int readLen = 0;
    if (bytes > packet->body_size_ - packet->len_) {
      readLen = packet->body_size_ - packet->len_;
    } else {
      readLen = bytes;
    }
    packet->resize(packet->body_size_);

    rbytes = read(fd_, packet->buffer() + packet->len_, readLen);
    char *ptr = packet->buffer() + packet->len_;
    for(uint i = 0; i < readLen; i++){
      int j = (packet->len_ + i) % 4;
      ptr[i] = ptr[i] ^ packet->masking_key_[j];
    }
    if (rbytes != readLen) {
      LOG_E << "not should this error.";
      error_ = ERROR_READ;
      return -1;
    }
    packet->len_ += rbytes;

    if (packet->len_ == packet->body_size_) {
      packet->state_ = PACKET_INIT;
    }
  } 
  return 0; 
}

int WebSocketData::sendResult(const char *result, int length) {
  std::shared_ptr<Packet> packet = writePacket_;
  size_t out_len_tmp = length;
  uint8_t mask = WEBSOCKET_NEED_NOT_MASK;
  uint8_t fin = WEBSOCKET_FIN_MSG_END;
  uint8_t opcode = WEBSOCKET_TEXT_DATA;
  
  int head_length=0;

  if (length < 126) {
    out_len_tmp += 2;
  } else if (length < 0xFFFF) {
    out_len_tmp += 4;
  } else {
    out_len_tmp += 8;
  }

  if (mask & 0x1) { 
    out_len_tmp += 4;
  }
  packet->resize(out_len_tmp);

  char *data = packet->buffer();
  memset(data, 0, out_len_tmp);
  *data = fin << 7;
  *data = *data | (0xF & opcode);
  *(data + 1) = mask << 7;
  if (length < 126) {
    *(data + 1) |= length;
    head_length += 2;
  } else if (length < 0xFFFF) {
    *(data + 1) |= 0x7E;
    *(data + 2) = (char)((length >> 8) & 0xFF);
    *(data + 3) = (unsigned char)((length >> 0) & 0xFF);
    head_length += 4;
  } else {
    *(data + 1) = *(data + 1) | 0x7F;
    uint32_t tmp = htonl((uint32_t)length);

    *(data + 1) |= 0x7F;
    *(data + 2) = 0;
    *(data + 3) = 0;
    *(data + 4) = 0;
    *(data + 5) = 0;
    *(data + 6) = (char)((length >> 24) & 0xFF);
    *(data + 7) = (char)((length >> 16) & 0xFF);
    *(data + 8) = (char)((length >> 8) & 0xFF);
    *(data + 9) = (char)((length >> 0) & 0xFF);
    head_length += 10;
  }
  if (mask & 0x1) {
    head_length += 4;
  }
  memcpy(data + head_length, result, length); 
  packet->len_ = head_length + length;
  return writeData(packet); 
}

int WebSocketData::processStart() {
  LOG_I << "process start.";
  this_conn_bytes_ = 0;
  return 0;
}

int WebSocketData::processPacket() {
  LOG_I << "process packet.";
  char *data = readPacket_->buffer();
  //std::string tmp(data, readPacket_->len_);
  //LOG_I << "DATA:" << tmp << "Len:" << readPacket_->len_;
  if (readPacket_->opcode_ == WEBSOCKET_CONNECT_CLOSE || 
      readPacket_->opcode_ == WEBSOCKET_TEXT_DATA && 
      readPacket_->state_ == PACKET_INIT && 
      readPacket_->len_ == 3) {
    if (readPacket_->opcode_ == WEBSOCKET_CONNECT_CLOSE) {
      is_final_ = true;
    } else {
      std::string eos(data, readPacket_->len_);
      if (eos == "EOS") {
        is_final_ = true;
      }
    }
  } else {
    if (readPacket_->state_ == PACKET_INIT) {
      this_conn_bytes_ += readPacket_->len_;
      std::string tmp_file = "file/" + uuid_;
      FILE *fp = fopen(tmp_file.c_str(), "a");
      fwrite(data, 1, readPacket_->len_, fp);
      fclose(fp);
    }
  }
  return 0;
}

int WebSocketData::processEnd() {
  LOG_I << "process end.";
  LOG_I << "receive_data_len: " << this_conn_bytes_;
  std::string result = "Websocket";
  sendResult(result.c_str(), result.size());
  return 0;
}

}
