
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __java_util_LinkedList$1__
#define __java_util_LinkedList$1__

#pragma interface

#include <java/lang/Object.h>

class java::util::LinkedList$1 : public ::java::lang::Object
{

public: // actually package-private
  LinkedList$1(::java::util::LinkedList *);
private:
  void checkMod();
public:
  jboolean hasNext();
  ::java::lang::Object * next();
  void remove();
private:
  jint __attribute__((aligned(__alignof__( ::java::lang::Object)))) knownMod;
  ::java::util::LinkedList$Entry * next__;
  ::java::util::LinkedList$Entry * lastReturned;
  jint position;
public: // actually package-private
  ::java::util::LinkedList * this$0;
public:
  static ::java::lang::Class class$;
};

#endif // __java_util_LinkedList$1__
