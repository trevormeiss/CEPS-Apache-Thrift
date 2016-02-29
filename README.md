# CEPS-Apache-Thrift
An implementation of Circuit Evaluation with Passive Security using Apache Thrift

An overview of the CEPS protocol can be found in chapter 3 of **Secure Multiparty Computation and Secret Sharing** by Ronald Cramer (Centrum Wiskunde & Informatica), Ivan Damgard, and Jesper Nielsen (Aarhus University).

This protocol allows for quantum-secure [multiparty computation](https://en.wikipedia.org/wiki/Secure_multi-party_computation) without a trusted third party. This protocol assumes "honest, but curious" players.

## Video Demonstration
https://youtu.be/3a-1CwefV3o

## Design Overview
![Schematic](https://github.com/trevormeiss/CEPS-Apache-Thrift/blob/master/CEPS_Apache_Thrift.png)
### Why Apache Thrift?
While this implementation solely uses C++, Apache Thrift allows for scalable cross-language communication. Therefore each player and admin can be implemented in Python, Java, or many other languages and still be able to perform secure multiparty computation.
### Why is there an Admin?
The admin exists to connect, initialize, and send expressions to the players. The admin does not view any of the communication between players. One could easily move the admin functionality to the players to get rid of the third party.

## Build
1. Download, Build, and Install [Apache Thrift](https://thrift.apache.org/)
  * On OSX, it's easiest to use `$ brew install thrift`
2. Navigate to `\gen-cpp`
3. `$ make`

## Run
1. Start two or more players on one or multiple machines
  * `$ ./CEPS_Player PORT`
2. Start an administrator on any machine to connect and initialize the players
  * `$ ./CEPS_Admin`
3. With the admin, connect to all the players
  * `c IP PORT`
4. For sanity check, list all connected players
  * `ls`
5. Optionally set the prime number to be used by all players
  * `p PRIME`
6. Begin input sharing phase
  * `s`
  * the admin will assign each player a number and random input in Zp
7. Enter expression for players to evaluate
  * Valid examples:
    * `p1+p2+p3`
    * `p1+p2*(p3+5)`
    * `(p1-p2)^3*p3`
    * `p1^3 * p2 + p3`
    * `p2 * p3 * p1 + p2^7`
  * Invalid examples:
    * `p1/5` - division not yet supported
    * `p1^p2` - power must be natural number in Zp
    * `x1+x2*3` - player variables are specified with 'p'
8. End CEPS
  * `q`
9. Add more players, change the prime, or quit. Starting the input sharing phase again will assign new random inputs to each player.
