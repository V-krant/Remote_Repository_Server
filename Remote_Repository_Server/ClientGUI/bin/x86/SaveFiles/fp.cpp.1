#pragma once
///////////////////////////////////////////////////////////////////////
// Function Pointer - Demonstrate Function Pointers                  //
//                                                                   //
// Jim Fawcett, CSE687 - Object Oriented Design, Spring 2017         //
///////////////////////////////////////////////////////////////////////

#include <string>
#include <iostream>

// alias for pointer to global function
using FPvv = void(*)();

// alias for pointer to global function with unspecified argument
template<typename Param>
using FPvp = void(*)(Param);

// alias for pointer to gloabal function with argument and return value
template<typename Return, typename Param>
using FPrp = Return(*)(Param);

// global function 
void say()
{
  std::cout << "\n  my name is say";
}

// global function with argument
template<typename Param>
void sayType(Param)
{
  std::cout << "\n  argument type is \"" << typeid(Param).name() << "\"";
}

// global function with argument and return value
template<typename Return, typename Param>
Return same(Param)
{
  if (typeid(Param).name() == typeid(Return).name())
  {
    std::cout << "\n  Param and Return are same type";
    return true;
  }
  else
  {
    std::cout << "\n  Param and Return are not same type";
    return false;
  }
}

///////////////////////////////////////////////////////////////////////
// FunctionPointer<Return, Param> class
// - used to demonstrate member function pointers

template<typename Return, typename Param>
class FunctionPointer
{
public:
  // alias's for member function pointers
  using FPvvm = void(FunctionPointer<Return, Param>::*)();
  using FPvpm = void(FunctionPointer<Return, Param>::*)(Param);
  using FPrpm = Return(FunctionPointer<Return, Param>::*)(Param);

  FunctionPointer(Param p)
  {
    p_ = p;
  }
  void say()
  {
    std::cout << "\n  my name is \"" << "say" << "\"";
  }
  void sayType(Param)
  {
    std::cout << "\n  argument type is \"" << typeid(Param).name() << "\"";
  }
  bool same(Param p)
  {
    return p == p_;
  }
private:
  Param p_;
};

