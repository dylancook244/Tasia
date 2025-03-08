#ifndef __TOKEN_H__
#define __TOKEN_H__

// The lexer returns tokens [0-255] if it's an unknown character
// otherwise it returns one of these for known things
enum Token {
  // End Of File or line
  tok_eof = -1,
  tok_line_end = -2,

  // Commands
  tok_func = -3,
  tok_extern = -4,

  // Primary
  tok_identifier = -5,
  tok_number = -6,
};

#endif
