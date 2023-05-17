#pragma once

// program mode settings
enum { DIMENSION = 4 };
// end of program mode settings

enum MoveStatus { Left, Up, Right, Down };

int Solve(const unsigned char* pInput, unsigned char* pResult);
