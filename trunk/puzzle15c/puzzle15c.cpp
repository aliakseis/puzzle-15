// puzzle15c.cpp : Defines the entry point for the console application.
//

#include "solver.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fstream>
#include <iostream>
#include <ostream>
#include <iomanip>
#include <algorithm>


using std::cerr;
using std::setw;
using std::setfill;
using std::endl;

char* GetFileName(char* szPath)
{
    if (!szPath) 
        return 0;
    for(int nPos = int(strlen(szPath)) - 1;
        nPos >= 0;
        nPos--)
    {
        char ch = szPath[nPos];
        if ('\\' == ch || '/' == ch || ':' == ch) 
        {
            return szPath + nPos + 1;
        }
    }

    return szPath;
}

const char g_arrHexvalues[] = "0123456789ABCDEF";

class DrawSolution
{
    typedef int Cell;
    typedef unsigned char Move;

    Cell m_cells[DIMENSION][DIMENSION];
    int m_x, m_y;
    std::ostream& m_output;

public:
    DrawSolution(std::ostream& output) : m_output(output) {}

    void SetCell(int x, int y, int value)
    {
        m_cells[x][y] = (Cell) value;
    }

    void SetBlank(int x, int y)
    {
        m_x = x;
        m_y = y;
    }

    void operator()(const Move& move)
    {
        MoveStatus nextMove = (MoveStatus) move;

        int newX = m_x;
        int newY = m_y;

        switch (nextMove)
        {
        case Left: m_y++; break;
        case Right: m_y--; break;
        case Up: m_x++; break;
        case Down: m_x--; break;
        }

        Cell value = m_cells[m_x][m_y];
        m_cells[newX][newY] = value;

        m_output << g_arrHexvalues[value];
    }
};

inline int GetCellValue(char ch)
{
    return (ch < 'A') ? ch - '0' : (ch & ~32) - 'A' + 10;
}

int main(int /*argc*/, char* argv[])
{
    // The input and output files should be in the same location as our executable. 
    char path[_MAX_PATH];
    strcpy(path, argv[0]);
    char* pFileName = GetFileName(path);

    strcpy(pFileName, "moves.txt");
    std::ifstream inputFile(path);
    if (!inputFile) 
    {
        cerr << "Unable to open input file.\n";
        return EXIT_FAILURE;
    }

    strcpy(pFileName, "results.txt");
    std::ofstream outputFile(path);
    if (!outputFile) 
    {
        cerr << "Unable to open output file.\n";
        return EXIT_FAILURE;
    }


    clock_t start = clock();
    int count = 0;

    while (!inputFile.eof())
    {
        char line[DIMENSION * DIMENSION + 1];
        inputFile.getline(line, sizeof(line) / sizeof(line[0]));

        if (0 == line[0])
            continue;


        outputFile << setfill('0') << setw(4) << ++count << ' ' << line << ' ';

        unsigned char input[DIMENSION * DIMENSION];
        DrawSolution drawSolution(outputFile);

        for (int i = 0; i < DIMENSION * DIMENSION; ++i)
        {
            int value = GetCellValue(line[i]);
            input[i] = value;
            if (0 != value)
            {
                drawSolution.SetCell(i / DIMENSION, i % DIMENSION, value);
            }
            else
            {
                drawSolution.SetBlank(i / DIMENSION, i % DIMENSION);
            }
        }

        unsigned char solution[100];

        const int solutionSize = Solve(input, solution);

        if (solutionSize >= 0)
        {
            std::for_each(solution, solution + solutionSize, drawSolution);
        }

        //outputFile << endl;
        outputFile << '\n';
    }

    outputFile << "\nIt took " << (double)(clock() - start) / CLOCKS_PER_SEC << " seconds." << endl;

    return 0;
}
