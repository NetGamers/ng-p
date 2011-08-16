/* levels.h - Contains access requirements for various commands */

#ifndef _LEVELS_H
#define _LEVELS_H

namespace gnuworld
{

namespace level
{

const int say = 450;

namespace jupe
	{
	const int info = 25;
	const int add = 100;
	const int del = 100;
	const int force = 400;
	} // namespace jupe

namespace stats
	{
	const int all = 25;
	const int pid = 400;
	} // Namespace stats

} // Namespace level

} // Namespace gnuworld

#endif
