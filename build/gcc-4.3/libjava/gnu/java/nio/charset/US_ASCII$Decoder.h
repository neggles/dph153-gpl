
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_java_nio_charset_US_ASCII$Decoder__
#define __gnu_java_nio_charset_US_ASCII$Decoder__

#pragma interface

#include <java/nio/charset/CharsetDecoder.h>
extern "Java"
{
  namespace gnu
  {
    namespace java
    {
      namespace nio
      {
        namespace charset
        {
            class US_ASCII$Decoder;
        }
      }
    }
  }
  namespace java
  {
    namespace nio
    {
        class ByteBuffer;
        class CharBuffer;
      namespace charset
      {
          class Charset;
          class CoderResult;
      }
    }
  }
}

class gnu::java::nio::charset::US_ASCII$Decoder : public ::java::nio::charset::CharsetDecoder
{

public: // actually package-private
  US_ASCII$Decoder(::java::nio::charset::Charset *);
public: // actually protected
  ::java::nio::charset::CoderResult * decodeLoop(::java::nio::ByteBuffer *, ::java::nio::CharBuffer *);
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_java_nio_charset_US_ASCII$Decoder__