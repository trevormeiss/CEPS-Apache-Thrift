#define POLYNOMIAL_DEGREE 1

#include <thrift/concurrency/ThreadManager.h>
#include <thrift/concurrency/PlatformThreadFactory.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/protocol/TMultiplexedProtocol.h>
#include <thrift/processor/TMultiplexedProcessor.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/server/TThreadPoolServer.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>
#include <thrift/TToString.h>

#include <boost/make_shared.hpp>
#include <mutex>

#include <iostream>
#include <stdexcept>
#include <sstream>
#include <string>
#include <fstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <map>
#include <random>
#include <atomic>
#include <queue>
#include <stack>
#include <deque>
#include <chrono>

#include "Administrator.h"
#include "Player.h"

using namespace apache::thrift;
using namespace apache::thrift::concurrency;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;

using boost::shared_ptr;

using namespace ceps;
using namespace std;

// A struct to hold a client and transport
//  - transport opens and closes communication
//  - methods are called on client
struct PlayerClientTransport {
  PlayerClient client;
  boost::shared_ptr<TTransport> transport;

  PlayerClientTransport(PlayerClient pClient, boost::shared_ptr<TTransport> pTransport)  : client(pClient) {
    transport = pTransport;
  }
};

//**********************************************************
// error
//
// logs errors and quits
//**********************************************************
void error(const string msg) {
  fprintf(stderr, "ERROR: %s\n", msg.c_str());
  exit(1);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Test if token is an operator
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool isTokenOperator( const std::string& token) {
  return token == "+" ||
         token == "-" ||
         token == "*" ||
         token == "^";
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// random number generator from Stroustrup:
// http://www.stroustrup.com/C++11FAQ.html#std-random
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int rand_int(int low, int high)
{
    static default_random_engine re (random_device{}());
    using Dist = std::uniform_int_distribution<int>;
    static Dist uid {};
    return uid(re, Dist::param_type{low,high});
}


//==========================================================
// PlayerStore
// all threads have a pointer to this object
//==========================================================
class PlayerStore {
private:
  int playerNum;
  int value;
  int prime;
  int numPlayers;
  vector<int> coefficients;
  vector<int> inputs;
  vector<int> outputs;
  vector<int> recomb_all;

  // Thread handling
  atomic<bool> receivingOpen;
  int numReceived;
  mutex input_mutex, output_mutex;

  // Connected Players
  map<ServerAddress, PlayerClientTransport*> connectedPlayers;

  // Runtime
  chrono::microseconds input_sharing_comp;
  chrono::microseconds input_sharing_trans;
  chrono::microseconds total_comp;
  chrono::microseconds total_trans;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // waitForReceivingOpen
  // do not accept share from player until desired
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  void waitForReceivingOpen() {
    while (!receivingOpen.load()) {;}
  }
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // incrementNumReceived
  // keep accepting shares until all are received
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  void incrementNumReceived() {
    waitForReceivingOpen();
    numReceived++;
    if (numReceived >= numPlayers) {
      receivingOpen.store(false);
      numReceived = 0;
    }
  }
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Evaluate polynomial for player i
  // val is the independent term
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  int generate_share(int i, int val) {
    auto comp_start = chrono::high_resolution_clock::now(); 

    // f(i) = val + a1*i + a2*i^2 + ... + an*i^n mod p
    int evaluation = val;
    for (int j=0; j<POLYNOMIAL_DEGREE; j++) {
        evaluation += coefficients[j] * pow(i,j+1);
    }

    auto comp_end = chrono::high_resolution_clock::now();    
    auto dur = comp_end - comp_start;
    auto us = chrono::duration_cast<chrono::microseconds>(dur);
    total_comp += us;
    total_trans -= us;

    return evaluation % prime;
  }
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Calculate the inverse modulus
  // Fermat’s Little Theorem
  // https://comeoncodeon.wordpress.com/2011/10/09/modular-multiplicative-inverse/
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /* This function calculates (a^b)%MOD */
  int fermatPow(int a, int b, int MOD) {
    int x = 1, y = a;
    while(b > 0) {
        if(b%2 == 1) {
            x=(x*y);
            if(x>MOD) x%=MOD;
        }
        y = (y*y);
        if(y>MOD) y%=MOD;
        b /= 2;
    }
    return x;
  }
   
  int modInverse(int a, int m) {
    if (a<0) a = (((a%m)+m) % m);
    return fermatPow(a,m-2,m);
  }
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Delta function
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  int delta(int i, int x, queue<int> players) {
    int product = 0;
    int j;
    while (!players.empty()) {
        j = players.front();
        players.pop();
        
        int y = (x-j)*modInverse(i-j, prime);
        if (product == 0)   product = y;
        else                product *= y;
    }
    product %= prime;
    product += prime;
    return product % prime;
  }
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Calculate recombination vector
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  vector<int> recombination_vector(queue<int> &players) {
    vector<int> recomb;
    int i;
    for (int j=0; j<players.size(); j++) {
        i = players.front();
        players.pop();
        recomb.push_back(delta(i, 0, players));
        players.push(i);
    }
    return recomb;
  }
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // addGate
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  string addGate(int num1, int num2) {
    return to_string((num1 + num2) % prime);
  }
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // subGate
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  string subGate(int num1, int num2) {
    return to_string((((num1 - num2) % prime) + prime) % prime);
  }
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // multGate
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  string multGate(int num1, int num2, bool bothPlayers) {
    int out = num1 * num2;
    
    // Two player shares are being multiplied
    if (bothPlayers) {
      // Allow this player to receive shares from other players
      receivingOpen.store(true);

      // Start recording transmission time
      auto trans_start = chrono::high_resolution_clock::now();

      // Give all players a share of output
      int i=1;
      for (auto player : connectedPlayers) {
        player.second->client.sendOutput(playerNum,generate_share(i,out));
        i++;
      }

      // Wait to obtain share from all players
      while (receivingOpen.load()) {;}

      // Save transmission time
      auto trans_end = chrono::high_resolution_clock::now();
      auto dur = trans_end - trans_start;
      auto us = chrono::duration_cast<chrono::microseconds>(dur);
      total_trans += us;
      total_comp -= us;

      out = 0;
      for (int i=0; i<numPlayers; i++) {
          out += (recomb_all[i]*outputs[i]) % prime;
      }
    }

    return to_string(out % prime);
  }
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // convertStringToInt
  //  - and indicate if constant or player value
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  int convertStringToInt(string str, bool &isPlayer) {
    int num;
    // Leading p signifies to use input share of player p
    if (tolower(str[0]) == 'p') {
      isPlayer = true;
      num = inputs[stoi(str.substr(1))-1];
    }
    // Trailing p signifies result from two player multiplication
    else if (str[str.size()-1] == 'p') {
      isPlayer = true;
      // Remove the trailing 'p' and convert to integer
      num = stoi(str.substr(0,str.size()-1));
    }
    else num = stoi(str);

    return num;
  }
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Output Reconstruction
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  void outputReconstruction(Answer &_return, string &result) {
    bool dummy = false;
    int out = convertStringToInt(result, dummy);

    // Allow this player to receive output from other players
    receivingOpen.store(true);

    // Start recording transmission time
    auto trans_start = chrono::high_resolution_clock::now();

    for (auto player : connectedPlayers) {
      player.second->client.sendOutput(playerNum,out);
    }

    // Wait to get output from all players
    while (receivingOpen.load()) {;}

    // Save transmission time
    auto trans_end = chrono::high_resolution_clock::now();
    auto dur = trans_end - trans_start;
    auto us = chrono::duration_cast<chrono::microseconds>(dur);
    total_trans += us;
    total_comp -= us;

    // Solve output
    out = 0;
    for (int i=0; i<numPlayers; i++) {
      out += recomb_all[i] * outputs[i];
    }
    _return.answer = out % prime;
  }

public:
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Constructor
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  PlayerStore() {
    playerNum = 1;
    value     = 1;
    prime     = 17;
    receivingOpen.store(false);
    // Generate random coefficients
    for (int i=0; i<POLYNOMIAL_DEGREE; i++) {
        coefficients.push_back(rand_int(0, prime-1));
    }
    input_sharing_trans = chrono::microseconds::zero();
    input_sharing_comp  = chrono::microseconds::zero();
    total_trans         = chrono::microseconds::zero();
    total_comp          = chrono::microseconds::zero();
  }
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // lockPlayers
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  void lockPlayers() {
    numPlayers    = (int)connectedPlayers.size();
    inputs        = vector<int> (numPlayers);
    outputs       = vector<int> (numPlayers);
    numReceived   = 0;
    receivingOpen.store(false);

    auto comp_start = chrono::high_resolution_clock::now(); 

    // Calculate recombination vector for all players
    queue<int> all_players;
    for (int i=1; i<=numPlayers; i++) {
        all_players.push(i);
    }
    recomb_all = recombination_vector(all_players);

    auto comp_end = chrono::high_resolution_clock::now();    
    auto dur = comp_end - comp_start;
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(dur);
    input_sharing_comp = us;
  }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Input sharing phase
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  void shareInput() {
    auto begin = chrono::high_resolution_clock::now();  
    int i=1;
    for (auto player : connectedPlayers) {
      player.second->client.sendInput(playerNum,generate_share(i, value));
      i++;
    }
    auto end = chrono::high_resolution_clock::now();    
    auto dur = end - begin;
    auto us = chrono::duration_cast<chrono::microseconds>(dur);
    input_sharing_trans += us;
    input_sharing_comp += total_comp;

    cout << "Share input trans time: " << input_sharing_trans.count() << "us" << endl;
    cout << "Share input comp time: " << input_sharing_comp.count() << "us" << endl;
  }
  void setNum(const int16_t num) {
    playerNum = num;
  }
  void setValue(const int32_t v) {
    value = v;
  }
  int getValue() {
    return value;
  }
  void setPrime(const int32_t p) {
    prime = p;
  }
  void setInput(const int pNum, const int val) {
    // Lock
    lock_guard<mutex> input_lk(input_mutex);
    inputs[pNum-1] = val;
    //cout << "Received input share from player " << pNum << endl;
  }
  void setOutput(const int pNum, const int val) {
    // Lock
    lock_guard<mutex> output_lk(output_mutex);
    incrementNumReceived();
    outputs[pNum-1] = val;
    //cout << "Received output share from player " << pNum << endl;
  }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Connect to another player
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  bool connectToPlayer(const ServerAddress &pServerAddress) {
    bool result = false;
    // Make sure not already connected
    auto it = connectedPlayers.find(pServerAddress);
    // Not found, try to connect
    if (it == connectedPlayers.end() || it->second->transport == NULL) {
      // Set up connection
      boost::shared_ptr<TTransport> socket(new TSocket(pServerAddress.hostname, pServerAddress.port));
      boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
      boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
      boost::shared_ptr<TProtocol> mp(new TMultiplexedProtocol(protocol, "Player"));
      PlayerClient pClient(mp);
      try {
        // Connect to server
        transport->open();
        connectedPlayers[pServerAddress] = new PlayerClientTransport(pClient, transport);
        result = true;
      } catch (TException& tx) {
        string what(tx.what());
        printf("Failed to connect to player\n");
      }
    } // end if
    return result;
  }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Evaluate RPN!
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  void evaluateRPN(Answer &_return, const vector<string> &RPN) {
    total_trans = chrono::microseconds::zero();
    total_comp  = chrono::microseconds::zero();
    auto comp_start = chrono::high_resolution_clock::now();  

    stack<string> st;
    // Convert vector to deque
    deque<string> tokens;
    for (auto &str : RPN) {
      tokens.push_back(str);
    }

    while (!tokens.empty()) {
      const string token = tokens.front();
      tokens.pop_front();

      if (!isTokenOperator(token)) {
        st.push(token);
      }
      else {
        string result = "";
        // Token is an operator: pop top two entries
        bool val1player = false;
        bool val2player = false;
        string val2 = st.top(); st.pop();
        int num2 = convertStringToInt(val2, val2player);

        // Pop another value if not empty
        if (!st.empty()) {
          
          string val1 = st.top(); st.pop();
          int num1 = convertStringToInt(val1, val1player);

          if (token == "+") result = addGate(num1, num2);
          else if (token == "-") result = subGate(num1, num2);
          else if (token == "*") result = multGate(num1, num2, (val1player && val2player));

          else if (token == "^" && !val2player) {
            if (num2 == 0) result = "1";
            else {
              result = val1;
              for (int j=1; j<num2; j++) {
                tokens.push_front("*");
                st.push(val1);
              }
            }
          } // end token is power operator
        } // end token is operator
        // Stack was empty
        else {
          result = val2;
        }
        // Append p to signify result from at least one player share
        if (val1player || val2player) result += "p";

        st.push(result);
      }
    }

    if (st.size() > 1) cout << "Stack size is " << st.size() << endl;
    // Share the final output with other players
    outputReconstruction(_return, st.top());

    auto comp_end = chrono::high_resolution_clock::now();    
    auto dur = comp_end - comp_start;
    auto us = chrono::duration_cast<chrono::microseconds>(dur);
    total_comp += us + input_sharing_comp;
    total_trans += input_sharing_trans;

    cout << "Total trans time: " << total_trans.count() << "us" << endl;
    cout << "Total comp time: " << total_comp.count() << "us" << endl;
  }
};


//==========================================================
// Handlers
//==========================================================

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// AdministratorHandler
// handles calls from the admin
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class AdministratorHandler : virtual public AdministratorIf {
private:
  PlayerStore* thisPlayer;
 public:
  AdministratorHandler(PlayerStore* playerStore) {
    thisPlayer = playerStore;
  }

  bool connectToPlayer(const ServerAddress& serverAddress) {
    cout << "Connecting to player at " << serverAddress.hostname << ":" << serverAddress.port << endl;
    return thisPlayer->connectToPlayer(serverAddress);
  }

  // Once a player number is set, no more players will be added
  bool setPlayerNum(const int16_t num) {
    thisPlayer->setNum(num);
    cout << "Player number set to " << num << endl;
    thisPlayer->lockPlayers();
    cout << "No more players will be added\n";
    return true;
  }

  // set the input for this player
  bool setInput(const int32_t value) {
    thisPlayer->setValue(value);
    cout << "Input set to " << value << endl;
    return true;
  }

  // Give share of input to all players
  bool shareInput() {
    thisPlayer->shareInput();
    cout << "Input sharing complete\n";
    return true;
  }

  bool setPrime(const int32_t prime) {
    cout << "Prime set to " << prime << endl;
    thisPlayer->setPrime(prime);
    return true;
  }

  void evaluateEquation(Answer& _return, const std::vector<std::string> & rpnExpression) {
    cout << "~\n";
    thisPlayer->evaluateRPN(_return, rpnExpression);
    cout << "Answer = " << _return.answer << endl;
  }

};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// PlayerHandler
// handles calls from other players
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class PlayerHandler : virtual public PlayerIf {
private:
  PlayerStore* thisPlayer;
 public:
  PlayerHandler(PlayerStore* playerStore) {
    thisPlayer = playerStore;
  }

  bool sendOutput(const int16_t pNum, const int32_t value) {
    thisPlayer->setOutput(pNum, value);
    return true;
  }

  bool sendInput(const int16_t pNum, const int32_t share) {
    thisPlayer->setInput(pNum, share);
    return true;
  }
};

//**********************************************************
// checkArgs
//
// validates arguments
//**********************************************************
void checkArgs(int argc, char **argv, int &port) {
  if (argc < 2) {
    error("please use arguments: ./CEPS_Player PORT");
  }

  port = atoi(argv[1]);
  if (port < 2000 || port > 65535) {
    error("please use a port number between 2000 and 65535");
  }
}

//**********************************************************
// Main
//**********************************************************
int main(int argc, char *argv[]) {

  int port;
  PlayerStore* thisPlayer;

  // Make sure valid port
  checkArgs(argc, argv, port);

  thisPlayer = new PlayerStore();

  // Handlers
  boost::shared_ptr<AdministratorHandler> admin_handler(new AdministratorHandler(thisPlayer));
  boost::shared_ptr<PlayerHandler> player_handler(new PlayerHandler(thisPlayer));

  // Processors
  boost::shared_ptr<TMultiplexedProcessor> processor(new TMultiplexedProcessor());
  boost::shared_ptr<TProcessor> admin_processor(new AdministratorProcessor(admin_handler));
  boost::shared_ptr<TProcessor> player_processor(new PlayerProcessor(player_handler));

  // Add all the services!!!
  processor->registerProcessor("Administrator", admin_processor);
  processor->registerProcessor("Player", player_processor);

  boost::shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
  boost::shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
  boost::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());
  boost::shared_ptr<ThreadManager> threadManager = ThreadManager::newSimpleThreadManager(4);
  boost::shared_ptr<PosixThreadFactory> threadFactory = boost::shared_ptr<PosixThreadFactory> (new PosixThreadFactory());
  threadManager->threadFactory(threadFactory);
  threadManager->start();

  TThreadedServer server(processor, serverTransport, transportFactory, protocolFactory);

  cout << "Player started, waiting for admin..." << endl;
  server.serve();
  cout << "Done." << endl;
  return 0;
}