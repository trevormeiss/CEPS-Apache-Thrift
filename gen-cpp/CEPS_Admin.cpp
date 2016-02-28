
#include <iostream>
#include <sstream>
#include <string>
#include <random>
#include <fstream>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <thread>
#include <future>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/protocol/TMultiplexedProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include <boost/algorithm/string.hpp>

#include "ParseExpression.hpp"
#include "Administrator.h"

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

using namespace ceps;
using namespace std;

// A struct to hold a client and transport
struct AdministratorClientTransport {
  AdministratorClient client;
  boost::shared_ptr<TTransport> transport;

  AdministratorClientTransport(AdministratorClient aClient, boost::shared_ptr<TTransport> aTransport)  : client(aClient) {
    transport = aTransport;
  }
};

//**********************************************************
// error
//
// prints error and quits
//**********************************************************
void error(string msg)
{
  fprintf(stderr, "ERROR: %s\n\n", msg.c_str());
  exit(1);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// random number generator from Stroustrup:
// http://www.stroustrup.com/C++11FAQ.html#std-random
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int rand_int(int low, int high)
{
    static default_random_engine re (random_device{}());
    using Dist = std::uniform_int_distribution<int>;
    static Dist uid {};
    return uid(re, Dist::param_type{low,high});
}

//**********************************************************
// Split string into vector
// http://stackoverflow.com/a/236803
//**********************************************************
vector<string> &split(const string &s, char delim, vector<string> &elems) {
    stringstream ss(s);
    string item;
    while (getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

vector<string> split(const string &s, char delim) {
    vector<string> elems;
    split(s, delim, elems);
    return elems;
}

//**********************************************************
// addNewPlayer
//  - connect to player
//  - add player to set of connected players
//  - tell all other players about new player
//**********************************************************
void addNewPlayer(ServerAddress &serverAddress, map<ServerAddress, AdministratorClientTransport*> &connectedPlayers) {
  // Make sure not already connected
  auto it = connectedPlayers.find(serverAddress);

  if (it == connectedPlayers.end() || it->second->transport == NULL) {
    // Set up connection
    boost::shared_ptr<TTransport> socket(new TSocket(serverAddress.hostname, serverAddress.port));
    boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    boost::shared_ptr<TProtocol> mp(new TMultiplexedProtocol(protocol, "Administrator"));
    AdministratorClient aPlayer(mp);
    try {
      // Connect to server
      transport->open();
      // Let this player know all the other players
      for (auto clientTransport : connectedPlayers) {
        aPlayer.connectToPlayer(clientTransport.first);
      }
      // Store transport and client
      connectedPlayers[serverAddress] = new AdministratorClientTransport(aPlayer, transport);
      cout << "Connection to player successful\n\n";
    } catch (TException& tx) {
      string what(tx.what());
      cout << "Failed to connect to player at " << serverAddress.hostname << ":" << serverAddress.port << endl;
    }
    // Tell all players about this new player
    for (auto player : connectedPlayers) {
      player.second->client.connectToPlayer(serverAddress);
    }
  } // end if
  else cout << "Already connected to player at " << serverAddress.hostname << ":" << serverAddress.port << "\n\n";
}

//**********************************************************
// player_thread
//**********************************************************
void player_thread(promise<Answer> && prms, AdministratorClient aClient, const vector<string> &RPN) {
  Answer _return;
  aClient.evaluateEquation(_return, RPN);
  prms.set_value(_return);
}

//**********************************************************
// beginCEPS
//**********************************************************
void beginCEPS(map<ServerAddress, AdministratorClientTransport*> &connectedPlayers, const vector<int> &playerValues, const int prime) {
  bool quit = false;

  while (!quit) {
    int num_players = connectedPlayers.size();
    string expression;
    printf("~\nEnter expression or (q)uit: ");
    getline(cin, expression);
    if (expression[0] == 'q') {
      quit = true;
    }
    else {
      // Convert the expression into reverse polish notation
      vector<string> RPN = parseExpression(expression);
      printRPN(RPN);
      int expectedAnswer = ((RPNtoInt(RPN, playerValues) % prime) + prime) % prime;
      cout << "           Expected Answer: " << expectedAnswer << "\n\n";

      // Start a thread for each player
      vector<thread> t;
      // This will hold all of the replies from players
      vector<future<Answer> > futureAnswers;
      int i=0;
      for (auto player : connectedPlayers) {
        promise<Answer> prms;
        futureAnswers.push_back(prms.get_future());
        // Start the thread
        t.push_back(thread(&player_thread, move(prms), player.second->client, RPN));
        i++;
      }
      //Join the threads with the main thread
      for (i = 0; i < num_players; ++i) {
        t[i].join();
        Answer thisAnswer = futureAnswers[i].get();
        cout << "Player " << i+1 << " thinks the answer is " << thisAnswer.answer << endl;
      }
    } // end send expression
  }
}

//**********************************************************
// closeTransports
//**********************************************************
void closeTransports(map<ServerAddress, AdministratorClientTransport*> &connectedPlayers) {
  for (auto server : connectedPlayers) {
    server.second->transport->close();
  }
  cout << "All transports closed\n";
}

//**********************************************************
// main
//
//**********************************************************
int main(int argc, char *argv[])
{
  string viableCommands = "\nViable commands are:\n"
    "c IP PORT:\tconnect to Player\n"
    "ls:\t\tlist connected Players\n"
    "p PRIME:\tset value of prime (default 17)\n"
    "s:\t\tshare inputs and begin CEPS\n"
    "q:\t\tquit\n\n";

  cout << "~\nWelcome to CEPS!\n~" << viableCommands;

  map<ServerAddress, AdministratorClientTransport*> connectedPlayers;
  string input;
  int prime = 17;
  bool primeSet = false;
  bool quit = false;

  // Continue accepting commands until user quits
  while (!quit) {
    // Receive command from user
    printf("Enter command: ");
    getline(cin, input);

    vector<int> playerValues;
    vector<string> splitInput = split(input, ' ');

    //
    // Connect to Player
    //
    if (splitInput.size() >= 3 && boost::iequals(splitInput[0], "c")) {
      ServerAddress server;
      server.hostname = splitInput[1];
      server.port = stoi(splitInput[2]);

      addNewPlayer(server, connectedPlayers);
      // If the user first changed the prime and then added another player
      if (primeSet && !connectedPlayers[server]->client.setPrime(prime)) {
        cout << "Unable to set prime for player " << server.hostname << ":" << server.port << "\n";
      }
    }
    //
    // List connected players
    //
    else if (boost::iequals(splitInput[0], "ls")) {
      int i=1;
      for (auto server : connectedPlayers) {
        cout << "Player " << i << " at " << server.first.hostname << ":" << server.first.port << "\n";
        i++;
      }
      cout << endl;
    }
    //
    // Set prime
    //
    else if (splitInput.size() >= 2 && boost::iequals(splitInput[0], "p")) {
      primeSet = true;
      prime = stoi(splitInput[1]);
      cout << "Setting prime to " << prime << endl;
      for (auto player : connectedPlayers) {
        if (!player.second->client.setPrime(prime))
          cout << "Unable to set prime for player " << player.first.hostname << ":" << player.first.port << "\n";
      }
      cout << endl;
    }
    //
    // Share inputs and begin CEPS
    //
    else if (boost::iequals(splitInput[0], "s")) {
      if (prime <= connectedPlayers.size()) printf("Prime number must be greater than the number of players\n");
      else {
        // CEPS has started, give each player their number
        int i=1;
        for (auto player : connectedPlayers) {
          // This will also let them know that no more players will be added
          if(!player.second->client.setPlayerNum(i))
            cout << "Failed to set player number for player " << i << endl;
          i++;
        }

        // Set input and share with all players
        i = 1;
        for (auto player : connectedPlayers) {
          // Generate random input in Zp for player
          int pVal = rand_int(0, prime-1);

          if (player.second->client.setInput(pVal)) {
            cout << "Player " << i << " input: " << pVal << endl;
            playerValues.push_back(pVal);
            if (!player.second->client.shareInput())
              cout << "Player " << i << " failed to share input\n";
          }
          else
            cout << "Player " << i << " failed to set input\n";
          
          i++;
        }
        cout << "\nInput sharing complete\n";

        // Ask for expressions for the players to evaluate
        beginCEPS(connectedPlayers, playerValues, prime);
        cout << viableCommands;
      }
    }
    //
    // Quit
    //
    else if (boost::iequals(splitInput[0], "q")) {
      cout << "Admin quit\n";
      quit = true;
    }
    //
    // Command not recognized
    //
    else {
      cout << "Command not recognized\n" << viableCommands;
    }
  } // end While(!quit)
  // Close all the transports
  closeTransports(connectedPlayers);
} // end main