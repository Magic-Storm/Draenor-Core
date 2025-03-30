/*
* This file is part of DreanorCore. 
* See LICENSE.md file for Copyright information
*/

#ifndef TRINITY_TYPELIST_H
#define TRINITY_TYPELIST_H

template<class ArgumentType, class ResultType>
struct unary_function
{
    typedef ArgumentType argument_type;
    typedef ResultType result_type;
};

#define DR_UNARY_FUNCTION unary_function

#endif 