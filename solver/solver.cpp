//#include "stdafx.h"

#include "solver.h"

#ifdef _MSC_VER
#pragma warning(disable:4100)
#pragma warning(disable:4127)
#endif //_MSC_VER

#include <iostream>

#include <vector>
#include <iomanip>

#include <algorithm>

#include <memory.h>


using std::cout;
using std::cerr;
using std::setw;

using std::vector;



template<int v> struct Int2Type_
{
	enum { value = v };
};

unsigned int GetBitsNumber(unsigned int v) // count the number of bits set in v
{
    // c accumulates the total bits set in v
    unsigned int c = 0;
    for (; v; c++)
    {
        v &= v - 1; // clear the least significant bit set
    }
    return c;
}



using Cell = unsigned int;

inline int GetManhattanChange(int pos, int newValue, int dir)
{
	return (pos == newValue)? -1 : ((newValue < pos)? dir : -dir);
}

/*

+-----> Y
|
|
|
V
X

*/


class BoardState
{
	template <int X, int Y>
	struct FinalPosition
	{
		static bool Is(const BoardState& board)
		{
			return X * DIMENSION + Y == board.m_cells[X][Y]
			&& FinalPosition<X, Y+1>::Is(board);
		}
	};

	template <int X>
	struct FinalPosition<X, DIMENSION-1>
	{
		static bool Is(const BoardState& board)
		{
			return DIMENSION-1 == X
				|| (X + 1) * DIMENSION - 1 == board.m_cells[X][DIMENSION-1]
			&& FinalPosition<X+1, 0>::Is(board);
		}
	};

	template <int Y>
	struct FinalPosition<DIMENSION, Y>
	{
		static bool Is(const BoardState& /*unused*/)
		{
			return true;
		}
	};

public:
	bool IsFinalPositionCandidate()
	{
		return FinalPosition<0, 1>::Is(*this);
	}

	Cell m_cells[DIMENSION][DIMENSION];

	int m_emptyX, m_emptyY;
};


class Solver
{

	inline bool HasLastMoves()
	{
		return 4 == m_boardState.m_cells[0][0]
			|| 4 == m_boardState.m_cells[0][1]
			|| 4 == m_boardState.m_cells[0][2]
			|| 4 == m_boardState.m_cells[0][3]
			|| 1 == m_boardState.m_cells[0][0]
			|| 1 == m_boardState.m_cells[1][0]
			|| 1 == m_boardState.m_cells[2][0]
			|| 1 == m_boardState.m_cells[3][0];
	}

	inline bool HasCornerTiles()
	{
		return ((8 != m_boardState.m_cells[2][0] && 13 != m_boardState.m_cells[3][1]) 
			|| 12 == m_boardState.m_cells[3][0])
		&& ((2 != m_boardState.m_cells[0][2] && 7 != m_boardState.m_cells[1][3]) 
			|| 3 == m_boardState.m_cells[0][3])
		&& ((14 != m_boardState.m_cells[3][2] && 11 != m_boardState.m_cells[2][3]) 
			|| 15 == m_boardState.m_cells[3][3]);
	}


	template <int row>
	inline bool HasNoHorizontalLinearConflict()
	{
		Cell value = m_boardState.m_cells[row][3];
		if (value >= row * DIMENSION && value < ((row + 1) * DIMENSION - 1))
			goto three;

		value = m_boardState.m_cells[row][2];
		if (value >= row * DIMENSION && value < ((row + 1) * DIMENSION - 1))
			goto two;

		value = m_boardState.m_cells[row][1];
		if (value >= row * DIMENSION && value < ((row + 1) * DIMENSION - 1))
			goto one;

		return true;

		Cell newValue;
	three:
		newValue = m_boardState.m_cells[row][2];
		if (row == newValue / DIMENSION)
		{
			if (newValue > value)
				return false;
			value = newValue;
		}
	two:
		newValue = m_boardState.m_cells[row][1];
		if (row == newValue / DIMENSION)
		{
			if (newValue > value)
				return false;
			value = newValue;
		}
	one:
		newValue = m_boardState.m_cells[row][0];
		return !(newValue > value && newValue < (row + 1) * DIMENSION);
	}


