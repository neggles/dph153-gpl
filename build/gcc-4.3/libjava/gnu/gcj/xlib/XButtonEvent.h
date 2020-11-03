
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_gcj_xlib_XButtonEvent__
#define __gnu_gcj_xlib_XButtonEvent__

#pragma interface

#include <gnu/gcj/xlib/XEvent.h>
extern "Java"
{
  namespace gnu
  {
    namespace gcj
    {
      namespace xlib
      {
          class XAnyEvent;
          class XButtonEvent;
      }
    }
  }
}

class gnu::gcj::xlib::XButtonEvent : public ::gnu::gcj::xlib::XEvent
{

public:
  XButtonEvent(::gnu::gcj::xlib::XAnyEvent *);
public: // actually package-private
  virtual void init();
public:
  static const jint MASK_SHIFT = 1;
  static const jint MASK_LOCK = 2;
  static const jint MASK_CONTROL = 4;
  static const jint MASK_MOD1 = 8;
  static const jint MASK_MOD2 = 16;
  static const jint MASK_MOD3 = 32;
  static const jint MASK_MOD4 = 64;
  static const jint MASK_MOD5 = 128;
  jlong __attribute__((aligned(__alignof__( ::gnu::gcj::xlib::XEvent)))) time;
  jint x;
  jint y;
  jint state;
  jint button;
  static ::java::lang::Class class$;
};

#endif // __gnu_gcj_xlib_XButtonEvent__
