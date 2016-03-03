//
//  ParseExpression.hpp
//  SecretSharing
//
//  Created by Trevor Meiss on 2/25/16.
//  Copyright © 2016 Trevor Meiss. All rights reserved.
//

#ifndef ParseExpression_hpp
#define ParseExpression_hpp

#include <stdio.h>
#include <string>
#include <vector>

std::vector<std::string> parseExpression(std::string s);
void printRPN(std::vector<std::string> &RPN);
int RPNtoInt(std::vector<std::string> &tokens, const std::vector<int> &playerValues, const int prime);

#endif /* ParseExpression_hpp */