	template <int col>
	inline bool HasNoVerticalLinearConflict()
	{
		Cell value = m_boardState.m_cells[3][col];
		if (col == value % DIMENSION)
			goto three;

		value = m_boardState.m_cells[2][col];
		if (col == value % DIMENSION)
			goto two;

		value = m_boardState.m_cells[1][col];
		if (col == value % DIMENSION)
			goto one;

		return true;

		Cell newValue;
three:
		newValue = m_boardState.m_cells[2][col];
		if (col == newValue % DIMENSION)
		{
			if (newValue > value)
				return false;
			value = newValue;
		}
two:
		newValue = m_boardState.m_cells[1][col];
		if (col == newValue % DIMENSION)
		{
			if (newValue > value)
				return false;
			value = newValue;
		}
one:
		newValue = m_boardState.m_cells[0][col];
		return !(newValue > value && col == newValue % DIMENSION);
	}


	template <int row, int c1, int c2, int c3>
	inline bool TripletHasNoHorizontalLinearConflict()
	{
		Cell value = m_boardState.m_cells[row][3];
		if (c1 == value || c2 == value)
			goto three;

		value = m_boardState.m_cells[row][2];
		if (c1 == value || c2 == value)
			goto two;

		value = m_boardState.m_cells[row][1];
		if (c1 == value || c2 == value)
			goto one;

		return true;

		Cell newValue;
three:
		newValue = m_boardState.m_cells[row][2];
		if (c3 == newValue || c2 == newValue)
			return false;
two:
		newValue = m_boardState.m_cells[row][1];
		if (c3 == newValue || c2 == newValue)
			return false;
one:
		newValue = m_boardState.m_cells[row][0];
		return !(c3 == newValue || c2 == newValue);
	}

	template <int col, int c1, int c2, int c3>
	inline bool TripletHasNoVerticalLinearConflict()
	{
		Cell value = m_boardState.m_cells[3][col];
		if (c1 == value || c2 == value)
			goto three;

		value = m_boardState.m_cells[2][col];
		if (c1 == value || c2 == value)
			goto two;

		value = m_boardState.m_cells[1][col];
		if (c1 == value || c2 == value)
			goto one;

		return true;

		Cell newValue;
three:
		newValue = m_boardState.m_cells[2][col];
		if (c3 == newValue || c2 == newValue)
			return false;
two:
		newValue = m_boardState.m_cells[1][col];
		if (c3 == newValue || c2 == newValue)
			return false;
one:
		newValue = m_boardState.m_cells[0][col];
		return !(c3 == newValue || c2 == newValue);
	}

	//*
	inline bool HasNoHorizontalLinearConflict0()
	{
		Cell value = m_boardState.m_cells[0][3];
		if (2 >= value && 1 <= value)
			goto three;

		value = m_boardState.m_cells[0][2];
		if (2 >= value && 1 <= value)
			goto two;

		value = m_boardState.m_cells[0][1];
		if (2 >= value && 1 <= value)
			goto one;

		return true;

		Cell newValue;
three:
		newValue = m_boardState.m_cells[0][2];
		if (3 >= newValue && 2 <= newValue)
			return false;
two:
		newValue = m_boardState.m_cells[0][1];
		if (3 >= newValue && 2 <= newValue)
			return false;
one:
		newValue = m_boardState.m_cells[0][0];
		return !(newValue < 4 && newValue > value);
	}

	inline bool HasNoVerticalLinearConflict0()
	{
		Cell value = m_boardState.m_cells[3][0];
		if (4 == value || 8 == value)
			goto three;

		value = m_boardState.m_cells[2][0];
		if (4 == value || 8 == value)
			goto two;

		value = m_boardState.m_cells[1][0];
		if (4 == value || 8 == value)
			goto one;

		return true;

		Cell newValue;
three:
		newValue = m_boardState.m_cells[2][0];
		if (12 == newValue || 8 == newValue)
			return false;
two:
		newValue = m_boardState.m_cells[1][0];
		if (12 == newValue || 8 == newValue)
			return false;
one:
		newValue = m_boardState.m_cells[0][0];
		return !(12 == newValue || 8 == newValue);
	}
	//*/

	/*
	inline bool HasNoHorizontalLinearConflict0()
	{
		return TripletHasNoHorizontalLinearConflict<0, 1, 2, 3>();
	}

	inline bool HasNoVerticalLinearConflict0()
	{
		return TripletHasNoVerticalLinearConflict<0, 4, 8, 12>();
	}
	//*/

