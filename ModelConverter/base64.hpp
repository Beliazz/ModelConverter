#ifndef _BASE64_HPP
#define _BASE64_HPP

#include <algorithm>


namespace base64
{
  typedef unsigned uint32;
  typedef unsigned char uint8;

  extern const char* to_table;
  extern const char* to_table_end;

  extern const char* from_table;


  template <class InputIterator, class OutputIterator>
  void encode(const InputIterator& begin, 
              const InputIterator& end, 
              OutputIterator out)
  {
    InputIterator it = begin;
    int lineSize = 0;
  
    int bytes;
    do
    {
      uint32 input = 0;

      // get the next three bytes into "in" (and count how many we actually get)
      bytes = 0;
      for(; (bytes < 3) && (it != end); ++bytes, ++it)
      {
        input <<= 8;
        input += static_cast<uint8>(*it);
      }

      // convert to base64
      int bits = bytes*8;
      while (bits > 0)
      {
        bits -= 6;
        const uint8 index = ((bits < 0) ? input << -bits : input >> bits) & 0x3F;
        *out = to_table[index];
        ++out;
        ++lineSize;
      }

      if (lineSize >= 76) // ensure proper line length 
      {
        *out = 13;
        ++out;
        *out = 10;
        ++out;
        lineSize = 0;
      }

    } while (bytes == 3);


    // add pad characters if necessary
    if (bytes > 0)
      for(int i=bytes; i < 3; ++i)
      {
        *out = '=';
        ++out;
      }
  }


  template <class InputIterator, class OutputIterator>
  void decode(const InputIterator& begin, 
              const InputIterator& end, 
              OutputIterator out)
  {
    InputIterator it = begin;
    int chars;

    do
    {
      uint8 input[4] = {0, 0, 0, 0};

      // get four characters
      chars=0;
      while((chars<4) && (it != end))
      {
        uint8 c = static_cast<char>(*it);
        if (c == '=') break; // pad character marks the end of the stream
        ++it;

        if (std::find(to_table, to_table_end, c) != to_table_end)
        {
          input[chars] = from_table[c];
          chars++;
        }
      }

      // output the binary data
      if (chars >= 2)
      {
        *out = static_cast<uint8>((input[0] << 2) + (input[1] >> 4));
        ++out;
        if (chars >= 3)
        {
          *out = static_cast<uint8>((input[1] << 4) + (input[2] >> 2));
          ++out;
          if (chars >= 4)
          {
            *out = static_cast<uint8>((input[2] << 6) + input[3]);
            ++out;
          }
        }
      }
    } while (chars == 4);

  }

} // end namespace

#endif