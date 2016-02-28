/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements. See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership. The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

namespace cpp ceps

struct ServerAddress {
  1: i16 port,
  2: string hostname,
}

struct Answer {
  1: i32 answer,
  2: i32 compTime,
  3: i32 transTime
}

exception InvalidOperation {
  1: i32 whatOp,
  2: string why
}

/*****************
 * Services
 *****************/
service Player {
  bool sendOutput(1:i16 pNum, 2:i32 value),
  bool sendInput(1:i16 pNum, 2:i32 share)
}

service Administrator {
  bool connectToPlayer(1:ServerAddress serverAddress),
  bool setPlayerNum(1:i16 num),
  bool setInput(1:i32 value),
  bool shareInput(),
  bool setPrime(1:i32 prime),
  Answer evaluateEquation(1:list<string> rpnExpression)
}