	template <int row>
	inline bool HorizontalLinearConflictTest(int& count)
	{
		Cell value = m_boardState.m_cells[row][3];
		if (value >= row * DIMENSION && value < ((row + 1) * DIMENSION - 1))
			goto three;

		value = m_boardState.m_cells[row][2];
		if (value >= row * DIMENSION && value < ((row + 1) * DIMENSION - 1))
			goto two;

		value = m_boardState.m_cells[row][1];
		if (value >= row * DIMENSION && value < ((row + 1) * DIMENSION - 1))
			goto one;

		return true;

		Cell newValue;
three:
		newValue = m_boardState.m_cells[row][2];
		if (row == newValue / DIMENSION)
		{
			if (newValue > value && --count == 0)
				return false;
			value = newValue;
		}
two:
		newValue = m_boardState.m_cells[row][1];
		if (row == newValue / DIMENSION)
		{
			if (newValue > value && --count == 0)
				return false;
			value = newValue;
		}
one:
		newValue = m_boardState.m_cells[row][0];
		return !(newValue > value && newValue < (row + 1) * DIMENSION && --count == 0);
	}

	template <int col>
	inline bool VerticalLinearConflictTest(int& count)
	{
		Cell value = m_boardState.m_cells[3][col];
		if (col == value % DIMENSION)
			goto three;

		value = m_boardState.m_cells[2][col];
		if (col == value % DIMENSION)
			goto two;

		value = m_boardState.m_cells[1][col];
		if (col == value % DIMENSION)
			goto one;

		return true;

		Cell newValue;
three:
		newValue = m_boardState.m_cells[2][col];
		if (col == newValue % DIMENSION)
		{
			if (newValue > value && --count == 0)
				return false;
			value = newValue;
		}
two:
		newValue = m_boardState.m_cells[1][col];
		if (col == newValue % DIMENSION)
		{
			if (newValue > value && --count == 0)
				return false;
			value = newValue;
		}
one:
		newValue = m_boardState.m_cells[0][col];
		return !(newValue > value && col == newValue % DIMENSION && --count == 0);
	}



	template <int row, int c1, int c2>
	inline bool DoubletHasNoHorizontalLinearConflict()
	{
		Cell value = m_boardState.m_cells[row][3];
		if (c1 == value)
			goto three;

		value = m_boardState.m_cells[row][2];
		if (c1 == value)
			goto two;

		value = m_boardState.m_cells[row][1];
		if (c1 == value)
			goto one;

		return true;

		Cell newValue;
three:
		newValue = m_boardState.m_cells[row][2];
		if (c2 == newValue)
			return false;
two:
		newValue = m_boardState.m_cells[row][1];
		if (c2 == newValue)
			return false;
one:
		newValue = m_boardState.m_cells[row][0];
		return c2 != newValue;
	}

	template <int col, int c1, int c2>
	inline bool DoubletHasNoVerticalLinearConflict()
	{
		Cell value = m_boardState.m_cells[3][col];
		if (c1 == value)
			goto three;

		value = m_boardState.m_cells[2][col];
		if (c1 == value)
			goto two;

		value = m_boardState.m_cells[1][col];
		if (c1 == value)
			goto one;

		return true;

		Cell newValue;
three:
		newValue = m_boardState.m_cells[2][col];
		if (c2 == newValue)
			return false;
two:
		newValue = m_boardState.m_cells[1][col];
		if (c2 == newValue)
			return false;
one:
		newValue = m_boardState.m_cells[0][col];
		return c2 != newValue;
	}



