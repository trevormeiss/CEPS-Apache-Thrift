/**
 * Autogenerated by Thrift Compiler (0.9.3)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#ifndef Player_H
#define Player_H

#include <thrift/TDispatchProcessor.h>
#include <thrift/async/TConcurrentClientSyncInfo.h>
#include "ceps_types.h"

namespace ceps {

#ifdef _WIN32
  #pragma warning( push )
  #pragma warning (disable : 4250 ) //inheriting methods via dominance 
#endif

class PlayerIf {
 public:
  virtual ~PlayerIf() {}
  virtual bool sendOutput(const int16_t pNum, const int32_t value) = 0;
  virtual bool sendInput(const int16_t pNum, const int32_t share) = 0;
};

class PlayerIfFactory {
 public:
  typedef PlayerIf Handler;

  virtual ~PlayerIfFactory() {}

  virtual PlayerIf* getHandler(const ::apache::thrift::TConnectionInfo& connInfo) = 0;
  virtual void releaseHandler(PlayerIf* /* handler */) = 0;
};

class PlayerIfSingletonFactory : virtual public PlayerIfFactory {
 public:
  PlayerIfSingletonFactory(const boost::shared_ptr<PlayerIf>& iface) : iface_(iface) {}
  virtual ~PlayerIfSingletonFactory() {}

  virtual PlayerIf* getHandler(const ::apache::thrift::TConnectionInfo&) {
    return iface_.get();
  }
  virtual void releaseHandler(PlayerIf* /* handler */) {}

 protected:
  boost::shared_ptr<PlayerIf> iface_;
};

class PlayerNull : virtual public PlayerIf {
 public:
  virtual ~PlayerNull() {}
  bool sendOutput(const int16_t /* pNum */, const int32_t /* value */) {
    bool _return = false;
    return _return;
  }
  bool sendInput(const int16_t /* pNum */, const int32_t /* share */) {
    bool _return = false;
    return _return;
  }
};

typedef struct _Player_sendOutput_args__isset {
  _Player_sendOutput_args__isset() : pNum(false), value(false) {}
  bool pNum :1;
  bool value :1;
} _Player_sendOutput_args__isset;

class Player_sendOutput_args {
 public:

  Player_sendOutput_args(const Player_sendOutput_args&);
  Player_sendOutput_args& operator=(const Player_sendOutput_args&);
  Player_sendOutput_args() : pNum(0), value(0) {
  }

  virtual ~Player_sendOutput_args() throw();
  int16_t pNum;
  int32_t value;

  _Player_sendOutput_args__isset __isset;

  void __set_pNum(const int16_t val);

  void __set_value(const int32_t val);

