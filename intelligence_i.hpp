/// @file intelligence_i.hpp

#ifndef __INTELLIGENCE_I_HPP__
#define __INTELLIGENCE_I_HPP__

namespace checkers
{
	inline intelligence::intelligence(const board& board) :
		_board(board)
	{
	}

	// ================================================================

	/**
	 *  @return >0 when the current player is ahead in game, and
	 *  @return <0 when the current player is behind in game
	 */
	inline int intelligence::evaluate(void)
	{
		return this->evaluate_pieces_strength() * 256 +
			this->evaluate_movers() * 2 +
			this->evaluate_kings_row() * 16 +
			this->evaluate_edges() * 8;
	}

	/**
	 *  A  man get 1 points
	 *  A king get 2 points
	 */
	inline int intelligence::evaluate_pieces_strength(void)
	{
		return this->_board.is_black_move() ?
			((this->_board.get_black_pieces().bit_count() -
			  this->_board.get_white_pieces().bit_count()) +
			 (this->_board.get_black_kings().bit_count() -
			  this->_board.get_white_kings().bit_count())) :
			((this->_board.get_white_pieces().bit_count() -
			  this->_board.get_black_pieces().bit_count()) +
			 (this->_board.get_white_kings().bit_count() -
			  this->_board.get_black_kings().bit_count()));
	}

	inline int intelligence::evaluate_movers(void)
	{
		return this->_board.is_black_move() ?
			(this->_board.get_black_movers().bit_count() -
			 this->_board.get_white_movers().bit_count()) :
			(this->_board.get_white_movers().bit_count() -
			 this->_board.get_black_movers().bit_count());
	}

	inline int intelligence::evaluate_kings_row(void)
	{
		return this->_board.is_black_move() ?
			((this->_board.get_black_pieces() &
				bitboard::WHITE_KINGS_ROW).bit_count() -
			 (this->_board.get_white_pieces() &
				bitboard::BLACK_KINGS_ROW).bit_count()) :
			((this->_board.get_white_pieces() &
				bitboard::BLACK_KINGS_ROW).bit_count() -
			 (this->_board.get_black_pieces() &
				bitboard::WHITE_KINGS_ROW).bit_count());
	}

	inline int intelligence::evaluate_edges(void)
	{
		return this->_board.is_black_move() ?
			((this->_board.get_black_pieces() &
				bitboard::EDGES).bit_count() -
			 (this->_board.get_white_pieces() &
				bitboard::EDGES).bit_count()) :
			((this->_board.get_white_pieces() &
				bitboard::EDGES).bit_count() -
			 (this->_board.get_black_pieces() &
				bitboard::EDGES).bit_count());
	}
}

#endif // __INTELLIGENCE_I_HPP__
// End of file