	inline bool MultiTilesTest(int count)
	{
		bool bHasLastMoves = HasLastMoves();
		if (!bHasLastMoves)
		{
			if (--count == 0)
				return false;
		}

		if (8 == m_boardState.m_cells[2][0] && 12 != m_boardState.m_cells[3][0]
		|| (bHasLastMoves? !HasNoVerticalLinearConflict0() 
			: !DoubletHasNoVerticalLinearConflict<0, 8, 12>()))
		{
			if (--count == 0)
				return false;
		}

		if (2 == m_boardState.m_cells[0][2] && 3 != m_boardState.m_cells[0][3]
		|| (bHasLastMoves? !HasNoHorizontalLinearConflict0()
			: !DoubletHasNoHorizontalLinearConflict<0, 2, 3>()))
		{
			if (--count == 0)
				return false;
		}

		bool horizontalTiles = false;
		if (13 == m_boardState.m_cells[3][1] && 12 != m_boardState.m_cells[3][0])
		{
			if(--count == 0)
				return false;
			horizontalTiles = true;
		}
		if (14 == m_boardState.m_cells[3][2] && 15 != m_boardState.m_cells[3][3])
		{
			if(--count == 0)
				return false;

			if ((horizontalTiles
				? 15 == m_boardState.m_cells[3][0] && 12 == m_boardState.m_cells[3][3]
				: !TripletHasNoHorizontalLinearConflict<3, 12, 13, 15>())
					&& --count == 0)
				return false;
		}
		else
		{
			if (!horizontalTiles)
			{
				if (!HorizontalLinearConflictTest<3>(count))
					return false;
			}
			else
			{
				if (!TripletHasNoHorizontalLinearConflict<3, 12, 14, 15>() && --count == 0)
					return false;
			}
		}

		bool verticalTiles = false;
		if (7 == m_boardState.m_cells[1][3] && 3 != m_boardState.m_cells[0][3])
		{
			if(--count == 0)
				return false;
			verticalTiles = true;
		}
		if (11 == m_boardState.m_cells[2][3] && 15 != m_boardState.m_cells[3][3])
		{
			if(--count == 0)
				return false;

			if ((verticalTiles
				? 15 == m_boardState.m_cells[0][3] && 3 == m_boardState.m_cells[3][3]
				: !TripletHasNoVerticalLinearConflict<3, 3, 7, 15>())
					&& --count == 0)
				return false;
		}
		else
		{
			if (!verticalTiles)
			{
				if (!VerticalLinearConflictTest<3>(count))
					return false;
			}
			else
			{
				if (!TripletHasNoVerticalLinearConflict<3, 3, 11, 15>() && --count == 0)
					return false;
			}
		}

		if (bHasLastMoves)
		{
			if (!HorizontalLinearConflictTest<1>(count)
				|| !VerticalLinearConflictTest<1>(count))
				return false;
		}
		else 
		{
			if (!TripletHasNoHorizontalLinearConflict<1, 5, 6, 7>() && --count == 0)
				return false;
			if (!TripletHasNoVerticalLinearConflict<1, 5, 9, 13>() && --count == 0)
				return false;
		}

		return HorizontalLinearConflictTest<2>(count)
			&& VerticalLinearConflictTest<2>(count);
	}

    inline bool HasSingleConflictPass()
    {
        return HasLastMoves()
            && HasCornerTiles()
            && HasNoHorizontalLinearConflict<2>()
            && HasNoVerticalLinearConflict<2>()
            // Intersects with other tests but can be used here
            && HasNoHorizontalLinearConflict<1>()
            && HasNoVerticalLinearConflict<1>()
            && HasNoHorizontalLinearConflict<3>()
            && HasNoVerticalLinearConflict<3>()

            && HasNoHorizontalLinearConflict0()
            && HasNoVerticalLinearConflict0();
    }


	// Solution search routines
	template <int X, int Y, bool edge>
	struct DispatchStep
	{
		static bool Do(Solver* pSolver, int emptyX, int emptyY)
		{
			if (X == emptyX && Y == emptyY)
				return pSolver->DistributeStep<X, Y, 0, 0, edge>();
			return DispatchStep<X, Y-1, edge>::Do(pSolver, emptyX, emptyY);
		}
	};

	template <int X, bool edge>
	struct DispatchStep<X, 0, edge>
	{
		static bool Do(Solver* pSolver, int emptyX, int emptyY)
		{
			if (X == emptyX && 0 == emptyY)
				return pSolver->DistributeStep<X, 0, 0, 0, edge>();
			return DispatchStep<X-1, DIMENSION-1, edge>::Do(pSolver, emptyX, emptyY);
		}
	};

	template <int Y, bool edge>
	struct DispatchStep<-1, Y, edge>
	{
		static bool Do(Solver* /*unused*/, int /*unused*/, int /*unused*/)
		{
			return false;
		}
	};


	template <int oldEmptyX, int oldEmptyY,
		int emptyXOffset, int emptyYOffset, bool returning, bool edge, typename Next>
	struct MakeStep
	{
		static bool Do (Solver* pSolver)
		{
            return pSolver->DoMakeStep<oldEmptyX, oldEmptyY,
                emptyXOffset, emptyYOffset, Next>(Int2Type_<edge>());
        }
	};

	template <int oldEmptyY, int emptyYOffset, bool returning, bool edge, typename Next>
	struct MakeStep<0, oldEmptyY, -1, emptyYOffset, returning, edge, Next>
	{
		static bool Do (Solver* pSolver)
		{
			return Next::Do(pSolver);
		}
	};

