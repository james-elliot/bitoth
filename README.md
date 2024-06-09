# bitoth: Various experiments with bitboard Reversi/Othello

Many functions for playing with a bitboard othello, that can be tested for speed. I am not sure it is more efficient than the regular representation but
it is fun. 

The program has "classical" hash table for iter alpha-beta and mtd(f).

The evaluation function has not been optimized at all. It is only based on minimization of liberties and very simple corner (3x3) structures.

Probably strong enough to defeat most human players.

Optimization for endgame play, usually solves W/D/L around 21 empty discs in less than 1s.