  bool operator == (const Player_sendOutput_args & rhs) const
  {
    if (!(pNum == rhs.pNum))
      return false;
    if (!(value == rhs.value))
      return false;
    return true;
  }
  bool operator != (const Player_sendOutput_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Player_sendOutput_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class Player_sendOutput_pargs {
 public:


  virtual ~Player_sendOutput_pargs() throw();
  const int16_t* pNum;
  const int32_t* value;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _Player_sendOutput_result__isset {
  _Player_sendOutput_result__isset() : success(false) {}
  bool success :1;
} _Player_sendOutput_result__isset;

class Player_sendOutput_result {
 public:

  Player_sendOutput_result(const Player_sendOutput_result&);
  Player_sendOutput_result& operator=(const Player_sendOutput_result&);
  Player_sendOutput_result() : success(0) {
  }

  virtual ~Player_sendOutput_result() throw();
  bool success;

  _Player_sendOutput_result__isset __isset;

  void __set_success(const bool val);

  bool operator == (const Player_sendOutput_result & rhs) const
  {
    if (!(success == rhs.success))
      return false;
    return true;
  }
  bool operator != (const Player_sendOutput_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Player_sendOutput_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _Player_sendOutput_presult__isset {
  _Player_sendOutput_presult__isset() : success(false) {}
  bool success :1;
} _Player_sendOutput_presult__isset;

class Player_sendOutput_presult {
 public:


  virtual ~Player_sendOutput_presult() throw();
  bool* success;

  _Player_sendOutput_presult__isset __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

typedef struct _Player_sendInput_args__isset {
  _Player_sendInput_args__isset() : pNum(false), share(false) {}
  bool pNum :1;
  bool share :1;
} _Player_sendInput_args__isset;

class Player_sendInput_args {
 public:

  Player_sendInput_args(const Player_sendInput_args&);
  Player_sendInput_args& operator=(const Player_sendInput_args&);
  Player_sendInput_args() : pNum(0), share(0) {
  }

  virtual ~Player_sendInput_args() throw();
  int16_t pNum;
  int32_t share;

  _Player_sendInput_args__isset __isset;

  void __set_pNum(const int16_t val);

  void __set_share(const int32_t val);

  bool operator == (const Player_sendInput_args & rhs) const
  {
    if (!(pNum == rhs.pNum))
      return false;
    if (!(share == rhs.share))
      return false;
    return true;
  }
  bool operator != (const Player_sendInput_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Player_sendInput_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class Player_sendInput_pargs {
 public:


  virtual ~Player_sendInput_pargs() throw();
  const int16_t* pNum;
  const int32_t* share;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _Player_sendInput_result__isset {
  _Player_sendInput_result__isset() : success(false) {}
  bool success :1;
} _Player_sendInput_result__isset;

class Player_sendInput_result {
 public:

  Player_sendInput_result(const Player_sendInput_result&);
  Player_sendInput_result& operator=(const Player_sendInput_result&);
  Player_sendInput_result() : success(0) {
  }

  virtual ~Player_sendInput_result() throw();
  bool success;

  _Player_sendInput_result__isset __isset;

  void __set_success(const bool val);

  bool operator == (const Player_sendInput_result & rhs) const
  {
    if (!(success == rhs.success))
      return false;
    return true;
  }
  bool operator != (const Player_sendInput_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Player_sendInput_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _Player_sendInput_presult__isset {
  _Player_sendInput_presult__isset() : success(false) {}
  bool success :1;
} _Player_sendInput_presult__isset;

class Player_sendInput_presult {
 public:


  virtual ~Player_sendInput_presult() throw();
  bool* success;

  _Player_sendInput_presult__isset __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

class PlayerClient : virtual public PlayerIf {
 public:
  PlayerClient(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) {
    setProtocol(prot);
  }
  PlayerClient(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, boost::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) {
    setProtocol(iprot,oprot);
  }
 private:
  void setProtocol(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) {
  setProtocol(prot,prot);
  }
  void setProtocol(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, boost::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) {
    piprot_=iprot;
    poprot_=oprot;
    iprot_ = iprot.get();
    oprot_ = oprot.get();
  }
 public:
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> getInputProtocol() {
    return piprot_;
  }
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> getOutputProtocol() {
    return poprot_;
  }
  bool sendOutput(const int16_t pNum, const int32_t value);
  void send_sendOutput(const int16_t pNum, const int32_t value);
  bool recv_sendOutput();
  bool sendInput(const int16_t pNum, const int32_t share);
  void send_sendInput(const int16_t pNum, const int32_t share);
  bool recv_sendInput();
 protected:
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> piprot_;
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> poprot_;
  ::apache::thrift::protocol::TProtocol* iprot_;
  ::apache::thrift::protocol::TProtocol* oprot_;
};

class PlayerProcessor : public ::apache::thrift::TDispatchProcessor {
 protected:
  boost::shared_ptr<PlayerIf> iface_;
  virtual bool dispatchCall(::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, const std::string& fname, int32_t seqid, void* callContext);
 private:
  typedef  void (PlayerProcessor::*ProcessFunction)(int32_t, ::apache::thrift::protocol::TProtocol*, ::apache::thrift::protocol::TProtocol*, void*);
  typedef std::map<std::string, ProcessFunction> ProcessMap;
  ProcessMap processMap_;
  void process_sendOutput(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
  void process_sendInput(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
 public:
  PlayerProcessor(boost::shared_ptr<PlayerIf> iface) :
    iface_(iface) {
    processMap_["sendOutput"] = &PlayerProcessor::process_sendOutput;
    processMap_["sendInput"] = &PlayerProcessor::process_sendInput;
  }

  virtual ~PlayerProcessor() {}
};

class PlayerProcessorFactory : public ::apache::thrift::TProcessorFactory {
 public:
  PlayerProcessorFactory(const ::boost::shared_ptr< PlayerIfFactory >& handlerFactory) :
      handlerFactory_(handlerFactory) {}

  ::boost::shared_ptr< ::apache::thrift::TProcessor > getProcessor(const ::apache::thrift::TConnectionInfo& connInfo);

 protected:
  ::boost::shared_ptr< PlayerIfFactory > handlerFactory_;
};

class PlayerMultiface : virtual public PlayerIf {
 public:
  PlayerMultiface(std::vector<boost::shared_ptr<PlayerIf> >& ifaces) : ifaces_(ifaces) {
  }
  virtual ~PlayerMultiface() {}
 protected:
  std::vector<boost::shared_ptr<PlayerIf> > ifaces_;
  PlayerMultiface() {}
  void add(boost::shared_ptr<PlayerIf> iface) {
    ifaces_.push_back(iface);
  }
 public:
  bool sendOutput(const int16_t pNum, const int32_t value) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->sendOutput(pNum, value);
    }
    return ifaces_[i]->sendOutput(pNum, value);
  }

  bool sendInput(const int16_t pNum, const int32_t share) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->sendInput(pNum, share);
    }
    return ifaces_[i]->sendInput(pNum, share);
  }

};

// The 'concurrent' client is a thread safe client that correctly handles
// out of order responses.  It is slower than the regular client, so should
// only be used when you need to share a connection among multiple threads
class PlayerConcurrentClient : virtual public PlayerIf {
 public:
  PlayerConcurrentClient(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) {
    setProtocol(prot);
  }
  PlayerConcurrentClient(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, boost::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) {
    setProtocol(iprot,oprot);
  }
 private:
  void setProtocol(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) {
  setProtocol(prot,prot);
  }
  void setProtocol(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, boost::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) {
    piprot_=iprot;
    poprot_=oprot;
    iprot_ = iprot.get();
    oprot_ = oprot.get();
  }
 public:
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> getInputProtocol() {
    return piprot_;
  }
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> getOutputProtocol() {
    return poprot_;
  }
  bool sendOutput(const int16_t pNum, const int32_t value);
  int32_t send_sendOutput(const int16_t pNum, const int32_t value);
  bool recv_sendOutput(const int32_t seqid);
  bool sendInput(const int16_t pNum, const int32_t share);
  int32_t send_sendInput(const int16_t pNum, const int32_t share);
  bool recv_sendInput(const int32_t seqid);
 protected:
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> piprot_;
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> poprot_;
  ::apache::thrift::protocol::TProtocol* iprot_;
  ::apache::thrift::protocol::TProtocol* oprot_;
  ::apache::thrift::async::TConcurrentClientSyncInfo sync_;
};

#ifdef _WIN32
  #pragma warning( pop )
#endif

} // namespace

#endif