    template <int oldEmptyX, int emptyXOffset, bool returning, bool edge, typename Next>
	struct MakeStep<oldEmptyX, 0, emptyXOffset, -1, returning, edge, Next>
	{
		static bool Do (Solver* pSolver)
		{
			return Next::Do(pSolver);
		}
	};

    template <int oldEmptyY, int emptyYOffset, bool returning, bool edge, typename Next>
	struct MakeStep<DIMENSION-1, oldEmptyY, 1, emptyYOffset, returning, edge, Next>
	{
		static bool Do (Solver* pSolver)
		{
			return Next::Do(pSolver);
		}
	};

    template <int oldEmptyX, int emptyXOffset, bool returning, bool edge, typename Next>
	struct MakeStep<oldEmptyX, DIMENSION-1, emptyXOffset, 1, returning, edge, Next>
	{
		static bool Do (Solver* pSolver)
		{
			return Next::Do(pSolver);
		}
	};

	template <int oldEmptyX, int oldEmptyY,
        int emptyXOffset, int emptyYOffset, bool edge, typename Next>
	struct MakeStep<oldEmptyX, oldEmptyY,
		emptyXOffset, emptyYOffset, true, edge, Next>
	{
		static bool Do (Solver* pSolver)
		{
			return Next::Do(pSolver);
		}
	};

	struct DoNothing
	{
		static bool Do (Solver* /*unused*/) { return false; }
	};


	BoardState m_boardState;
	vector<unsigned char> m_solution;

    int m_nDerivation;

    bool m_bTopLeftBlank;

public:
	Solver(bool bTopLeftBlank) : m_bTopLeftBlank(bTopLeftBlank) {}

	void SetCell(int x, int y, int value)
	{
		if (!m_bTopLeftBlank)
		{
			x = DIMENSION - 1 - x;
			y = DIMENSION - 1 - y;
			value = DIMENSION * DIMENSION - value;
		}
		m_boardState.m_cells[x][y] = value;
	}

	void SetBlank(int x, int y)
	{
		if (!m_bTopLeftBlank)
		{
			x = DIMENSION - 1 - x;
			y = DIMENSION - 1 - y;
		}
		m_boardState.m_emptyX = x;
		m_boardState.m_emptyY = y;
	}

	int GetOptimalSolution(vector<unsigned char>& solution)
	{
		if (m_boardState.IsFinalPositionCandidate())
		{
			if (0 == m_boardState.m_emptyX && 0 == m_boardState.m_emptyY)
				return 0;
		}

        if (!DispatchStep<DIMENSION - 1, DIMENSION - 1, true>::Do(
            this, m_boardState.m_emptyX, m_boardState.m_emptyY))
        {
            int nDerivation = 1;

            while (m_nDerivation = nDerivation,
                !DispatchStep<DIMENSION - 1, DIMENSION - 1, false>::Do(
                this, m_boardState.m_emptyX, m_boardState.m_emptyY))
            {
                ++nDerivation;
            }
        }

		if (!m_bTopLeftBlank)
		{
			auto end = m_solution.end();
			for (auto it = m_solution.begin()
				; it != end
				; ++it)
			{
				//it->m_x = DIMENSION - 1 - it->m_x;
				//it->m_y = DIMENSION - 1 - it->m_y;
				*it ^= 2;
			}
		}

		std::reverse(m_solution.begin(), m_solution.end());

		solution = m_solution;
		return (int) solution.size();
	}

private:
	template <int emptyX, int emptyY,
		int emptyXOffset, int emptyYOffset, bool edge>
		bool DistributeStep()
	{
		return
			MakeStep<emptyX, emptyY, 0, -1, 0 == emptyXOffset && 1 == emptyYOffset, edge,
			MakeStep<emptyX, emptyY, -1, 0, 1 == emptyXOffset && 0 == emptyYOffset, edge,
			MakeStep<emptyX, emptyY, 0, 1, 0 == emptyXOffset && -1 == emptyYOffset, edge,
			MakeStep<emptyX, emptyY, 1, 0, -1 == emptyXOffset && 0 == emptyYOffset, edge,
			DoNothing> > > > :: Do(this);
	}

	template <int oldEmptyX, int oldEmptyY,
		int emptyXOffset, int emptyYOffset, typename Next>
        bool DoMakeStep(Int2Type_<false> /*unused*/)
	{
		enum { newEmptyX = oldEmptyX + emptyXOffset };
		enum { newEmptyY = oldEmptyY + emptyYOffset };

		Cell cell = m_boardState.m_cells[newEmptyX][newEmptyY];

		int manhattanChange = (emptyXOffset != 0)
			? GetManhattanChange(cell / DIMENSION, oldEmptyX, emptyXOffset)
			: GetManhattanChange(cell % DIMENSION, oldEmptyY, emptyYOffset);

		if (manhattanChange > 0)
		{
			if (Next::Do(this))
				return true;

			m_boardState.m_cells[oldEmptyX][oldEmptyY] = cell;
			m_boardState.m_cells[newEmptyX][newEmptyY] = 0;

            if (m_nDerivation > 1)
            {
                if (MultiTilesTest(m_nDerivation))
                {
                    --m_nDerivation;
                    if (DistributeStep<newEmptyX, newEmptyY, emptyXOffset, emptyYOffset, false>())
                        goto found;
                    ++m_nDerivation;
                }
            }
            else if (HasSingleConflictPass()
                    && DistributeStep<newEmptyX, newEmptyY, emptyXOffset, emptyYOffset, true>())
                goto found;

			m_boardState.m_cells[newEmptyX][newEmptyY]
    			= m_boardState.m_cells[oldEmptyX][oldEmptyY];
		}
		else
		{
			m_boardState.m_cells[oldEmptyX][oldEmptyY] = cell;

            if (DistributeStep<newEmptyX, newEmptyY, emptyXOffset, emptyYOffset, false>())
                goto found;

            m_boardState.m_cells[newEmptyX][newEmptyY]
    			= m_boardState.m_cells[oldEmptyX][oldEmptyY];

			if (Next::Do(this))
				return true;
		}

		return false;
found:
		m_solution.push_back((emptyXOffset & 3) + (emptyYOffset & 2)); //Move(newEmptyX, newEmptyY));
		return true;
	}


	template <int oldEmptyX, int oldEmptyY,
		int emptyXOffset, int emptyYOffset, typename Next>
        bool DoMakeStep(Int2Type_<true> /*unused*/)
	{
		enum { newEmptyX = oldEmptyX + emptyXOffset };
		enum { newEmptyY = oldEmptyY + emptyYOffset };

		Cell cell = m_boardState.m_cells[newEmptyX][newEmptyY];

		int manhattanChange = (emptyXOffset != 0)
			? GetManhattanChange(cell / DIMENSION, oldEmptyX, emptyXOffset)
			: GetManhattanChange(cell % DIMENSION, oldEmptyY, emptyYOffset);

		if (manhattanChange < 0)
		{
			m_boardState.m_cells[oldEmptyX][oldEmptyY] = cell;

			if (DistributeStep<newEmptyX, newEmptyY,
    				emptyXOffset, emptyYOffset, true>()
				|| (0 == newEmptyX && 0 == newEmptyY)
				&& m_boardState.IsFinalPositionCandidate())
			{
				m_solution.push_back((emptyXOffset & 3) + (emptyYOffset & 2)); //Move(newEmptyX, newEmptyY));
				return true;
			}

			m_boardState.m_cells[newEmptyX][newEmptyY]
			= m_boardState.m_cells[oldEmptyX][oldEmptyY];
		}

		return Next::Do(this);
	}
};


int Solve(const unsigned char* pInput, unsigned char* pResult)
{
	// Calculate parity
	unsigned int bitMask = 0;
	int parity = 0;
	int i;
	for (i = 0; i < DIMENSION * DIMENSION; ++i)
	{
		int value = pInput[i];
		if (value < 0 || value >= DIMENSION * DIMENSION)
		{
			return -1;
		}
		unsigned int bits = bitMask >> value;
		if (bits & 1)
		{
			return -1;
		}
		if (0 != value)
		{
			parity += GetBitsNumber(bits);
		}
		else if (!(DIMENSION & 1))
		{
			parity += i / DIMENSION;
		}
		bitMask |= 1 << value;
	}

	Solver solver(!(parity & 1));

	for (i = 0; i < DIMENSION * DIMENSION; ++i)
	{
		int value = pInput[i];
		if (0 != value)
		{
			solver.SetCell(i / DIMENSION, i % DIMENSION, value);
		}
		else
		{
			solver.SetBlank(i / DIMENSION, i % DIMENSION);
		}
	}

	vector<unsigned char> solution;

	int nSolution = solver.GetOptimalSolution(solution);

	if (nSolution == -1)
	{
		return -1;
	}

	if (!solution.empty())
		memcpy(pResult, &solution[0], solution.size());

	return solution.size();
}
